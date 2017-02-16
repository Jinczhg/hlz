/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 * 
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __CALC__H
#define __CALC__H

int calc_init();

void calc_deinit();

void calc_recv_frame_start(int channel);

void calc_recv_frame_end(int channel);

void calc_dec_frame_start(int channel);

void calc_dec_frame_end(int channel);

void calc_stitch_frame_start();

void calc_stitch_frame_end();

void calc_disp_frame_start();

void calc_disp_frame_end();

void calc_disp_frame_info();

void calc_flush_log();

void calc_disp_frame_signal();

#endif //__CALC__H
