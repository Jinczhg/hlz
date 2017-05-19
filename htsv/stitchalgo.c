/*
 * Copyright (c) 2016, Shanghai Hinge Electronic Technology Co.,Ltd
 * All rights reserved.
 *
 * Date: 2016-06-01
 * Author: ryan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stitchalgo.h"

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#ifndef ERROR
#define ERROR(FMT, ...) printf("%s:%d:\t%s\terror: " FMT "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif

#ifndef DEBUG
#define DEBUG(FMT, ...) printf("%s:%d:\t%s\tdebug: " FMT "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif

#ifndef INFO
#define INFO(FMT, ...) printf("%s:%d:\t%s\tinfo: " FMT "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif

#define CLIP3(x, minv, maxv) ((x) > (maxv) ? (maxv) : ((x) < (minv) ? (minv) : x)) 

#define OVERLAP_SIDE 64
#define OVERLAP_Y_SIDE OVERLAP_SIDE
#define OVERLAP_UV_SIDE 32 //(OVERLAP_SIDE >> 1)
#define OVERLAP_Y_POINTS (OVERLAP_Y_SIDE * OVERLAP_Y_SIDE)
#define OVERLAP_UV_POINTS (OVERLAP_UV_SIDE * OVERLAP_UV_SIDE)
#define OVERLAP_Y_SHIFT 12  //2 ^ OVERLAP_Y_SHIFT = OVERLAP_Y_POINTS
#define OVERLAP_UV_SHIFT 10 //2 ^ OVERLAP_UV_SHIFT = OVERLAP_UV_POINTS

struct offset_data
{
    long begin;
    long end;
};

typedef struct __point
{
    unsigned short x;
    unsigned short y;
} point;

typedef struct __line
{
    int size;
    point *p;
} line;

struct camera_mapping
{
    uint16_t row;
    uint16_t col;
    uint16_t *data;

    int x; //locate x
    int y; //locate y

    int row_begin;
    int row_end;

    int row_door_open_begin;
    int row_door_open_end;

    int **y_offset;  //offset from camera mapping data, for input
    int **uv_offset; //offset fromt camera mapping data, for input

    struct offset_data *y_col_offset;  //offset of y_offset
    struct offset_data *uv_col_offset; //offset of uv_offset
    
    struct offset_data *dst_y_col_offset;  //offset of output y address
    struct offset_data *dst_uv_col_offset; //offset of output uv address

    struct offset_data *col_offset;
    
    //overlap
    int y_fix_1;
    int y_fix_2;
    float y_fix_step;
    int y_fix_step_divisor;
    int u_fix_1;
    int u_fix_2;
    float u_fix_step;
    int u_fix_step_divisor;
    int v_fix_1;
    int v_fix_2;
    float v_fix_step;
    int v_fix_step_divisor;
    struct offset_data *overlap_1_y_col_offset;  //offset of y_offset
    struct offset_data *overlap_2_y_col_offset;  //offset of y_offset
    struct offset_data *overlap_1_uv_col_offset; //offset of y_offset
    struct offset_data *overlap_2_uv_col_offset; //offset of y_offset
    struct offset_data overlap_1_row_offset;  //row offset
    struct offset_data overlap_2_row_offset;  //row offset
    struct offset_data overlap_1_col_offset;  //col offset
    struct offset_data overlap_2_col_offset;  //col offset


};

struct camera_data
{
    uint16_t width;
    uint16_t height;
    struct camera_mapping front;
    struct camera_mapping rear;
    struct camera_mapping left;
    struct camera_mapping right;
    line overlap_line[4];
};

struct car_mapping
{
    uint16_t img_width;
    uint16_t img_height;
    uint16_t row;
    uint16_t col;
    uint16_t car_width;
    uint16_t car_height;
    uint16_t width_offset;
    uint16_t height_offset;
    uint16_t *all_data;
    uint16_t *refill_data;
    
    int x; //locate x
    int y; //locate y
    int img_size; // = img_width * img_height
    
    int **y_offset;  //offset from car mapping data, for input
    int **uv_offset; //offset fromt car mapping data, for input
    
    struct offset_data *y_col_offset;  //offset of y_offset
    struct offset_data *uv_col_offset; //offset of uv_offset
    
    struct offset_data *dst_y_col_offset;  //offset of output y address
    struct offset_data *dst_uv_col_offset; //offset of output uv address
    
    int **refill_y_offset;  //offset from car mapping data, for input
    int **refill_uv_offset; //offset fromt car mapping data, for input
    
    struct offset_data *refill_y_col_offset;  //offset of y_offset
    struct offset_data *refill_uv_col_offset; //offset of uv_offset
    
    int *refill_y_all_offset;
    int *refill_dst_y_all_offset;
    
    int *refill_uv_all_offset;
    int *refill_dst_uv_all_offset;
    
    int refill_y_count;
    int refill_uv_count;
    
    struct offset_data *col_offset;
};

struct camera_undistorted_mapping
{
    uint16_t row;
    uint16_t col;
    uint16_t *data;

    int **y_offset;  //offset from camera mapping data, for input
    int **uv_offset; //offset fromt camera mapping data, for input
};

struct camera_undistorted_data
{
    int width_normal;
    int height_normal;
    int size_normal;
    int width_wide;
    int height_wide;
    int size_wide;

    int left_right_pad;

    struct camera_undistorted_mapping front;
    struct camera_undistorted_mapping rear;
    struct camera_undistorted_mapping left_front;
    struct camera_undistorted_mapping right_front;
    struct camera_undistorted_mapping left_rear;
    struct camera_undistorted_mapping right_rear;
};


typedef struct _stitch_control
{
    int value[CONTROL_MAX_NUM];
} stitch_control;

static uint32_t s_in_stride = 0, s_y_offset = 0, s_uv_offset, s_in_width = 0, s_in_height = 0, s_img_size = 0; 
static uint16_t *s_data = NULL;
static uint16_t *s_car_data = NULL;
static uint16_t *s_undistorted_data = NULL;
static char *s_car_img_data = NULL;
static struct camera_data s_camera;
static struct car_mapping s_car;
static struct camera_undistorted_data s_camera_undistorted; 
static int *dst_y_row_offset = NULL;
static int *dst_uv_row_offset = NULL;
static int s_car_is_fill = 0;
static char s_dir[PATH_MAX] = {0};

static char *black_data = NULL;
static stitch_control s_control = {{0}};

static int checkCameraData(struct camera_data *camera)
{
    int ret_val = 0;

    // because two rows Y share one row UV, and we will calculate half of the row or col, so they must be multiple of 4 
    if ((camera->front.row % 4) != 0 || (camera->front.col % 4) != 0
	    || (camera->rear.row % 4) != 0 || (camera->rear.col % 4) != 0
	    || (camera->left.row % 4) != 0 || (camera->left.col % 4) != 0
	    || (camera->right.row % 4) != 0 || (camera->right.col % 4) != 0)
    {
	ERROR("the row and col of all the camera mapping data must be multiple of 4");
	ret_val = -1;
    }

    return ret_val;
}

/*
 ****************************************
 *      *                       *       *
 *       *        right        *        *
 *        *                   *         *    
 *         *******************          *
 *         *                 *          *
 * front   *       car       * rear     *
 *         *                 *          *
 *         *******************          *
 *        *                   *         *
 *       *        left         *        *
 *      *                       *       *
 ****************************************
 ***************************
 *                         *
 *           front         *
 * *                     * *
 *   *                 *   *    
 *     *             *     *
 *       ***********       *
 *       *         *       *
 *       *         *       *
 *       *         *       *
 * left  *   car   * right *
 *       *         *       *
 *       *         *       *
 *       *         *       *
 *       ***********       *
 *     *             *     *
 *   *                 *   *
 * *         rear        * *
 *                         *
 *                         * 
 ***************************
 */
static int calcColOffset()
{
    struct camera_mapping *front = &s_camera.front;
    struct camera_mapping *rear = &s_camera.rear;
    struct camera_mapping *left = &s_camera.left;
    struct camera_mapping *right = &s_camera.right;
    int r = 0;
    int ret_val = 0;

    //front
    front->col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * front->row);
    if (!front->col_offset)
    {
        ERROR("malloc error:%s", strerror(errno));
	ret_val = -1;
	goto out;
    }
    
    front->overlap_1_col_offset.begin = left->col - OVERLAP_SIDE;
    front->overlap_1_col_offset.end = left->col;
    front->overlap_1_row_offset.begin = front->row - OVERLAP_SIDE;
    front->overlap_1_row_offset.end = front->row;
    
    front->overlap_2_col_offset.begin = front->col - right->col;
    front->overlap_2_col_offset.end = (front->col - right->col) + OVERLAP_SIDE;
    front->overlap_2_row_offset.begin = front->row - OVERLAP_SIDE;
    front->overlap_2_row_offset.end = front->row;

    front->y_fix_step_divisor = front->col - left->col - right->col;
    front->u_fix_step_divisor = front->y_fix_step_divisor >> 1;
    front->v_fix_step_divisor = front->y_fix_step_divisor >> 1;
    
    //rear
    rear->col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * rear->row);
    if (!rear->col_offset)
    {
        ERROR("malloc error:%s", strerror(errno));
	ret_val = -1;
	goto out;
    }
  
    rear->overlap_1_col_offset.begin = left->col - OVERLAP_SIDE;
    rear->overlap_1_col_offset.end = left->col;
    rear->overlap_1_row_offset.begin = 0;
    rear->overlap_1_row_offset.end = OVERLAP_SIDE;
    
    rear->overlap_2_col_offset.begin = rear->col - right->col;
    rear->overlap_2_col_offset.end = (rear->col - right->col) + OVERLAP_SIDE;
    rear->overlap_2_row_offset.begin = 0;
    rear->overlap_2_row_offset.end = OVERLAP_SIDE;
    
    rear->y_fix_step_divisor = rear->col - left->col - right->col;
    rear->u_fix_step_divisor = rear->y_fix_step_divisor >> 1;
    rear->v_fix_step_divisor = rear->y_fix_step_divisor >> 1;

    //left
    left->col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * left->row);
    if (!left->col_offset)
    {
        ERROR("malloc error:%s", strerror(errno));
	ret_val = -1;
	goto out;
    }
  
    left->overlap_1_col_offset.begin = left->col - OVERLAP_SIDE;
    left->overlap_1_col_offset.end = left->col;
    left->overlap_1_row_offset.begin = front->row - OVERLAP_SIDE;
    left->overlap_1_row_offset.end = front->row;
    
    left->overlap_2_col_offset.begin = left->col - OVERLAP_SIDE;
    left->overlap_2_col_offset.end = left->col;
    left->overlap_2_row_offset.begin = left->row - rear->row;
    left->overlap_2_row_offset.end = (left->row - rear->row) + OVERLAP_SIDE;

    left->y_fix_step_divisor = left->row - front->row - rear->row;
    left->u_fix_step_divisor = left->y_fix_step_divisor >> 1;
    left->v_fix_step_divisor = left->y_fix_step_divisor >> 1;
    
    //right
    right->col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * right->row);
    if (!right->col_offset)
    {
        ERROR("malloc error:%s", strerror(errno));
	ret_val = -1;
	goto out;
    }
    
    right->overlap_1_col_offset.begin = 0;
    right->overlap_1_col_offset.end = OVERLAP_SIDE;
    right->overlap_1_row_offset.begin = front->row - OVERLAP_SIDE;
    right->overlap_1_row_offset.end = front->row;
    
    right->overlap_2_col_offset.begin = 0;
    right->overlap_2_col_offset.end = OVERLAP_SIDE;
    right->overlap_2_row_offset.begin = right->row - rear->row;
    right->overlap_2_row_offset.end = (right->row - rear->row) + OVERLAP_SIDE;

    right->y_fix_step_divisor = right->row - front->row - rear->row;
    right->u_fix_step_divisor = right->y_fix_step_divisor >> 1;
    right->v_fix_step_divisor = right->y_fix_step_divisor >> 1;

#ifdef VERTICAL_SCREEN
    //front row
    for (r = 0; r < front->row; r++)
    {
	if (r < front->row / 2) // front->row / 2
	{
            front->col_offset[r].begin = 0;
	    front->col_offset[r].end = front->col;	    
	}
	else //the other rows
	{
            front->col_offset[r].begin = ((2 * r * left->col) / front->row - left->col) >> 1 << 1;
	    front->col_offset[r].end = (front->col - ((2 * r * right->col) / front->row - right->col)) >> 1 << 1;

	    left->col_offset[r].begin = 0;
	    left->col_offset[r].end = front->col_offset[r].begin;

	    right->col_offset[r].begin = right->col - (front->col - front->col_offset[r].end);
	    right->col_offset[r].end = right->col;

            s_camera.overlap_line[0].p[s_camera.overlap_line[0].size].y = r;
	    s_camera.overlap_line[0].p[s_camera.overlap_line[0].size].x = CLIP3(front->col_offset[r].begin, 0, left->col - 1);
	    s_camera.overlap_line[0].size++;

            s_camera.overlap_line[1].p[s_camera.overlap_line[1].size].y = r;
	    s_camera.overlap_line[1].p[s_camera.overlap_line[1].size].x = CLIP3(front->col_offset[r].end, front->col - right->col, front->col - 1);
	    s_camera.overlap_line[1].size++;
	}
    }

    //rear row
    for (r = 0; r < rear->row; r++)
    {
	if (r >= rear->row / 2) // rear->row / 2
	{
            rear->col_offset[r].begin = 0;
	    rear->col_offset[r].end = rear->col;
	}
	else //the other rows
	{
	    rear->col_offset[r].begin =  (left->col - (2 * r * left->col) / rear->row) >> 1 << 1;
	    rear->col_offset[r].end = (front->col - right->col + (2 * r * right->col) / rear->row) >> 1 << 1;

	    left->col_offset[left->row - rear->row + r].begin = 0;
	    left->col_offset[left->row - rear->row + r].end = rear->col_offset[r].begin;

	    right->col_offset[right->row - rear->row + r].begin = right->col - (rear->col - rear->col_offset[r].end);
	    right->col_offset[right->row - rear->row + r].end = right->col;

            s_camera.overlap_line[2].p[s_camera.overlap_line[2].size].y = left->row - rear->row + r;
	    s_camera.overlap_line[2].p[s_camera.overlap_line[2].size].x = CLIP3(rear->col_offset[r].begin, 0, left->col - 1);
	    s_camera.overlap_line[2].size++;

            s_camera.overlap_line[3].p[s_camera.overlap_line[3].size].y = right->row - rear->row + r;
	    s_camera.overlap_line[3].p[s_camera.overlap_line[3].size].x = CLIP3(rear->col_offset[r].end, rear->col - right->col, rear->col - 1);
	    s_camera.overlap_line[3].size++;

	}
    }

    //left row
    for (r = front->row; r < left->row - rear->row; r++)
    {
	left->col_offset[r].begin = 0;
	left->col_offset[r].end = left->col;
    }

    //right row
    for (r = front->row; r < right->row - rear->row; r++)
    {
	right->col_offset[r].begin = 0;
	right->col_offset[r].end = right->col;
    }
#else
    //front row
    for (r = 0; r < front->row; r++)
    {
	if (r < right->row) //right row
	{
	    front->col_offset[r].begin = 0;
	    front->col_offset[r].end = front->col / 2 + ((r * front->col) / (2 * right->row) >> 1 << 1);

	    right->col_offset[r].begin = front->col_offset[r].end;
	}
	else if (r >= front->row - left->row) //left row
	{
	    front->col_offset[r].begin = 0;
	    front->col_offset[r].end = front->col - (((r + left->row - front->row) * front->col) / (2 * left->row) >> 1 << 1);

	    left->col_offset[r + left->row - front->row].begin = front->col_offset[r].end;
	}
	else //the middle rows
	{
	    front->col_offset[r].begin = 0;
	    front->col_offset[r].end = front->col;
	}
    }

    //rear row
    for (r = 0; r < rear->row; r++)
    {
	if (r < right->row) //right row
	{
	    rear->col_offset[r].begin = rear->col / 2 - ((r * rear->col) / (2 * right->row) >> 1 << 1);
	    rear->col_offset[r].end = rear->col;

	    right->col_offset[r].end = right->col - rear->col + rear->col_offset[r].begin;
	}
	else if (r >= rear->row - left->row) //left row
	{
	    rear->col_offset[r].begin = (((r + left->row - front->row) * rear->col) / (2 * left->row) >> 1 << 1);
	    rear->col_offset[r].end = rear->col;

	    left->col_offset[r + left->row - rear->row].end = left->col - rear->col + rear->col_offset[r].begin;
	}
	else //the middle rows
	{
	    rear->col_offset[r].begin = 0;
	    rear->col_offset[r].end = rear->col;
	}
    }
#endif

out:
    return ret_val;
}

static int calcCarColOffset()
{
    int r = 0;
    int ret_val = 0;

    s_car.col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * s_car.row);
    if (!s_car.col_offset)
    {
        ERROR("malloc error:%s", strerror(errno));
	ret_val = -1;
	goto out;
    }

    for (r = 0; r < s_car.row; r++)
    {
	s_car.col_offset[r].begin = 0;
	s_car.col_offset[r].end = s_car.col;
    }
out:
    return ret_val;
}

static int parseCameraData(uint16_t *data, struct camera_data *camera)
{
    int ret_val = 0;

    if (data)
    {
        camera->width = *data;
	data += 1;
	camera->height = *data;
	data += 1;

        printf("width=%d, height=%d\n", camera->width, camera->height);

	data += 1;//point_type and reseve

	//front
	camera->front.row = *data;
	data++;
	camera->front.col = *data;
	data++;
	camera->front.data = data;
	data += camera->front.row * camera->front.col * 2;

        DEBUG("front: row=%d, col=%d", camera->front.row, camera->front.col);

	//rear
	camera->rear.row = *data;
	data++;
	camera->rear.col = *data;
	data++;
	camera->rear.data = data;
	data += camera->rear.row * camera->rear.col * 2;

	DEBUG("rear: row=%d, col=%d", camera->rear.row, camera->rear.col);
	
	//left
	camera->left.row = *data;
	data++;
	camera->left.col = *data;
	data++;
	camera->left.data = data;
	data += camera->left.row * camera->left.col * 2;

	DEBUG("left: row=%d, col=%d", camera->left.row, camera->left.col);
	
	//right
	camera->right.row = *data;
	data++;
	camera->right.col = *data;
	data++;
	camera->right.data = data;
	data += camera->right.row * camera->right.col * 2;

	DEBUG("right: row=%d, col=%d", camera->right.row, camera->right.col);

#ifdef VERTICAL_SCREEN
        camera->front.x = 0;
	camera->front.y = 0;
	camera->front.row_begin = 0;
	camera->front.row_end = camera->front.row;

	camera->rear.x = 0;
	camera->rear.y = camera->height - camera->rear.row;
	camera->rear.row_begin = 0;
	camera->rear.row_end = camera->rear.row;

        camera->left.x = 0;
	camera->left.y = 0;
	camera->left.row_begin = camera->front.row / 2;
	camera->left.row_end = camera->left.row - camera->rear.row / 2;
        camera->left.row_door_open_begin = camera->front.row;
	camera->left.row_door_open_end = camera->left.row - camera->rear.row;

	camera->right.x = camera->width - camera->right.col;
	camera->right.y = 0;
	camera->right.row_begin =  camera->front.row / 2;
	camera->right.row_end = camera->right.row - camera->rear.row / 2;
        camera->right.row_door_open_begin =  camera->front.row;
	camera->right.row_door_open_end = camera->right.row - camera->rear.row;
#else
	camera->front.x = 0;
	camera->front.y = 0;
	camera->front.row_begin = 0;
	camera->front.row_end = camera->front.row;

	camera->rear.x = camera->width - camera->rear.col;
	camera->rear.y = 0;
	camera->rear.row_begin = 0;
	camera->rear.row_end = camera->rear.row;

        camera->left.x = 0;
	camera->left.y = camera->height - camera->left.row;
	camera->left.row_begin = 0;
	camera->left.row_end = camera->left.row;

	camera->right.x = 0;
	camera->right.y = 0;
	camera->right.row_begin = 0;
	camera->right.row_end = camera->right.row;
#endif

	camera->overlap_line[0].p = (point*)malloc(sizeof(point) * (camera->front.row / 2 + 1));
	camera->overlap_line[0].size = 0;

        camera->overlap_line[1].p = (point*)malloc(sizeof(point) * (camera->front.row / 2 + 1));
	camera->overlap_line[1].size = 0;

        camera->overlap_line[2].p = (point*)malloc(sizeof(point) * (camera->rear.row / 2 + 1));
	camera->overlap_line[2].size = 0;

	camera->overlap_line[3].p = (point*)malloc(sizeof(point) * (camera->rear.row / 2 + 1));
	camera->overlap_line[3].size = 0;

	ret_val = checkCameraData(camera);
    }
    else
    {
	ret_val = -1;
    }

    return ret_val;
}

static int parseCameraUndistortedData(uint16_t *data, struct camera_undistorted_data *camera)
{
    int ret_val = 0;

    if (data)
    {
	//front
	camera->front.row = *data;
	data++;
	camera->front.col = *data;
	data++;
	camera->front.data = data;
	data += camera->front.row * camera->front.col * 2;

        DEBUG("front: row=%d, col=%d", camera->front.row, camera->front.col);

	//rear
	camera->rear.row = *data;
	data++;
	camera->rear.col = *data;
	data++;
	camera->rear.data = data;
	data += camera->rear.row * camera->rear.col * 2;

	DEBUG("rear: row=%d, col=%d", camera->rear.row, camera->rear.col);
	
	//left_front
	camera->left_front.row = *data;
	data++;
	camera->left_front.col = *data;
	data++;
	camera->left_front.data = data;
	data += camera->left_front.row * camera->left_front.col * 2;

	DEBUG("left_front: row=%d, col=%d", camera->left_front.row, camera->left_front.col);
	
	//right_front
	camera->right_front.row = *data;
	data++;
	camera->right_front.col = *data;
	data++;
	camera->right_front.data = data;
	data += camera->right_front.row * camera->right_front.col * 2;

	DEBUG("right_front: row=%d, col=%d", camera->right_front.row, camera->right_front.col);

        //left_rear
	camera->left_rear.row = *data;
	data++;
	camera->left_rear.col = *data;
	data++;
	camera->left_rear.data = data;
	data += camera->left_rear.row * camera->left_rear.col * 2;

	DEBUG("left_rear: row=%d, col=%d", camera->left_rear.row, camera->left_rear.col);
	
	//right_front
	camera->right_rear.row = *data;
	data++;
	camera->right_rear.col = *data;
	data++;
	camera->right_rear.data = data;
	data += camera->right_rear.row * camera->right_rear.col * 2;

	DEBUG("right_rear: row=%d, col=%d", camera->right_rear.row, camera->right_rear.col);

	camera->width_normal = camera->rear.col;
	camera->height_normal = camera->rear.row;
	camera->size_normal = camera->width_normal *  camera->height_normal; 

	camera->width_wide = camera->front.col;
	camera->height_wide = camera->front.row;
        camera->size_wide = camera->width_wide *  camera->height_wide;
	camera->left_right_pad = camera->width_normal - camera->left_front.col - camera->right_front.col;
    }
    else
    {
	ret_val = -1;
    }

    return ret_val;
}


static int parseCarData(uint16_t *data, struct car_mapping *car)
{
    int ret_val = 0;

    if (data)
    {
        car->img_width = *data;
	data += 1;
	car->img_height = *data;
	data += 1;

        car->row = *data;
	data += 1;
	car->col = *data;
	data += 1;
	
	car->car_width = *data;
	data += 1;
	car->car_height = *data;
	data += 1;
	
	car->width_offset = *data;
	data += 1;
	car->height_offset = *data;
	data += 1;
	
	car->all_data = data;
	data += car->row * car->col * 2;
	car->refill_data = data;
	data += car->row * car->col * 2;
	
	printf("car img_width=%d, car img_height=%d\n", car->img_width, car->img_height);
	printf("car row=%d, car col=%d\n", car->row, car->col);
	printf("car car_width=%d, car car_height=%d\n", car->car_width, car->car_height);
	printf("car width_offset=%d, car height_offset=%d\n", car->width_offset, car->height_offset);
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
    if (!mapping->y_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->uv_offset = (int**)malloc(sizeof(int*) * (mapping->row >> 1));
    if (!mapping->uv_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    for (r = 0; r < mapping->row; r++)
    {
	mapping->y_offset[r] = (int*)malloc(sizeof(int) * mapping->col);
	if (!mapping->y_offset[r])
	{
	    ERROR("malloc error:%s", strerror(errno));
	    goto out;
	}

	if (r % 2 == 0)
	{
	    mapping->uv_offset[r>>1] = (int*)malloc(sizeof(int) * (mapping->col >> 1));
	    if (!mapping->uv_offset[r>>1])
	    {
		ERROR("malloc error:%s", strerror(errno));
		goto out;
	    }
	}

	for (c = 0; c < mapping->col; c++)
	{
	    c_offset = *row_data; //x
	    r_offset = *(row_data + 1);//y
	    
	    if (c_offset >= s_in_width)
	    {
	        //DEBUG("c_offset >= s_in_width, c_offset = %d", c_offset);
	        c_offset = s_in_width - 1;
	    }
	    if (r_offset >= s_in_height)
	    {
	        //DEBUG("r_offset >= s_in_height, r_offset = %d", r_offset);
	        r_offset = s_in_height -1;
	    }
	    row_data += 2;
	    mapping->y_offset[r][c] = r_offset * s_in_stride + c_offset;

	    if (r % 2 == 0 && c % 2 == 0)
	    {
		mapping->uv_offset[r>>1][c>>1] = (r_offset >> 1) * (s_in_stride >> 1) + (c_offset >> 1);
	    }
	}
    }

    mapping->y_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * mapping->row);
    if (!mapping->y_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->uv_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->row >> 1));
    if (!mapping->uv_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->dst_y_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * mapping->row);
    if (!mapping->dst_y_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->dst_uv_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->row >> 1));
    if (!mapping->dst_uv_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    for (r = mapping->row_begin; r < mapping->row_end; r++)
    {
	mapping->y_col_offset[r].begin = (long)(mapping->y_offset[r] + mapping->col_offset[r].begin);
        mapping->y_col_offset[r].end = (long)(mapping->y_offset[r] + mapping->col_offset[r].end);

	mapping->dst_y_col_offset[r].begin = dst_y_row_offset[y+r] + x + mapping->col_offset[r].begin;
        mapping->dst_y_col_offset[r].end = dst_y_row_offset[y+r] + x + mapping->col_offset[r].end;

	if (r % 2 == 0)
	{
            mapping->uv_col_offset[r >> 1].begin = (long)(mapping->uv_offset[r >> 1] + (mapping->col_offset[r].begin >> 1));
            mapping->uv_col_offset[r >> 1].end = (long)(mapping->uv_offset[r >> 1] + (mapping->col_offset[r].end >> 1));

	    mapping->dst_uv_col_offset[r >> 1].begin = dst_uv_row_offset[(y+r) >> 1] + ((x + mapping->col_offset[r].begin) >> 1);
	    mapping->dst_uv_col_offset[r >> 1].end = dst_uv_row_offset[(y+r) >> 1] + ((x + mapping->col_offset[r].end) >> 1);
	}
    }
    
    //overlap
    mapping->overlap_1_y_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->overlap_1_row_offset.end - mapping->overlap_1_row_offset.begin));
    mapping->overlap_1_uv_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->overlap_1_row_offset.end - mapping->overlap_1_row_offset.begin) / 2);
    for (r = 0; r < mapping->overlap_1_row_offset.end - mapping->overlap_1_row_offset.begin; r++)
    {
	mapping->overlap_1_y_col_offset[r].begin = (long)(mapping->y_offset[r + mapping->overlap_1_row_offset.begin] + mapping->overlap_1_col_offset.begin);
        mapping->overlap_1_y_col_offset[r].end = (long)(mapping->y_offset[r + mapping->overlap_1_row_offset.begin] + mapping->overlap_1_col_offset.end);

	if (r % 2 == 0)
	{
            mapping->overlap_1_uv_col_offset[r >> 1].begin = (long)(mapping->uv_offset[(r + mapping->overlap_1_row_offset.begin) >> 1] + (mapping->overlap_1_col_offset.begin >> 1));
            mapping->overlap_1_uv_col_offset[r >> 1].end = (long)(mapping->uv_offset[(r + mapping->overlap_1_row_offset.begin) >> 1] + (mapping->overlap_1_col_offset.end >> 1));
	}
    }
    
    mapping->overlap_2_y_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->overlap_2_row_offset.end - mapping->overlap_2_row_offset.begin));
    mapping->overlap_2_uv_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->overlap_2_row_offset.end - mapping->overlap_2_row_offset.begin) / 2);
    for (r = 0; r < mapping->overlap_2_row_offset.end - mapping->overlap_2_row_offset.begin; r++)
    {
	mapping->overlap_2_y_col_offset[r].begin = (long)(mapping->y_offset[r + mapping->overlap_2_row_offset.begin] + mapping->overlap_2_col_offset.begin);
        mapping->overlap_2_y_col_offset[r].end = (long)(mapping->y_offset[r + mapping->overlap_2_row_offset.begin] + mapping->overlap_2_col_offset.end);
    
        if (r % 2 == 0)
	{
            mapping->overlap_2_uv_col_offset[r >> 1].begin = (long)(mapping->uv_offset[(r + mapping->overlap_2_row_offset.begin) >> 1] + (mapping->overlap_2_col_offset.begin >> 1));
            mapping->overlap_2_uv_col_offset[r >> 1].end = (long)(mapping->uv_offset[(r + mapping->overlap_2_row_offset.begin) >> 1] + (mapping->overlap_2_col_offset.end >> 1));
	}
    }

out:
    return ret_val;
}

static int setCarMappingOffset(struct car_mapping *mapping)
{
    int r = 0;
    int c = 0;
    uint16_t *row_data = mapping->all_data;
    int c_offset = 0;
    int r_offset = 0;

    int x = mapping->x;
    int y = mapping->y;
    int ret_val = 0;

    mapping->y_offset = (int**)malloc(sizeof(int*) * mapping->row);
    if (!mapping->y_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->uv_offset = (int**)malloc(sizeof(int*) * (mapping->row >> 1));
    if (!mapping->uv_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    for (r = 0; r < mapping->row; r++)
    {
	mapping->y_offset[r] = (int*)malloc(sizeof(int) * mapping->col);
	if (!mapping->y_offset[r])
	{
	    ERROR("malloc error:%s", strerror(errno));
	    goto out;
	}

	if (r % 2 == 0)
	{
	    mapping->uv_offset[r>>1] = (int*)malloc(sizeof(int) * (mapping->col >> 1));
	    if (!mapping->uv_offset[r>>1])
	    {
		ERROR("malloc error:%s", strerror(errno));
		goto out;
	    }
	}

	for (c = 0; c < mapping->col; c++)
	{
	    c_offset = *row_data; //x
	    r_offset = *(row_data + 1);//y
	    row_data += 2;
	    mapping->y_offset[r][c] = r_offset * mapping->img_width + c_offset;

	    if (r % 2 == 0 && c % 2 == 0)
	    {
		mapping->uv_offset[r>>1][c>>1] = (r_offset >> 1) * (mapping->img_width >> 1) + (c_offset >> 1);
	    }
	}
    }

    mapping->y_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * mapping->row);
    if (!mapping->y_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->uv_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->row >> 1));
    if (!mapping->uv_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->dst_y_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * mapping->row);
    if (!mapping->dst_y_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->dst_uv_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->row >> 1));
    if (!mapping->dst_uv_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    for (r = 0; r < mapping->row; r++)
    {
	mapping->y_col_offset[r].begin = (long)(mapping->y_offset[r] + mapping->col_offset[r].begin);
        mapping->y_col_offset[r].end = (long)(mapping->y_offset[r] + mapping->col_offset[r].end);

	mapping->dst_y_col_offset[r].begin = dst_y_row_offset[y+r] + x + mapping->col_offset[r].begin;
        mapping->dst_y_col_offset[r].end = dst_y_row_offset[y+r] + x + mapping->col_offset[r].end;

	if (r % 2 == 0)
	{
            mapping->uv_col_offset[r >> 1].begin = (long)(mapping->uv_offset[r >> 1] + (mapping->col_offset[r].begin >> 1));
            mapping->uv_col_offset[r >> 1].end = (long)(mapping->uv_offset[r >> 1] + (mapping->col_offset[r].end >> 1));

	    mapping->dst_uv_col_offset[r >> 1].begin = dst_uv_row_offset[(y+r) >> 1] + ((x + mapping->col_offset[r].begin) >> 1);
	    mapping->dst_uv_col_offset[r >> 1].end = dst_uv_row_offset[(y+r) >> 1] + ((x + mapping->col_offset[r].end) >> 1);
	}
    }

out:
    return ret_val;
}

static int setUndistortedMappingOffset(struct camera_undistorted_mapping *mapping)
{
    int r = 0;
    int c = 0;
    uint16_t *row_data = mapping->data;
    int c_offset = 0;
    int r_offset = 0;

    int ret_val = 0;

    mapping->y_offset = (int**)malloc(sizeof(int*) * mapping->row);
    if (!mapping->y_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->uv_offset = (int**)malloc(sizeof(int*) * (mapping->row >> 1));
    if (!mapping->uv_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    for (r = 0; r < mapping->row; r++)
    {
	mapping->y_offset[r] = (int*)malloc(sizeof(int) * mapping->col);
	if (!mapping->y_offset[r])
	{
	    ERROR("malloc error:%s", strerror(errno));
	    goto out;
	}

	if (r % 2 == 0)
	{
	    mapping->uv_offset[r>>1] = (int*)malloc(sizeof(int) * (mapping->col >> 1));
	    if (!mapping->uv_offset[r>>1])
	    {
		ERROR("malloc error:%s", strerror(errno));
		goto out;
	    }
	}

	for (c = 0; c < mapping->col; c++)
	{
	    c_offset = *row_data; //x
	    r_offset = *(row_data + 1);//y
	    
	    if (c_offset >= s_in_width)
	    {
	        c_offset = s_in_width - 1;
	    }
	    if (r_offset >= s_in_height)
	    {
	        r_offset = s_in_height -1;
	    }
	    row_data += 2;
	    mapping->y_offset[r][c] = r_offset * s_in_stride + c_offset;

	    if (r % 2 == 0 && c % 2 == 0)
	    {
		mapping->uv_offset[r>>1][c>>1] = (r_offset >> 1) * (s_in_stride >> 1) + (c_offset >> 1);
	    }
	}
    }
    
out:
    return ret_val;
}


static int setCarRefillMappingOffset(struct car_mapping *mapping)
{
    int r = 0;
    int c = 0;
    uint16_t *row_data = mapping->refill_data;
    int c_offset = 0;
    int r_offset = 0;
    int ret_val = 0;

    mapping->refill_y_offset = (int**)malloc(sizeof(int*) * mapping->row);
    if (!mapping->refill_y_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->refill_uv_offset = (int**)malloc(sizeof(int*) * (mapping->row >> 1));
    if (!mapping->refill_uv_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    for (r = 0; r < mapping->row; r++)
    {
	mapping->refill_y_offset[r] = (int*)malloc(sizeof(int) * mapping->col);
	if (!mapping->refill_y_offset[r])
	{
	    ERROR("malloc error:%s", strerror(errno));
	    goto out;
	}

	if (r % 2 == 0)
	{
	    mapping->refill_uv_offset[r>>1] = (int*)malloc(sizeof(int) * (mapping->col >> 1));
	    if (!mapping->refill_uv_offset[r>>1])
	    {
		ERROR("malloc error:%s", strerror(errno));
		goto out;
	    }
	}

	for (c = 0; c < mapping->col; c++)
	{
	    c_offset = *row_data; //x
	    r_offset = *(row_data + 1);//y
	    row_data += 2;
	    mapping->refill_y_offset[r][c] = r_offset * mapping->img_width + c_offset;
	    if (mapping->refill_y_offset[r][c] != 0)
	    {
	        mapping->refill_y_count++;
	    }

	    if (r % 2 == 0 && c % 2 == 0)
	    {
		mapping->refill_uv_offset[r>>1][c>>1] = (r_offset >> 1) * (mapping->img_width >> 1) + (c_offset >> 1);
		
		if (mapping->refill_uv_offset[r>>1][c>>1] != 0)
		{
		    mapping->refill_uv_count++;
		}
	    }
	}
    }

    mapping->refill_y_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * mapping->row);
    if (!mapping->refill_y_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    mapping->refill_uv_col_offset = (struct offset_data*)malloc(sizeof(struct offset_data) * (mapping->row >> 1));
    if (!mapping->refill_uv_col_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }

    for (r = 0; r < mapping->row; r++)
    {
	mapping->refill_y_col_offset[r].begin = (long)(mapping->refill_y_offset[r] + mapping->col_offset[r].begin);
        mapping->refill_y_col_offset[r].end = (long)(mapping->refill_y_offset[r] + mapping->col_offset[r].end);

	if (r % 2 == 0)
	{
            mapping->refill_uv_col_offset[r >> 1].begin = (long)(mapping->refill_uv_offset[r >> 1] + (mapping->col_offset[r].begin >> 1));
            mapping->refill_uv_col_offset[r >> 1].end = (long)(mapping->refill_uv_offset[r >> 1] + (mapping->col_offset[r].end >> 1));
	}
    }
    
    mapping->refill_y_all_offset = (int*)malloc(sizeof(int*) * mapping->refill_y_count);
    if (!mapping->refill_y_all_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }
    mapping->refill_dst_y_all_offset = (int*)malloc(sizeof(int*) * mapping->refill_y_count);
    if (!mapping->refill_dst_y_all_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }
    
    mapping->refill_uv_all_offset = (int*)malloc(sizeof(int*) * mapping->refill_uv_count);
    if (!mapping->refill_uv_all_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }
    mapping->refill_dst_uv_all_offset = (int*)malloc(sizeof(int*) * mapping->refill_uv_count);
    if (!mapping->refill_dst_uv_all_offset)
    {
	ERROR("malloc error:%s", strerror(errno));
	goto out;
    }
    
    int row_y = 0;
    int row_y_end = mapping->row;
    int row_uv = 0;
    int row_uv_end = mapping->row >> 1;
    int *y_offset = NULL;
    int *uv_offset = NULL;
    int dst_y_offset = 0;
    int dst_y_offset_end = 0;
    int dst_uv_offset = 0;
    int dst_uv_offset_end = 0;
    int offset = 0;
    int index = 0;

    //Y
    do
    {
	
	y_offset = (int*)mapping->refill_y_col_offset[row_y].begin;
        dst_y_offset = mapping->dst_y_col_offset[row_y].begin;
	dst_y_offset_end = mapping->dst_y_col_offset[row_y].end;
	if (dst_y_offset < dst_y_offset_end)
	{
	    do
	    {
		offset = *y_offset++;
		if (offset > 0)
		{
		   //dst_y[dst_y_offset++] = y[offset];
		   mapping->refill_y_all_offset[index] = offset;
		   mapping->refill_dst_y_all_offset[index] = dst_y_offset++;
		   index++;
		}
		else
		{
		   dst_y_offset++;
		}
	    } while (dst_y_offset < dst_y_offset_end);
	}
    } while (++row_y < row_y_end);

    index = 0;
    //UV
    do
    {
	uv_offset = (int*)mapping->refill_uv_col_offset[row_uv].begin;
	dst_uv_offset = mapping->dst_uv_col_offset[row_uv].begin;
	dst_uv_offset_end = mapping->dst_uv_col_offset[row_uv].end;

	if (dst_uv_offset < dst_uv_offset_end)
	{
	    do
	    {
		offset = *uv_offset++;
		if (offset > 0)
		{
		    //dst_uv[dst_uv_offset++] = uv[offset];
		    mapping->refill_uv_all_offset[index] = offset;
		    mapping->refill_dst_uv_all_offset[index] = dst_uv_offset++;
		    index++;
		}
		else
		{
		    dst_uv_offset++;
		}
	    } while (dst_uv_offset < dst_uv_offset_end);
	}
    } while (++row_uv < row_uv_end);

out:
    return ret_val;
}

static void fillCarImage(uint8_t *dst)
{
    int fd = -1;
    struct stat statbuf;
    int len = 0;
    char path[PATH_MAX + 1] = {0};

    strcpy(path, s_dir);
    strcat(path, "carImage.yuv");

    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
	ERROR("open error: %s--%s", strerror(errno), path);
	goto out;
    }

    if (stat(path, &statbuf) < 0)
    {
	ERROR("stat error: %s--%s", strerror(errno), path);
	goto out;
    }

    len = statbuf.st_size;

    if (len != s_car.img_width * s_car.img_height * 3 / 2)
    {
        ERROR("the size of carImage.yuv is %d, not equal to the size of the output", len);
	goto out;
    }
    
    s_car_img_data = malloc(s_car.img_width * s_car.img_height * 3 / 2);

    if (read(fd, s_car_img_data, len) != len)
    {
	ERROR("read error: %s", strerror(errno));
	goto out;
    }
    
    struct car_mapping *mapping = &s_car;
    unsigned char *src = (unsigned char *)s_car_img_data;
    int row_y = 0;
    int row_y_end = mapping->row;
    int row_uv = 0;
    int row_uv_end = mapping->row >> 1;

    uint8_t *y = src;
    uint16_t *uv = (uint16_t*)(src + s_car.img_size);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);

    int *y_offset = NULL;
    int *uv_offset = NULL;
    int dst_y_offset = 0;
    int dst_y_offset_end = 0;
    int dst_uv_offset = 0;
    int dst_uv_offset_end = 0;

    //Y
    do
    {
	
	y_offset = (int*)mapping->y_col_offset[row_y].begin;
        dst_y_offset = mapping->dst_y_col_offset[row_y].begin;
	dst_y_offset_end = mapping->dst_y_col_offset[row_y].end;
	if (dst_y_offset < dst_y_offset_end)
	{
	    do
	    {
		dst_y[dst_y_offset++] = y[*y_offset++];
	    } while (dst_y_offset < dst_y_offset_end);
	}
    } while (++row_y < row_y_end);

    //UV
    do
    {
	uv_offset = (int*)mapping->uv_col_offset[row_uv].begin;
	dst_uv_offset = mapping->dst_uv_col_offset[row_uv].begin;
	dst_uv_offset_end = mapping->dst_uv_col_offset[row_uv].end;

	if (dst_uv_offset < dst_uv_offset_end)
	{
	    do
	    {
		dst_uv[dst_uv_offset++] = uv[*uv_offset++];
	    } while (dst_uv_offset < dst_uv_offset_end);
	}
    } while (++row_uv < row_uv_end);
    
    DEBUG("OK");

out:
    if (fd != -1)
    {
	close(fd);
    }

    s_car_is_fill = 1;
}

static void refillCarImage(uint8_t *dst)
{
    if (!s_car_img_data)
    {
        return;
    }
    
    struct car_mapping *mapping = &s_car;
    unsigned char *src = (unsigned char*)s_car_img_data;

    uint8_t *y = src;
    uint16_t *uv = (uint16_t*)(src + s_car.img_size);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);
    
    int index = 0;
    
    if (mapping->refill_y_all_offset && mapping->refill_dst_y_all_offset)
    {
        do
        {
            dst_y[mapping->refill_dst_y_all_offset[index]] = y[mapping->refill_y_all_offset[index]];
            index++;
        } while (index < mapping->refill_y_count);
    }
    
    index = 0;
    if (mapping->refill_uv_all_offset && mapping->refill_dst_uv_all_offset)
    {
        do
        {
            dst_uv[mapping->refill_dst_uv_all_offset[index]] = uv[mapping->refill_uv_all_offset[index]];
            index++;
        } while (index < mapping->refill_uv_count);
    }
}

static void refillOverlapLine(uint8_t *dst, uint8_t *front, uint8_t *rear, uint8_t *left, uint8_t *right)
{
    int i = 0;
    int j = 0;
    point *p = NULL;
    int dst_offset = 0;
    int front_offset = 0;
    int rear_offset = 0;
    int left_offset = 0;
    int right_offset = 0;
    int right_col_diff = s_camera.front.col - s_camera.right.col;
    int rear_row_diff = s_camera.left.row - s_camera.rear.row;
    float f_1 = 0.5, f_2 = 0.5;
    int value = 0;

    uint8_t *front_y = front + s_y_offset;
    uint8_t *rear_y = rear + s_y_offset;
    uint8_t *left_y = left + s_y_offset;
    uint8_t *right_y = right + s_y_offset;

    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);
    uint16_t *front_uv = (uint16_t*)(front + s_uv_offset);
    uint16_t *rear_uv = (uint16_t*)(rear + s_uv_offset);
    uint16_t *left_uv = (uint16_t*)(left + s_uv_offset);
    uint16_t *right_uv = (uint16_t*)(right + s_uv_offset);
    uint8_t *src_uv_p_1 = NULL;
    uint8_t *src_uv_p_2 = NULL;
    uint8_t *dst_uv_p = NULL;

    //front_left
    if (s_control.value[CONTROL_LEFT_DOOR_OPEN] == 0)
    {
	for (i = 0; i < s_camera.overlap_line[0].size; i++)
	{
	    p = s_camera.overlap_line[0].p + i;
	    dst_offset = dst_y_row_offset[p->y] + p->x;
	    front_offset = s_camera.front.y_offset[p->y][p->x];
	    left_offset = s_camera.left.y_offset[p->y][p->x];
	    value = (front_y[front_offset] + s_camera.front.y_fix_1) * 0.5 + (left_y[left_offset] + s_camera.left.y_fix_1) * 0.5;
	    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

	    if (p->y % 2 == 0 && p->x % 2 == 0)
	    {
		dst_offset = dst_uv_row_offset[p->y >> 1] + (p->x >> 1);
		front_offset = s_camera.front.uv_offset[p->y >> 1][p->x >> 1];
		left_offset = s_camera.left.uv_offset[p->y >> 1][p->x >> 1];
		
		dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
		src_uv_p_1 = (uint8_t*)(&front_uv[front_offset]);
		src_uv_p_2 = (uint8_t*)(&left_uv[left_offset]);

		dst_uv_p[0] = (src_uv_p_1[0] + s_camera.front.u_fix_1) * 0.5 + (src_uv_p_2[0] + s_camera.left.u_fix_1) * 0.5;
		dst_uv_p[1] = (src_uv_p_1[1] + s_camera.front.v_fix_1) * 0.5 + (src_uv_p_2[1] + s_camera.left.v_fix_1) * 0.5;
	    }

	    f_1 = 0.5;
	    f_2 = 0.5;

	    for (j = 1; j < 9; j++)
	    {
		f_1 -= 0.05;
		f_2 += 0.05;

		if (p->x >= j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x - j;
		    front_offset = s_camera.front.y_offset[p->y][p->x - j];
		    left_offset = s_camera.left.y_offset[p->y][p->x - j];
		    value = (front_y[front_offset] + s_camera.front.y_fix_1) * f_1 + (left_y[left_offset] + s_camera.left.y_fix_1) * f_2;
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && (p->x - j) % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x - j) >> 1);
			front_offset = s_camera.front.uv_offset[p->y >> 1][(p->x - j) >> 1];
			left_offset = s_camera.left.uv_offset[p->y >> 1][(p->x - j) >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&front_uv[front_offset]);
			src_uv_p_2 = (uint8_t*)(&left_uv[left_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.front.u_fix_1) * f_1 + (src_uv_p_2[0] + s_camera.left.u_fix_1) * f_2;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.front.v_fix_1) * f_1 + (src_uv_p_2[1] + s_camera.left.v_fix_1) * f_2;
		    }
		}

		if (s_camera.left.col - p->x > j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x + j;
		    front_offset = s_camera.front.y_offset[p->y][p->x + j];
		    left_offset = s_camera.left.y_offset[p->y][p->x + j];
		    value = (front_y[front_offset] + s_camera.front.y_fix_1) * f_2 + (left_y[left_offset] + s_camera.left.y_fix_1) * f_1; 
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && p->x + j % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x + j) >> 1);
			front_offset = s_camera.front.uv_offset[p->y >> 1][(p->x + j) >> 1];
			left_offset = s_camera.left.uv_offset[p->y >> 1][(p->x + j) >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&front_uv[front_offset]);
			src_uv_p_2 = (uint8_t*)(&left_uv[left_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.front.u_fix_1) * f_2 + (src_uv_p_2[0] + s_camera.left.u_fix_1) * f_1;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.front.v_fix_1) * f_2 + (src_uv_p_2[1] + s_camera.left.v_fix_1) * f_1;
		    }
		}
	    }
	}
    }
    
    //front_right
    if (s_control.value[CONTROL_RIGHT_DOOR_OPEN] == 0)
    {
	for (i = 0; i < s_camera.overlap_line[1].size; i++)
	{
	    p = s_camera.overlap_line[1].p + i;
	    dst_offset = dst_y_row_offset[p->y] + p->x;
	    front_offset = s_camera.front.y_offset[p->y][p->x];
	    right_offset = s_camera.right.y_offset[p->y][p->x - right_col_diff];
	    value = (front_y[front_offset] + s_camera.front.y_fix_2) * 0.5 + (right_y[right_offset] + s_camera.right.y_fix_1) * 0.5;
	    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

	    if (p->y % 2 == 0 && p->x % 2 == 0)
	    {
		dst_offset = dst_uv_row_offset[p->y >> 1] + (p->x >> 1);
		front_offset = s_camera.front.uv_offset[p->y >> 1][p->x >> 1];
		right_offset = s_camera.right.uv_offset[p->y >> 1][(p->x - right_col_diff) >> 1];
		
		dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
		src_uv_p_1 = (uint8_t*)(&front_uv[front_offset]);
		src_uv_p_2 = (uint8_t*)(&right_uv[right_offset]);

		dst_uv_p[0] = (src_uv_p_1[0] + s_camera.front.u_fix_2) * 0.5 + (src_uv_p_2[0] + s_camera.right.u_fix_1) * 0.5;
		dst_uv_p[1] = (src_uv_p_1[1] + s_camera.front.v_fix_2) * 0.5 + (src_uv_p_2[1] + s_camera.right.v_fix_1) * 0.5;
	    }

	    f_1 = 0.5;
	    f_2 = 0.5;

	    for (j = 1; j < 9; j++)
	    {
		f_1 -= 0.05;
		f_2 += 0.05;

		if (p->x - right_col_diff >= j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x - j;
		    front_offset = s_camera.front.y_offset[p->y][p->x - j];
		    right_offset = s_camera.right.y_offset[p->y][p->x - right_col_diff - j];
		    value = (front_y[front_offset] + s_camera.front.y_fix_2) * f_2 + (right_y[right_offset] + s_camera.right.y_fix_1) * f_1; 
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && (p->x - j) % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x - j) >> 1);
			front_offset = s_camera.front.uv_offset[p->y >> 1][(p->x - j) >> 1];
			right_offset = s_camera.right.uv_offset[p->y >> 1][(p->x - right_col_diff - j) >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&front_uv[front_offset]);
			src_uv_p_2 = (uint8_t*)(&right_uv[right_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.front.u_fix_2) * f_2 + (src_uv_p_2[0] + s_camera.right.u_fix_1) * f_1;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.front.v_fix_2) * f_2 + (src_uv_p_2[1] + s_camera.right.v_fix_1) * f_1;
		    }
		}

		if (s_camera.front.col - p->x > j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x + j;
		    front_offset = s_camera.front.y_offset[p->y][p->x + j];
		    right_offset = s_camera.right.y_offset[p->y][p->x - right_col_diff + j];
		    value = (front_y[front_offset] + s_camera.front.y_fix_2) * f_1 + (right_y[right_offset] + s_camera.right.y_fix_1) * f_2;
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && p->x + j % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x + j) >> 1);
			front_offset = s_camera.front.uv_offset[p->y >> 1][(p->x + j) >> 1];
			right_offset = s_camera.right.uv_offset[p->y >> 1][(p->x - right_col_diff + j) >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&front_uv[front_offset]);
			src_uv_p_2 = (uint8_t*)(&right_uv[right_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.front.u_fix_2) * f_1 + (src_uv_p_2[0] + s_camera.right.u_fix_1) * f_2;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.front.v_fix_2) * f_1 + (src_uv_p_2[1] + s_camera.right.v_fix_1) * f_2;
		    }
		}
	    }
	}
    }

    //rear_left
    if (s_control.value[CONTROL_LEFT_DOOR_OPEN] == 0)
    {
	for (i = 0; i < s_camera.overlap_line[2].size; i++)
	{
	    p = s_camera.overlap_line[2].p + i;
	    dst_offset = dst_y_row_offset[p->y] + p->x;
	    rear_offset = s_camera.rear.y_offset[p->y - rear_row_diff][p->x];
	    left_offset = s_camera.left.y_offset[p->y][p->x];
	    dst[dst_offset] = (rear_y[rear_offset] + s_camera.rear.y_fix_1) * 0.5 + (left_y[left_offset] + s_camera.left.y_fix_2) * 0.5;
	    
	    if (p->y % 2 == 0 && p->x % 2 == 0)
	    {
		dst_offset = dst_uv_row_offset[p->y >> 1] + (p->x >> 1);
		rear_offset = s_camera.rear.uv_offset[(p->y - rear_row_diff) >> 1][p->x >> 1];
		left_offset = s_camera.left.uv_offset[p->y >> 1][p->x >> 1];
		
		dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
		src_uv_p_1 = (uint8_t*)(&rear_uv[rear_offset]);
		src_uv_p_2 = (uint8_t*)(&left_uv[left_offset]);

		dst_uv_p[0] = (src_uv_p_1[0] + s_camera.rear.u_fix_1) * 0.5 + (src_uv_p_2[0] + s_camera.left.u_fix_2) * 0.5;
		dst_uv_p[1] = (src_uv_p_1[1] + s_camera.rear.v_fix_1) * 0.5 + (src_uv_p_2[1] + s_camera.left.v_fix_2) * 0.5;
	    }
	    f_1 = 0.5;
	    f_2 = 0.5;

	    for (j = 1; j < 9; j++)
	    {
		f_1 -= 0.05;
		f_2 += 0.05;

		if (p->x >= j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x - j;
		    rear_offset = s_camera.rear.y_offset[p->y - rear_row_diff][p->x - j];
		    left_offset = s_camera.left.y_offset[p->y][p->x - j];
		    value = (rear_y[rear_offset] + s_camera.rear.y_fix_1) * f_1 + (left_y[left_offset] + s_camera.left.y_fix_2) * f_2;
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && (p->x - j) % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x - j) >> 1);
			rear_offset = s_camera.rear.uv_offset[(p->y - rear_row_diff) >> 1][(p->x - j) >> 1];
			left_offset = s_camera.left.uv_offset[p->y >> 1][p->x >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&rear_uv[rear_offset]);
			src_uv_p_2 = (uint8_t*)(&left_uv[left_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.rear.u_fix_1) * f_1 + (src_uv_p_2[0] + s_camera.left.u_fix_2) * f_2;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.rear.v_fix_1) * f_1 + (src_uv_p_2[1] + s_camera.left.v_fix_2) * f_2;
		    }
		}

		if (s_camera.left.col - p->x > j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x + j;
		    rear_offset = s_camera.rear.y_offset[p->y - rear_row_diff][p->x + j];
		    left_offset = s_camera.left.y_offset[p->y][p->x + j];
		    value = (rear_y[rear_offset] + s_camera.rear.y_fix_1) * f_2 + (left_y[left_offset] + s_camera.left.y_fix_2) * f_1; 
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && p->x + j % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x + j) >> 1);
			rear_offset = s_camera.rear.uv_offset[(p->y - rear_row_diff) >> 1][(p->x + j) >> 1];
			left_offset = s_camera.left.uv_offset[p->y >> 1][(p->x + j) >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&rear_uv[rear_offset]);
			src_uv_p_2 = (uint8_t*)(&left_uv[left_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.rear.u_fix_1) * f_2 + (src_uv_p_2[0] + s_camera.left.u_fix_2) * f_1;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.rear.v_fix_1) * f_2 + (src_uv_p_2[1] + s_camera.left.v_fix_2) * f_1;
		    }
		}
	    }
	}
    }

    //rear_right
    if (s_control.value[CONTROL_RIGHT_DOOR_OPEN] == 0)
    {
	for (i = 0; i < s_camera.overlap_line[3].size; i++)
	{
	    p = s_camera.overlap_line[3].p + i;
	    dst_offset = dst_y_row_offset[p->y] + p->x;
	    rear_offset = s_camera.rear.y_offset[p->y - rear_row_diff][p->x];
	    right_offset = s_camera.right.y_offset[p->y][p->x - right_col_diff];
	    value = (rear_y[rear_offset] + s_camera.rear.y_fix_2) * 0.5 + (right_y[right_offset] + s_camera.right.y_fix_2) * 0.5;
	    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

	    if (p->y % 2 == 0 && p->x % 2 == 0)
	    {
		dst_offset = dst_uv_row_offset[p->y >> 1] + (p->x >> 1);
		rear_offset = s_camera.rear.uv_offset[(p->y - rear_row_diff) >> 1][p->x >> 1];
		right_offset = s_camera.right.uv_offset[p->y >> 1][(p->x - right_col_diff) >> 1];
		
		dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
		src_uv_p_1 = (uint8_t*)(&rear_uv[rear_offset]);
		src_uv_p_2 = (uint8_t*)(&right_uv[right_offset]);

		dst_uv_p[0] = (src_uv_p_1[0] + s_camera.rear.u_fix_2) * 0.5 + (src_uv_p_2[0] + s_camera.right.u_fix_2) * 0.5;
		dst_uv_p[1] = (src_uv_p_1[1] + s_camera.rear.v_fix_2) * 0.5 + (src_uv_p_2[1] + s_camera.right.v_fix_2) * 0.5;
	    }

	    f_1 = 0.5;
	    f_2 = 0.5;

	    for (j = 1; j < 9; j++)
	    {
		f_1 -= 0.05;
		f_2 += 0.05;

		if (p->x - right_col_diff >= j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x - j;
		    rear_offset = s_camera.rear.y_offset[p->y - rear_row_diff][p->x - j];
		    right_offset = s_camera.right.y_offset[p->y][p->x - right_col_diff - j];
		    value = (rear_y[rear_offset] + s_camera.rear.y_fix_2) * f_2 + (right_y[right_offset] + s_camera.right.y_fix_2) * f_1; 
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && (p->x - j) % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x - j) >> 1);
			rear_offset = s_camera.rear.uv_offset[(p->y - rear_row_diff) >> 1][(p->x - j) >> 1];
			right_offset = s_camera.right.uv_offset[p->y >> 1][(p->x - right_col_diff - j) >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&rear_uv[rear_offset]);
			src_uv_p_2 = (uint8_t*)(&right_uv[right_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.rear.u_fix_2) * f_2 + (src_uv_p_2[0] + s_camera.right.u_fix_2) * f_1;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.rear.v_fix_2) * f_2 + (src_uv_p_2[1] + s_camera.right.v_fix_2) * f_1;
		    }
		}

		if (s_camera.rear.col - p->x > j)
		{
		    dst_offset = dst_y_row_offset[p->y] + p->x + j;
		    rear_offset = s_camera.rear.y_offset[p->y - rear_row_diff][p->x + j];
		    right_offset = s_camera.right.y_offset[p->y][p->x - right_col_diff + j];
		    value = (rear_y[rear_offset] + s_camera.rear.y_fix_2) * f_1 + (right_y[right_offset] + s_camera.right.y_fix_2) * f_2; 
		    dst[dst_offset] = value > 255 ? 255 : (value < 0 ? 0 : value);

		    if (p->y % 2 == 0 && p->x + j % 2 == 0)
		    {
			dst_offset = dst_uv_row_offset[p->y >> 1] + ((p->x + j) >> 1);
			rear_offset = s_camera.rear.uv_offset[(p->y - rear_row_diff) >> 1][(p->x + j) >> 1];
			right_offset = s_camera.right.uv_offset[p->y >> 1][(p->x - right_col_diff + j) >> 1];
			
			dst_uv_p = (uint8_t*)(&dst_uv[dst_offset]);
			src_uv_p_1 = (uint8_t*)(&rear_uv[rear_offset]);
			src_uv_p_2 = (uint8_t*)(&right_uv[right_offset]);

			dst_uv_p[0] = (src_uv_p_1[0] + s_camera.rear.u_fix_2) * f_1 + (src_uv_p_2[0] + s_camera.right.u_fix_2) * f_2;
			dst_uv_p[1] = (src_uv_p_1[1] + s_camera.rear.v_fix_2) * f_1 + (src_uv_p_2[1] + s_camera.right.v_fix_2) * f_2;
		    }
		}
	    }
	}
    }
}


static void fillWithLeftDoorOpen(uint8_t *dst, uint8_t *front, uint8_t *rear)
{
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);
    uint16_t *front_uv = (uint16_t*)(front + s_uv_offset);
    uint16_t *rear_uv = (uint16_t*)(rear + s_uv_offset);

    int dst_offset = 0;
    int front_offset = 0;
    int rear_offset = 0;

    int r = 0, c = 0;

    int rear_row_diff = s_camera.left.row - s_camera.rear.row;
   
    //front left
    for (r = s_camera.left.row_begin; r < s_camera.left.row_door_open_begin; r++)
    {
	for (c = 0; c < s_camera.front.col_offset[r].begin; c++)
	{
	    dst_offset = dst_y_row_offset[r] + c;
	    front_offset = s_camera.front.y_offset[r][c];
	    dst[dst_offset] = front[front_offset];

	    if ((r % 2 == 0) && (c % 2 == 0))
	    {
		dst_offset = dst_uv_row_offset[r >> 1] + (c >> 1);
		front_offset = s_camera.front.uv_offset[r >> 1][c >> 1];
		dst_uv[dst_offset] = front_uv[front_offset];
	    }
	}
    }
    
    //rear left
    for (r = s_camera.left.row_door_open_end; r < s_camera.left.row_end; r++)
    {
	for (c = 0; c < s_camera.rear.col_offset[r - rear_row_diff].begin; c++)
	{
	    dst_offset = dst_y_row_offset[r] + c;
	    rear_offset = s_camera.rear.y_offset[r - rear_row_diff][c];
	    dst[dst_offset] = rear[rear_offset];
	    if ((r % 2 == 0) && (c % 2 == 0))
	    {
		dst_offset = dst_uv_row_offset[r >> 1] + (c >> 1);
		rear_offset = s_camera.rear.uv_offset[(r - rear_row_diff) >> 1][c >> 1];
		dst_uv[dst_offset] = rear_uv[rear_offset];
	    }
	}
    }
}

static void fillWithRightDoorOpen(uint8_t *dst, uint8_t *front, uint8_t *rear)
{
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);
    uint16_t *front_uv = (uint16_t*)(front + s_uv_offset);
    uint16_t *rear_uv = (uint16_t*)(rear + s_uv_offset);

    int dst_offset = 0;
    int front_offset = 0;
    int rear_offset = 0;

    int r = 0, c = 0;

    int rear_row_diff = s_camera.right.row - s_camera.rear.row;
   
    //front right
    for (r = s_camera.right.row_begin; r < s_camera.right.row_door_open_begin; r++)
    {
	for (c = s_camera.front.col_offset[r].end; c < s_camera.front.col; c++)
	{
	    dst_offset = dst_y_row_offset[r] + c;
	    front_offset = s_camera.front.y_offset[r][c];
	    dst[dst_offset] = front[front_offset];

	    if ((r % 2 == 0) && (c % 2 == 0))
	    {
		dst_offset = dst_uv_row_offset[r >> 1] + (c >> 1);
		front_offset = s_camera.front.uv_offset[r >> 1][c >> 1];
		dst_uv[dst_offset] = front_uv[front_offset];
	    }
	}
    }
    
    //rear right
    for (r = s_camera.right.row_door_open_end; r < s_camera.right.row_end; r++)
    {
	for (c = s_camera.rear.col_offset[r - rear_row_diff].end; c < s_camera.rear.col; c++)
	{
	    dst_offset = dst_y_row_offset[r] + c;
	    rear_offset = s_camera.rear.y_offset[r - rear_row_diff][c];
	    dst[dst_offset] = rear[rear_offset];
	    if ((r % 2 == 0) && (c % 2 == 0))
	    {
		dst_offset = dst_uv_row_offset[r >> 1] + (c >> 1);
		rear_offset = s_camera.rear.uv_offset[(r - rear_row_diff) >> 1][c >> 1];
		dst_uv[dst_offset] = rear_uv[rear_offset];
	    }
	}
    }
}

static void fillImage(uint8_t *dst, uint8_t *src, struct camera_mapping *mapping, int door_open)
{
    int row_y = 0;
    int row_y_end = 0;
    int row_uv = 0;
    int row_uv_end = 0;

    if (door_open == 0)
    {
        row_y = mapping->row_begin;
        row_y_end = mapping->row_end;
    }
    else
    {
        row_y = mapping->row_door_open_begin;
        row_y_end = mapping->row_door_open_end;
    }
    
    row_uv = row_y >> 1;
    row_uv_end = row_y_end >> 1;


    uint8_t *y = src + s_y_offset;
    uint16_t *uv = (uint16_t*)(src + s_uv_offset);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);

    int *y_offset = NULL;
    int *uv_offset = NULL;
    int dst_y_offset = 0;
    int dst_y_offset_end = 0;
    int dst_uv_offset = 0;
    int dst_uv_offset_end = 0;

    //Y
    do
    {
	
	y_offset = (int*)mapping->y_col_offset[row_y].begin;
        dst_y_offset = mapping->dst_y_col_offset[row_y].begin;
	dst_y_offset_end = mapping->dst_y_col_offset[row_y].end;

	if (dst_y_offset < dst_y_offset_end)
	{
	    do
	    {
		dst_y[dst_y_offset++] = y[*y_offset++];
	    } while (dst_y_offset < dst_y_offset_end);
	}
    } while (++row_y < row_y_end);

    //UV
    do
    {
	uv_offset = (int*)mapping->uv_col_offset[row_uv].begin;
	dst_uv_offset = mapping->dst_uv_col_offset[row_uv].begin;
	dst_uv_offset_end = mapping->dst_uv_col_offset[row_uv].end;

	if (dst_uv_offset < dst_uv_offset_end)
	{
	    do
	    {
		dst_uv[dst_uv_offset++] = uv[*uv_offset++];
	    } while (dst_uv_offset < dst_uv_offset_end);
	}
    } while (++row_uv < row_uv_end);
}

static void fillImageFix(uint8_t *dst, uint8_t *src, struct camera_mapping *mapping)
{
    int row_y = mapping->row_begin;
    int row_y_end = mapping->row_end;
    int row_uv = mapping->row_begin >> 1;
    int row_uv_end = mapping->row_end >> 1;

    uint8_t *y = src + s_y_offset;
    uint16_t *uv = (uint16_t*)(src + s_uv_offset);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);

    int *y_offset = NULL;
    int *uv_offset = NULL;
    int dst_y_offset = 0;
    int dst_y_offset_end = 0;
    int dst_uv_offset = 0;
    int dst_uv_offset_end = 0;
    uint8_t *uv_dst = NULL;
    uint8_t *uv_src = NULL;
    int value = 0;

    //Y
    do
    {
	
	y_offset = (int*)mapping->y_col_offset[row_y].begin;
        dst_y_offset = mapping->dst_y_col_offset[row_y].begin;
	dst_y_offset_end = mapping->dst_y_col_offset[row_y].end;
	if (mapping->y_fix_1 > 0)
	{
	    if (dst_y_offset < dst_y_offset_end)
	    {
	        do
	        {
		    //dst_y[dst_y_offset++] = y[*y_offset++] + mapping->y_fix_1;
		    value = y[*y_offset++] + mapping->y_fix_1;
		    dst_y[dst_y_offset++] = value > 255 ? 255 : value;
	        } while (dst_y_offset < dst_y_offset_end);
	    }
	}
	else
	{
	    if (dst_y_offset < dst_y_offset_end)
	    {
	        do
	        {
		    value = y[*y_offset++] + mapping->y_fix_1;
		    dst_y[dst_y_offset++] = value < 0 ? 0 : value;
	        } while (dst_y_offset < dst_y_offset_end);
	    }
	}
    } while (++row_y < row_y_end);

    //UV
    do
    {
	uv_offset = (int*)mapping->uv_col_offset[row_uv].begin;
	dst_uv_offset = mapping->dst_uv_col_offset[row_uv].begin;
	dst_uv_offset_end = mapping->dst_uv_col_offset[row_uv].end;

	if (dst_uv_offset < dst_uv_offset_end)
	{
	    do
	    {
		uv_src = (uint8_t*)(&uv[*uv_offset++]);
		uv_dst = (uint8_t*)(&dst_uv[dst_uv_offset++]);
		uv_dst[0] = uv_src[0] + mapping->u_fix_1;
		uv_dst[1] = uv_src[1] + mapping->v_fix_1;
	    } while (dst_uv_offset < dst_uv_offset_end);
	}
    } while (++row_uv < row_uv_end);
}

static void fillImageFixStep(uint8_t *dst, uint8_t *src, struct camera_mapping *mapping)
{
    int row_y = mapping->row_begin;
    int row_y_end = mapping->row_end;
    int row_uv = mapping->row_begin >> 1;
    int row_uv_end = mapping->row_end >> 1;

    uint8_t *y = src + s_y_offset;
    uint16_t *uv = (uint16_t*)(src + s_uv_offset);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_img_size);

    int *y_offset = NULL;
    int *uv_offset = NULL;
    int dst_y_offset = 0;
    int dst_y_offset_end = 0;
    int dst_uv_offset = 0;
    int dst_uv_offset_end = 0;
    uint8_t *uv_dst = NULL;
    uint8_t *uv_src = NULL;
    float y_step = 0.0;
    float u_step = 0.0;
    float v_step = 0.0;
    int col = 0;
    int col_uv_1 = mapping->overlap_1_col_offset.end >> 1;
    int col_uv_2 = mapping->overlap_2_col_offset.begin >> 1;
    int value = 0;

    //Y
    do
    {
	
	y_offset = (int*)mapping->y_col_offset[row_y].begin;
        dst_y_offset = mapping->dst_y_col_offset[row_y].begin;
	dst_y_offset_end = mapping->dst_y_col_offset[row_y].end;

	if (dst_y_offset < dst_y_offset_end)
	{
	    col = mapping->col_offset[row_y].begin;
	    y_step = 0.0;

	    do
	    {
		if (col < mapping->overlap_1_col_offset.end)
		{
		    //dst_y[dst_y_offset++] = y[*y_offset++] + mapping->y_fix_1;
		    value = y[*y_offset++] + mapping->y_fix_1;
		    dst_y[dst_y_offset++] = value > 255 ? 255 : ((value < 0) ? 0 : value);
		}
		else if (col >= mapping->overlap_2_col_offset.begin)
		{
		    //dst_y[dst_y_offset++] = y[*y_offset++] + mapping->y_fix_2;
		    value = y[*y_offset++] + mapping->y_fix_2;
		    dst_y[dst_y_offset++] = value > 255 ? 255 : ((value < 0) ? 0 : value);
		}
		else
		{
		    //dst_y[dst_y_offset++] = y[*y_offset++] + mapping->y_fix_1 + (int)y_step;
		    value = y[*y_offset++] + mapping->y_fix_1 + (int)y_step;
		    dst_y[dst_y_offset++] = value > 255 ? 255 : ((value < 0) ? 0 : value);
		    y_step += mapping->y_fix_step;
		}
		col++;
	    } while (dst_y_offset < dst_y_offset_end);
	}
    } while (++row_y < row_y_end);

    //UV
    do
    {
	uv_offset = (int*)mapping->uv_col_offset[row_uv].begin;
	dst_uv_offset = mapping->dst_uv_col_offset[row_uv].begin;
	dst_uv_offset_end = mapping->dst_uv_col_offset[row_uv].end;

	if (dst_uv_offset < dst_uv_offset_end)
	{
	    col = mapping->col_offset[row_uv << 1].begin >> 1;
	    u_step = 0.0;
	    v_step = 0.0;

	    do
	    {
		if (col < col_uv_1)
		{
		    uv_src = (uint8_t*)(&uv[*uv_offset++]);
		    uv_dst = (uint8_t*)(&dst_uv[dst_uv_offset++]);
		    uv_dst[0] = uv_src[0] + mapping->u_fix_1;
		    uv_dst[1] = uv_src[1] + mapping->v_fix_1;
		}
		else if (col >= col_uv_2)
		{
		    uv_src = (uint8_t*)(&uv[*uv_offset++]);
		    uv_dst = (uint8_t*)(&dst_uv[dst_uv_offset++]);
		    uv_dst[0] = uv_src[0] + mapping->u_fix_2;
		    uv_dst[1] = uv_src[1] + mapping->v_fix_2;
		}
		else
		{
		    uv_src = (uint8_t*)(&uv[*uv_offset++]);
		    uv_dst = (uint8_t*)(&dst_uv[dst_uv_offset++]);
		    uv_dst[0] = uv_src[0] + mapping->u_fix_1 + (int)u_step;
		    uv_dst[1] = uv_src[1] + mapping->v_fix_1 + (int)v_step;

		    u_step += mapping->u_fix_step;
		    v_step += mapping->v_fix_step;
		}
		col++;

	    } while (dst_uv_offset < dst_uv_offset_end);
	}
    } while (++row_uv < row_uv_end);
}

static void calcYfixBaseFront(uint8_t* front_src, uint8_t* rear_src, uint8_t* left_src, uint8_t* right_src)
{
    struct camera_mapping *front = &s_camera.front;
    struct camera_mapping *rear = &s_camera.rear;
    struct camera_mapping *left = &s_camera.left;
    struct camera_mapping *right = &s_camera.right;
    
    uint8_t *front_y = front_src + s_y_offset;
    uint8_t *rear_y = rear_src + s_y_offset;
    uint8_t *left_y = left_src + s_y_offset;
    uint8_t *right_y = right_src + s_y_offset;
    
    int front_sum_1 = 0;
    int front_sum_2 = 0;
    int rear_sum_1 = 0;
    int rear_sum_2 = 0;
    int left_sum_1 = 0;
    int left_sum_2 = 0;
    int right_sum_1 = 0;
    int right_sum_2 = 0;
    int *y_offset = NULL;
    int *y_offset_end = NULL;
    int row = 0;
    
    front->y_fix_1 = front->y_fix_2 = 0;
    front->y_fix_step = 0;

    //front_left
    row = 0;
    do
    {
	y_offset = (int*)front->overlap_1_y_col_offset[row].begin;
	y_offset_end = (int*)front->overlap_1_y_col_offset[row].end;
	
	do
	{
	    front_sum_1 += front_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    row = 0;
    do
    {
	y_offset = (int*)left->overlap_1_y_col_offset[row].begin;
	y_offset_end = (int*)left->overlap_1_y_col_offset[row].end;
	
	do
	{
	    left_sum_1 += left_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    left->y_fix_1 = left->y_fix_2 = (front_sum_1 - left_sum_1) >> OVERLAP_Y_SHIFT; 
    left->y_fix_step = 0;
    
    //front_right
    row = 0;
    do
    {
	y_offset = (int*)front->overlap_2_y_col_offset[row].begin;
	y_offset_end = (int*)front->overlap_2_y_col_offset[row].end;
	
	do
	{
	    front_sum_2 += front_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    row = 0;
    do
    {
	y_offset = (int*)right->overlap_1_y_col_offset[row].begin;
	y_offset_end = (int*)right->overlap_1_y_col_offset[row].end;
	
	do
	{
	    right_sum_1 += right_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    right->y_fix_1 = right->y_fix_2 = (front_sum_2 - right_sum_1) >> OVERLAP_Y_SHIFT; 
    
    //rear_left
    row = 0;
    do
    {
	y_offset = (int*)rear->overlap_1_y_col_offset[row].begin;
	y_offset_end = (int*)rear->overlap_1_y_col_offset[row].end;
	
	do
	{
	    rear_sum_1 += rear_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    row = 0;
    do
    {
	y_offset = (int*)left->overlap_2_y_col_offset[row].begin;
	y_offset_end = (int*)left->overlap_2_y_col_offset[row].end;

	do
	{
	    left_sum_2 += left_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    rear->y_fix_1 = (left_sum_2 + (left->y_fix_1 << OVERLAP_Y_SHIFT) - rear_sum_1) >> OVERLAP_Y_SHIFT;
    
    //rear_right 
    row = 0;
    do
    {
	y_offset = (int*)rear->overlap_2_y_col_offset[row].begin;
	y_offset_end = (int*)rear->overlap_2_y_col_offset[row].end;
	
	do
	{
	    rear_sum_2 += rear_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    row = 0;
    do
    {
	y_offset = (int*)right->overlap_2_y_col_offset[row].begin;
	y_offset_end = (int*)right->overlap_2_y_col_offset[row].end;

	do
	{
	    right_sum_2 += right_y[*y_offset++];
	} while (y_offset < y_offset_end);
    } while (++row < OVERLAP_Y_SIDE);
    
    rear->y_fix_2 = (right_sum_2 + (right->y_fix_1 << OVERLAP_Y_SHIFT) - rear_sum_2) >> OVERLAP_Y_SHIFT;
    rear->y_fix_step = 1.0 * (rear->y_fix_2 - rear->y_fix_1) / rear->y_fix_step_divisor; 
}

static void calcYfixBaseRear(uint8_t* front_src, uint8_t* rear_src, uint8_t* left_src, uint8_t* right_src)
{
    struct camera_mapping *front = &s_camera.front;
    struct camera_mapping *rear = &s_camera.rear;
    struct camera_mapping *left = &s_camera.left;
    struct camera_mapping *right = &s_camera.right;
    
    uint8_t *front_y = front_src + s_y_offset;
    uint8_t *rear_y = rear_src + s_y_offset;
    uint8_t *left_y = left_src + s_y_offset;
    uint8_t *right_y = right_src + s_y_offset;
    
    int front_sum_1 = 0;
    int front_sum_2 = 0;
    int rear_sum_1 = 0;
    int rear_sum_2 = 0;
    int left_sum_1 = 0;
    int left_sum_2 = 0;
    int right_sum_1 = 0;
    int right_sum_2 = 0;
    int *y_offset = NULL;
    int *y_offset_end = NULL;
    int row = 0;
    
    rear->y_fix_1 = rear->y_fix_2 = 0;
    rear->y_fix_step = 0;

    //rear_left
    if (s_control.value[CONTROL_LEFT_DOOR_OPEN] == 0)
    {
	row = 0;
	do
	{
	    y_offset = (int*)rear->overlap_1_y_col_offset[row].begin;
	    y_offset_end = (int*)rear->overlap_1_y_col_offset[row].end;
	    
	    do
	    {
		rear_sum_1 += rear_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	row = 0;
	do
	{
	    y_offset = (int*)left->overlap_2_y_col_offset[row].begin;
	    y_offset_end = (int*)left->overlap_2_y_col_offset[row].end;
	    
	    do
	    {
		left_sum_2 += left_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	left->y_fix_1 = left->y_fix_2 = (rear_sum_1 - left_sum_2) >> OVERLAP_Y_SHIFT; 
	left->y_fix_step = 0;
    }
    
    //rear_right
    if (s_control.value[CONTROL_RIGHT_DOOR_OPEN] == 0)
    {
	row = 0;
	do
	{
	    y_offset = (int*)rear->overlap_2_y_col_offset[row].begin;
	    y_offset_end = (int*)rear->overlap_2_y_col_offset[row].end;
	    
	    do
	    {
		rear_sum_2 += rear_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	row = 0;
	do
	{
	    y_offset = (int*)right->overlap_2_y_col_offset[row].begin;
	    y_offset_end = (int*)right->overlap_2_y_col_offset[row].end;
	    
	    do
	    {
		right_sum_2 += right_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	right->y_fix_1 = right->y_fix_2 = (rear_sum_2 - right_sum_2) >> OVERLAP_Y_SHIFT; 
    }
    
    //front_left
    if (s_control.value[CONTROL_LEFT_DOOR_OPEN] == 0)
    {
	row = 0;
	do
	{
	    y_offset = (int*)front->overlap_1_y_col_offset[row].begin;
	    y_offset_end = (int*)front->overlap_1_y_col_offset[row].end;
	    
	    do
	    {
		front_sum_1 += front_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	row = 0;
	do
	{
	    y_offset = (int*)left->overlap_1_y_col_offset[row].begin;
	    y_offset_end = (int*)left->overlap_1_y_col_offset[row].end;

	    do
	    {
		left_sum_1 += left_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	front->y_fix_1 = (left_sum_1 + (left->y_fix_2 << OVERLAP_Y_SHIFT) - front_sum_1) >> OVERLAP_Y_SHIFT;
    }

    //front_right 
    if (s_control.value[CONTROL_RIGHT_DOOR_OPEN] == 0)
    {
	row = 0;
	do
	{
	    y_offset = (int*)front->overlap_2_y_col_offset[row].begin;
	    y_offset_end = (int*)front->overlap_2_y_col_offset[row].end;
	    
	    do
	    {
		front_sum_2 += front_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	row = 0;
	do
	{
	    y_offset = (int*)right->overlap_1_y_col_offset[row].begin;
	    y_offset_end = (int*)right->overlap_1_y_col_offset[row].end;

	    do
	    {
		right_sum_1 += right_y[*y_offset++];
	    } while (y_offset < y_offset_end);
	} while (++row < OVERLAP_Y_SIDE);
	
	front->y_fix_2 = (right_sum_1 + (right->y_fix_2 << OVERLAP_Y_SHIFT) - front_sum_2) >> OVERLAP_Y_SHIFT;
    }

    front->y_fix_step = 1.0 * (front->y_fix_2 - front->y_fix_1) / front->y_fix_step_divisor; 
}

static void calcUVfixBaseFront(uint8_t* front_src, uint8_t* rear_src, uint8_t* left_src, uint8_t* right_src)
{
    struct camera_mapping *front = &s_camera.front;
    struct camera_mapping *rear = &s_camera.rear;
    struct camera_mapping *left = &s_camera.left;
    struct camera_mapping *right = &s_camera.right;
    
    uint16_t *front_uv = (uint16_t*)(front_src + s_uv_offset);
    uint16_t *rear_uv = (uint16_t*)(rear_src + s_uv_offset);
    uint16_t *left_uv = (uint16_t*)(left_src + s_uv_offset);
    uint16_t *right_uv = (uint16_t*)(right_src + s_uv_offset);
    
    int front_u_sum_1 = 0;
    int front_u_sum_2 = 0;
    int rear_u_sum_1 = 0;
    int rear_u_sum_2 = 0;
    int left_u_sum_1 = 0;
    int left_u_sum_2 = 0;
    int right_u_sum_1 = 0;
    int right_u_sum_2 = 0;
    int front_v_sum_1 = 0;
    int front_v_sum_2 = 0;
    int rear_v_sum_1 = 0;
    int rear_v_sum_2 = 0;
    int left_v_sum_1 = 0;
    int left_v_sum_2 = 0;
    int right_v_sum_1 = 0;
    int right_v_sum_2 = 0;
    
    int *uv_offset = NULL;
    int *uv_offset_end = NULL;
    uint8_t *uv = NULL;
    int row = 0;
   
    front->u_fix_1 = front->u_fix_2 = 0;
    front->u_fix_step = 0;
    front->v_fix_1 = front->v_fix_2 = 0;
    front->v_fix_step = 0;
    
    //front_left
    row = 0;
    do
    {
	uv_offset = (int*)front->overlap_1_uv_col_offset[row].begin;
	uv_offset_end = (int*)front->overlap_1_uv_col_offset[row].end;
	
	do
	{
	    uv = (uint8_t*)(&front_uv[*uv_offset++]);
            front_u_sum_1 += *uv;
	    front_v_sum_1 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    row = 0;
    do
    {
	uv_offset = (int*)left->overlap_1_uv_col_offset[row].begin;
	uv_offset_end = (int*)left->overlap_1_uv_col_offset[row].end;
	
	do
	{
	    uv = (uint8_t*)(&left_uv[*uv_offset++]);
	    left_u_sum_1 += *uv;
	    left_v_sum_1 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    left->u_fix_1 = left->u_fix_2 = (front_u_sum_1 - left_u_sum_1) >> OVERLAP_UV_SHIFT;
    left->u_fix_step = 0;
    left->v_fix_1 = left->v_fix_2 = (front_v_sum_1 - left_v_sum_1) >> OVERLAP_UV_SHIFT;
    left->v_fix_step = 0;
    
    //front_right
    row = 0;
    do
    {
	uv_offset = (int*)front->overlap_2_uv_col_offset[row].begin;
	uv_offset_end = (int*)front->overlap_2_uv_col_offset[row].end;
	
	do
	{
	    uv = (uint8_t*)(&front_uv[*uv_offset++]);
            front_u_sum_2 += *uv;
	    front_v_sum_2 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    row = 0;
    do
    {
	uv_offset = (int*)right->overlap_1_uv_col_offset[row].begin;
	uv_offset_end = (int*)right->overlap_1_uv_col_offset[row].end;
	
	do
	{
	    uv = (uint8_t*)(&right_uv[*uv_offset++]);
	    right_u_sum_1 += *uv;
	    right_v_sum_1 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    right->u_fix_1 = right->u_fix_2 = (front_u_sum_2 - right_u_sum_1) >> OVERLAP_UV_SHIFT;
    right->u_fix_step = 0;
    right->v_fix_1 = right->v_fix_2 = (front_v_sum_2 - right_v_sum_1) >> OVERLAP_UV_SHIFT;
    right->v_fix_step = 0;

    //rear_left
    row = 0;
    do
    {
	uv_offset = (int*)rear->overlap_1_uv_col_offset[row].begin;
	uv_offset_end = (int*)rear->overlap_1_uv_col_offset[row].end;
	
	do
	{
	    uv = (uint8_t*)(&rear_uv[*uv_offset++]);
	    rear_u_sum_1 += *uv;
	    rear_v_sum_1 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    row = 0;
    do
    {
	uv_offset = (int*)left->overlap_2_uv_col_offset[row].begin;
	uv_offset_end = (int*)left->overlap_2_uv_col_offset[row].end;

	do
	{
	    uv = (uint8_t*)(&left_uv[*uv_offset++]);
	    left_u_sum_2 += *uv;
	    left_v_sum_2 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    rear->u_fix_1 = (left_u_sum_2 + (left->u_fix_1 << OVERLAP_UV_SHIFT) - rear_u_sum_1) >> OVERLAP_UV_SHIFT;
    rear->v_fix_1 = (left_v_sum_2 + (left->v_fix_1 << OVERLAP_UV_SHIFT) - rear_v_sum_1) >> OVERLAP_UV_SHIFT;
  
    //rear_right
    row = 0;
    do
    {
	uv_offset = (int*)rear->overlap_2_uv_col_offset[row].begin;
	uv_offset_end = (int*)rear->overlap_2_uv_col_offset[row].end;
	
	do
	{
	    uv = (uint8_t*)(&rear_uv[*uv_offset++]);
	    rear_u_sum_2 += *uv;
	    rear_v_sum_2 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    row = 0;
    do
    {
	uv_offset = (int*)right->overlap_2_uv_col_offset[row].begin;
	uv_offset_end = (int*)right->overlap_2_uv_col_offset[row].end;

	do
	{
	    uv = (uint8_t*)(&right_uv[*uv_offset++]);
	    right_u_sum_2 += *uv;
	    right_v_sum_2 += *(uv + 1);
	} while (uv_offset < uv_offset_end);
    } while (++row < OVERLAP_UV_SIDE);
    
    rear->u_fix_2 = (right_u_sum_2 + (right->u_fix_1 << OVERLAP_UV_SHIFT) - rear_u_sum_2) >> OVERLAP_UV_SHIFT;
    rear->u_fix_step = 1.0 * (rear->u_fix_2 - rear->u_fix_1) / rear->u_fix_step_divisor;
    rear->v_fix_2 = (right_v_sum_2 + (right->v_fix_1 << OVERLAP_UV_SHIFT) - rear_v_sum_2) >> OVERLAP_UV_SHIFT;
    rear->v_fix_step = 1.0 * (rear->v_fix_2 - rear->v_fix_1) / rear->v_fix_step_divisor;
}

static void calcUVfixBaseRear(uint8_t* front_src, uint8_t* rear_src, uint8_t* left_src, uint8_t* right_src)
{
    struct camera_mapping *front = &s_camera.front;
    struct camera_mapping *rear = &s_camera.rear;
    struct camera_mapping *left = &s_camera.left;
    struct camera_mapping *right = &s_camera.right;
    
    uint16_t *front_uv = (uint16_t*)(front_src + s_uv_offset);
    uint16_t *rear_uv = (uint16_t*)(rear_src + s_uv_offset);
    uint16_t *left_uv = (uint16_t*)(left_src + s_uv_offset);
    uint16_t *right_uv = (uint16_t*)(right_src + s_uv_offset);
    
    int front_u_sum_1 = 0;
    int front_u_sum_2 = 0;
    int rear_u_sum_1 = 0;
    int rear_u_sum_2 = 0;
    int left_u_sum_1 = 0;
    int left_u_sum_2 = 0;
    int right_u_sum_1 = 0;
    int right_u_sum_2 = 0;
    int front_v_sum_1 = 0;
    int front_v_sum_2 = 0;
    int rear_v_sum_1 = 0;
    int rear_v_sum_2 = 0;
    int left_v_sum_1 = 0;
    int left_v_sum_2 = 0;
    int right_v_sum_1 = 0;
    int right_v_sum_2 = 0;
    
    int *uv_offset = NULL;
    int *uv_offset_end = NULL;
    uint8_t *uv = NULL;
    int row = 0;
   
    rear->u_fix_1 = rear->u_fix_2 = 0;
    rear->u_fix_step = 0;
    rear->v_fix_1 = rear->v_fix_2 = 0;
    rear->v_fix_step = 0;
    
    //rear_left
    if (s_control.value[CONTROL_LEFT_DOOR_OPEN])
    {
	row = 0;
	do
	{
	    uv_offset = (int*)rear->overlap_1_uv_col_offset[row].begin;
	    uv_offset_end = (int*)rear->overlap_1_uv_col_offset[row].end;
	    
	    do
	    {
		uv = (uint8_t*)(&rear_uv[*uv_offset++]);
		rear_u_sum_1 += *uv;
		rear_v_sum_1 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
	
	row = 0;
	do
	{
	    uv_offset = (int*)left->overlap_2_uv_col_offset[row].begin;
	    uv_offset_end = (int*)left->overlap_2_uv_col_offset[row].end;
	    
	    do
	    {
		uv = (uint8_t*)(&left_uv[*uv_offset++]);
		left_u_sum_2 += *uv;
		left_v_sum_2 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
	
	left->u_fix_1 = left->u_fix_2 = (rear_u_sum_1 - left_u_sum_2) >> OVERLAP_UV_SHIFT;
	left->u_fix_step = 0;
	left->v_fix_1 = left->v_fix_2 = (rear_v_sum_1 - left_v_sum_2) >> OVERLAP_UV_SHIFT;
	left->v_fix_step = 0;
    }
    
    //rear_right
    if (s_control.value[CONTROL_RIGHT_DOOR_OPEN] == 0)
    {
	row = 0;
	do
	{
	    uv_offset = (int*)rear->overlap_2_uv_col_offset[row].begin;
	    uv_offset_end = (int*)rear->overlap_2_uv_col_offset[row].end;
	    
	    do
	    {
		uv = (uint8_t*)(&rear_uv[*uv_offset++]);
		rear_u_sum_2 += *uv;
		rear_v_sum_2 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
	
	row = 0;
	do
	{
	    uv_offset = (int*)right->overlap_2_uv_col_offset[row].begin;
	    uv_offset_end = (int*)right->overlap_2_uv_col_offset[row].end;
	    
	    do
	    {
		uv = (uint8_t*)(&right_uv[*uv_offset++]);
		right_u_sum_2 += *uv;
		right_v_sum_2 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
	
	right->u_fix_1 = right->u_fix_2 = (rear_u_sum_2 - right_u_sum_2) >> OVERLAP_UV_SHIFT;
	right->u_fix_step = 0;
	right->v_fix_1 = right->v_fix_2 = (rear_v_sum_2 - right_v_sum_2) >> OVERLAP_UV_SHIFT;
	right->v_fix_step = 0;
    }

    //front_left
    if (s_control.value[CONTROL_LEFT_DOOR_OPEN] == 0)
    {
	row = 0;
	do
	{
	    uv_offset = (int*)front->overlap_1_uv_col_offset[row].begin;
	    uv_offset_end = (int*)front->overlap_1_uv_col_offset[row].end;
	    
	    do
	    {
		uv = (uint8_t*)(&front_uv[*uv_offset++]);
		front_u_sum_1 += *uv;
		front_v_sum_1 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
	
	row = 0;
	do
	{
	    uv_offset = (int*)left->overlap_1_uv_col_offset[row].begin;
	    uv_offset_end = (int*)left->overlap_1_uv_col_offset[row].end;

	    do
	    {
		uv = (uint8_t*)(&left_uv[*uv_offset++]);
		left_u_sum_1 += *uv;
		left_v_sum_1 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
	
	front->u_fix_1 = (left_u_sum_1 + (left->u_fix_2 << OVERLAP_UV_SHIFT) - front_u_sum_1) >> OVERLAP_UV_SHIFT;
	front->v_fix_1 = (left_v_sum_1 + (left->v_fix_2 << OVERLAP_UV_SHIFT) - front_v_sum_1) >> OVERLAP_UV_SHIFT;
    }

    //front_right
    if (s_control.value[CONTROL_RIGHT_DOOR_OPEN] == 0)
    {
	row = 0;
	do
	{
	    uv_offset = (int*)front->overlap_2_uv_col_offset[row].begin;
	    uv_offset_end = (int*)front->overlap_2_uv_col_offset[row].end;
	    
	    do
	    {
		uv = (uint8_t*)(&front_uv[*uv_offset++]);
		front_u_sum_2 += *uv;
		front_v_sum_2 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
	
	row = 0;
	do
	{
	    uv_offset = (int*)right->overlap_1_uv_col_offset[row].begin;
	    uv_offset_end = (int*)right->overlap_1_uv_col_offset[row].end;

	    do
	    {
		uv = (uint8_t*)(&right_uv[*uv_offset++]);
		right_u_sum_1 += *uv;
		right_v_sum_1 += *(uv + 1);
	    } while (uv_offset < uv_offset_end);
	} while (++row < OVERLAP_UV_SIDE);
    }
    
    front->u_fix_2 = (right_u_sum_1 + (right->u_fix_2 << OVERLAP_UV_SHIFT) - front_u_sum_2) >> OVERLAP_UV_SHIFT;
    front->u_fix_step = 1.0 * (front->u_fix_2 - front->u_fix_1) / front->u_fix_step_divisor;
    front->v_fix_2 = (right_v_sum_1 + (right->v_fix_2 << OVERLAP_UV_SHIFT) - front_v_sum_2) >> OVERLAP_UV_SHIFT;
    front->v_fix_step = 1.0 * (front->v_fix_2 - front->v_fix_1) / front->v_fix_step_divisor;
}

static int valid_all_data(uint8_t* front_src, uint8_t* rear_src, uint8_t* left_src, uint8_t* right_src)
{
    uint8_t *front_y = front_src + s_y_offset;
    uint8_t *rear_y = rear_src + s_y_offset;
    uint8_t *left_y = left_src + s_y_offset;
    uint8_t *right_y = right_src + s_y_offset;
    
    uint16_t *front_uv = (uint16_t*)(front_src + s_uv_offset);
    uint16_t *rear_uv = (uint16_t*)(rear_src + s_uv_offset);
    uint16_t *left_uv = (uint16_t*)(left_src + s_uv_offset);
    uint16_t *right_uv = (uint16_t*)(right_src + s_uv_offset);
    
    if ((*front_y == 0 && *front_uv == 0) 
        || (*rear_y == 0 && *rear_uv == 0)
        || (*left_y == 0 && *left_uv == 0)
        || (*right_y == 0 && *right_uv == 0))
    {
        return 0;
    }

    return 1;
}

int initStitching(uint32_t in_stride, uint32_t y_offset, uint32_t uv_offset,
	uint32_t in_width, uint32_t in_height)
{
    int data_fd = -1;
    int car_fd = -1;
    int undistorted_data_fd = -1;
    struct stat statbuf;
    int file_len = 0;
    char path[PATH_MAX + 1] = {0};
    char exefile[PATH_MAX] = {0};
    char carfile[PATH_MAX] = {0};
    char undistortedfile[PATH_MAX] = {0};
    int i = 0;
    int ret_val = 0;
    int len = 0;

    s_in_stride = in_stride;
    s_y_offset = y_offset;
    s_uv_offset = uv_offset;
    s_in_width = in_width;
    s_in_height = in_height;
    
    printf("\nstitchalgo lib build on %s\n", __DATE__);
    
    black_data = malloc(in_width * in_height * 3 / 2);
    memset(black_data, 0, in_width * in_height);
    memset(black_data + in_width * in_height, 128, in_width * in_height / 2);
    
#ifdef __QNXNTO__
    sprintf(exefile, "/proc/self/exefile");
    int exe_fd = open(exefile, O_RDONLY);
    len = read(exe_fd, path,  PATH_MAX);
    close(exe_fd);
    path[len] = '\0';
#endif

#ifdef __linux__
    sprintf(exefile, "/proc/self/exe");
    len = readlink(exefile, path, PATH_MAX);
    path[len] = '\0';
#endif

    for (i = len; i > 0; i--)
    {
	if (path[i] != '/')
	{
	    path[i] = '\0';
	}
	else
	{
	    break;
	}
    }

    strcpy(s_dir, path);

    strcat(path, "camera.dat");
    
    sprintf(carfile, "%s/%s", s_dir, "car.dat");
    sprintf(undistortedfile, "%s/%s", s_dir, "camera_undistorted.dat");

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
   
    //parse
    memset(&s_camera, 0, sizeof(struct camera_data));
    if (parseCameraData(s_data, &s_camera) == -1)
    {
	ret_val = -1;
	goto out;
    }

    s_img_size = s_camera.width * s_camera.height;

    //calculate the output row offset
    dst_y_row_offset = (int*)malloc(sizeof(int) * s_camera.height);
    dst_uv_row_offset = (int*)malloc(sizeof(int) * (s_camera.height >> 1));
    if (!dst_y_row_offset || !dst_uv_row_offset)
    {
        ERROR("malloc error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }

    for (i = 0; i < s_camera.height; i++)
    {
	dst_y_row_offset[i] = i * s_camera.width;

	if (i % 2 == 0)
	{
	    dst_uv_row_offset[i >> 1] = (i >> 1) * (s_camera.width >> 1);
	}
    }

    //calculate the col offset
    if (calcColOffset() == -1)
    {
	ret_val = -1;
	goto out;
    }

    //set every pixel offset to buffer
    setMappingOffset(&s_camera.front);
    setMappingOffset(&s_camera.rear);
    setMappingOffset(&s_camera.left);
    setMappingOffset(&s_camera.right);
      
    //car
    car_fd = open(carfile, O_RDONLY);
    if (car_fd < 0)
    {
	ERROR("open error: %s--%s", strerror(errno), carfile);
	ret_val = -1;
	goto out;
    }

    if (stat(carfile, &statbuf) < 0)
    {
	ERROR("stat error: %s--%s", strerror(errno), carfile);
	ret_val = -1;
	goto out;
    }

    file_len = statbuf.st_size;
    s_car_data = (uint16_t*)malloc(file_len);
    if (!s_car_data)
    {
	ERROR("malloc error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }

    if (read(car_fd, s_car_data, file_len) != file_len)
    {
	ERROR("read error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }
   
    //parse
    memset(&s_car, 0, sizeof(struct car_mapping));
    if (parseCarData(s_car_data, &s_car) == -1)
    {
	ret_val = -1;
	goto out;
    }
    
    s_car.x = s_camera.left.col - s_car.width_offset;
    s_car.y = s_camera.front.row - s_car.height_offset;
    s_car.img_size = s_car.img_width * s_car.img_height;
    
    calcCarColOffset();
    setCarMappingOffset(&s_car);
    setCarRefillMappingOffset(&s_car);

    //undistorted data
    undistorted_data_fd = open(undistortedfile, O_RDONLY);
    if (car_fd < 0)
    {
	ERROR("open error: %s--%s", strerror(errno), carfile);
	ret_val = -1;
	goto out;
    }

    if (stat(undistortedfile, &statbuf) < 0)
    {
	ERROR("stat error: %s--%s", strerror(errno), carfile);
	ret_val = -1;
	goto out;
    }

    file_len = statbuf.st_size;
    s_undistorted_data = (uint16_t*)malloc(file_len);
    if (!s_car_data)
    {
	ERROR("malloc error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }

    if (read(undistorted_data_fd, s_undistorted_data, file_len) != file_len)
    {
	ERROR("read error: %s", strerror(errno));
	ret_val = -1;
	goto out;
    }
   
    memset(&s_camera_undistorted, 0, sizeof(struct camera_undistorted_data));
    if (parseCameraUndistortedData(s_undistorted_data, &s_camera_undistorted) == -1)
    {
	ret_val = -1;
	goto out;
    }

    setUndistortedMappingOffset(&s_camera_undistorted.front);
    setUndistortedMappingOffset(&s_camera_undistorted.rear);
    setUndistortedMappingOffset(&s_camera_undistorted.left_front);
    setUndistortedMappingOffset(&s_camera_undistorted.right_front);
    setUndistortedMappingOffset(&s_camera_undistorted.left_rear);
    setUndistortedMappingOffset(&s_camera_undistorted.right_rear);

out:
    if (data_fd != -1)
    {
	close(data_fd);
	data_fd = -1;
    }
    
    if (car_fd != -1)
    {
	close(car_fd);
	car_fd = -1;
    }

    if (undistorted_data_fd != -1)
    {
	close(undistorted_data_fd);
	undistorted_data_fd = -1;
    }

    DEBUG("initStitching OK");

    return ret_val;
}

int getStitchingSize(uint32_t *out_width, uint32_t *out_height)
{
    *out_width = s_camera.width;
    *out_height = s_camera.height;
    return 0;
}

int doStitching(YUVBuffer *front, YUVBuffer* rear, YUVBuffer *left, YUVBuffer *right, YUVBuffer *output)
{
    if (front == 0 || front->bufYUV == 0)
    {
	DEBUG("front addr is bad");
	return -1;
    }

    if (rear == 0 || rear->bufYUV == 0)
    {
	DEBUG("rear addr is bad");
	return -1;
    }

    if (left == 0 || left->bufYUV == 0)
    {
	DEBUG("left addr is bad");
	return -1;
    }

    if (right == 0 || right->bufYUV == 0)
    {
	DEBUG("right addr is bad");
	return -1;
    }

    if (output == 0 || output->bufYUV == 0)
    {
	DEBUG("output addr is bad");
	return -1;
    }

    //fill the car image once
    if (!s_car_is_fill)
    {
	fillCarImage((uint8_t*)output->bufYUV);
    }

#if 0
    fillImage((uint8_t*)output->bufYUV, (uint8_t*)front->bufYUV, &s_camera.front, 0);
    fillImage((uint8_t*)output->bufYUV, (uint8_t*)rear->bufYUV, &s_camera.rear, 0);
    fillImage((uint8_t*)output->bufYUV, (uint8_t*)left->bufYUV, &s_camera.left, 0);
    fillImage((uint8_t*)output->bufYUV, (uint8_t*)right->bufYUV, &s_camera.right, 0);
#else
    if (valid_all_data((uint8_t*)front->bufYUV, (uint8_t*)rear->bufYUV, (uint8_t*)left->bufYUV, (uint8_t*)right->bufYUV) == 1)
    {
        calcYfixBaseRear((uint8_t*)front->bufYUV, (uint8_t*)rear->bufYUV, (uint8_t*)left->bufYUV, (uint8_t*)right->bufYUV);
        calcUVfixBaseRear((uint8_t*)front->bufYUV, (uint8_t*)rear->bufYUV, (uint8_t*)left->bufYUV, (uint8_t*)right->bufYUV);

        fillImageFixStep((uint8_t*)output->bufYUV, (uint8_t*)front->bufYUV, &s_camera.front);
        fillImageFix((uint8_t*)output->bufYUV, (uint8_t*)rear->bufYUV, &s_camera.rear);
	
	if (s_control.value[CONTROL_LEFT_DOOR_OPEN] == 0)
	{
            fillImageFix((uint8_t*)output->bufYUV, (uint8_t*)left->bufYUV, &s_camera.left);
	}
	else
	{
	    fillImage((uint8_t*)output->bufYUV, (uint8_t*)black_data, &s_camera.left, 1);
	    fillWithLeftDoorOpen((uint8_t*)output->bufYUV, (uint8_t*)front->bufYUV, (uint8_t*)rear->bufYUV);
	}

	if (s_control.value[CONTROL_RIGHT_DOOR_OPEN] == 0)
	{
            fillImageFix((uint8_t*)output->bufYUV, (uint8_t*)right->bufYUV, &s_camera.right);
	}
	else
	{
	    fillImage((uint8_t*)output->bufYUV, (uint8_t*)black_data, &s_camera.right, 1);
	    fillWithRightDoorOpen((uint8_t*)output->bufYUV, (uint8_t*)front->bufYUV, (uint8_t*)rear->bufYUV);
	}
    }
    else
    {
        fillImage((uint8_t*)output->bufYUV, (uint8_t*)front->bufYUV, &s_camera.front, 0);
        fillImage((uint8_t*)output->bufYUV, (uint8_t*)rear->bufYUV, &s_camera.rear, 0);
        fillImage((uint8_t*)output->bufYUV, (uint8_t*)left->bufYUV, &s_camera.left, 0);
        fillImage((uint8_t*)output->bufYUV, (uint8_t*)right->bufYUV, &s_camera.right, 0);
    }
#endif

    refillOverlapLine((uint8_t*)output->bufYUV, (uint8_t*)front->bufYUV, (uint8_t*)rear->bufYUV, (uint8_t*)left->bufYUV, (uint8_t*)right->bufYUV);

    refillCarImage((uint8_t*)output->bufYUV);

    return 0;
}
 
int deInitStitching(void)
{
    if (s_data)
    {
	    free(s_data);
	    s_data = NULL;
    }

    //@todo
    //free more memory

    return 0;
}

int controlStitching(ControlType type, int value)
{
    s_control.value[type] = value;

    return 0;
}

int undistorted_front(int view, YUVBuffer *front, YUVBuffer *output)
{
    int col_begin = 0;
    int col_end = 0;
    int row = 0;

    int r = 0;
    int c = 0;
    int offset = 0;

    uint8_t *src = (uint8_t*)front->bufYUV;
    uint8_t *dst = (uint8_t*)output->bufYUV;

    uint8_t *y = src + s_y_offset;
    uint16_t *uv = (uint16_t*)(src + s_uv_offset);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + (view == 0 ? s_camera_undistorted.size_normal : s_camera_undistorted.size_wide));

    if (view == 0) //normal view
    {
	col_begin = (s_camera_undistorted.width_wide - s_camera_undistorted.width_normal) >> 1;
	col_end = s_camera_undistorted.width_wide - col_begin;

	row = s_camera_undistorted.height_normal;
    }
    else if (view == 1) //wide view
    {
	col_begin = 0;
	col_end = s_camera_undistorted.width_wide;

	row = s_camera_undistorted.height_wide;
    }

    for (r = 0; r < row; r++)
    {
	for (c = col_begin; c < col_end; c++)
	{
	    offset = s_camera_undistorted.front.y_offset[r][c];
	    *dst_y++ = y[offset];

	    if ((r % 2 == 0) && (c % 2 == 0))
	    {
		offset = s_camera_undistorted.front.uv_offset[r >> 1][c >> 1];
		*dst_uv++ = uv[offset];
	    }
	}
    }

    return 0;
}

int undistorted_rear(YUVBuffer *rear, YUVBuffer *output)
{
    int col_begin = 0;
    int col_end = 0;
    int row = 0;

    int r = 0;
    int c = 0;
    int offset = 0;

    uint8_t *src = (uint8_t*)rear->bufYUV;
    uint8_t *dst = (uint8_t*)output->bufYUV;

    uint8_t *y = src + s_y_offset;
    uint16_t *uv = (uint16_t*)(src + s_uv_offset);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_camera_undistorted.size_normal);

    col_begin = 0;
    col_end = 800;//s_camera_undistorted.width_normal;
    row = 560;//s_camera_undistorted.height_normal;

    for (r = 0; r < row; r++)
    {
	for (c = col_begin; c < col_end; c++)
	{
	    offset = s_camera_undistorted.rear.y_offset[r][c];
	    *dst_y++ = y[offset];

	    if ((r % 2 == 0) && (c % 2 == 0))
	    {
		offset = s_camera_undistorted.rear.uv_offset[r >> 1][c >> 1];
		*dst_uv++ = uv[offset];
	    }
	}
    }

    return 0;
}

int undistorted_left_and_right(int view, YUVBuffer *left, YUVBuffer *right, YUVBuffer *output)
{
    int col_begin = 0;
    int col_end = 0;
    int row = 0;

    int r = 0;
    int c = 0;
    int c_diff = 0;
    int offset = 0;

    uint8_t *src_left = (uint8_t*)left->bufYUV;
    uint8_t *src_right = (uint8_t*)right->bufYUV;
    uint8_t *dst = (uint8_t*)output->bufYUV;

    uint8_t *y_left = src_left + s_y_offset;
    uint8_t *y_right = src_right + s_y_offset;
    uint16_t *uv_left = (uint16_t*)(src_left + s_uv_offset);
    uint16_t *uv_right = (uint16_t*)(src_right + s_uv_offset);

    uint8_t *dst_y = dst;
    uint16_t *dst_uv = (uint16_t*)(dst + s_camera_undistorted.size_normal);
   
    col_begin = 0;
    col_end = s_camera_undistorted.width_normal;
    row = s_camera_undistorted.height_normal;

    if (view == 0)
    {
	for (r = 0; r < row; r++)
	{
	    for (c = col_begin; c < s_camera_undistorted.left_front.col; c++)
	    {
		offset = s_camera_undistorted.left_front.y_offset[r][c];
		*dst_y++ = y_left[offset];

		if ((r % 2 == 0) && (c % 2 == 0))
		{
		    offset = s_camera_undistorted.left_front.uv_offset[r >> 1][c >> 1];
		    *dst_uv++ = uv_left[offset];
		}
	    }

	    for (c = 0; c < s_camera_undistorted.left_right_pad; c++)
	    {
		*dst_y++ = 0;

		if ((r % 2 == 0) && (c % 2 == 0))
		{
		    *dst_uv++ = 0x8080;
		}
	    }

            for (c = 0; c < s_camera_undistorted.right_front.col; c++)
	    {
		offset = s_camera_undistorted.right_front.y_offset[r][c];
		*dst_y++ = y_right[offset];

		if ((r % 2 == 0) && (c % 2 == 0))
		{
		    offset = s_camera_undistorted.right_front.uv_offset[r >> 1][c >> 1];
		    *dst_uv++ = uv_right[offset];
		}
	    }
	}
    }
    else if (view == 1)
    {
        for (r = 0; r < row; r++)
	{
            for (c = col_begin; c < s_camera_undistorted.left_rear.col; c++)
	    {
		offset = s_camera_undistorted.left_rear.y_offset[r][c];
		*dst_y++ = y_left[offset];

		if ((r % 2 == 0) && (c % 2 == 0))
		{
		    offset = s_camera_undistorted.left_rear.uv_offset[r >> 1][c >> 1];
		    *dst_uv++ = uv_left[offset];
		}
	    }
            
	    for (c = 0; c < s_camera_undistorted.left_right_pad; c++)
	    {
		*dst_y++ = 0;

		if ((r % 2 == 0) && (c % 2 == 0))
		{
		    *dst_uv++ = 0x8080;
		}
	    }

            for (c = 0; c < s_camera_undistorted.right_rear.col; c++)
	    {
		offset = s_camera_undistorted.right_rear.y_offset[r][c];
		*dst_y++ = y_right[offset];

		if ((r % 2 == 0) && (c % 2 == 0))
		{
		    offset = s_camera_undistorted.right_rear.uv_offset[r >> 1][c >> 1];
		    *dst_uv++ = uv_right[offset];
		}
	    }
	}
    }
    return 0;
}
