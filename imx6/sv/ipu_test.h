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
#define IPU_TASK_NUM 3

typedef struct _ipu_buffer
{
    int input_paddr;
    void *input_vaddr;
    int output_paddr;
    void *output_vaddr;
    int read_lock;
    int write_lock;
} ipu_buffer;

int ipu_init();

int ipu_deinit();

ipu_buffer* ipu_get_buffer(int channel, int num);

int ipu_do(int channel, int num);

void* ipu_thread(void *arg);

void ipu_signal(int channel, unsigned long addr);

void ipu_output_lock(int num);

void ipu_output_unlock(int num);

#endif //__IPU_TEST_H 
