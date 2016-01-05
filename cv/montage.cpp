#include <iostream>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

struct camera_mapping
{
    uint16_t row;
    uint16_t col;
    uint16_t *data;
    int x; //offset x
    int y; //offset y
};

struct camera_data
{
    uint16_t square_size;
    struct camera_mapping front;
    struct camera_mapping rear;
    struct camera_mapping left;
    struct camera_mapping right;
};

struct yuv_420_nv21
{
    int width;
    int height;
    uint8_t *y;
    uint8_t *uv;
};

void fillImage(yuv_420_nv21 *dst, yuv_420_nv21 *src, camera_mapping *mapping)
{
    int r_src;
    int c_src;
    int r_dst;
    int c_dst;

    for (int r = 0; r < mapping->row; r++)
    {
	r_dst = r + mapping->y;
	for (int c = 0; c < mapping->col; c++)
	{
	    c_dst = c + mapping->x;

	    c_src = *(mapping->data + (r * mapping->col + c) * 2);
	    r_src = *(mapping->data + (r * mapping->col + c) * 2 + 1);
	    
	    *(dst->y + r_dst * dst->width + c_dst) = *(src->y + r_src * src->width + c_src);
	    
	    if (r_dst % 2 == 1 && c_dst % 2 == 1)
	    {
                *(dst->uv + (r_dst - 1) * dst->width / 2 + c_dst - 1) = 
		    *(src->uv + r_src * (src->width / 2) + (c_src % 2 == 1 ? c_src - 1 : c_src));
                
		*(dst->uv + (r_dst - 1) * dst->width / 2 + c_dst) = 
		    *(src->uv + r_src * (src->width / 2) + (c_src % 2 == 1 ? c_src : c_src + 1));
	    }
	}
    }
}

int main(int argc, char **argv)
{
    std::string camera_data = "./camera.dat";

    std::string path = ".";

    if (argc > 1)
    {
	path = argv[1];
    }

    // init camera data
    int data_fd;
    struct stat statbuf;
    int file_len = 0;
    uint16_t *data = NULL;

    struct camera_data camera;
    int index = 0;
    int diff_w = 0;
    int diff_h = 0;

    data_fd = open(camera_data.c_str(), O_RDONLY);
    if (data_fd < 0)
    {
	std::cout << "open error:" << strerror(errno) << "--"
	    << camera_data.c_str() << std::endl;
	return 0;
    }
    
    if (stat(camera_data.c_str(), &statbuf) < 0)
    {
        std::cout << "stat error:" << strerror(errno) << std::endl;
	return 0;
    }
    
    file_len = statbuf.st_size;
    data = new uint16_t[file_len/2];

    if (read(data_fd, data, file_len) != file_len)
    {
	std::cout << "read error:" << strerror(errno) << std::endl;
	return 0;
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

    diff_w = 1280 - (camera.front.col + camera.left.col + camera.rear.col);
    diff_h = 800 - camera.front.row;
    camera.front.x = diff_w / 2;
    camera.front.y = diff_h / 2;

    camera.rear.x = diff_w / 2 + camera.front.col + camera.left.col;
    camera.rear.y = diff_h / 2;

    camera.left.x = diff_w / 2 + camera.front.col;
    camera.left.y = diff_h / 2 + camera.front.row - camera.left.row;

    camera.right.x = diff_w / 2 + camera.front.col;
    camera.right.y = diff_h / 2;

    //end init camera data

    int height = 800;//camera.front.row;
    int width = 1280;//camera.front.col+camera.left.col+camera.rear.col;
    int len = width*(height+height/2);
    int width_src = 1280;
    int height_src = 800;
    int len_src = 1280*(800+800/2);

    yuv_420_nv21 dst;
    dst.width = width;
    dst.height = height;
    dst.y = new uint8_t[len];
    dst.uv = dst.y + dst.width*dst.height;
    memset(dst.y, 0, width*height);
    memset(dst.uv, 128, width*height/2);

    yuv_420_nv21 front;
    front.width = width_src;
    front.height = height_src;
    front.y = new uint8_t[width_src*(height_src+height_src/2)];
    front.uv = front.y + width_src*height_src;

    int fd_front = open((path+"/FrontPicBeforeStitch.yuv").c_str(), O_RDONLY);
    if (read(fd_front, front.y, len_src) != len_src)
    {

    }
    close(fd_front);
    
    yuv_420_nv21 left;
    left.width = width_src;
    left.height = height_src;
    left.y = new uint8_t[width_src*(height_src+height_src/2)];
    left.uv = left.y + width_src*height_src;

    int fd_left = open((path+"/LeftPicBeforeStitch.yuv").c_str(), O_RDONLY);
    if (read(fd_left, left.y, len_src) != len_src)
    {
    }
    close(fd_left);

    yuv_420_nv21 rear;
    rear.width = width_src;
    rear.height = height_src;
    rear.y = new uint8_t[width_src*(height_src+height_src/2)];
    rear.uv = rear.y + width_src*height_src;

    int fd_rear = open((path+"/RearPicBeforeStitch.yuv").c_str(), O_RDONLY);
    if (read(fd_rear, rear.y, len_src) != len_src)
    {
    }
    close(fd_rear);
    
    yuv_420_nv21 right;
    right.width = width_src;
    right.height = height_src;
    right.y = new uint8_t[width_src*(height_src+height_src/2)];
    right.uv = right.y + width_src*height_src;

    int fd_right = open((path+"/RightPicBeforeStitch.yuv").c_str(), O_RDONLY);
    if (read(fd_right, right.y, len_src) != len_src)
    {
    }
    close(fd_right);

    fillImage(&dst, &front, &camera.front);
    fillImage(&dst, &rear, &camera.rear);
    fillImage(&dst, &left, &camera.left); 
    fillImage(&dst, &right, &camera.right);

#if 0
    int x, y, r_l, c_l;

    //front
    for (int r = 0; r < camera.front.row; r++)
    {
	for (int c = 0; c < camera.front.col; c++)
	{
	    x = *(camera.front.data + (r * camera.front.col + c) * 2);
	    y = *(camera.front.data + (r * camera.front.col + c) * 2 + 1);
	    
            *(dst.y + r * width + c) = *(front.y + y * 1280 + x);

	    if (r % 2 == 1 && c % 2 == 1)
	    {
		*(dst.uv + r * width / 2 + c - 1) = 
		    *(front.uv + y * 1280 / 2 + (x % 2 == 1 ? x - 1 : x));
                
		*(dst.uv + r * width / 2 + c) = 
		    *(front.uv + y * 1280 / 2 + (x % 2 == 1 ? x : x + 1));
	    }
	}
    }

    //left
    for (int r = 0; r < camera.left.row; r++)
    {
	r_l = r + camera.front.row;
	for (int c = 0; c < camera.left.col; c++)
	{
	    x = *(camera.left.data + (r * camera.left.col + c) * 2);
	    y = *(camera.left.data + (r * camera.left.col + c) * 2 + 1);
	    
	    *(dst.y + r_l * width + c) = *(left.y + y * 1280 + x);

	    if (r_l % 2 == 1 && c % 2 == 1)
	    {
                *(dst.uv + r_l * width / 2 + c - 1) = 
		    *(left.uv + y * 1280 / 2 + (x % 2 == 1 ? x - 1 : x));
                
		*(dst.uv + r_l * width / 2 + c) = 
		    *(left.uv + y * 1280 / 2 + (x % 2 == 1 ? x : x + 1));
	    }
	}
    }

    //rear
    for (int r = 0; r < camera.rear.row; r++)
    {
	r_l = r + camera.front.row + camera.left.row;
	for (int c = 0; c < camera.rear.col; c++)
	{
	    x = *(camera.rear.data + (r * camera.rear.col + c) * 2);
	    y = *(camera.rear.data + (r * camera.rear.col + c) * 2 + 1);
	    
	    *(dst.y + r_l * width + c) = *(rear.y + y * 1280 + x);
	    
	    if (r_l % 2 == 1 && c % 2 == 1)
	    {
                *(dst.uv + r_l * width / 2 + c - 1) = 
		    *(rear.uv + y * 1280 / 2 + (x % 2 == 1 ? x - 1 : x));
                
		*(dst.uv + r_l * width / 2 + c) = 
		    *(rear.uv + y * 1280 / 2 + (x % 2 == 1 ? x : x + 1));
	    }
	}
    }

    //right
    for (int r = 0; r < camera.right.row; r++)
    {
	r_l = r + camera.front.row;
	c_l = width - camera.right.col;
	for (int c = 0; c < camera.right.col; c++)
	{
	    x = *(camera.right.data + (r * camera.right.col + c) * 2);
	    y = *(camera.right.data + (r * camera.right.col + c) * 2 + 1);
	    
            *(dst.y + r_l * width + c_l + c) = *(right.y + y * 1280 + x);
	    
	    if (r_l % 2 == 1 && (c_l + c) % 2 == 1)
	    {
                *(dst.uv + r_l * width / 2 + c_l + c - 1) = 
		    *(right.uv + y * 1280 / 2 + (x % 2 == 1 ? x - 1 : x));
                
		*(dst.uv + r_l * width / 2 + c_l + c) = 
		    *(right.uv + y * 1280 / 2 + (x % 2 == 1 ? x : x + 1));
	    }
	}
    }
#endif

    int dst_fd = open("./yuv/montage.yuv", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (write(dst_fd, dst.y, len) != len)
    {
    }
    close(dst_fd);

    delete []data;
    delete []dst.y;
    delete []front.y;
    delete []left.y;
    delete []rear.y;
    delete []right.y;
    
    return 0;
}
