/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __RVM__H
#define __RVM__H

#define IMAGE_WIDTH  1280
#define IMAGE_HEIGHT 720

#define WIN_WIDTH 1440
#define WIN_HEIGHT 328

typedef enum _GPU_VIEW
{
    GPU_VIEW_0 = 0,
    GPU_VIEW_1,
    GPU_VIEW_NUM
} GPU_VIEW;

typedef enum _CAR_CONTROL
{
    CAR_MOVE_FORWARD = 0,
    CAR_TURN_LEFT,
    CAR_TURN_RIGHT,
    CAR_REVERSING,
    CAR_CONTROL_NUM
} CAR_CONTROL;

typedef enum _CAR_MANUAL_VISION
{
    CAR_MANUAL_VISION_LEFT,
    CAR_MANUAL_VISION_RIGHT,
    CAR_MANUAL_VISION_NUM
} CAR_MANUAL_VISION;


typedef enum _CAR_SIDE
{
    CAR_SIDE_LEFT = 0,
    CAR_SIDE_RIGHT,
    CAR_SIDE_NUM
} CAR_SIDE;

typedef struct _rvm_config
{
    int channel_front;
    int channel_rear;
    int camera_count;
    
    int screen_direction;

    CAR_SIDE car_side;
    CAR_CONTROL car_control;
    CAR_MANUAL_VISION car_vision;
    float vision_move_step;
} rvm_config;

#endif
