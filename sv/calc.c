/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Data: 2016-06-01
 * Author: ryan
 */

#include <sys/time.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "calc.h"
#include "dec.h"
#include "trace.h"

#define CALC_MAX_COUNT 1080000

static struct timeval *s_recv_s[CHANNEL_NUM_MAX] = {NULL}, *s_recv_e[CHANNEL_NUM_MAX] = {NULL};
static struct timeval *s_dec_s[CHANNEL_NUM_MAX] = {NULL}, *s_dec_e[CHANNEL_NUM_MAX] = {NULL};
static struct timeval *s_stitch_s = NULL, *s_stitch_e = NULL;
static struct timeval *s_disp_s = NULL, *s_disp_e = NULL;
static int s_recv_count[CHANNEL_NUM_MAX] = {0},  s_dec_count[CHANNEL_NUM_MAX] = {0}, s_gpu_count = 0;
static struct timeval s_disp_time_start, s_disp_time_end;
static int s_disp_frame_c = -1;
static int s_disp_signal_count = 0;
static float diff = 0.0;

int calc_init()
{
    int i = 0;
    
    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
        s_recv_s[i] = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
        s_recv_e[i] = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
        s_recv_count[i] = 0; 

	s_dec_s[i] = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
        s_dec_e[i] = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
	s_dec_count[i] = 0;
    }

    s_stitch_s = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
    s_stitch_e = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
    memset(s_stitch_s, 0, sizeof(struct timeval) * CALC_MAX_COUNT);
    memset(s_stitch_e, 0, sizeof(struct timeval) * CALC_MAX_COUNT);
    
    s_disp_s = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
    s_disp_e = (struct timeval *)malloc(sizeof(struct timeval) * CALC_MAX_COUNT);
    memset(s_disp_s, 0, sizeof(struct timeval) * CALC_MAX_COUNT);
    memset(s_disp_e, 0, sizeof(struct timeval) * CALC_MAX_COUNT);
    
    s_gpu_count = 0;
    
    s_disp_frame_c = -1;
    s_disp_signal_count = 0;

    DEBUG("calc_init OK");

    return 0;
}

void calc_deinit()
{
    int i = 0;
    
    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
        free(s_recv_s[i]);
        free(s_recv_e[i]);
        s_recv_count[i] = 0; 

	free(s_dec_s[i]);
        free(s_dec_e[i]);
	s_dec_count[i] = 0;
    }

    free(s_stitch_s);
    free(s_stitch_e);
    free(s_disp_s);
    free(s_disp_e);
    s_gpu_count = 0;

    DEBUG("calc_init OK");

}

void calc_recv_frame_start(int channel)
{
    struct timeval *t = s_recv_s[channel];
    int c = s_recv_count[channel];

    gettimeofday(t + c, NULL);
}

void calc_recv_frame_end(int channel)
{
    struct timeval *t = s_recv_e[channel];
    int c = s_recv_count[channel];

    gettimeofday(t + c, NULL);
    if (c < (CALC_MAX_COUNT - 1))
    {
        s_recv_count[channel]++;
    }
}

void calc_dec_frame_start(int channel)
{
    struct timeval *t = s_dec_s[channel];
    int c = s_dec_count[channel];

    gettimeofday(t + c, NULL);
}

void calc_dec_frame_end(int channel)
{
    struct timeval *t = s_dec_e[channel];
    int c = s_dec_count[channel];

    gettimeofday(t + c, NULL);
    if (c < (CALC_MAX_COUNT - 1))
    {
        s_dec_count[channel]++;
    }
}

void calc_stitch_frame_start()
{
    struct timeval *t = s_stitch_s;
    int c = s_gpu_count;

    gettimeofday(t + c, NULL);
}

void calc_stitch_frame_end()
{
    struct timeval *t = s_stitch_e;
    int c = s_gpu_count;

    gettimeofday(t + c, NULL);
}

void calc_disp_frame_signal()
{
    if (s_disp_signal_count < (CALC_MAX_COUNT - 1))
    {
        s_disp_signal_count++;
    }
}

void calc_disp_frame_start()
{
    struct timeval *t = s_disp_s;
    int c = s_gpu_count;

    gettimeofday(t + c, NULL);
}

void calc_disp_frame_end()
{
    struct timeval *t = s_disp_e;
    int c = s_gpu_count;

    gettimeofday(t + c, NULL);
    
    if (c < (CALC_MAX_COUNT - 1))
    {
        s_gpu_count++;
    }

    if (s_gpu_count < s_disp_signal_count)
    {
        s_gpu_count = s_disp_signal_count;
    }
}

void calc_disp_frame_info()
{
    if (s_disp_frame_c == -1)
    {
	gettimeofday(&s_disp_time_start, NULL);
	s_disp_frame_c = 0;
    }

    if (++s_disp_frame_c == 150)
    {
	gettimeofday(&s_disp_time_end, NULL);
	diff = (s_disp_time_end.tv_sec - s_disp_time_start.tv_sec) + (s_disp_time_end.tv_usec - s_disp_time_start.tv_usec) / 1000000.0;
	DEBUG("%f fps", 150.0 / diff);
	s_disp_frame_c = 0;
	s_disp_time_start = s_disp_time_end;
    }
}

void calc_flush_log()
{
    char *log = (char*)malloc(1024);
    char file[PATH_MAX] = {0};
    int fd = -1;
    int i = 0;
    int c = 0;

    
#ifdef CAMERA_CHANNEL
    DEBUG("flush result start");
    //display
    sprintf(file, "/tmp/sv_calc.log");
    fd = open(file, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
	ERROR("open error:%s", strerror(errno));
	goto out;
    }

    for (c = 0; c < s_disp_count; c++)
    {
	if ((s_disp_e + c)->tv_sec + (s_disp_e + c)->tv_usec == 0)
	{
	    sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], missing this frame\n",
		    (s_recv_s[CAMERA_CHANNEL] + c)->tv_sec, (s_recv_s[CAMERA_CHANNEL] + c)->tv_usec, (s_recv_e[CAMERA_CHANNEL] + c)->tv_sec, (s_recv_e[CAMERA_CHANNEL] + c)->tv_usec,
		    (s_dec_s[CAMERA_CHANNEL] + c)->tv_sec, (s_dec_s[CAMERA_CHANNEL] + c)->tv_usec, (s_dec_e[CAMERA_CHANNEL] + c)->tv_sec, (s_dec_e[CAMERA_CHANNEL] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec);
	}
	else
	{
            sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], use: %u usec\n",
		    (s_recv_s[CAMERA_CHANNEL] + c)->tv_sec, (s_recv_s[CAMERA_CHANNEL] + c)->tv_usec, (s_recv_e[CAMERA_CHANNEL] + c)->tv_sec, (s_recv_e[CAMERA_CHANNEL] + c)->tv_usec,
		    (s_dec_s[CAMERA_CHANNEL] + c)->tv_sec, (s_dec_s[CAMERA_CHANNEL] + c)->tv_usec, (s_dec_e[CAMERA_CHANNEL] + c)->tv_sec, (s_dec_e[CAMERA_CHANNEL] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec,
		    ((s_disp_e + c)->tv_sec - (s_recv_s[CAMERA_CHANNEL] + c)->tv_sec) * 1000000 + ((s_disp_e + c)->tv_usec - (s_recv_s[CAMERA_CHANNEL] + c)->tv_usec));
	}
	write(fd, log, strlen(log));
    }
    close(fd);
    DEBUG("flush result end");

#else
    DEBUG("flush multi result start");
    //display
    sprintf(file, "/tmp/sv_calc_multi.log");
    fd = open(file, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
	ERROR("open error:%s", strerror(errno));
	goto out;
    }

    for (c = 0; c < s_gpu_count; c++)
    {
	if ((s_disp_e + c)->tv_sec + (s_disp_e + c)->tv_usec == 0)
	{
	    sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], missing this frame\n",
		    (s_recv_s[0] + c)->tv_sec, (s_recv_s[0] + c)->tv_usec, (s_recv_e[0] + c)->tv_sec, (s_recv_e[0] + c)->tv_usec,
		    (s_dec_s[0] + c)->tv_sec, (s_dec_s[0] + c)->tv_usec, (s_dec_e[0] + c)->tv_sec, (s_dec_e[0] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec);
	    write(fd, log, strlen(log));
            
	    sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], missing this frame\n",
		    (s_recv_s[1] + c)->tv_sec, (s_recv_s[1] + c)->tv_usec, (s_recv_e[1] + c)->tv_sec, (s_recv_e[1] + c)->tv_usec,
		    (s_dec_s[1] + c)->tv_sec, (s_dec_s[1] + c)->tv_usec, (s_dec_e[1] + c)->tv_sec, (s_dec_e[1] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec);
	    write(fd, log, strlen(log));

            sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], missing this frame\n",
		    (s_recv_s[2] + c)->tv_sec, (s_recv_s[2] + c)->tv_usec, (s_recv_e[2] + c)->tv_sec, (s_recv_e[2] + c)->tv_usec,
		    (s_dec_s[2] + c)->tv_sec, (s_dec_s[2] + c)->tv_usec, (s_dec_e[2] + c)->tv_sec, (s_dec_e[2] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec);
	    write(fd, log, strlen(log));

	    sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], missing this frame\n\n",
		    (s_recv_s[3] + c)->tv_sec, (s_recv_s[3] + c)->tv_usec, (s_recv_e[3] + c)->tv_sec, (s_recv_e[3] + c)->tv_usec,
		    (s_dec_s[3] + c)->tv_sec, (s_dec_s[3] + c)->tv_usec, (s_dec_e[3] + c)->tv_sec, (s_dec_e[3] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec);
	    write(fd, log, strlen(log));
	}
	else
	{
            sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], use: %u usec\n",
		    (s_recv_s[0] + c)->tv_sec, (s_recv_s[0] + c)->tv_usec, (s_recv_e[0] + c)->tv_sec, (s_recv_e[0] + c)->tv_usec,
		    (s_dec_s[0] + c)->tv_sec, (s_dec_s[0] + c)->tv_usec, (s_dec_e[0] + c)->tv_sec, (s_dec_e[0] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec,
		    ((s_disp_e + c)->tv_sec - (s_recv_s[0] + c)->tv_sec) * 1000000 + ((s_disp_e + c)->tv_usec - (s_recv_s[0] + c)->tv_usec));
            write(fd, log, strlen(log));

            sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], use: %u usec\n",
		    (s_recv_s[1] + c)->tv_sec, (s_recv_s[1] + c)->tv_usec, (s_recv_e[1] + c)->tv_sec, (s_recv_e[1] + c)->tv_usec,
		    (s_dec_s[1] + c)->tv_sec, (s_dec_s[1] + c)->tv_usec, (s_dec_e[1] + c)->tv_sec, (s_dec_e[1] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec,
		    ((s_disp_e + c)->tv_sec - (s_recv_s[1] + c)->tv_sec) * 1000000 + ((s_disp_e + c)->tv_usec - (s_recv_s[1] + c)->tv_usec));
            write(fd, log, strlen(log));

           sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], use: %u usec\n",
		    (s_recv_s[2] + c)->tv_sec, (s_recv_s[2] + c)->tv_usec, (s_recv_e[2] + c)->tv_sec, (s_recv_e[2] + c)->tv_usec,
		    (s_dec_s[2] + c)->tv_sec, (s_dec_s[2] + c)->tv_usec, (s_dec_e[2] + c)->tv_sec, (s_dec_e[2] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec,
		    ((s_disp_e + c)->tv_sec - (s_recv_s[2] + c)->tv_sec) * 1000000 + ((s_disp_e + c)->tv_usec - (s_recv_s[2] + c)->tv_usec));
            write(fd, log, strlen(log));

            sprintf(log, "recv[sec:%u,usec:%u; sec:%u,usec:%u], dec[sec:%u,usec:%u; sec:%u,usec:%u],"
		    "stitch[sec:%u,usec:%u; sec:%u,usec:%u], disp[sec:%u,usec:%u; sec:%u,usec:%u], use: %u usec\n\n",
		    (s_recv_s[3] + c)->tv_sec, (s_recv_s[3] + c)->tv_usec, (s_recv_e[3] + c)->tv_sec, (s_recv_e[3] + c)->tv_usec,
		    (s_dec_s[3] + c)->tv_sec, (s_dec_s[3] + c)->tv_usec, (s_dec_e[3] + c)->tv_sec, (s_dec_e[3] + c)->tv_usec,
		    (s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec,
		    (s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec,
		    ((s_disp_e + c)->tv_sec - (s_recv_s[3] + c)->tv_sec) * 1000000 + ((s_disp_e + c)->tv_usec - (s_recv_s[3] + c)->tv_usec));
            write(fd, log, strlen(log));
	}
    }
    close(fd);
    DEBUG("flush multi result end");
#if 0
    DEBUG("flush recv start");
    //recv
    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	sprintf(file, "/tmp/sv_recv_%d.log", i);
	fd = open(file, O_RDWR | O_CREAT, 0666);
	if (fd < 0)
	{
	    ERROR("open error:%s", strerror(errno));
	    goto out;
	}

        for (c = 0; c < s_recv_count[i]; c++)
	{
	    sprintf(log, "start: sec:%u, usec:%u, end: sec:%u, usec:%u\n",
		    (s_recv_s[i] + c)->tv_sec, (s_recv_s[i] + c)->tv_usec, (s_recv_e[i] + c)->tv_sec, (s_recv_e[i] + c)->tv_usec);
	    write(fd, log, strlen(log));
	}
	close(fd);
    }
    DEBUG("flush recv end");

    DEBUG("flush dec start");
    //dec
    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
        sprintf(file, "/tmp/sv_dec_%d.log", i);
        fd = open(file, O_RDWR | O_CREAT, 0666);
	if (fd < 0)
	{
	    ERROR("open error:%s", strerror(errno));
	    goto out;
	}

        for (c = 0; c < s_dec_count[i]; c++)
	{
	    sprintf(log, "start: sec:%u, usec:%u, end: sec:%u, usec:%u\n",
		    (s_dec_s[i] + c)->tv_sec, (s_dec_s[i] + c)->tv_usec, (s_dec_e[i] + c)->tv_sec, (s_dec_e[i] + c)->tv_usec);
	    write(fd, log, strlen(log));
	}
	close(fd);
    }
    DEBUG("flush dec end");

    DEBUG("flush stitch start");
    //stitch
    sprintf(file, "/tmp/sv_stitch.log");
    fd = open(file, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
	ERROR("open error:%s", strerror(errno));
	goto out;
    }

    for (c = 0; c < s_gpu_count; c++)
    {
	sprintf(log, "start: sec:%u, usec:%u, end: sec:%u, usec:%u\n",
		(s_stitch_s + c)->tv_sec, (s_stitch_s + c)->tv_usec, (s_stitch_e + c)->tv_sec, (s_stitch_e + c)->tv_usec);
	write(fd, log, strlen(log));
    }
    close(fd);
    DEBUG("flush stitch end");

    DEBUG("flush display start");
    //display
    sprintf(file, "/tmp/sv_display.log");
    fd = open(file, O_RDWR | O_CREAT, 0666);
    if (fd < 0)
    {
	ERROR("open error:%s", strerror(errno));
	goto out;
    }

    for (c = 0; c < s_gpu_count; c++)
    {
	sprintf(log, "start: sec:%u, usec:%u, end: sec:%u, usec:%u\n",
		(s_disp_s + c)->tv_sec, (s_disp_s + c)->tv_usec, (s_disp_e + c)->tv_sec, (s_disp_e + c)->tv_usec);
	write(fd, log, strlen(log));
    }
    close(fd);
    DEBUG("flush display end");
#endif

#endif

out:
    if (log)
    {
	free(log);
    }

    DEBUG("OK");
}
