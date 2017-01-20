/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __STITCHALGO__H
#define __STITCHALGO__H

void* stitch_thread(void *arg); 

void stitch_signal(int channel, int num);

int stitch_move_camera();

#endif // __STITCHALGO__H

