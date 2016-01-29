#ifndef STITCHALGO_H
#define STITCHALGO_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef unsigned char  Uint8;
typedef unsigned long  Uint32;
typedef unsigned short Uint16;
typedef Uint32 VirtualAddress;

typedef struct {
	Uint32 width;
	Uint32 height;
	VirtualAddress bufYUV; //Format: YUV420,NV12
} YUVBuffer;

struct stitch_thread_arg
{
    Uint8 *dst;
    Uint8 *src;
    Uint8 num;
};

/*
拼接算法初始化接口
算法矫正文件路径为：/usr/local/stitchalgo/CALIB_FILENAME
in_width：摄像头输入图像宽度
in_hight：摄像头输出图像高度
out_width：拼接输出图像宽度
out_hight：拼接输出图像高度
return: >=0为初始化成功；<0为错误代码，具体由算法提供方定义

*/
int initStitching(Uint32 in_width, Uint32 in_hight, Uint32 out_width, Uint32 out_hight);
 
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

#endif
