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

#include "output.h"
#include "rvm.h"

int gpu_init(rvm_config *config);

void gpu_deinit();

OutputBuffer* gpu_get_buffer(int channel);

void* gpu_thread(void *arg);

void gpu_signal(int channel, OutputBuffer *buffer);

void gpu_visuable(int visuable);

void gpu_exit();

#endif // __GPU__H
