/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "vpu_lib.h"
#include "dec.h"
#include "gpu.h"
#include "avtp.h"
#include "trace.h"
#include "sv.h"
#include "stitch_3d.h"
#include "stitch_2d.h"

#include "ipu_test.h"

#ifdef CALC_ALGO_TIME
#include "calc.h"
#endif

sv_config g_config;
int g_exit = 0;

int main(int argc, char **argv)
{
    pthread_t tid_avtp_recv = 0;
    pthread_t tid_dec[CHANNEL_NUM_MAX] = {0};
    pthread_t tid_ipu = 0;
    pthread_t tid_gpu = 0;
    pthread_t tid_stitch_3d = 0;
    pthread_t tid_stitch_2d = 0;
    int arg_dec[CHANNEL_NUM_MAX] = {0};
    int arg_ipu[CHANNEL_NUM_MAX] = {0};

    char cmd[256] = {0};
    int value = 0;

    int i = 0;
    int rc = 0;
    vpu_versioninfo ver;
    
    const char *fifo_name = "/tmp/sv_fifo";
    int pipe_fd = -1; 
    int nread = 0;
    char *buf = (char*)malloc(256);

    rc = vpu_Init(NULL);
    if (rc)
    {
	ERROR("VPU Init Failure.");
        return -1;
    }

    rc = vpu_GetVersionInfo(&ver);
    if (rc)
    {
	ERROR("Cannot get version info, err:%d", rc);
	vpu_UnInit();
	return -1;
    }

    printf("VPU firmware version: %d.%d.%d_r%d\n", ver.fw_major, ver.fw_minor,
	    ver.fw_release, ver.fw_code);
    printf("VPU library version: %d.%d.%d\n", ver.lib_major, ver.lib_minor, ver.lib_release);

    memset(&g_config, 0, sizeof(g_config));
    g_config.format = 2;
    g_config.reorderEnable = 0;
    g_config.chromaInterleave=1;
    g_config.bitstreamMode = 1;
    g_config.mp4Class = 0;

    g_config.channel_front = 0;
    g_config.channel_rear = 1;
    g_config.channel_left = 2;
    g_config.channel_right = 3;

    g_config.channel_display = 0;
    
    g_config.camera_count = 4;
    
    g_config.display_3d = 1;
    
    pthread_create(&tid_stitch_3d, NULL, stitch_3d_thread, NULL);
    sleep(1);
    
    rc = ipu_init();
    if (rc)
    {
	DEBUG("ipu_init fail");
	return -1;
    }
    
    rc = decode_init();
    if (rc)
    {
	DEBUG("decode_init fail");
	return -1;
    }

    rc = gpu_init();
    if (rc)
    {
	DEBUG("gpu_init fail");
	return -1;
    }

    rc = avtp_init();
    if (rc)
    {
	DEBUG("avtp_init fail");
	return -1;
    }
    
#ifdef CALC_ALGO_TIME    
    calc_init();
#endif

    DEBUG("create threads......");

    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	arg_dec[i] = i;
        pthread_create(&tid_dec[i], NULL, decode_thread, &arg_dec[i]);
    }
    

    pthread_create(&tid_ipu, NULL, ipu_thread, NULL);

    pthread_create(&tid_gpu, NULL, gpu_thread, NULL);
    pthread_create(&tid_stitch_2d, NULL, stitch_2d_thread, NULL);

    pthread_create(&tid_avtp_recv, NULL, avtp_recv_thread, NULL);

    DEBUG("create threads finished");

    avtp_signal(1);
    
    if(access(fifo_name, F_OK) == -1)  
    {  
        mkfifo(fifo_name, 0777);
    } 
    pipe_fd = open(fifo_name, O_RDONLY);
    if (pipe_fd < 0)
    {
        ERROR("failed to open fifo");
    }
    else
    {
	{
            while (g_exit == 0)
            {
                pipe_fd = open(fifo_name, O_RDONLY);
                if (pipe_fd < 0)
                {
                    ERROR("failed to open fifo");
                    break;
                }
                nread = read(pipe_fd, buf, 256);
	        close(pipe_fd);
	
                if (nread >= 1)
                {
	            value = buf[0] - '0';

	            if (value > 0 && value <= 4)
	            {
	                g_config.channel_display = value - 1;
	                stitch_3d_move_camera(1);
	            }
	            else if (value == 0)
	            {
	                g_config.channel_display = 0;
	                stitch_3d_move_camera(0);
	            }
	            else if (value == 5)
	            {
	                g_config.display_3d = g_config.display_3d == 0 ? 1 : 0;
	            }
#ifdef CALC_ALGO_TIME    
                    else if (value == 6)
	            {
	                calc_flush_log();
	            }
#endif
	            else
	            {
	                printf("the cmd %d is not supported.\n", value);
	            }
                }
            }
        }
    }

    pthread_join(tid_avtp_recv, NULL);

    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	pthread_join(tid_dec[i], NULL);
    }
    
    pthread_join(tid_ipu, NULL);
    pthread_join(tid_gpu, NULL);
    pthread_join(tid_stitch_3d, NULL);
    pthread_join(tid_stitch_2d, NULL);

    avtp_deinit();
    gpu_deinit();
    stitch_2d_deinit();
    decode_deinit();
    ipu_deinit();

    return 0;
}
