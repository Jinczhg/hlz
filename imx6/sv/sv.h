/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */

#ifndef __SV_H
#define __SV_H

#define SV_IMAGE_WIDTH 1280
#define SV_IMAGE_HEIGHT 720

#define SV_DISPLAY_WIDTH 576
#define SV_DISPLAY_HEIGHT 720

typedef struct _sv_config
{
    int format;
    int mapType;
    int reorderEnable;
    int deblockEnable;
    int tiled2LinearEnable;
    int chromaInterleave;
    int mp4Class;
    int jpgLineBufferMode;
    int bitstreamMode;
    
    int channel_front;
    int channel_rear;
    int channel_left;
    int channel_right;
    int camera_count;

    int channel_display;
} sv_config;

#endif //__SV_H

