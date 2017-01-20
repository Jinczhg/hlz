#include <string.h>

#include "yuv.h"

void yuv_merge(int stride, int height, char *front, char *rear, char *left, char *right, char *dst)
{
    int i = 0;
    int y_half = (stride * height) << 1;
    int uv_half = stride * height;
    int s_2 = stride << 1;
    
    if (!front && !right)
    {
        dst += y_half;
    }
    else
    {
        if (front)
        {
            for (i = 0; i < height; i++)
            {
	        memcpy(dst, front, stride);
	        front += stride;
	        dst += s_2;
	    }
	}
	
	if (right)
	{
	    dst += stride;
            for (i = 0; i < height; i++)
            {
	        memcpy(dst, right, stride);
	        right += stride;
	        dst += s_2;
	    }
	}
    }

    if (!rear && !left)
    {
        dst += y_half;
    }
    else
    {
        if (rear)
        {
            for (i = 0; i < height; i++)
            {
	        memcpy(dst, rear, stride);
	        rear += stride;
	        dst += s_2;
	    }
	}

	if (left)
	{
	    dst += stride;
            for (i = 0; i < height; i++)
            {
	        memcpy(dst, left, stride);
	        left += stride;
	        dst += s_2;
	    }
	}
    }
    
    if (!front && !right)
    {
        dst += uv_half;
    }
    else
    {
        if (front)
        {
            for (i = 0; i < height / 2; i++)
            {
	        memcpy(dst, front, stride);
	        front += stride;
	        dst += s_2;
	    }
	}
	
	if (right)
        {
            for (i = 0; i < height / 2; i++)
            {
	        memcpy(dst, right, stride);
	        right += stride;
	        dst += s_2;
	    }
	}
    }

    if (rear || left)
    {
        if (rear)
        {
            for (i = 0; i < height / 2; i++)
            {
	        memcpy(dst, rear, stride);
	        rear += stride;
	        dst += s_2;
	    }
	}
	
	if (left)
        {
            for (i = 0; i < height / 2; i++)
            {
	        memcpy(dst, left, stride);
	        left += stride;
	        dst += s_2;
	    }
	}
    }
}

#if 0
void uyvy_merge(int stride, int height, char *front, char *rear, char *left, char *right, char *dst)
{
    int i = 0;
    stride = stride * 2;
    
    for (i = 0; i < height; i++)
    {
	memcpy(dst, front, stride);
	dst += stride;
	front += stride;
	memcpy(dst, right, stride);
	dst += stride;
	right += stride;
    }

    for (i = 0; i < height; i++)
    {
	memcpy(dst, rear, stride);
	dst += stride;
	rear += stride;
	memcpy(dst, left, stride);
	dst += stride;
	left += stride;
    }
}
#else
void uyvy_merge(int stride, int height, char *front, char *rear, char *left, char *right, char *dst)
{
    int i = 0;
    stride = stride * 2;
    
    for (i = 0; i < height; i++)
    {
        if (front)
        {
	    memcpy(dst, front, stride);
	    front += stride;
	}
	dst += stride;
	
	if (right)
	{
	    memcpy(dst, right, stride);
	    right += stride;
	}
	dst += stride;
    }

    for (i = 0; i < height; i++)
    {
        if (rear)
        {
	    memcpy(dst, rear, stride);
	    rear += stride;
	}
	dst += stride;
	
	if (left)
	{
	    memcpy(dst, left, stride);
	    left += stride;
	}
	dst += stride;
    }
}
#endif

void yuv_nv12_2_uyvy_merge(int stride, int height, char *front, char *rear, char *left, char *right, char *dst)
{
    int w = 0, h = 0;
    int uyvy_w = stride << 1;
    int uyvy_half = (uyvy_w * height) << 1;
    
    char *dst_m = dst;
    
    char *front_y = front;
    char *rear_y = rear;
    char *left_y = left;
    char *right_y = right; 
    

    if (!front && !right)
    {
        dst_m += uyvy_half;
    }
    else
    {
    for (h = 0; h < height; h++)
    {
        if (front)
        {
            for (w = 0; w < stride; w++)
            {
                *(dst_m++) = 128;
                *(dst_m++) = *(front_y++);
	    }
	}
	else
	{
	    dst_m += uyvy_w;
	}
	
	if (right)
        {
            for (w = 0; w < stride; w++)
            {
                *(dst_m++) = 128;
                *(dst_m++) = *(right_y++);
	    }
	}
	else
	{
	    dst_m += uyvy_w;
	}
    }
    }
    
    for (h = 0; h < height; h++)
    {
        if (rear)
        {
            for (w = 0; w < stride; w++)
            {
                *(dst_m++) = 128;
                *(dst_m++) = *(rear_y++);
	    }
	}
	else
	{
	    dst_m += uyvy_w;
	}
	
	if (left)
        {
            for (w = 0; w < stride; w++)
            {
                *(dst_m++) = 128;
                *(dst_m++) = *(left_y++);
	    }
	}
	else
	{
	    dst_m += uyvy_w;
	}
    }
}
