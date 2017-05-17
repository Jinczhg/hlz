#include <stdlib.h>
#include <stdio.h>

#include "htsv_calib.h"

int main(int argc, char **argv)
{
    char *config_file = "./calib_config.xml";
    char *camera_para_file = "./caream_paraters.txt";
    char *front_file = "./front.jpg";
    char *rear_file = "./rear.jpg";
    char *left_file = "./left.jpg";
    char *right_file = "./right.jpg";
    char *output_file = "./camera.dat";
    calib_2d_offset offset;
    
    int ret = 0;
    
    if (argc > 1 && argc < 6)
    {
    	printf("usage: %s front_file rear_file left_file right_file output_file\n", argv[0]);
    	return 0;
    }

    if (argc >= 6)
    {
        front_file = argv[1];
        rear_file = argv[2];
        left_file = argv[3];
        right_file = argv[4];
        output_file = argv[5];
    }

    ret = htsv_2d_calib(config_file, camera_para_file, offset, front_file, rear_file, left_file, right_file, output_file);
    
    printf("htsv_2d_calib ret %d\n", ret);

    return 0;
}
