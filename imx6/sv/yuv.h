#ifndef __YUV_H
#define __YUV_H

void yuv_merge_all(int stride, int height, char *front, char *rear, char *left, char *right,
	char *dst, int y_half_size, int uv_half_size);

void uyvy_merge(int stride, int height, char *front, char *rear, char *left, char *right,
	char *dst);
	
void yuv_nv12_2_uyvy_merge(int stride, int height, char *front, char *rear, char *left, char *right, char *dst);
#endif
