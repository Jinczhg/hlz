/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#ifndef __STITCHALGO__H
#define __STITCHALGO__H

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct {
    uint32_t width;
    uint32_t height;
    long bufYUV; //Format: YUV420,NV12
} YUVBuffer;

typedef enum {
    CONTROL_LEFT_DOOR_OPEN = 0,
    CONTROL_RIGHT_DOOR_OPEN = 1,
    CONTROL_MAX_NUM
} ControlType;

/*
   拼接算法初始化接口

   in_stride: 解码宽度

   y_offset:

   uv_offset:

   in_width：摄像头输入图像宽度

   in_height：摄像头输出图像高度

return: >=0为初始化成功；<0为错误代码，具体由算法提供方定义
*/
int initStitching(uint32_t in_stride, uint32_t y_offset, uint32_t uv_offset,
	uint32_t in_width, uint32_t in_height);

/*
   获取拼接输出图片尺寸接口

   out_width：拼接输出图像宽度

   out_height：拼接输出图像高度

return: >=0为初始化成功；<0为错误代码，具体由算法提供方定义
*/
int getStitchingSize(uint32_t *out_width, uint32_t *out_height);


/*
   拼接算法执行拼接接口

   front：前摄像头帧图像

   rear：后摄像头帧图像

   left：左侧摄像头帧图像

   right：右侧摄像头帧图像

   output：拼接完成的帧图像

return: >=0为成功拼接完成；<0为错误代码，具体由算法提供方定义
*/
int doStitching(YUVBuffer *front, YUVBuffer* rear, YUVBuffer *left, YUVBuffer *right, YUVBuffer *output);
 

/*
   拼接算法资源释放接口

return: >=0为成功；<0为错误代码，具体由算法提供方定义

*/
int deInitStitching(void);

/*
    设置车身状态,控制拼接效果
    type:控制类型
    value: 0 关, 1 开
return: >=0为成功；<0为错误代码，具体由算法提供方定义
*/
int controlStitching(ControlType type, int value);

/*
    前摄像头矫正
    view: 0 表示通常视角, 1 表示大视角
    front: 前摄像头帧图像
    output: 矫正完成的帧图像
return: >=0为成功拼接完成；<0为错误代码，具体由算法提供方定义
*/
int undistorted_front(int view, YUVBuffer *front, YUVBuffer *output);

/*
    后摄像头矫正
    rear: 后摄像头帧图像
    output: 矫正完成的帧图像
return: >=0为成功拼接完成；<0为错误代码，具体由算法提供方定义
*/
int undistorted_rear(YUVBuffer *rear, YUVBuffer *output);

/*
    左右摄像头矫正,侧视拼接
    view: 0 表示前侧视, 1 表示后侧视
    left: 左摄像头帧图像
    right: 右摄像头帧图像
    output: 矫正拼接完成的帧图像
return: >=0为成功拼接完成；<0为错误代码，具体由算法提供方定义
*/

int undistorted_left_and_right(int view, YUVBuffer *left, YUVBuffer *right, YUVBuffer *output);

#ifdef __cplusplus
}
#endif

#endif // __STITCHALGO__H

