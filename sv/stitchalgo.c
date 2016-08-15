/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include "stitchalgo.h"
#include "trace.h"

struct col_offset_data
{
    int begin;
    int end;
};

struct camera_mapping
{
    uint16_t row;
    uint16_t col;
    uint16_t *data;

    int x;
    int y;

    int row_begin;
    int row_end;
    int col_begin;
    int col_end;

    int **y_offset;
    int **uv_offset;

    uint8_t **y_addr;
    uint16_t **uv_addr;

    struct col_offset_data *y_col_offset;
    struct col_offset_data *uv_col_offset;
    
    struct col_offset_data *dst_y_col_offset;
    struct col_offset_data *dst_uv_col_offset;
};

struct camera_data
{
    uint16_t square_size;
    struct camera_mapping front;
    struct camera_mapping rear;
    struct camera_mapping left;
    struct camera_mapping right;
};

static uint32_t s_in_stride = 0, s_y_offset = 0, s_uv_offset, s_in_width = 0, s_in_hight = 0, s_out_width = 0, s_out_hight = 0, s_img_size = 0; 
static uint16_t *s_data = NULL;
static struct camera_data s_camera;
static int *dst_y_row_offset = NULL;
static int *dst_uv_row_offset = NULL;
static int s_buffer_is_map = 0;

static int checkCameraData(struct camera_data *camera)
{
    int ret_val = 0;

    if ((camera->front.row % 2) != 0 || (camera->front.col % 2) != 0
	    || (camera->rear.row % 2) != 0 || (camera->rear.col % 2) != 0
	    || (camera->left.row % 2) != 0 || (camera->left.col % 2) != 0
	    || (camera->right.row % 2) != 0 || (camera->right.col % 2) != 0)
    {
       ret_val = -1;
    }

    return ret_val;
}

static int parseCameraData(uint16_t *data, struct camera_data *camera)
{
    int ret_val = 0;

    if (data)
    {
	camera->square_size = *data;
	data++;

	camera->front.row = *data;
	data++;
	camera->front.col = *data;
	data++;
	camera->front.data = data;
	data += camera->front.row * camera->front.col * 2;

        DEBUG("front: row=%d, col=%d", camera->front.row, camera->front.col);

	camera->rear.row = *data;
	data++;
	camera->rear.col = *data;
	data++;
	camera->rear.data = data;
	data += camera->rear.row * camera->rear.col * 2;

	DEBUG("rear: row=%d, col=%d", camera->rear.row, camera->rear.col);
	
	camera->left.row = *data;
	data++;
	camera->left.col = *data;
	data++;
	camera->left.data = data;
	data += camera->left.row * camera->left.col * 2;

	DEBUG("left: row=%d, col=%d", camera->left.row, camera->left.col);
	
	camera->right.row = *data;
	data++;
	camera->right.col = *data;
	data++;
	camera->right.data = data;
	data += camera->right.row * camera->right.col * 2;

	DEBUG("right: row=%d, col=%d", camera->right.row, camera->right.col);
	
	camera->front.x = 0;
	camera->front.y = 0;
	camera->front.row_begin = 0;
	camera->front.row_end = camera->front.row;
	camera->front.col_begin = 0;
	camera->front.col_end = camera->front.col;

	camera->rear.x = s_out_width - camera->rear.col;
	camera->rear.y = 0;
	camera->rear.row_begin = 0;
	camera->rear.row_end = camera->rear.row;
	camera->rear.col_begin = 0;
	camera->rear.col_end = camera->rear.col;

        camera->left.x = 0;
	camera->left.y = s_out_hight - camera->left.row;
	camera->left.row_begin = 0;
	camera->left.row_end = camera->left.row;
	camera->left.col_begin = camera->front.col;
	camera->left.col_end = s_out_width - camera->rear.col;

	camera->right.x = 0;
	camera->right.y = 0;
	camera->right.row_begin = 0;
	camera->right.row_end = camera->right.row;
	camera->right.col_begin = camera->front.col;
	camera->right.col_end = s_out_width - camera->rear.col;

	ret_val = checkCameraData(camera);
    }
    else
    {
	ret_val = -1;
    }

    return ret_val;
}

static int setMappingOffset(struct camera_mapping *mapping)
{
    int r = 0;
    int c = 0;
    uint16_t *row_data = mapping->data;
    int c_offset = 0;
    int r_offset = 0;

    int x = mapping->x;
    int y = mapping->y;
    int ret_val = 0;

    mapping->y_offset = (int**)malloc(sizeof(int*) * mapping->row);
    mapping->uv_offset = (int**)malloc(sizeof(int*) * (mapping->row >> 1));
    mapping->y_addr = (uint8_t**)malloc(sizeof(uint8_t*) * mapping->row * (mapping->col_end - mapping->col_begin));
    mapping->uv_addr = (uint16_t**)malloc(sizeof(uint16_t*) * (mapping->row >> 1) * ((mapping->col_end - mapping->col_begin) >> 1));

    for (r = 0; r < mapping->row; r++)
    {
	mapping->y_offset[r] = (int*)malloc(sizeof(int) * mapping->col);

	if (r % 2 == 0)
	{
	    mapping->uv_offset[r>>1] = (int*)malloc(sizeof(int) * (mapping->col >> 1));
	}

	for (c = 0; c < mapping->col; c++)
	{
	    c_offset = *row_data; //x
	    r_offset = *(row_data + 1);//y
	    row_data += 2;
	    mapping->y_offset[r][c] = r_offset * s_in_stride + c_offset;

	    if (r % 2 == 0 && c % 2 == 0)
	    {
		mapping->uv_offset[r>>1][c>>1] = (r_offset >> 1) * (s_in_stride >> 1) + (c_offset >> 1);
	    }
	}
    }

    mapping->y_col_offset = (struct col_offset_data*)malloc(sizeof(struct col_offset_data) * mapping->row);
    mapping->uv_col_offset = (struct col_offset_data*)malloc(sizeof(struct col_offset_data) * (mapping->row >> 1));
    mapping->dst_y_col_offset = (struct col_offset_data*)malloc(sizeof(struct col_offset_data) * mapping->row);
    mapping->dst_uv_col_offset = (struct col_offset_data*)malloc(sizeof(struct col_offset_data) * (mapping->row >> 1));

    for (r = 0; r < mapping->row; r++)
    {
	mapping->y_col_offset[r].begin = (int)(mapping->y_offset[r] + mapping->col_begin);
        mapping->y_col_offset[r].end = (int)(mapping->y_offset[r] + mapping->col_end);

	mapping->dst_y_col_offset[r].begin = dst_y_row_offset[y+r] + x + mapping->col_begin;
        mapping->dst_y_col_offset[r].end = dst_y_row_offset[y+r] + x + mapping->col_end;


	if (r % 2 == 0)
	{
            mapping->uv_col_offset[r >> 1].begin = (int)(mapping->uv_offset[r >> 1] + (mapping->col_begin >> 1));
            mapping->uv_col_offset[r >> 1].end = (int)(mapping->uv_offset[r >> 1] + (mapping->col_end >> 1));

	    mapping->dst_uv_col_offset[r >> 1].begin = dst_uv_row_offset[(y+r) >> 1] + ((x + mapping->col_begin) >> 1);
	    mapping->dst_uv_col_offset[r >> 1].end = dst_uv_row_offset[(y+r) >> 1] + ((x + mapping->col_end) >> 1);
	}
    }

out:
    return ret_val;
}

static void setBufferMap(uint8_t *addr, struct camera_mapping *mapping)
{
    uint8_t **y_addr = mapping->y_addr;
    uint16_t **uv_addr = mapping->uv_addr;
    int r = 0;
    int row_y_begin = mapping->row_begin;
    int row_y_end = mapping->row_end;
    int row_uv_begin = mapping->row_begin >> 1;
    int row_uv_end = mapping->row_end >> 1;

    uint8_t *y = addr + s_y_offset;
    uint16_t *uv = (uint16_t*)(addr + s_uv_offset);

    struct col_offset_data *y_col_offset = NULL;
    struct col_offset_data *uv_col_offset = NULL;
    int *y_offset = NULL;
    int *uv_offset = NULL;
    int *offset_end = 0;
    
    r = row_y_begin;
    do
    {
        y_col_offset = mapping->y_col_offset + r;
	y_offset = (int*)y_col_offset->begin;
	offset_end = (int*)y_col_offset->end;
        
	do
	{
	    *(y_addr++) = y + *(y_offset++);
	} while (y_offset < offset_end);
    } while (++r < row_y_end);

    r = row_uv_begin;
    do
    {
        uv_col_offset = mapping->uv_col_offset + r;
	uv_offset = (int*)uv_col_offset->begin;
	offset_end = (int*)uv_col_offset->end;

	do
	{
	    *(uv_addr++) = uv + *(uv_offset++);
	} while (uv_offset < offset_end);
    } while (++r < row_uv_end);
}

static void setAllBufferMap(uint8_t *front_addr, uint8_t *rear_addr, uint8_t *left_addr, uint8_t *right_addr, struct camera_data *camera)
{
    setBufferMap(front_addr, &camera->front);
    setBufferMap(rear_addr, &camera->rear);
    setBufferMap(left_addr, &camera->left);
    setBufferMap(right_addr, &camera->right);

    s_buffer_is_map = 1; 
}

static void fillImage(uint8_t *dst, uint8_t *src, struct camera_mapping *mapping)
{
    int row_y = mapping->row_begin;
    int row_y_end = mapping->row_end;
    int row_uv = mapping->row_begin >> 1;
    int row_uv_end = mapping->row_end >> 1;

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);

    uint8_t *col_dst_y = NULL;
    uint8_t *col_dst_y_end = NULL;
    uint16_t *col_dst_uv = NULL;
    uint16_t *col_dst_uv_end = NULL;

    uint8_t **y_addr = mapping->y_addr;
    uint16_t **uv_addr = mapping->uv_addr;

    //Y
    do
    {
        col_dst_y = dst_y + mapping->dst_y_col_offset[row_y].begin;
	col_dst_y_end = dst_y + mapping->dst_y_col_offset[row_y].end;

	do
	{
	    *(col_dst_y++) = **(y_addr++);
	} while (col_dst_y < col_dst_y_end);
    } while (++row_y < row_y_end);

    //UV
    do
    {
	col_dst_uv = dst_uv + mapping->dst_uv_col_offset[row_uv].begin;
	col_dst_uv_end = dst_uv + mapping->dst_uv_col_offset[row_uv].end;
	
	do
	{
	    *(col_dst_uv++) = **(uv_addr++);
	} while (col_dst_uv < col_dst_uv_end);
    } while (++row_uv < row_uv_end);
}

int initStitching(uint32_t in_stride, uint32_t y_offset, uint32_t uv_offset,
	uint32_t in_width, uint32_t in_hight, uint32_t out_width, uint32_t out_hight)
{
    int data_fd = -1;
    struct stat statbuf;
    int file_len = 0;
    char path[PATH_MAX] = {0};
    int i = 0;
    int ret_val = 0;

    s_in_stride = in_stride;
    s_y_offset = y_offset;
    s_uv_offset = uv_offset;
    s_in_width = in_width;
    s_in_hight = in_hight;
    s_out_width = out_width;
    s_out_hight = out_hight;
    s_img_size = out_width * out_hight;

    readlink("proc/self/exefile", path, PATH_MAX);
    for (i = strlen(path) - 1; i > 0; i--)
    {
	if (path[i] != '/')
	{
	    path[i] = '0';
	}
	else
	{
	    break;
	}
    }

    strcat(path, "camera.dat");

    data_fd = open(path, O_RDONLY);
    if (data_fd < 0)
    {
	ERROR("open error: %s--%s", strerror(errno), path);
	ret_val = -1;
	goto out;
    }

    if (stat(path, &statbuf) < 0)
    {
	ERROR("stat error: %s--%s", strerror(errno), path);
	ret_val = -1;
	goto out;
    }

    file_len = statbuf.st_size;
    s_data = (uint16_t*)malloc(file_len);
    if (!s_data)
    {
	ERROR("malloc error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }

    if (read(data_fd, s_data, file_len) != file_len)
    {
	ERROR("read error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }

    dst_y_row_offset = (int*)malloc(sizeof(int) * s_out_hight);
    dst_uv_row_offset = (int*)malloc(sizeof(int) * (s_out_hight >> 1));

    if (!dst_y_row_offset || !dst_y_row_offset)
    {
        ERROR("malloc error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }

    for (i = 0; i < s_out_hight; i++)
    {
	dst_y_row_offset[i] = i * s_out_width;

	if (i % 2 == 0)
	{
	    dst_uv_row_offset[i >> 1] = (i >> 1) * (s_out_width >> 1);
	}
    }
   
    memset(&s_camera, 0, sizeof(struct camera_data));
    if (parseCameraData(s_data, &s_camera) == -1)
    {
	ret_val = -1;
	goto out;
    }

    setMappingOffset(&s_camera.front);
    setMappingOffset(&s_camera.rear);
    setMappingOffset(&s_camera.left);
    setMappingOffset(&s_camera.right);

out:
    if (data_fd != -1)
    {
	close(data_fd);
	data_fd = -1;
    }

    DEBUG("initStitching OK");

    return ret_val;
}

int doStitching(YUVBuffer *front, YUVBuffer* rear, YUVBuffer *left, YUVBuffer *right, YUVBuffer *output)
{
#if 0
    if (!s_buffer_is_map)
    {
	setAllBufferMap((uint8_t*)front->bufYUV, (uint8_t*)rear->bufYUV, (uint8_t*)left->bufYUV, (uint8_t*)right->bufYUV, &s_camera);
    }

    fillImage((uint8_t*)output->bufYUV, (uint8_t*)front->bufYUV, &s_camera.front);
    fillImage((uint8_t*)output->bufYUV, (uint8_t*)rear->bufYUV, &s_camera.rear);
    fillImage((uint8_t*)output->bufYUV, (uint8_t*)left->bufYUV, &s_camera.left);
    fillImage((uint8_t*)output->bufYUV, (uint8_t*)right->bufYUV, &s_camera.right);

#else

#if 0
    register int r = output->height;
    register uint8_t *o, *i;
    register int width = output->width;
    int uv_offset = output->width * output->height;

    o = (uint8_t*)output->bufYUV;
    i = (uint8_t*)rear->bufYUV + s_y_offset;
    
    do
    {
	memcpy(o, i, width);
	o += width;
	i += s_in_stride;
    } while (--r);

    o = (uint8_t*)output->bufYUV + uv_offset;
    i = (uint8_t*)rear->bufYUV + s_uv_offset;
    r = output->height >> 1;

    do
    {
	memcpy(o, i, width);
	o += width;
	i += s_in_stride;
    } while (--r);

#else
    int r = 0, c = 0;
    int c_b, c_e;
    register int offset;
    register int i_offset;
    uint8_t *o, *i;
    register int width = output->width;
    int uv_offset = output->width * output->height;

    uint8_t *output_y = (uint8_t*)output->bufYUV;
    uint8_t *output_uv = output_y + uv_offset;
    
    uint8_t *front_y = (uint8_t*)front->bufYUV + s_y_offset;
    uint8_t *front_uv = (uint8_t*)front->bufYUV + s_uv_offset;
    
    uint8_t *rear_y = (uint8_t*)rear->bufYUV + s_y_offset;
    uint8_t *rear_uv = (uint8_t*)rear->bufYUV + s_uv_offset;
    
    uint8_t *left_y = (uint8_t*)left->bufYUV + s_y_offset;
    uint8_t *left_uv = (uint8_t*)left->bufYUV + s_uv_offset;
    
    uint8_t *right_y = (uint8_t*)right->bufYUV + s_y_offset;
    uint8_t *right_uv = (uint8_t*)right->bufYUV + s_uv_offset;
    

    //right Y
    c_b = 80;
    c_e = 1200;
    offset = 0;
    i_offset = 0;
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
	i_offset += s_in_stride;
	c_b += 1;
	c_e -= 1;
	o = output_y + offset + c_b;
	i = right_y + i_offset + c_b;
    }

    //left Y
    c_b = 320;
    c_e = 960;
    offset = 480 * width;
    i_offset = 480 * s_in_stride;
    o = output_y + offset + c_b;
    i = left_y + i_offset + c_b;

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
	i_offset += s_in_stride;
	c_b -= 1;
	c_e += 1;
	o = output_y + offset + c_b;
	i = left_y + i_offset + c_b;
    }

    //front Y
    c_b = 0;
    c_e = 80;
    offset = 0;
    i_offset = 0;
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
	i_offset += s_in_stride;
	c_e += 1;
	o = output_y + offset + c_b;
	i = front_y + i_offset + c_b;
    }
    
    c_b = 0;
    c_e = 320;
    offset = 240 * width;
    i_offset = 240 * s_in_stride;
    o = output_y + offset;
    i = front_y + i_offset;

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
	i_offset += s_in_stride;
	o = output_y + offset;
	i = front_y + i_offset;
    }

    c_b = 0;
    c_e = 320;
    offset = 480 * width;
    i_offset = 480 * s_in_stride;
    o = output_y + offset;
    i = front_y + i_offset;

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
	i_offset += s_in_stride;
	o = output_y + offset;
	i = front_y + i_offset;
    }

    //rear Y
    c_b = 1200;
    c_e = 1280;
    offset = 0;
    i_offset = 0;
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
	i_offset += s_in_stride;
	c_b -= 1;
	o = output_y + offset + c_b;
	i = rear_y + i_offset + c_b;
    }
    
    c_b = 960;
    c_e = 1280;
    offset = 240 * width;
    i_offset = 240 * s_in_stride;
    o = output_y + offset + c_b;
    i = rear_y + i_offset + c_b;

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
	i_offset += s_in_stride;
	o = output_y + offset + c_b;
	i = rear_y + i_offset + c_b;
    }

    c_b = 960;
    c_e = 1280;
    offset = 480 * width;
    i_offset = 480 * s_in_stride;
    o = output_y + offset + c_b;
    i = rear_y + i_offset + c_b;

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
	i_offset += s_in_stride;
	o = output_y + offset + c_b;
	i = rear_y + i_offset + c_b;
    }

    //right UV
    c_b = 80;
    c_e = 1200;
    offset = 0;
    i_offset = 0;
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
	i_offset += s_in_stride;
	c_b += 2;
	c_e -= 2;
	o = output_uv + offset + c_b;
	i = right_uv + i_offset + c_b;
    }

    //left UV
    c_b = 320;
    c_e = 960;
    offset = 240 * width;
    i_offset = 240 * s_in_stride;
    o = output_uv + offset + c_b;
    i = left_uv + i_offset + c_b;

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
	i_offset += s_in_stride;
	c_b -= 2;
	c_e += 2;
	o = output_uv + offset + c_b;
	i = left_uv + i_offset + c_b;
    }

    //front UV
    c_b = 0;
    c_e = 80;
    offset = 0;
    i_offset = 0;
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
	i_offset += s_in_stride;
	c_e += 2;
	o = output_uv + offset + c_b;
	i = front_uv + i_offset + c_b;
    }
    
    c_b = 0;
    c_e = 320;
    offset = 120 * width;
    i_offset = 120 * s_in_stride;
    o = output_uv + offset;
    i = front_uv + i_offset;

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
	i_offset += s_in_stride;
	o = output_uv + offset;
	i = front_uv + i_offset;
    }

    c_b = 0;
    c_e = 320;
    offset = 240 * width;
    i_offset = 240 * s_in_stride;
    o = output_uv + offset;
    i = front_uv + i_offset;

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
	i_offset += s_in_stride;
	o = output_uv + offset;
	i = front_uv + i_offset;
    }

    //rear UV
    c_b = 1200;
    c_e = 1280;
    offset = 0;
    i_offset = 0;
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
	i_offset += s_in_stride;
	c_b -= 2;
	o = output_uv + offset + c_b;
	i = rear_uv + i_offset + c_b;
    }
    
    c_b = 960;
    c_e = 1280;
    offset = 120 * width;
    i_offset = 120 * s_in_stride;
    o = output_uv + offset + c_b;
    i = rear_uv + i_offset + c_b;

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
	i_offset += s_in_stride;
	o = output_uv + offset + c_b;
	i = rear_uv + i_offset + c_b;
    }

    c_b = 960;
    c_e = 1280;
    offset = 240 * width;
    i_offset = 240 * s_in_stride;
    o = output_uv + offset + c_b;
    i = rear_uv + i_offset + c_b;

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
	i_offset += s_in_stride;
	o = output_uv + offset + c_b;
	i = rear_uv + i_offset + c_b;
    }
#endif
#endif

    return 0;
}
 
int deInitStitching(void)
{
    if (s_data)
    {
	free(s_data);
	s_data = NULL;
    }

    return 0;
}
