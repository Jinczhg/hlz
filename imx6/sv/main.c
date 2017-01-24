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

#include "vpu_lib.h"
#include "dec.h"
#include "gpu.h"
#include "avtp.h"
#include "trace.h"
#include "sv.h"
#include "stitchalgo.h"

#include "ipu_test.h"

sv_config g_config;
int g_exit = 0;

int main(int argc, char **argv)
{
    pthread_t tid_avtp_recv = 0;
    pthread_t tid_dec[CHANNEL_NUM_MAX] = {0};
    pthread_t tid_ipu = 0;
    pthread_t tid_gpu = 0;
    pthread_t tid_stitch = 0;
    int arg_dec[CHANNEL_NUM_MAX] = {0};
    int arg_ipu[CHANNEL_NUM_MAX] = {0};

    char cmd[256] = {0};
    int value = 0;

    int i = 0;
    int rc = 0;
    vpu_versioninfo ver;

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
    
    pthread_create(&tid_stitch, NULL, stitch_thread, NULL);
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

    DEBUG("create threads......");

    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	arg_dec[i] = i;
        pthread_create(&tid_dec[i], NULL, decode_thread, &arg_dec[i]);
    }
    

    pthread_create(&tid_ipu, NULL, ipu_thread, NULL);

    pthread_create(&tid_gpu, NULL, gpu_thread, NULL);

    pthread_create(&tid_avtp_recv, NULL, avtp_recv_thread, NULL);

    DEBUG("create threads finished");

    avtp_signal(1);
    
    while (g_exit == 0)
    {
        if (argc > 1)
        {
	    printf("usage:\n");
	    printf("value[1,2,3,4] is channel to display\n");
	
	    printf("input value [1,2,3,4]:");
	    
	    gets(cmd);
	    value = atoi(cmd);

	    if (value > 0 && value <= 4)
	    {
	        g_config.channel_display = value - 1;
	        stitch_move_camera();
	    }
	    else
	    {
	        printf("the channel %d is not supported.\n", value);
	    }
        }
        else
        {
	    sleep(20);
	    if (value == 0)
	    {
	        value = 2;
	    }
	    else if (value == 1)
	    {
	        value = 3;
	    }
	    else if (value == 2)
	    {
	        value = 1;
	    }
	    else if (value == 3)
	    {
	        value = 0;
	    }
	    g_config.channel_display = value;
	    stitch_move_camera();
	}
    }

    pthread_join(tid_avtp_recv, NULL);

    for (i = 0; i < CHANNEL_NUM_MAX; i++)
    {
	pthread_join(tid_dec[i], NULL);
    }
    
    pthread_join(tid_ipu, NULL);

    avtp_deinit();
    gpu_deinit();
    decode_deinit();
    ipu_deinit();

    return 0;
}
