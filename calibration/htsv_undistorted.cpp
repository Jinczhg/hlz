#include "htsv_undistorted.h"
#include "ADAS_algorithm.h"

Surveying_system  ADAS;

int htsv_camera_undistorted(char *config_file, char *output_file)
{
    printf("\n*******************************************\n");
    printf("\%s build on %s\n", "htsv lib", __DATE__);
    printf("*******************************************\n\n");
    

    
    if (!ADAS.Init_camera_undistorted_config(config_file))
    {
        return -1;
    }
    
    if (ADAS.Save_Camera_undistorted_file(output_file))
    {
        printf("save %s ok\n", output_file);
    }
    else
    {
        printf("save %s fail\n", output_file);
        return -1;
    }
    
    return 0;
}

