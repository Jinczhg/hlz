/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#ifndef __OUTPUT__H
#define __OUTPUT__H

#include <stdint.h>
#include <stdbool.h>

#include <ti/shmemallocator/SharedMemoryAllocatorUsr.h>

typedef struct OutputBuffer OutputBuffer;

struct OutputBuffer
{
    char *buf;          //virtual address for local access, 4kb stride
    uint32_t y;         //virtual address for Y for remote access
    uint32_t uv;        //virtual address for UV remote access
    OutputBuffer *next; //next free buffer
    bool tiler;
    uint32_t len;
    shm_buf shmBuf;
};

/******************** output operations **************************/

/********
 * allocate buffers
 * @cnt, the count of buffers
 * @width, for calculate the size of each buffer
 * @height, for calculate the size of each buffer
 ******/
OutputBuffer* output_alloc(int cnt, int width, int height);

/********
 * free all buffers
 ********/
void output_free(OutputBuffer* buf, int cnt);

#endif //__OUTPUT__H
