/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __STITCH_3D__H
#define __STITCH_3D__H

void* stitch_3d_thread(void *arg); 

void stitch_3d_signal();

char* stitch_3d_get_buffer();

int stitch_3d_move_camera(int move);

#endif // __STITCHALGO__H

