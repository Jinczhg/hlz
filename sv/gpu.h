/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#ifndef __GPU__H
#define __GPU__H

#include <stdint.h>

#include "stitchalgo.h"

int gpu_init();

void gpu_deinit();

uint8_t* gpu_get_buffer(int channel);

void* gpu_thread(void *arg);

void gpu_signal();

void gpu_visuable(int visuable);

void gpu_exit();

#endif // __GPU__H
