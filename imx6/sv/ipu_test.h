/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-11-30
 * Author: ryan
 */

#ifndef __IPU_TEST_H
#define __IPU_TEST_H

#define IPU_TASK_CHANNEL 4
#define IPU_TASK_NUM 2

int ipu_init();

int ipu_deinit();

void* ipu_thread(void *arg);

void ipu_signal(int channel, unsigned long addr);

#endif //__IPU_TEST_H 
