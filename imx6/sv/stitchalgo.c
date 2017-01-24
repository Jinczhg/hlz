/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h> 
#include <sys/time.h>
#include <sys/resource.h>

#include <sys/ioctl.h>
#include <linux/fb.h>
#include <linux/mxcfb.h> 
#include "g2d.h"

#include "stitchalgo.h"
#include "mBVStitch3D_2_0.h"
#include "sv.h"
#include "yuv.h"
#include "trace.h"
#include "ipu_test.h"

extern sv_config g_config;
extern int g_exit;

static char *s_data[2] = {NULL};
static int s_index = 0;
static int s_index_ready = -1;
static int s_width = 0;
static int s_height = 0;
static int s_angle = 0;
static int s_stitching = 0;

static pthread_cond_t stitch_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t stitch_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t move_mutex = PTHREAD_MUTEX_INITIALIZER;

static int stitch_init()
{
    bool ret = false;
    int fbnum = 0;
    char *szInter = "./param.internal";
    char *szExter = "./param.external"; 
    char *szCarMatrixFile = "./car.matrix";
    char *szCarModelFile = "./car.mesh";
    int src_w = 2560;//1440;
    int src_h = 1440;//1152;
    int dst_w = 640;//1280;
    int dst_h = 800;
    float radratio = 1.8;
    
    int width = 2560;//1440;
    int height = 1440;//1152;
    int data_len = src_w * src_h * 3 / 2;

    int fp = 0;  
    struct fb_var_screeninfo var_info;
    struct fb_fix_screeninfo fix_info;   
    struct mxcfb_gbl_alpha gbl_alpha;
    struct mxcfb_pos pos;
    
    struct g2d_buf *buf;
    
    buf = g2d_alloc(data_len, 0);
    if (!buf)
    {
        printf("Fail to allocate physical memory for src buffer!\n");
        goto err;
    }
    s_data[0] = buf->buf_vaddr;
    if (!s_data[0])
    {
	ERROR("malloc error: %s", strerror(errno));
	goto err;
    }
    
    buf = g2d_alloc(data_len, 0);
    if (!buf)
    {
        printf("Fail to allocate physical memory for src buffer!\n");
        goto err;
    }
    s_data[1] = buf->buf_vaddr;
    if (!s_data[1])
    {
	ERROR("malloc error: %s", strerror(errno));
	goto err;
    }
      
    fp = open ("/dev/fb0",O_RDWR);  
  
    if (fp < 0)
    {  
      ERROR("open fb error:%s", strerror(errno));  
      return -1;
    }
    
    DEBUG("open fb OK\n");
    
    if (ioctl(fp, FBIOGET_FSCREENINFO, &fix_info))
    {  
        ERROR("Error reading FBIOGET_FSCREENINFO\n");  
        return -1;  
    } 
    
    DEBUG("ioctl FBIOGET_FSCREENINFO OK\n");
       
    if (ioctl(fp, FBIOGET_VSCREENINFO, &var_info))
    {  
        ERROR("Error reading FBIOGET_VSCREENINFO");  
        return -1;  
    }
    
    DEBUG("ioctl FBIOGET_FSCREENINFO OK");
    
    var_info.xres = 1280;//dst_w;
    var_info.yres = 800;//dst_h;
    var_info.xres_virtual = var_info.xres;
    var_info.yres_virtual = var_info.yres * 2;
    var_info.yoffset = var_info.yres;
    gbl_alpha.enable = 1;
    gbl_alpha.alpha = 0xFF;
    
    if (ioctl(fp, FBIOPUT_VSCREENINFO, &var_info) < 0)
    {
        ERROR("0x%x", var_info.xres);
        ERROR("0x%x", var_info.yres);
        ERROR("0x%x", var_info.xres_virtual);
        ERROR("0x%x", var_info.yres_virtual);
        ERROR("fb_display_setup FBIOPUT_VSCREENINFO failed");
    }


    if ((strcmp(fix_info.id, "DISP4 FG") == 0) || (strcmp(fix_info.id, "DISP3 FG") == 0))
    {
        pos.x = 0;
        pos.y = 0;
        ERROR("Set pos x = %d, y = %d", pos.x, pos.y);
        if (ioctl(fp, MXCFB_SET_OVERLAY_POS, &pos) < 0)
        {
            ERROR("fb_display_setup MXCFB_SET_OVERLAY_POS failed\n");
        }
    }
    
    if (ioctl(fp, FBIOPAN_DISPLAY, &var_info) < 0)
    {
        ERROR("FBIOPAN_DISPLAY failed");
        //goto err;
    }

    if (ioctl(fp, MXCFB_SET_GBL_ALPHA, &gbl_alpha) < 0)
    {
        ERROR("SET_GBL_ALPHA failed");
        //goto err;
    }
    
    close(fp); 
    
    DEBUG("config fb OK");
 
    s_width = width;
    s_height = height;
    
    ret = mBVStitch_2_0_Init(fbnum, szInter, szExter, szCarMatrixFile, szCarModelFile, 
	    src_w, src_h, 0, 0, dst_w, dst_h, radratio);
	    
    if (ret)
    {
        mBVStitch_2_0_MoveCameraAngle(0.0f, 89.0f, 45.0f);
    }
    else
    {
        ERROR("mBVStitch_2_0_Init fail......");
    }
  
    return 0;

err:
    return -1;
}

int stitch_deinit()
{
    return 0;
}

void* stitch_thread(void *arg)
{
    bool ret = false;
    int bNeedCar = 1;
    int angle = 4;
    unsigned char *buf = NULL;
    
#ifdef CALC_ALGO_TIME      
    struct timeval t_start;
    struct timeval t_end;
    unsigned int t_diff = 0;
    int fcnt = 0;
    
    gettimeofday(&t_start, NULL);
#endif
   
    setpriority(PRIO_PROCESS, 0, -20);

    if (stitch_init() != 0)
    {
        ERROR("stitch_init failed");
        goto out;
    }
    
    while (1)
    {
        if (s_index_ready == -1)
        {
            pthread_mutex_lock(&stitch_mutex);
	    pthread_cond_wait(&stitch_cond, &stitch_mutex); 
	    pthread_mutex_unlock(&stitch_mutex);
	}

        s_stitching = 1;
        buf = s_data[s_index_ready];
	s_index_ready = -1;
#if 1	
        if (s_angle < 360)
        {
            mBVStitch_2_0_MoveCamera(0, angle);
            s_angle += angle;
        }
        
        //stitch and draw
	pthread_mutex_lock(&move_mutex);
        
        mBVStitch_2_0_StitchImage(buf, s_width, s_height, VIDEO_FORMAT_NV12);
        s_stitching = 0;
        
        ret = mBVStitch_2_0_Draw(bNeedCar);
        if (!ret)
        {
            ERROR("mBVStitch_2_0_Draw fail......");
        }
        
	pthread_mutex_unlock(&move_mutex);
#endif
        
#ifdef CALC_ALGO_TIME
	fcnt++;
	if(fcnt % 500 == 0)
	{
            gettimeofday(&t_end, NULL);
            t_diff = t_end.tv_sec - t_start.tv_sec;
            DEBUG("image end %d, time us:%ds fps:%d !\n",
		fcnt,
		t_diff,
		500/t_diff);
            gettimeofday(&t_start, NULL);
	}
#endif
    }

    stitch_deinit();

out:
    DEBUG("exit");
    
    return NULL;
}

void stitch_signal()
{
    static int cnt = 0;
    
    //if (++cnt == g_config.camera_count)
    {
        cnt = 0;
        pthread_mutex_lock(&stitch_mutex);
        if (s_stitching == 0)
        {
            s_index_ready = s_index;
            s_index = s_index == 1 ? 0 : 1;
            pthread_cond_signal(&stitch_cond);
        }
        pthread_mutex_unlock(&stitch_mutex);
    }
}

char* stitch_get_buffer()
{
    return s_data[s_index];
}

int stitch_move_camera()
{
    pthread_mutex_lock(&move_mutex);
    
    if (g_config.channel_display == g_config.channel_front)
    {
        mBVStitch_2_0_MoveCameraAngle(0.0f, 89.0f, 45.0f);
        s_angle = 0;
    }
    else if (g_config.channel_display == g_config.channel_rear)
    {
        mBVStitch_2_0_MoveCameraAngle(180.0f, 89.0f, 45.0f);
        s_angle = 0;
    }
    else if (g_config.channel_display == g_config.channel_left)
    {
        mBVStitch_2_0_MoveCameraAngle(90.0f, 89.0f, 45.0f);
        s_angle = 0;
    }
    else if (g_config.channel_display == g_config.channel_right)
    {
        mBVStitch_2_0_MoveCameraAngle(-90.0f, 89.0f, 45.0f);
        s_angle = 0;
    }

    pthread_mutex_unlock(&move_mutex);

    return 0;
}
