/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <linux/ipu.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "ipu_test.h"
#include "sv.h"
#include "trace.h"
#include "stitchalgo.h"

static pthread_cond_t ipu_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t ipu_mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned long s_src[IPU_TASK_CHANNEL] = {0};
static int s_ready = 0;

extern sv_config g_config;

int ipu_init()
{
    return 0;
}

int ipu_deinit()
{
    return 0;
}

void* ipu_thread(void *arg)
{
    int channel = *(int*)arg;
    char *src = NULL;
    int ret = 0;
    
    setpriority(PRIO_PROCESS, 0, -20);
    
    while (1)
    {
        if (s_ready == 0)
        {
            pthread_mutex_lock(&ipu_mutex);
            pthread_cond_wait(&ipu_cond, &ipu_mutex); 
            pthread_mutex_unlock(&ipu_mutex);
        }

        s_ready = 0;
        
        src = (char*)s_src[g_config.channel_front];
        if (src)
        {
            yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, src, NULL, NULL, NULL, stitch_get_buffer());
        }

        src = (char*)s_src[g_config.channel_rear];
        if (src)
        {
            yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, NULL, src, NULL, NULL, stitch_get_buffer());
        }

        src = (char*)s_src[g_config.channel_left];
        if (src)
        {
            yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, NULL, NULL, src, NULL, stitch_get_buffer());
        }

        src = (char*)s_src[g_config.channel_right];
        if (src)
        {
            yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, NULL, NULL, NULL, src, stitch_get_buffer());
        }

        stitch_signal();
    }
}

void ipu_signal(int channel, unsigned long addr)
{ 
    static int cnt = 0;
    
    s_src[channel] = addr;
    if (++cnt == g_config.camera_count)
    {
        cnt = 0;
        pthread_mutex_lock(&ipu_mutex);
        s_ready = 1;
        pthread_cond_signal(&ipu_cond);
        pthread_mutex_unlock(&ipu_mutex);    
    }
    
}
