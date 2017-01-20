/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __GPU__H
#define __GPU__H

#include <stdint.h>

int gpu_init();

void gpu_deinit();

void* gpu_thread(void *arg);

void gpu_signal(int channel, unsigned long addr);

void gpu_exit();

#endif // __GPU__H
