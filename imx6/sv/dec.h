/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */

#ifndef __DEC_H
#define __DEC_H

#include <pthread.h>
#include <stdint.h>

#include "vpu_lib.h"
#include "vpu_io.h"

#define STREAM_BUF_SIZE    0x200000
#define STREAM_FILL_SIZE   0x40000
#define PS_SAVE_SIZE       0x080000

#define DEC_FRAME_NUM 2
#define DEC_FRAME_SIZE 307200 //300K 

typedef enum CHANNEL {
    CHANNEL_FRONT = 0,
    CHANNEL_REAR,
    CHANNEL_LEFT,
    CHANNEL_RIGHT,
    CHANNEL_NUM_MAX
} CHANNEL;

typedef struct __frame_buffer
{
    vpu_mem_desc *mem_desc;
    FrameBuffer *fb;
} frame_buffer;

typedef struct _frame
{
    uint8_t *buf;
    int len;
    int offset;
    struct _frame *next;
} frame;

typedef struct __decoder
{
    CHANNEL channel;
    DecHandle handle;

    PhysicalAddress phy_bs_buf_addr;
    Uint32 cpu_bs_buf_addr;
    PhysicalAddress phy_ps_buf_addr;
    PhysicalAddress phy_slice_buf_addr;
    VirtualAddress virt_bs_buf_addr;

    int phy_bs_buf_size;
    int phy_slice_buf_size;

    int pic_width;
    int pic_height;
    int stride;
    int last_pic_width;
    int last_pic_height;
    int fmt;
    int reg_fb_count;
    int min_fb_count;
    int extra_fb_count;
    frame_buffer fb_pool;
    Rect pic_crop_rect;

    DecReportInfo mb_info;
    DecReportInfo mv_info;
    DecReportInfo frame_buf_stat;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_mutex_t ipu_mutex;
    pthread_cond_t ipu_cond;

    frame frames[DEC_FRAME_NUM];
    frame *frame_read;
    frame *frame_write;
    int frame_reading;
} decoder;

int decode_init();

void decode_deinit();

void* decode_thread(void *arg);

void decode_exit(int channel);

int decode_fill(int channel, uint8_t *data, int len);

void decode_reset(int channel);

int decode_channel(int streamId);

void decode_ipu_signal(int channel);

#endif
