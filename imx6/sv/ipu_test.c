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

struct ipu_task s_tasks[IPU_TASK_CHANNEL][IPU_TASK_NUM];
ipu_buffer s_buffers[IPU_TASK_CHANNEL][IPU_TASK_NUM];
static int fd_ipu = -1;
static int s_input_size = 0;

static pthread_cond_t ipu_cond[IPU_TASK_CHANNEL] = {PTHREAD_COND_INITIALIZER};
static pthread_mutex_t ipu_mutex[IPU_TASK_CHANNEL] = {PTHREAD_MUTEX_INITIALIZER};

static pthread_cond_t ipu_merge_cond[IPU_TASK_CHANNEL] = {PTHREAD_COND_INITIALIZER};
static pthread_mutex_t ipu_merge_mutex[IPU_TASK_CHANNEL] = {PTHREAD_MUTEX_INITIALIZER};

static pthread_mutex_t ipu_do_mutex[IPU_TASK_NUM] = {PTHREAD_MUTEX_INITIALIZER};

static unsigned long s_channel_num[IPU_TASK_CHANNEL] = {0};

static unsigned long s_src[IPU_TASK_CHANNEL] = {0};

extern sv_config g_config;
extern void decode_ipu_signal(int channel);

int ipu_init()
{
    int channel = 0, num = 0;
    int isize = SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT * 3 / 2;
    int osize = SV_IMAGE_WIDTH * SV_IMAGE_HEIGHT * 2 * 4;
    ipu_buffer *buf = NULL;
    struct ipu_task *t = NULL;
    int ret = 0;

    s_input_size = isize;

    fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0);
    if (fd_ipu < 0)
    {	    	
	ERROR("open ipu dev fail");
	goto err;
    }

    for (channel = 0; channel < IPU_TASK_CHANNEL; channel++)
    {
    for (num = 0; num < IPU_TASK_NUM; num++)
    {
	DEBUG("channel = %d, ipu task %d", channel, num);

	buf = &s_buffers[channel][num];
        t = &(s_tasks[channel][num]);
	memset(buf, 0, sizeof(ipu_buffer));
	memset(t, 0, sizeof(struct ipu_task));
        
        buf->read_lock = 1;
        buf->write_lock = 0;
        
	t->input.width = SV_IMAGE_WIDTH;
	t->input.height = SV_IMAGE_HEIGHT;
	t->input.format = IPU_PIX_FMT_NV12;
	t->input.paddr = isize;
	
	ret = ioctl(fd_ipu, IPU_ALLOC, &t->input.paddr);
	if (ret < 0)
	{
	    ERROR("ioctl IPU_ALLOC fail: %s", strerror(errno));
	    goto err;
	}

        DEBUG("channel = %d, num = %d : input paddr=%ld", channel, num, t->input.paddr);
        
	buf->input_paddr = t->input.paddr;
	
	buf->input_vaddr = mmap(0, isize, PROT_READ | PROT_WRITE, MAP_SHARED,
		fd_ipu, t->input.paddr);
	if (!buf->input_vaddr)
	{
	    ERROR("mmap fail");
	    goto err;
	}
	memset(buf->input_vaddr, 0, isize);

        t->output.width = SV_IMAGE_WIDTH * 2;
	t->output.height = SV_IMAGE_HEIGHT * 2;
	t->output.format = IPU_PIX_FMT_UYVY;
	t->output.crop.w = SV_IMAGE_WIDTH;
	t->output.crop.h = SV_IMAGE_HEIGHT;
	if (channel == g_config.channel_front)
	{
	    t->output.crop.pos.x = 0;
	    t->output.crop.pos.y = 0;
	}
	else if (channel == g_config.channel_rear)
	{
            t->output.crop.pos.x = 0;
	    t->output.crop.pos.y = SV_IMAGE_HEIGHT;
	}
	else if (channel == g_config.channel_left)
	{
            t->output.crop.pos.x = SV_IMAGE_WIDTH;
	    t->output.crop.pos.y = SV_IMAGE_HEIGHT;
	}
	else if (channel == g_config.channel_right)
	{
            t->output.crop.pos.x = SV_IMAGE_WIDTH;
	    t->output.crop.pos.y = 0;
	}
	
	if (channel == 0)
	{
	    t->output.paddr = osize;
	    ret = ioctl(fd_ipu, IPU_ALLOC, &t->output.paddr);
	    if (ret < 0)
	    {
	        ERROR("ioctl IPU_ALLOC fail: %s", strerror(errno));
	        goto err;
	    }

            buf->output_paddr = t->output.paddr;

	    buf->output_vaddr = mmap(0, osize, PROT_READ | PROT_WRITE, MAP_SHARED,
		fd_ipu, t->output.paddr);
	    if (!buf->output_vaddr)
	    {
	        ERROR("mmap fail");
	        goto err;
	    }
	    memset(buf->output_vaddr, 0, osize);
	}
	else
	{
	    t->output.paddr = s_tasks[0][num].output.paddr;
            buf->output_paddr = s_buffers[0][num].output_paddr;
            buf->output_vaddr = s_buffers[0][num].output_vaddr;
	}
	
	DEBUG("channel = %d, num = %d : output paddr=%ld", channel, num, t->output.paddr);

	ret = ioctl(fd_ipu, IPU_CHECK_TASK, t);
	if (ret != IPU_CHECK_OK)
	{
	    ERROR("ioctl IPU_CHECK_TASK fail");
	    goto err;
	}
    }
    }

    DEBUG("OK");
    
    return 0;

err:
    if (fd_ipu > 0)
    {
	close(fd_ipu);
	fd_ipu = -1;
    }
    return -1;
}

int ipu_deinit()
{
    if (fd_ipu > 0)
    {
	close(fd_ipu);
	fd_ipu = -1;
    }
    return 0;
}

ipu_buffer* ipu_get_buffer(int channel, int num)
{
    if (channel >= 0 || channel < IPU_TASK_CHANNEL)
    {
	return &s_buffers[channel][num];
    }

    return NULL;
}

int ipu_do(int channel, int num)
{
    int ret = 0;
    struct ipu_task *t = NULL;

    t = &s_tasks[channel][num];
    
    ret = ioctl(fd_ipu, IPU_QUEUE_TASK, t);
    
    if (ret < 0)
    {
	ERROR("ioct IPU_QUEUE_TASK fail");
        return -1;
    }

    return 0;
}

void* ipu_thread(void *arg)
{
    int channel = *(int*)arg;
    int num = 0;
    ipu_buffer *buf = NULL;
    struct ipu_task *t = NULL;
    int ret = 0;
    
    setpriority(PRIO_PROCESS, 0, -19);
    
    while (1)
    {
        buf = &s_buffers[channel][num];

        if (buf->read_lock)
        {
            decode_ipu_signal(channel);
            pthread_mutex_lock(&ipu_mutex[channel]);
            pthread_cond_wait(&ipu_cond[channel], &ipu_mutex[channel]);     
            pthread_mutex_unlock(&ipu_mutex[channel]);
            if (buf->read_lock)
            {
                continue;
            }
        }
 #if 1       
        t = &s_tasks[channel][num];
        ret = ioctl(fd_ipu, IPU_QUEUE_TASK, t);
        if (ret < 0)
        {
	   ERROR("ioct IPU_QUEUE_TASK fail");
        }
 #else
 #if 0
         if (channel == g_config.channel_front)
         {
             yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, (char*)s_src[channel], NULL, NULL, NULL, stitch_get_buffer());
         }
         else if (channel == g_config.channel_rear)
         {
             yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, NULL, (char*)s_src[channel], NULL, NULL, stitch_get_buffer());
         }
         else if (channel == g_config.channel_left)
         {
             yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, NULL, NULL, (char*)s_src[channel], NULL, stitch_get_buffer());
         }
         else if (channel == g_config.channel_right)
         {
             yuv_merge(SV_IMAGE_WIDTH, SV_IMAGE_HEIGHT, NULL, NULL, NULL, (char*)s_src[channel], stitch_get_buffer());
         }
#endif
#endif       
        buf->read_lock = 1;
        buf->write_lock = 0;
        
        decode_ipu_signal(channel);

        stitch_signal(channel, num);
        
        num++;
        num = num % IPU_TASK_NUM;
    }
}

void ipu_signal(int channel, unsigned long addr)
{
    pthread_mutex_lock(&ipu_mutex[channel]);
    s_src[IPU_TASK_CHANNEL] = addr;
    pthread_cond_signal(&ipu_cond[channel]);
    pthread_mutex_unlock(&ipu_mutex[channel]);
}

void ipu_merge_signal(int channel, unsigned long addr)
{
    s_src[IPU_TASK_CHANNEL] = addr;
    pthread_mutex_lock(&ipu_mutex[channel]);
    pthread_cond_signal(&ipu_cond[channel]);
    pthread_mutex_unlock(&ipu_mutex[channel]);
}

void ipu_output_lock(int num)
{
    pthread_mutex_lock(&ipu_do_mutex[num]);
}

void ipu_output_unlock(int num)
{
    pthread_mutex_unlock(&ipu_do_mutex[num]);
}
