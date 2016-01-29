#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#include "stitchalgo.h"

#define IMG_WIDTH 1280
#define IMG_HEIGHT 800

struct camera_mapping
{
    Uint16 row;
    Uint16 col;
    Uint16 *data;
    int x; //offset x
    int y; //offset y
    
    int **yuv_offset;
    int **uv_offset;

    Uint16 row_begin;
    Uint16 row_end;
    Uint16 col_begin;
    Uint16 col_end;
};

struct camera_data
{
    Uint16 square_size;
    struct camera_mapping front;
    struct camera_mapping rear;
    struct camera_mapping left;
    struct camera_mapping right;
};

enum corner_type
{
    front_right,
    front_left,
    rear_right,
    rear_left
};

struct corner_data
{
    float rate;
    float rate_uv;
    float match_begin;
    int y_offset;
    int uv_offset;
};

static Uint16 *data = 0;
static struct camera_data camera;
static int yuv_y_row_offset[IMG_HEIGHT];
static int yuv_uv_row_offset[IMG_HEIGHT];
static int yuv_uv_col[IMG_WIDTH];
static int img_size;
static struct corner_data corners[4];

void fillImage(Uint8 *dst, Uint8 *src, struct camera_mapping *mapping)
{
    int row_begin = mapping->row_begin;
    int row_end = mapping->row_end;
    int row_uv_begin = (mapping->row_begin >> 1) + mapping->row_begin % 2;
    int row_uv_end = (mapping->row_end >> 1) + mapping->row_end % 2;
    int r = 0;
    int col = mapping->col_end - mapping->col_begin;
    int space_y = IMG_WIDTH - col;
    int space_uv = space_y >> 1;
    
    Uint8 *dst_yuv = dst;
    Uint16 *dst_uv = (Uint16 *)(dst_yuv + img_size);
    
    Uint8 *src_yuv = src;
    Uint16 *src_uv = (Uint16 *)(src_yuv + img_size);
    
    Uint8 *row_dst_y = dst_yuv + yuv_y_row_offset[mapping->y] + mapping->x;
    Uint16 *row_dst_uv = dst_uv + yuv_uv_row_offset[mapping->y] + (mapping->x >> 1);

    int *row_src_y_offset = 0;
    int *row_src_uv_offset = 0;
    int *offset_end = 0;
    int col_begin =  mapping->col_begin;
    int col_uv_begin = mapping->col_begin >> 1;
    int col_uv = (col >> 1) + col % 2;
   
    r = row_begin;
    do
    {
	row_src_y_offset = mapping->yuv_offset[r] + col_begin;
	offset_end = row_src_y_offset + col;
	
	do
	{
	    *(row_dst_y++) = *(src_yuv + *(row_src_y_offset++));
        } while (row_src_y_offset < offset_end);

	row_dst_y += space_y;
        r++;
    } while (r < row_end);

    r = row_uv_begin;
    do
    {
	row_src_uv_offset = mapping->uv_offset[r] + col_uv_begin;
	offset_end = row_src_uv_offset + col_uv;
	
	do
	{
	    *(row_dst_uv++) = *(src_uv + *(row_src_uv_offset++));
        } while (row_src_uv_offset < offset_end);

	row_dst_uv += space_uv;
        r++;
    } while (r < row_uv_end);
}

void fillImageCorner(Uint8 *dst, Uint8 *src1, Uint8 *src2,
	struct camera_mapping *mapping1,
	struct camera_mapping *mapping2,
	enum corner_type type)
{
    Uint8 *dst_yuv = dst;
    Uint16 *dst_uv = (Uint16 *)(dst_yuv + img_size);
    
    Uint8 *src_yuv_1 = src1;
    Uint8 *src_yuv_2 = src2;
    Uint16 *src_uv_1 = (Uint16 *)(src_yuv_1 + img_size);
    Uint16 *src_uv_2 = (Uint16 *)(src_yuv_2 + img_size);
    
    Uint8 *row_dst_y = NULL;
    Uint16 *row_dst_uv = NULL;

    int *row_src_y_offset_1 = 0;
    int *row_src_y_offset_2 = 0;
    int *row_src_uv_offset_1 = 0;
    int *row_src_uv_offset_2 = 0;
    int *offset_end = 0;
    
    int space_y = IMG_WIDTH - (mapping1->col_end - mapping1->col_begin);
    int space_uv = space_y >> 1;
    int row = mapping2->row;
    int row_uv = (row >> 1) + row % 2;
    int col = mapping1->col;
    int col_uv = (col >> 1) + col % 2;
    int r1 = 0;
    int r2 = 0;
    int r1_begin = 0;
    int c = 0;
    struct corner_data *corner = &corners[type];
    float rate = corner->rate;
    float rate_uv = corner->rate_uv;
    float match = corner->match_begin;

    if (type == front_left || type == rear_left)
    {
	r1_begin = mapping1->row - row;
    }
 
    if (type == front_left || type == front_right)
    {
        row_dst_y = dst_yuv + corner->y_offset;
        row_dst_uv = dst_uv + corner->uv_offset;

	match = corner->match_begin;
        r1 = r1_begin;
	r2 = 0;
	
	do
	{
	    row_src_y_offset_1 = mapping1->yuv_offset[r1];
	    offset_end = mapping1->yuv_offset[r1] + (int)match;
	    do
	    {
		*(row_dst_y++) = *(src_yuv_1 + *(row_src_y_offset_1++));
	    } while(row_src_y_offset_1 < offset_end);

	    row_src_y_offset_2 = mapping2->yuv_offset[r2] + (int)match;
	    offset_end = mapping2->yuv_offset[r2] + col;
	    do
	    {
	        *(row_dst_y++) = *(src_yuv_2 + *(row_src_y_offset_2++));
	    } while (row_src_y_offset_2 < offset_end);

	    row_dst_y += space_y;

	    r1++;
	    r2++;
	    match += rate;
	} while (r2 < row);

	match = corner->match_begin;
	r1 = r1_begin>>1;
	r2 = 0;

	do
	{
	    row_src_uv_offset_1 = mapping1->uv_offset[r1];
	    offset_end = mapping1->uv_offset[r1] + (int)match / 2;	
	 
	    do
	    {
	        *(row_dst_uv++) = *(src_uv_1 + *(row_src_uv_offset_1++));
	    } while (row_src_uv_offset_1 < offset_end);
            
	    row_src_uv_offset_2 = mapping2->uv_offset[r2] + (int)match / 2;
	    offset_end = mapping2->uv_offset[r2] + col_uv;
	    do
	    {
		*(row_dst_uv++) = *(src_uv_2 + *(row_src_uv_offset_2++));
		c += 2;
	    } while (row_src_uv_offset_2 < offset_end);

	    row_dst_uv += space_uv;

	    r1++;
	    r2++;
	    match += rate_uv;
	} while (r2 < row_uv);
    }
    else
    {
	int col_begin = mapping1->x;
	int col_uv_begin = mapping1->x >> 1;
        row_dst_y = dst_yuv + corner->y_offset;
        row_dst_uv = dst_uv + corner->uv_offset;

	match = corner->match_begin;
	r1 = r1_begin;
	r2 = 0;
	
	do
	{
	    row_src_y_offset_2 = mapping2->yuv_offset[r2] + col_begin;
	    offset_end = mapping2->yuv_offset[r2] + col_begin + (int)match;
	    for (; row_src_y_offset_2 < offset_end; row_src_y_offset_2++)
	    {
		*(row_dst_y++) = *(src_yuv_2 + *row_src_y_offset_2);
	    }

	    row_src_y_offset_1 = mapping1->yuv_offset[r1] + (int)match;
	    offset_end = mapping1->yuv_offset[r1] + col;
	    for (; row_src_y_offset_1 < offset_end; row_src_y_offset_1++)
	    {
	        *(row_dst_y++) = *(src_yuv_1 + *row_src_y_offset_1);
	    }
	    
	    row_dst_y += space_y;

	    r1++;
	    r2++;
	    match += rate;
	} while (r2 < row);

        match = corner->match_begin;
	r1 = r1_begin >> 1;
	r2 = 0;

	do
	{
	    row_src_uv_offset_2 = mapping2->uv_offset[r2] + col_uv_begin;
	    offset_end = mapping2->uv_offset[r2] + col_uv_begin + (int)match / 2;    
	    
	    for (; row_src_uv_offset_2 < offset_end; row_src_uv_offset_2++)
	    {
	        *(row_dst_uv++) = *(src_uv_2 + *row_src_uv_offset_2);
	    }
            
	    row_src_uv_offset_1 = mapping1->uv_offset[r1] + (int)match / 2;
	    offset_end = mapping1->uv_offset[r1] + col_uv;
	    for (;row_src_uv_offset_1 < offset_end; row_src_uv_offset_1++)
	    {
		*(row_dst_uv++) = *(src_uv_1 + *row_src_uv_offset_1);
	    }

	    row_dst_uv += space_uv;

	    r1++;
	    r2++;
	    match += rate_uv;
	} while (r2 < row_uv);
    }
}

/*
拼接算法初始化接口
算法矫正文件路径为：/usr/local/stitchalgo/CALIB_FILENAME
in_width：摄像头输入图像宽度
in_hight：摄像头输出图像高度
out_width：拼接输出图像宽度
out_hight：拼接输出图像高度
return: >=0为初始化成功；<0为错误代码，具体由算法提供方定义
*/
int initStitching(Uint32 in_width, Uint32 in_hight, Uint32 out_width, Uint32 out_hight)
{
    int data_fd;
    struct stat statbuf;
    int file_len = 0;
    int index = 0;
    char path[256];
    int i = 0;
    int r, c, r_src, c_src;
    Uint16 *row_data = 0;
 
    //init camera data
    readlink("/proc/self/exe", path, 256);
    
    for (i = strlen(path) - 1; i > 0; i--)
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
    
    strcat(path, "camera.dat");

    data_fd = open(path, O_RDONLY);
    if (data_fd < 0)
    {
	printf("open error:%s--%s", strerror(errno), path);
	return -2;
    }
    
    if (stat(path, &statbuf) < 0)
    {
        printf("stat error:%s--%s", strerror(errno), path);
	return -1;
    }
    
    file_len = statbuf.st_size;
    data = (Uint16*)malloc(file_len);
    if (!data)
    {
        printf("malloc error:%s", strerror(errno));
	return -1;
    }

    if (read(data_fd, data, file_len) != file_len)
    {
	printf("read error:%s--%s", strerror(errno), path);
	return -1;
    }

    close(data_fd);
    
    camera.square_size = *(data + index);
    index++;
    
    camera.front.row = *(data + index);
    index++;
    camera.front.col = *(data + index);
    index++;
    camera.front.data = data + index;
    index += camera.front.row * camera.front.col * 2;
    
    camera.rear.row = *(data + index);
    index++;
    camera.rear.col = *(data + index);
    index++;
    camera.rear.data = data + index;
    index += camera.rear.row * camera.rear.col * 2;
        
    camera.left.row = *(data + index);
    index++;
    camera.left.col = *(data + index);
    index++;
    camera.left.data = data + index;
    index += camera.left.row * camera.left.col * 2;
    
    camera.right.row = *(data + index);
    index++;
    camera.right.col = *(data + index);
    index++;
    camera.right.data = data + index;
    index += camera.right.row * camera.right.col * 2;

    camera.front.x = 0;    
    camera.front.y = camera.right.row;
    camera.front.row_begin = camera.right.row;
    camera.front.row_end = camera.front.row - camera.left.row;
    camera.front.col_begin = 0;
    camera.front.col_end = camera.front.col;
    
    camera.rear.x = IMG_WIDTH - camera.rear.col;
    camera.rear.y = camera.right.row;
    camera.rear.row_begin = camera.right.row;
    camera.rear.row_end = camera.rear.row - camera.left.row;
    camera.rear.col_begin = 0;
    camera.rear.col_end = camera.rear.col;
								    
    camera.left.x = camera.front.col;
    camera.left.y = IMG_HEIGHT - camera.left.row;
    camera.left.row_begin = 0;
    camera.left.row_end = camera.left.row;
    camera.left.col_begin = camera.front.col;
    camera.left.col_end = camera.left.col - camera.rear.col;
    
    camera.right.x = camera.front.col;
    camera.right.y = 0;
    camera.right.row_begin = 0;
    camera.right.row_end = camera.right.row;
    camera.right.col_begin = camera.front.col;
    camera.right.col_end = camera.right.col - camera.rear.col;
    //end init camera data

    //init yuv offset
    for (i = 0; i < IMG_HEIGHT; i++)
    {
        yuv_y_row_offset[i] = i * IMG_WIDTH;
	yuv_uv_row_offset[i] = i / 2 * (IMG_WIDTH / 2);   
    }

    for (i = 0; i < IMG_WIDTH; i++)
    {
        yuv_uv_col[i] = i / 2;
    }

    img_size = IMG_WIDTH * IMG_HEIGHT;

    row_data = camera.front.data;
    camera.front.yuv_offset = (int**)malloc(camera.front.row*sizeof(int));
    camera.front.uv_offset = 
	(int**)malloc(((camera.front.row>>1)+camera.front.row%2)*sizeof(int));
    for (r = 0; r < camera.front.row; r++)
    {
	camera.front.yuv_offset[r] = (int*)malloc(camera.front.col*sizeof(int));
	if (r % 2 == 0)
	{
	    camera.front.uv_offset[r>>1] = 
		(int*)malloc(((camera.front.col>>1)+camera.front.col%2)*sizeof(int));
	}
	for (c = 0; c < camera.front.col; c++)
	{
	    c_src = *row_data;
	    r_src = *(row_data + 1);
	    row_data += 2;

	    camera.front.yuv_offset[r][c] = yuv_y_row_offset[r_src] + c_src;
	    if (r % 2 == 0 && c % 2 == 0)
	    {
	        camera.front.uv_offset[r>>1][c>>1] = yuv_uv_row_offset[r_src] 
		    + yuv_uv_col[c_src];
	    }
	}
    }

    row_data = camera.rear.data;
    camera.rear.yuv_offset = (int**)malloc(camera.rear.row*sizeof(int));
    camera.rear.uv_offset = (int**)malloc(((camera.rear.row>>1)+camera.rear.row%2)*sizeof(int));
    for (r = 0; r < camera.rear.row; r++)
    {
	camera.rear.yuv_offset[r] = (int*)malloc(camera.rear.col*sizeof(int));
	if (r % 2 == 0)
	{
	    camera.rear.uv_offset[r>>1] = 
		(int*)malloc(((camera.rear.col>>1)+camera.rear.col%2)*sizeof(int));
	}
	for (c = 0; c < camera.rear.col; c++)
	{
	    c_src = *row_data;
	    r_src = *(row_data + 1);
	    row_data += 2;

	    camera.rear.yuv_offset[r][c] = yuv_y_row_offset[r_src] + c_src;
	    if (r % 2 == 0 && c % 2 == 0)
	    {
	        camera.rear.uv_offset[r>>1][c>>1] = yuv_uv_row_offset[r_src] + yuv_uv_col[c_src];
	    }
	}
    }

    row_data = camera.left.data;
    camera.left.yuv_offset = (int**)malloc(camera.left.row*sizeof(int));
    camera.left.uv_offset = (int**)malloc(((camera.left.row>>1)+camera.left.row%2)*sizeof(int));
    for (r = 0; r < camera.left.row; r++)
    {
	camera.left.yuv_offset[r] = (int*)malloc(camera.left.col*sizeof(int));
	if (r % 2 == 0)
	{
	    camera.left.uv_offset[r>>1] = 
		(int*)malloc(((camera.left.col>>1)+camera.left.col%2)*sizeof(int));
	}
	for (c = 0; c < camera.left.col; c++)
	{
	    c_src = *row_data;
	    r_src = *(row_data + 1);
	    row_data += 2;

	    camera.left.yuv_offset[r][c] = yuv_y_row_offset[r_src] + c_src;
	    if (r % 2 == 0 && c % 2 == 0)
	    {
	        camera.left.uv_offset[r>>1][c>>1] = yuv_uv_row_offset[r_src] + yuv_uv_col[c_src];
	    }
	}
    }
    
    row_data = camera.right.data;
    camera.right.yuv_offset = (int**)malloc(camera.right.row*sizeof(int));
    camera.right.uv_offset = 
	(int**)malloc(((camera.right.row>>1)+camera.right.row%2)*sizeof(int));
    for (r = 0; r < camera.right.row; r++)
    {
	camera.right.yuv_offset[r] = (int*)malloc(camera.right.col*sizeof(int));
	if (r % 2 == 0)
	{
	    camera.right.uv_offset[r>>1] = 
		(int*)malloc(((camera.right.col>>1)+camera.right.col%2)*sizeof(int));
	}
	for (c = 0; c < camera.right.col; c++)
	{
	    c_src = *row_data;
	    r_src = *(row_data + 1);
	    row_data += 2;

	    camera.right.yuv_offset[r][c] = yuv_y_row_offset[r_src] + c_src;
	    if (r % 2 == 0 && c % 2 == 0)
	    {
	        camera.right.uv_offset[r>>1][c>>1] = yuv_uv_row_offset[r_src] 
		    + yuv_uv_col[c_src];
	    }
	}
    }
    //end init yuv offset

    //init corner data
    corners[front_left].rate = -0.5 * camera.front.col / camera.left.row;
    corners[front_left].rate_uv = -1.0 * camera.front.col / camera.left.row;
    corners[front_left].match_begin = -corners[front_left].rate * camera.left.row
	+ 0.5 * camera.front.col;
    corners[front_left].y_offset = yuv_y_row_offset[camera.left.y];
    corners[front_left].uv_offset = yuv_uv_row_offset[camera.left.y];

    corners[front_right].rate = 0.5 * camera.front.col / camera.right.row;
    corners[front_right].rate_uv = 1.0 * camera.front.col / camera.right.row;
    corners[front_right].match_begin = 0.5 * camera.front.col;
    corners[front_right].y_offset = yuv_y_row_offset[camera.right.y];
    corners[front_right].uv_offset = yuv_uv_row_offset[camera.right.y];

    corners[rear_left].rate = 0.5 * camera.rear.col / camera.left.row;
    corners[rear_left].rate_uv = 1.0 * camera.rear.col / camera.left.row;
    corners[rear_left].match_begin = corners[rear_left].rate;
    corners[rear_left].y_offset = yuv_y_row_offset[camera.left.y] + camera.rear.x;
    corners[rear_left].uv_offset = yuv_uv_row_offset[camera.left.y] + (camera.rear.x >> 1);
    
    corners[rear_right].rate = -0.5 * camera.rear.col / camera.right.row;
    corners[rear_right].rate_uv = -1.0 * camera.rear.col / camera.right.row;
    corners[rear_right].match_begin = -corners[rear_right].rate * camera.right.row;
    corners[rear_right].y_offset = yuv_y_row_offset[camera.right.y] + camera.rear.x;
    corners[rear_right].uv_offset = yuv_uv_row_offset[camera.right.y] + (camera.rear.x >> 1);
    //end init corner data

    return 0;
}

/*
拼接算法执行拼接接口
front：前摄像头帧图像
rear：后摄像头帧图像
left：左侧摄像头帧图像
right：右侧摄像头帧图像
output：拼接完成的帧图像
return: >=0为成功拼接完成；<0为错误代码，具体由算法提供方定义
*/
int doStitching(YUVBuffer *front, YUVBuffer* rear, YUVBuffer *left, YUVBuffer *right,
	YUVBuffer *output)
{
#if 1
    
    fillImage((Uint8 *)output->bufYUV, (Uint8 *)front->bufYUV, &camera.front);
    usleep(1);
    fillImage((Uint8 *)output->bufYUV, (Uint8 *)rear->bufYUV, &camera.rear);
    usleep(1);
    fillImage((Uint8 *)output->bufYUV, (Uint8 *)left->bufYUV, &camera.left); 
    usleep(1);
    fillImage((Uint8 *)output->bufYUV, (Uint8 *)right->bufYUV, &camera.right);
    usleep(1);
    
    fillImageCorner((Uint8 *)output->bufYUV, (Uint8 *)front->bufYUV, (Uint8 *)right->bufYUV,
	    &camera.front, &camera.right, front_right);
    usleep(1);
   
    fillImageCorner((Uint8 *)output->bufYUV, (Uint8 *)front->bufYUV, (Uint8 *)left->bufYUV,
	    &camera.front, &camera.left, front_left);
    usleep(1);
    
    fillImageCorner((Uint8 *)output->bufYUV, (Uint8 *)rear->bufYUV, (Uint8 *)right->bufYUV,
	    &camera.rear, &camera.right, rear_right);
    usleep(1);
    
    fillImageCorner((Uint8 *)output->bufYUV, (Uint8 *)rear->bufYUV, (Uint8 *)left->bufYUV,
	    &camera.rear, &camera.left, rear_left);
    usleep(1);

#else
    static int i = 0;
    static int j = 0;
    int fd = 0;
    
    if (i == 0)
    {
        memcpy((char*)output->bufYUV, (char*)front->bufYUV, 1280*300);
	usleep(1);
        memcpy((char*)output->bufYUV + 1280*300, (char*)front->bufYUV + 1280*300, 1280*300);
        usleep(1);
	memcpy((char*)output->bufYUV + 1280*600, (char*)front->bufYUV + 1280*600, 1280*300);
        usleep(1);
	memcpy((char*)output->bufYUV + 1280*900, (char*)front->bufYUV + 1280*900, 1280*300);
        usleep(1);

	fd = open("./front.yuv", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	write(fd, (char*)front->bufYUV, 1280*300);
	usleep(1);
        write(fd, (char*)front->bufYUV + 1280*300, 1280*300);
	usleep(1);
        write(fd, (char*)front->bufYUV + 1280*600, 1280*300);
	usleep(1);
        write(fd, (char*)front->bufYUV + 1280*900, 1280*300);
	usleep(1);
	close(fd);
    }
    else if (i == 1)
    {
        memcpy((char*)output->bufYUV, (char*)rear->bufYUV, 1280*300);
	usleep(1);
	memcpy((char*)output->bufYUV + 1280*300, (char*)rear->bufYUV + 1280*300, 1280*300);
        usleep(1);
	memcpy((char*)output->bufYUV + 1280*600, (char*)rear->bufYUV + 1280*600, 1280*300);
        usleep(1);
	memcpy((char*)output->bufYUV + 1280*900, (char*)rear->bufYUV + 1280*900, 1280*300);
	usleep(1);

	fd = open("./rear.yuv", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	write(fd, (char*)rear->bufYUV, 1280*300);
	usleep(1);
	write(fd, (char*)rear->bufYUV + 1280*300, 1280*300);
	usleep(1);
        write(fd, (char*)rear->bufYUV + 1280*600, 1280*300);
	usleep(1);
        write(fd, (char*)rear->bufYUV + 1280*900, 1280*300);
	usleep(1);
	close(fd);
    }
    else if (i == 2)
    {
        memcpy((char*)output->bufYUV, (char*)left->bufYUV, 1280*300);
        usleep(1);
	memcpy((char*)output->bufYUV + 1280*300, (char*)left->bufYUV + 1280*300, 1280*300);
        usleep(1);
	memcpy((char*)output->bufYUV + 1280*600, (char*)left->bufYUV + 1280*600, 1280*300);
        usleep(1);
	memcpy((char*)output->bufYUV + 1280*900, (char*)left->bufYUV + 1280*900, 1280*300);
	usleep(1);

	fd = open("./left.yuv", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	write(fd, (char*)left->bufYUV, 1280*300);
	usleep(1);
	write(fd, (char*)left->bufYUV + 1280*300, 1280*300);
	usleep(1);
        write(fd, (char*)left->bufYUV + 1280*600, 1280*300);
	usleep(1);
        write(fd, (char*)left->bufYUV + 1280*900, 1280*300);
	usleep(1);
	close(fd);
    }
    else if (i == 3)
    {
        memcpy((char*)output->bufYUV, (char*)right->bufYUV, 1280*300);
	usleep(1);
	memcpy((char*)output->bufYUV + 1280*300, (char*)right->bufYUV + 1280*300, 1280*300);
	usleep(1);
	memcpy((char*)output->bufYUV + 1280*600, (char*)right->bufYUV + 1280*600, 1280*300);
	usleep(1);
	memcpy((char*)output->bufYUV + 1280*900, (char*)right->bufYUV + 1280*900, 1280*300);
        usleep(1);

	fd = open("./right.yuv", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	write(fd, (char*)right->bufYUV, 1280*300);
	usleep(1);
	write(fd, (char*)right->bufYUV + 1280*300, 1280*300);
	usleep(1);
        write(fd, (char*)right->bufYUV + 1280*600, 1280*300);
	usleep(1);
        write(fd, (char*)right->bufYUV + 1280*900, 1280*300);
	usleep(1);
	close(fd);
    }

    j++;
    
    if (j == 100)
    {
	j = 0;
        i++;
        i = i % 4;
    }

#if 0
    fd = open("./montage.yuv", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd > 0)
    {
        if (write(fd, (char*)output->bufYUV, 1280*1200) == 1280*1200)
	{
	    printf("write ok......\n");
	}
        close(fd);
    }
    else
    {
	printf("open error......\n");
    }
#endif

#endif

    return 0;
}

/*
拼接算法资源释放接口
return: >=0为成功；<0为错误代码，具体由算法提供方定义
*/
int deInitStitching(void)
{
    int r = 0;

    if (data)
    {
        free(data);
    }
    
    for (r = 0; r < camera.front.row; r++)
    {
	free(camera.front.yuv_offset[r]);
	free(camera.front.uv_offset[r]);
    }
    free(camera.front.yuv_offset);
    free(camera.front.uv_offset);
    
    for (r = 0; r < camera.rear.row; r++)
    {
	free(camera.rear.yuv_offset[r]);
	free(camera.rear.uv_offset[r]);
    }
    free(camera.left.yuv_offset);
    free(camera.left.uv_offset);

    for (r = 0; r < camera.left.row; r++)
    {
	free(camera.left.yuv_offset[r]);
	free(camera.left.uv_offset[r]);
    }
    free(camera.left.yuv_offset);
    free(camera.left.uv_offset);
    
    for (r = 0; r < camera.right.row; r++)
    {
	free(camera.right.yuv_offset[r]);
	free(camera.right.uv_offset[r]);
    }
    free(camera.right.yuv_offset);
    free(camera.right.uv_offset);

    return 0;
}
