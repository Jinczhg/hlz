#ifndef __YUV_H
#define __YUV_H

void yuv_merge(int stride, int height, char *front, char *rear, char *left, char *right,
	char *dst);

void uyvy_merge(int stride, int height, char *front, char *rear, char *left, char *right,
	char *dst);
	
void yuv_nv12_2_uyvy_merge(int stride, int height, char *front, char *rear, char *left, char *right, char *dst);
#endif
