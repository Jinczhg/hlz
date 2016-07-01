/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#ifndef __AVB__H
#define __AVB__H

int avb_init();

void avb_deinit();

void* avb_proc_thread(void *arg);

void* avb_recv_thread(void *arg);

void avb_signal(int capture);

void avb_exit();

#endif //__AVB__H
