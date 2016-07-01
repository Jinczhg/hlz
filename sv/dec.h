/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#ifndef __DEC__H
#define __DEC__H

#include <stdint.h>
#include <pthread.h>
#include <libdce.h>

#define DEC_BUFFER_NUM 2
#define DEC_BUFFER_LEN 524288 // 0.5M

typedef enum CHANNEL {
    CHANNEL_FRONT = 0,
    CHANNEL_REAR,
    CHANNEL_LEFT,
    CHANNEL_RIGHT,
    CHANNEL_NUM_MAX
} CHANNEL;

typedef struct dec_buffer {
    int len;
    int offset;
    int write_lock;
    int read_lock;
    uint8_t *buf;
    struct dec_buffer *next;
} dec_buffer;

typedef struct decode {
    CHANNEL channel;
    Engine_Handle engine;
    VIDDEC3_Handle codec;
    VIDDEC3_Params *params;
    VIDDEC3_DynamicParams *dynParams;
    VIDDEC3_Status *status;
    VIDDEC3_InArgs *inArgs;
    VIDDEC3_OutArgs *outArgs;
    XDM2_BufDesc *inBufs;
    XDM2_BufDesc *outBufs;

    int width;
    int height;
    int padded_width;
    int padded_height;
    uint8_t *yuv; //output buffer for gpu

    pthread_mutex_t mutex;
    pthread_cond_t cond;


    //int buf_id;  // identify the buf to fill
    //int buf_len; // len of buf for decode
    //int buf_offset; // offset of buf for fill
    dec_buffer bufs[DEC_BUFFER_NUM]; // one to be filled data, the others to be decoded, alternately
    dec_buffer *buf; // the current filling buf
} decode;

int decode_init();

void decode_deinit();

void* decode_thread(void *arg);

int decode_start(decode *dec);

void decode_exit(int channel);

int decode_fill(int channel, const uint8_t *data, int len);

void decode_reset(int channel);

void decode_output(uint8_t *yuv, uint8_t *y, uint8_t *uv, int width, int height, int stride);

#endif // __DEC__H
