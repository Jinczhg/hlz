#include <stdlib.h>
#include <stdio.h>

#include "htsv_undistorted.h"

int main(int argc, char **argv)
{
    char *config_file = "./camera_undistorted_config.xml";
    char *output_file = "./camera_undistorted.dat";
    
    int ret = 0;
    
    if (argc > 1 && argc < 3)
    {
    	printf("usage: %s config_file output_file\n", argv[0]);
    	return 0;
    }

    if (argc >= 3)
    {
        config_file = argv[1];
        output_file = argv[2];
    }

    ret = htsv_camera_undistorted(config_file, output_file);
    
    printf("htsv_camera_undistorted ret %d\n", ret);

    return 0;
}
