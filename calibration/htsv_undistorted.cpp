#include "htsv_undistorted.h"
#include "ADAS_algorithm.h"

Surveying_system  ADAS;

int htsv_camera_undistorted(char *camera_para_file, char *config_file, char *output_file)
{
    printf("\n*******************************************\n");
    printf("\%s build on %s\n", "htsv lib", __DATE__);
    printf("*******************************************\n\n");
    
    std::vector<char*> files;
    files.push_back("front.jpg");
    files.push_back("left.jpg");
    files.push_back("rear.jpg");
    files.push_back("right.jpg");
    
    for (char *file : files)
    {
        cv::Mat image = cv::imread(file);
    
        if (image.empty())
        {
            printf("file %s is bad\n", file);
            return -1;
        }
    
        ADAS.V_image.push_back(image);
    }
    
    if (!ADAS.Init_Ex_calib("calib_config.xml"))
    {
        return -1;
    }
    
    ADAS.board_size = Size(ADAS.ex_calib_collect.calib_bord_x, ADAS.ex_calib_collect.calib_bord_y);
    ADAS.ImageSize.height = 720;
	ADAS.ImageSize.width = 1280*4;
    
    if (!ADAS.Read_Caream_paraters(camera_para_file))
    {
        return -1;
    }
    
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

