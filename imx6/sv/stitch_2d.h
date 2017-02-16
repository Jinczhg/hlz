/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __STITCH_2D__H
#define __STITCH_2D__H

#include <stdint.h>

int stitch_2d_init();

void stitch_2d_deinit();

void* stitch_2d_thread(void *arg);

void stitch_2d_signal(int channel, unsigned long addr);

void stitch_2d_exit();

#endif // __GPU_STITCH__H
