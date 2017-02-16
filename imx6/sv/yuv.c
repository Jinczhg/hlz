#include <string.h>

#include "yuv.h"

struct data_type
{
    char data[1280];
};

void yuv_merge_all(int stride, int height, char *front, char *rear, char *left, char *right, char *dst, int y_half_size, int uv_half_size)
{
#if 0
    int i = 0;
    int uv_height = height >> 1;
    char *y_above = dst;
    char *y_below = dst + y_half_size;
    char *uv_above = y_below + y_half_size;
    char *uv_below = uv_above + uv_half_size;

    //Y
    do
    {
        memcpy(y_above, front, stride);
        front += stride;
        y_above += stride;
        memcpy(y_above, right, stride);
        right += stride;
        y_above += stride;
        
        memcpy(y_below, rear, stride);
        rear += stride;
        y_below += stride;
        memcpy(y_below, left, stride);
        left += stride;
        y_below += stride;
    } while (++i < height);

    //UV
    i = 0;
    do
    {
        memcpy(uv_above, front, stride);
        front += stride;
        uv_above += stride;
        memcpy(uv_above, right, stride);
        right += stride;
        uv_above += stride;
        
        memcpy(uv_below, rear, stride);
        rear += stride;
        uv_below += stride;
        memcpy(uv_below, left, stride);
        left += stride;
        uv_below += stride;
    } while (++i < uv_height);
#else
    int i = 0;
    int uv_height = height >> 1;
    struct data_type *y_above = (struct data_type *)dst;
    struct data_type *y_below = (struct data_type *)(dst + y_half_size);
    struct data_type *uv_above = (struct data_type *)(dst + y_half_size + y_half_size);
    struct data_type *uv_below = (struct data_type *)(dst + y_half_size + y_half_size + uv_half_size);
    struct data_type *front_t = (struct data_type *)front;
    struct data_type *rear_t = (struct data_type *)rear;
    struct data_type *left_t = (struct data_type *)left;
    struct data_type *right_t = (struct data_type *)right;

    //Y
    do
    {
        //memcpy(y_above++, front_t++, stride);
        //memcpy(y_above++, right_t++, stride);
        *(y_above++) = *(front_t++);
        *(y_above++) = *(right_t++);
        
        //memcpy(y_below++, rear_t++, stride);
        //memcpy(y_below++, left_t++, stride);
        *(y_below++) = *(rear_t++);
        *(y_below++) = *(left_t++);
    } while (++i < height);
    
    //UV
    i = 0;
    do
    {
        //memcpy(uv_above++, front_t++, stride);
        //memcpy(uv_above++, right_t++, stride);
        *(uv_above++) = *(front_t++);
        *(uv_above++) = *(right_t++);
        
        //memcpy(uv_below++, rear_t++, stride);
        //memcpy(uv_below++, left_t++, stride);
        *(uv_below++) = *(rear_t++);
        *(uv_below++) = *(left_t++);
    } while (++i < uv_height);
    
    usleep(1000);
#endif
}

void yuv_merge(int stride, int height, char *front, char *rear, char *left, char *right, char *dst)
{
    int i = 0;
    int y_half = (stride * height) << 1;
    int uv_half = stride * height;
    int s_2 = stride << 1;
    int uv_height = height >> 1;
    
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
            for (i = 0; i < uv_height; i++)
            {
	        memcpy(dst, front, stride);
	        front += stride;
	        dst += s_2;
	    }
	}
	
	if (right)
        {
            for (i = 0; i < uv_height; i++)
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
            for (i = 0; i < uv_height; i++)
            {
	        memcpy(dst, rear, stride);
	        rear += stride;
	        dst += s_2;
	    }
	}
	
	if (left)
        {
            for (i = 0; i < uv_height; i++)
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
