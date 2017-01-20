/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */

#ifndef __AVTP__H
#define __AVTP__H

#include <stdint.h>

int avtp_init();

void avtp_deinit();

void* avtp_recv_thread(void *arg);

void avtp_signal(int capture);

uint8_t* avtp_get_h264(uint8_t *buf);

int avtp_get_h264_len(uint8_t *buf);

#endif //__AVTP__H
