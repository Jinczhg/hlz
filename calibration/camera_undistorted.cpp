#include <stdlib.h>
#include <stdio.h>

#include "htsv_undistorted.h"

int main(int argc, char **argv)
{
    char *camera_para_file = "caream_paraters.txt";
    char *config_file = "./camera_undistorted_config.xml";
    char *output_file = "./camera_undistorted.dat";
    
    int ret = 0;
    
    if (argc > 1 && argc < 4)
    {
    	printf("usage: %s config_file output_file\n", argv[0]);
    	return 0;
    }

    if (argc >= 4)
    {
	camera_para_file = argv[1];
        config_file = argv[2];
        output_file = argv[3];
    }

    ret = htsv_camera_undistorted(camera_para_file, config_file, output_file);
    
    printf("htsv_camera_undistorted ret %d\n", ret);

    return 0;
}
