/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#include <sys/neutrino.h>
#include <net/bpf.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

#include "trace.h"
#include "dec.h"
#include "avb.h"

#define BPF_NODE "/dev/bpf"
#define DEFAULT_BUFSIZE 524288 // 0.5M

#define AVB_BUFFER_NUM 3

#define AVB_STREAMID_BASE    0x3350
#define AVTP_HEADER_LENGTH   (14 + 36 + 4) // Mac Header (14byte) + AVTP Header (36Byte) + CRC (4Byte)
#define AVTP_PAYLOAD_LENGTH  (992)

typedef struct avb_buffer {
    uint8_t *buf;
    int len;
    int write_lock;
    int read_lock;
    struct avb_buffer *next;
} avb_buffer;

static pthread_cond_t avb_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t avb_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t avb_buf_read_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t avb_buf_read_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t avb_buf_write_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t avb_buf_write_mutex = PTHREAD_MUTEX_INITIALIZER;

static int s_sn[CHANNEL_NUM_MAX];

static int s_capture = 0;

static int s_exit = 0;

static avb_buffer s_bufs[AVB_BUFFER_NUM];

int avb_init()
{
    int i = 0;
    for (i = 0; i < AVB_BUFFER_NUM; i++)
    {
	memset(&s_bufs[i], 0, sizeof(avb_buffer));
	
	s_bufs[i].buf = NULL;
	s_bufs[i].read_lock = 1;
	s_bufs[i].next = &s_bufs[(i + 1) % AVB_BUFFER_NUM];
    }

    DEBUG("init OK");

    return 0;
}

void avb_deinit()
{
    int i = 0;
    for (i = 0; i < AVB_BUFFER_NUM; i++)
    {
	if (s_bufs[i].buf)
	{
	    free(s_bufs[i].buf);
	    s_bufs[i].buf = NULL;
	}
    }

    DEBUG("deinit OK");
}

static void avb_packet(int len, const u_char * packet)
{
    int streamId = 0;
    int channel = 0;
    const uint8_t *h264 = NULL;
    int h264_len = 0;
    int sn = 0;

    if (len > AVTP_HEADER_LENGTH)
    {
        streamId = packet[24] << 8;
	streamId += packet[25];
        channel = (streamId - AVB_STREAMID_BASE - 1) >> 1; //streamId: 3351 3353 3355 3357
	if (channel >= CHANNEL_NUM_MAX || channel < 0)
	{
	    ERROR("channel = %d, not support", channel);
	    return;
	}

	h264_len = packet[34] << 8;
	h264_len += packet[35];

	sn = packet[16];

	if (1 != sn - s_sn[channel] && 255 != s_sn[channel] - sn && s_sn[channel] != -1)
	{
	    ERROR("channel = %d, sequence number = %d, pre = %d", channel, sn, s_sn[channel]);
	    decode_reset(channel);
	    s_sn[channel] = sn;
	    return;
	}

	s_sn[channel] = sn;

	if (len < AVTP_HEADER_LENGTH + h264_len)
	{
	    ERROR("avb data error\n");
	    return;
	}

	h264 = packet + AVTP_HEADER_LENGTH;

	decode_fill(channel, h264, h264_len);
    }
    else
    {
	ERROR("packet error, pkthdr->len = %d", len);
    }
}

void* avb_proc_thread(void *arg)
{
    avb_buffer *buf = s_bufs;
    register uint8_t *bp, *ep;
    register int caplen, hdrlen;
    uint8_t *datap = NULL;

#define bhp ((struct bpf_hdr *)bp)

    while (!s_exit)
    {
        if (buf->read_lock)
	{
	    pthread_mutex_lock(&avb_buf_read_mutex);
	    pthread_cond_wait(&avb_buf_read_cond, &avb_buf_read_mutex);
	    pthread_mutex_unlock(&avb_buf_read_mutex);
	}

	if (buf->len == 0)
	{
	    continue;
	}

	bp = buf->buf;
	ep = bp + buf->len;
        while (bp < ep)
	{
	    caplen = bhp->bh_caplen;
	    hdrlen = bhp->bh_hdrlen;
	    datap = bp + hdrlen;

	    avb_packet(bhp->bh_datalen, datap);

	    bp += BPF_WORDALIGN(caplen + hdrlen);
	}

	buf->write_lock = 0;
	buf->read_lock = 1;
	buf->len = 0;

        pthread_mutex_lock(&avb_buf_write_mutex);
	pthread_cond_signal(&avb_buf_write_cond);
	pthread_mutex_unlock(&avb_buf_write_mutex);

	buf = buf->next;
    }
#undef bhp

    DEBUG("avb proc thread exit");
    return 0;
}

void* avb_recv_thread(void *arg)
{
    char *devStr = "dm0";
    int fd = -1;
    struct ifreq ifr;
    struct bpf_program fp1, fp2;
    avb_buffer *buf = NULL;
    uint32_t buf_len = 0;
    int read_len = 0;
    int i = 0;
    int data = 0x02;
    //ThreadCtl(_NTO_TCTL_RUNMASK, &data);

    struct bpf_insn insn;
    insn.code = (u_short)(BPF_RET | BPF_K);
    insn.jt = 0;
    insn.jf = 0;
    insn.k = 65535;

    fp1.bf_len = 1;
    fp1.bf_insns = &insn;

    struct bpf_insn insns[] = {
	BPF_STMT(BPF_LD+BPF_H+BPF_ABS, 12),
	BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0x22F0, 0, 1),
	BPF_STMT(BPF_RET+BPF_K, (u_int)-1),
	BPF_STMT(BPF_RET+BPF_K, 0)
    };

    fp2.bf_len = 4;
    fp2.bf_insns = insns;

    strncpy(ifr.ifr_name, devStr, sizeof(ifr.ifr_name));

    buf = s_bufs;

start:
    pthread_mutex_lock(&avb_mutex);
    pthread_cond_wait(&avb_cond, &avb_mutex);
    pthread_mutex_unlock(&avb_mutex);

    if (s_exit)
    {
	goto exit;
    }

    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	s_sn[i] = -1;
	decode_reset(i);
    }

    /* open a bpf device */
    fd = open(BPF_NODE, O_RDONLY);
    if (fd < 0)
    {
	ERROR("open bpf device fail: %s", strerror(errno));
	goto exit;
    }

    if (ioctl(fd, BIOCGBLEN, (caddr_t)&buf_len) < 0)
    {
        ERROR("ioctl BIOCGBLEN fail: %s", strerror(errno));
        close(fd);
	goto exit;
    }
    DEBUG("get buf_len = %d", buf_len);
    
    if (buf_len < DEFAULT_BUFSIZE)
    {
	buf_len = DEFAULT_BUFSIZE;

	for (;buf_len > 0; buf_len >>= 1)
	{
	    if (ioctl(fd, BIOCSBLEN, (caddr_t)&buf_len) < 0)
	    {
		DEBUG("ioctl BIOCSBLEN fail: %s", strerror(errno));
	    }

	    if (ioctl(fd, BIOCSETIF, (caddr_t)&ifr) >= 0)
	    {
		DEBUG("set buf_len = %d", buf_len);
		break;
	    }

	    if (errno != ENOBUFS)
	    {
                ERROR("ioctl BIOCSETIF fail: %s", strerror(errno));
		close(fd);
		goto exit;
	    }
	    
	}
    }
    else
    {
	if (ioctl(fd, BIOCSETIF, (caddr_t)&ifr) < 0) 
	{
	    ERROR("ioctl BIOCSETIF fail: %s", strerror(errno));
	    close(fd);
	    goto exit;
	}
    }

    if (ioctl(fd, BIOCGBLEN, (caddr_t)&buf_len) < 0)
    {
        ERROR("ioctl BIOCGBLEN fail: %s", strerror(errno));
        close(fd);
	goto exit;
    }
    DEBUG("get buf_len = %d", buf_len);

    for (i = 0; i < AVB_BUFFER_NUM; i++)
    {
	if (s_bufs[i].buf == NULL)
	{
	    s_bufs[i].buf = (uint8_t*)malloc(buf_len);
	    if (s_bufs[0].buf == NULL)
	    {
		ERROR("malloc fail: %s", strerror(errno));
		close(fd);
		goto exit;
	    }
	}
	s_bufs[i].len = 0;
    }

    if (ioctl(fd, BIOCPROMISC, NULL) < 0)
    {
        ERROR("ioctl BIOCPROMISC fail: %s", strerror(errno));
        close(fd);
	goto exit;
    }

    if (ioctl(fd, BIOCSETF, (caddr_t)&fp1) < 0)
    {
        ERROR("ioctl BIOCSETF fail: %s", strerror(errno));
        close(fd);
	goto exit;
    }

    if (ioctl(fd, BIOCSETF, (caddr_t)&fp2) < 0)
    {
        ERROR("ioctl BIOCSETF fail: %s", strerror(errno));
        close(fd);
	goto exit;
    }

    while (s_capture)
    {
	if (buf->write_lock)
	{
	    DEBUG("proccessing not fast enough");
	    pthread_mutex_lock(&avb_buf_write_mutex);
	    pthread_cond_wait(&avb_buf_write_cond, &avb_buf_write_mutex);
	    pthread_mutex_unlock(&avb_buf_write_mutex);
	}

again:
        read_len = read(fd, buf->buf, buf_len);
	if (read_len < 0)
	{
	    ERROR("read fail: %s", strerror(errno));
            
	    if (errno == EINTR && s_capture)
	    {
		continue;
	    }

	    break;
	}

	buf->len = read_len;
	buf->read_lock = 0;
	buf->write_lock = 1;

        pthread_mutex_lock(&avb_buf_read_mutex);
	pthread_cond_signal(&avb_buf_read_cond);
	pthread_mutex_unlock(&avb_buf_read_mutex);

	buf = buf->next;
    }

    DEBUG("capture stop");

    close(fd);
    
    goto start;

exit:
    pthread_mutex_lock(&avb_buf_read_mutex);
    pthread_cond_signal(&avb_buf_read_cond);
    pthread_mutex_unlock(&avb_buf_read_mutex);

    DEBUG("avb recv thread exit");
    return 0;
}

void avb_signal(int capture)
{
    if (s_capture == capture)
    {
	return;
    }

    s_capture = capture;

    if (capture)
    {
        pthread_mutex_lock(&avb_mutex);
        pthread_cond_signal(&avb_cond);
        pthread_mutex_unlock(&avb_mutex);
    }
}

void avb_exit()
{
    s_exit = 1;

    pthread_mutex_lock(&avb_mutex);
    pthread_cond_signal(&avb_cond);
    pthread_mutex_unlock(&avb_mutex);
}
