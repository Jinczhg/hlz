/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 */

#include <string.h>

#include "stitchalgo.h"

static uint32_t s_in_width = 0, s_in_hight = 0, s_out_width = 0, s_out_hight = 0; 

int initStitching(uint32_t in_width, uint32_t in_hight, uint32_t out_width, uint32_t out_hight)
{
    s_in_width = in_width;
    s_in_hight = in_hight;
    s_out_width = out_width;
    s_out_hight = out_hight;

    return 0;
}

int doStitching(YUVBuffer *front, YUVBuffer* rear, YUVBuffer *left, YUVBuffer *right, YUVBuffer *output)
{
    int r = 0, c = 0;
    int c_b, c_e;
    register int offset;
    uint8_t *o, *i;
    register int width = output->width;
    int uv_offset = output->width * output->height;

    uint8_t *output_y = (uint8_t*)output->bufYUV;
    uint8_t *output_uv = output_y + uv_offset;
    
    uint8_t *front_y = (uint8_t*)front->bufYUV;
    uint8_t *front_uv = front_y + uv_offset;
    
    uint8_t *rear_y = (uint8_t*)rear->bufYUV;
    uint8_t *rear_uv = rear_y + uv_offset;
    
    uint8_t *left_y = (uint8_t*)left->bufYUV;
    uint8_t *left_uv = left_y + uv_offset;
    
    uint8_t *right_y = (uint8_t*)right->bufYUV;
    uint8_t *right_uv = right_y + uv_offset;
    

    //right Y
    c_b = 80;
    c_e = 1200;
    offset = 0;
    o = output_y + c_b;
    i = right_y + c_b;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_b += 1;
	c_e -= 1;
	o = output_y + offset + c_b;
	i = right_y + offset + c_b;
    }

    //left Y
    c_b = 320;
    c_e = 960;
    offset = 480 * width;
    o = output_y + offset + c_b;
    i = left_y + offset + c_b;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_b -= 1;
	c_e += 1;
	o = output_y + offset + c_b;
	i = left_y + offset + c_b;
    }

    //front Y
    c_b = 0;
    c_e = 80;
    offset = 0;
    o = output_y;
    i = front_y;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_e += 1;
	o = output_y + offset + c_b;
	i = front_y + offset + c_b;
    }
    
    c_b = 0;
    c_e = 320;
    offset = 240 * width;
    o = output_y + offset;
    i = front_y + offset;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	o = output_y + offset;
	i = front_y + offset;
    }

    c_b = 0;
    c_e = 320;
    offset = 480 * width;
    o = output_y + offset;
    i = front_y + offset;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	c_e -= 1;
	offset += width;
	o = output_y + offset;
	i = front_y + offset;
    }

    //rear Y
    c_b = 1200;
    c_e = 1280;
    offset = 0;
    o = output_y + c_b;
    i = rear_y + c_b;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_b -= 1;
	o = output_y + offset + c_b;
	i = rear_y + offset + c_b;
    }
    
    c_b = 960;
    c_e = 1280;
    offset = 240 * width;
    o = output_y + offset + c_b;
    i = rear_y + offset + c_b;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	o = output_y + offset + c_b;
	i = rear_y + offset + c_b;
    }

    c_b = 960;
    c_e = 1280;
    offset = 480 * width;
    o = output_y + offset + c_b;
    i = rear_y + offset + c_b;

    for (r = 0; r < 240; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	c_b += 1;
	offset += width;
	o = output_y + offset + c_b;
	i = rear_y + offset + c_b;
    }

    //right UV
    c_b = 80;
    c_e = 1200;
    offset = 0;
    o = output_uv + c_b;
    i = right_uv + c_b;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_b += 2;
	c_e -= 2;
	o = output_uv + offset + c_b;
	i = right_uv + offset + c_b;
    }

    //left UV
    c_b = 320;
    c_e = 960;
    offset = 240 * width;
    o = output_uv + offset + c_b;
    i = left_uv + offset + c_b;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_b -= 2;
	c_e += 2;
	o = output_uv + offset + c_b;
	i = left_uv + offset + c_b;
    }

    //front UV
    c_b = 0;
    c_e = 80;
    offset = 0;
    o = output_uv;
    i = front_uv;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_e += 2;
	o = output_uv + offset + c_b;
	i = front_uv + offset + c_b;
    }
    
    c_b = 0;
    c_e = 320;
    offset = 120 * width;
    o = output_uv + offset;
    i = front_uv + offset;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	o = output_uv + offset;
	i = front_uv + offset;
    }

    c_b = 0;
    c_e = 320;
    offset = 240 * width;
    o = output_uv + offset;
    i = front_uv + offset;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	c_e -= 2;
	offset += width;
	o = output_uv + offset;
	i = front_uv + offset;
    }

    //rear UV
    c_b = 1200;
    c_e = 1280;
    offset = 0;
    o = output_uv + c_b;
    i = rear_uv + c_b;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	c_b -= 2;
	o = output_uv + offset + c_b;
	i = rear_uv + offset + c_b;
    }
    
    c_b = 960;
    c_e = 1280;
    offset = 120 * width;
    o = output_uv + offset + c_b;
    i = rear_uv + offset + c_b;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	offset += width;
	o = output_uv + offset + c_b;
	i = rear_uv + offset + c_b;
    }

    c_b = 960;
    c_e = 1280;
    offset = 240 * width;
    o = output_uv + offset + c_b;
    i = rear_uv + offset + c_b;

    for (r = 0; r < 120; r++)
    {
	/*
	for (c = c_b; c < c_e; c++)
	{
	    *(o++) = *(i++);
	}
	*/
	memcpy(o, i, c_e - c_b);
	c_b += 2;
	offset += width;
	o = output_uv + offset + c_b;
	i = rear_uv + offset + c_b;
    }

    return 0;
}
 
int deInitStitching(void)
{
    return 0;
}
