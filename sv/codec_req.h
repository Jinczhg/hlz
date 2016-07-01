/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#ifndef __CODEC_REQ__H
#define __CODEC_REQ__H

//align x to next highest multiple of 2^n
#define ALIGN2(x, n) (((x) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))


//Padding for width and height as per Codec Requirement
#define PADX_H264 32
#define PADY_H264 24

#define PADX_MPEG4 32
#define PADY_MPEG4 32

#define PADX_VC1 32
#define PADY_VC1 40

#endif //__CODEC_REQ__H
