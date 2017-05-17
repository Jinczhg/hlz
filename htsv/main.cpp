#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "stitchalgo.h"

unsigned char clip(float in)
{
    if (in > 255) return 255;
    if (in <  0) return 0;
    return (unsigned char)in;
}

void yuv2rgb(unsigned char* yuv, unsigned char *rgb, int width, int height)
{
    float r, g, b;
    float y, u, v;
    unsigned char *py;
    unsigned char *pcbcr;
    int idx = 0;
    
    py = yuv;
    pcbcr = yuv + width * height;

    for (int dy = 0; dy < height; dy++)
    {
	for (int dx = 0; dx < width; dx++)
	{
	    y = (float)py[dx];
	    u = (float)pcbcr[dx&0xFFFE];
	    v = (float)pcbcr[(dx&0xFFFE)+1];

	    r = y + 1.402 * (v - 128);
	    g = y - 0.344 * (u - 128) - 0.714 * (v - 128);
	    b = y + 1.772 * (u - 128);
	    rgb[idx] = clip(b);
	    rgb[idx + 1] = clip(g);
	    rgb[idx + 2] = clip(r);

	    idx += 3;
	}

	py += width;
	pcbcr += width * (dy & 0x01);
    }
}

int main(int argc, char** argv)
{
    uint32_t out_width = 1280, out_height = 720;
    int in_width = 1280, in_height = 720;
    std::string front_file, rear_file, left_file, right_file, dst_file;
    unsigned char *front = NULL;
    unsigned char *rear = NULL;
    unsigned char *left = NULL;
    unsigned char *right = NULL;
    unsigned char *dst = NULL;
    int size = 0;
    int out_size = 0;
    int fd = -1;
    int fd_w = -1;
    YUVBuffer front_yuv, rear_yuv, left_yuv, right_yuv, dst_yuv;
    uint16_t *uv = 0;
    int door_status = 0;
    
    if (argc < 2)
    {
        std::cout << "usage: " << argv[0] << " confingure_file door_status" << std::endl;
	return 0;
    }

    if (argc > 2)
    {
	door_status = atoi(argv[2]);
    }
    
    cv::FileStorage fs(argv[1], cv::FileStorage::READ);

    if (!fs.isOpened())
    {
	std::cout << "can't open " << argv[1] << " :" << strerror(errno) << std::endl;
    }

    fs["in_width"] >> in_width;
    fs["in_height"] >> in_height;
    fs["front"] >> front_file;
    fs["rear"] >> rear_file;
    fs["left"] >> left_file;
    fs["right"] >> right_file;
    fs["dst"] >> dst_file;

    printf("in_height=%d, in_width=%d\n", in_height, in_width);

    initStitching(in_width, 0, in_width * in_height, in_width, in_height);
    getStitchingSize(&out_width, &out_height);

    printf("out_width=%d, out_height=%d\n", out_width, out_height);
    
    cv::Mat img(out_height, out_width, CV_8UC3);
    unsigned char *rgb = img.data;

    size = in_width * in_height * 3 / 2;
    out_size = out_width * out_height * 3 / 2;
    front = (unsigned char*)malloc(size);
    rear = (unsigned char*)malloc(size);
    left = (unsigned char*)malloc(size);
    right = (unsigned char*)malloc(size);
    dst = (unsigned char*)malloc(out_size);
    memset(dst, 0, out_size);

    //front
    fd = open(front_file.c_str(), O_RDONLY);
    if (fd < 0)
    {
        std::cout << "open error: " << strerror(errno) << std::endl;
	goto out;
    }

    if (read(fd, front, size) != size)
    {
	std::cout << "read error: " << strerror(errno) << std::endl;
	goto out;
    }
    close(fd);
    fd = -1;

    //rear
    fd = open(rear_file.c_str(), O_RDONLY);
    if (fd < 0)
    {
        std::cout << "open error: " << strerror(errno) << std::endl;
	goto out;
    }

    if (read(fd, rear, size) != size)
    {
	std::cout << "read error: " << strerror(errno) << std::endl;
	goto out;
    }
    close(fd);
    fd = -1;

    //left
    fd = open(left_file.c_str(), O_RDONLY);
    if (fd < 0)
    {
        std::cout << "open error: " << strerror(errno) << std::endl;
	goto out;
    }

    if (read(fd, left, size) != size)
    {
	std::cout << "read error: " << strerror(errno) << std::endl;
	goto out;
    }
    close(fd);
    fd = -1;

    //right
    fd = open(right_file.c_str(), O_RDONLY);
    if (fd < 0)
    {
        std::cout << "open error: " << strerror(errno) << std::endl;
	goto out;
    }

    if (read(fd, right, size) != size)
    {
	std::cout << "read error: " << strerror(errno) << std::endl;
	goto out;
    }
    close(fd);
    fd = -1;
    
    front_yuv.bufYUV = (long)front;
    rear_yuv.bufYUV = (long)rear;
    left_yuv.bufYUV = (long)left;
    right_yuv.bufYUV = (long)right;
    dst_yuv.bufYUV = (long)dst;

    printf("to doStitching\n");

    if (door_status == 1 || door_status == 3)
    {
        controlStitching(CONTROL_LEFT_DOOR_OPEN, 1);
    }

    if (door_status == 2 || door_status == 3)
    {
        controlStitching(CONTROL_RIGHT_DOOR_OPEN, 1);
    }

    doStitching(&front_yuv, &rear_yuv, &left_yuv, &right_yuv, &dst_yuv);

    yuv2rgb(dst, rgb, out_width, out_height);

    imwrite("sv.jpg", img);
    fd_w = open("sv.yuv", O_WRONLY | O_CREAT, 0666);
    if (write(fd_w, dst, out_size) != out_size)
    {
    }
    close(fd_w);
    
    imshow("Image View", img);
    cv::waitKey();

out:
    if (fd > 0)
    {
	close(fd);
    }

    if (front)
    {
	free(front);
    }

    if (rear)
    {
	free(rear);
    }

    if (left)
    {
	free(left);
    }

    if (right)
    {
	free(right);
    }

    return 0;
}
