/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "trace.h"
#include "dec.h"
#include "avtp.h"

#define ETHER_TYPE 0x22F0
#define ETHER_MTU 1518
#define AVTP_STREAMID_BASE   0x3350
#define AVTP_HEADER_LENGTH   (14 + 36 + 4) // Mac Header (14byte) + AVTP Header (36Byte) + CRC (4Byte)
#define AVTP_PAYLOAD_LENGTH  (992)
#define AVTP_BUFFER_NUM 1

typedef struct __avtp_buffer
{
    int len;
    int offset;
    uint8_t *buf;
    uint8_t *h264;
    int h264_len;
    int write_lock;
    int read_lock;
    struct __avtp_buffer *next;
} avtp_buffer;

static pthread_cond_t avtp_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t avtp_mutex = PTHREAD_MUTEX_INITIALIZER;

static int s_sn[CHANNEL_NUM_MAX];
static int s_capture = 1;
static avtp_buffer *s_bufs = NULL;

extern int g_exit;

static int avtp_packet(avtp_buffer *buf)
{
    uint8_t *packet = buf->buf;
    int len = buf->len;
    int streamId = 0;
    int channel = -1;
    int h264_len = 0;
    int sn = 0;
    int ret = 0;

    if (len > AVTP_HEADER_LENGTH)
    {
	streamId = packet[24] << 8;
	streamId += packet[25];

	channel = decode_channel(streamId);
	if (channel >= CHANNEL_NUM_MAX || channel < 0)
	{
	    ERROR("channnel = %d, not support", channel);
	    ret = -1;
	    goto err;
	}

	sn = packet[16];

	if (1 != sn - s_sn[channel] && 255 != s_sn[channel] - sn && s_sn[channel] != -1)
	{
            ERROR("channel = %d, sequence number = %d, pre =%d", channel, sn, s_sn[channel]);
	    decode_reset(channel);
	    s_sn[channel] = sn;
	    ret = -1;
	    goto err;
	}

	s_sn[channel] = sn;

	h264_len = packet[34] << 8;
	h264_len += packet[35];

	if (len < AVTP_HEADER_LENGTH + h264_len)
	{
	    ERROR("avtp data error");
	    ret = -1;
	    goto err;
	}

	buf->h264 = buf->buf + AVTP_HEADER_LENGTH;
	buf->h264_len = h264_len;

	decode_fill(channel, buf->h264, buf->h264_len);
    }
    else
    {
	ERROR("packet error, len = %d", len);
	ret = -1;
    }

err:
    return ret;
}

int avtp_init()
{
    int ret = 0;
    int i = 0;
    avtp_buffer *buf = NULL;

    DEBUG("init......");
    
    s_bufs = (avtp_buffer*)malloc(sizeof(avtp_buffer) * AVTP_BUFFER_NUM);
    if (s_bufs == NULL)
    {
        ERROR("malloc error");
        return -1;
    }
    
    for (i = 0; i < AVTP_BUFFER_NUM; i++)
    {
        buf = &s_bufs[i];
        memset(buf, 0, sizeof(avtp_buffer));
        
        buf->buf = (uint8_t*)malloc(ETHER_MTU);
        if (buf->buf == NULL)
        {
            ERROR("malloc error");
            return -1;
        }
        buf->read_lock = 1;
        
        if (i < AVTP_BUFFER_NUM - 1)
        {
            buf->next = &s_bufs[i+1];
        }
        else
        {
            buf->next = &s_bufs[0];
        }
    }

    DEBUG("init OK");
 
    return ret;
}

void avtp_deinit()
{
    DEBUG("deinit OK");
}

void* avtp_recv_thread(void *arg)
{
    int sd = -1;
    struct sockaddr src_addr;
    socklen_t addrlen = 0;
    avtp_buffer *buf = &s_bufs[0];
    int i = 0;

    DEBUG("start");
start:
    //pthread_mutex_lock(&avtp_mutex);
    //pthread_cond_wait(&avtp_cond, &avtp_mutex);
    //pthread_mutex_unlock(&avtp_mutex);

    if (g_exit)
    {
	goto exit;
    }

    if ((sd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) < 0)
    {
	ERROR("socket error: %s", strerror(errno));
	goto exit;
    }

    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	s_sn[i] = -1;
	decode_reset(i);
    }

    DEBUG("capture start");

    while (s_capture)
    {
again:
        buf->len = recvfrom(sd, buf->buf, ETHER_MTU, 0, &src_addr, &addrlen);
        if (buf->len <= 0)
	{
	    ERROR("recvfrom error: %s\n", strerror(errno));

	    if (errno == EINTR && s_capture)
	    {
		goto again;
	    }

	    ERROR("need to break the recv loop");
	    break;
	}
	
	avtp_packet(buf);
    }

    DEBUG("capture stop");
    close(sd);

    goto start;

exit:

    DEBUG("avtp recv thread exit");

    return 0;
}

void avtp_signal(int capture)
{
    if (s_capture != capture)
    {
	s_capture = capture;
	
	if (capture)
	{
	    pthread_mutex_lock(&avtp_mutex);
	    pthread_cond_signal(&avtp_cond);
	    pthread_mutex_unlock(&avtp_mutex);
	}
    }
}

uint8_t* avtp_get_h264(uint8_t *buf)
{
    return buf + AVTP_HEADER_LENGTH;
}

int avtp_get_h264_len(uint8_t *buf)
{
    int len = 0;

    len  = buf[34] << 8;
    len += buf[35];

    return len;
}

