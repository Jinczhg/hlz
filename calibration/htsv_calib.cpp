#include "htsv_calib.h"
#include "ADAS_algorithm.h"

Surveying_system  ADAS;

int htsv_2d_calib(char *config_file, char *camera_para_file, calib_2d_offset offset, char *front_file, char *rear_file, char *left_file, char *right_file, char *output_file)
{
    printf("\n*******************************************\n");
    printf("\%s build on %s\n", "htsv lib", __DATE__);
    printf("*******************************************\n\n");
    
    std::vector<char*> files;
    files.push_back(front_file);
    files.push_back(left_file);
    files.push_back(rear_file);
    files.push_back(right_file);
    
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
    
    if (!ADAS.Init_Ex_calib(config_file))
    {
        return -1;
    }
    
    if (!ADAS.Read_Caream_paraters(camera_para_file))
    {
        return -1;
    }
    
    ADAS.board_size = Size(ADAS.ex_calib_collect.calib_bord_x, ADAS.ex_calib_collect.calib_bord_y);
    ADAS.ImageSize.height = ADAS.ex_calib_space.height;
	ADAS.ImageSize.width = ADAS.ex_calib_space.width;
		
	Mat registim(ADAS.ImageSize.height, ADAS.ImageSize.width, CV_8UC3, Scalar(0, 0, 0));
	vector<vector<Point2d>> V_Coners;
	vector<Point2d> Bconers;
	Mat udist_transed;
	Mat mapx, mapy;
    
    if (!ADAS.Creat_calib_model(V_Coners))
    {
        return -1;
    }

    ADAS.Init_caram_data(ADAS.ImageSize.height, ADAS.ImageSize.width, ADAS.ex_calib_space.height, ADAS.ex_calib_space.width);
	int number;
	bool succed = false;
	cv::String string;
	Mat imageun;
	for (int i = 1; i < 5; i++)
	{										
	    ADAS.V_image[i-1].copyTo(imageun);

		Bconers = V_Coners[i - 1];
		
		if (ADAS.calibration_by_ployfit(i, 0, imageun, Bconers, mapx, mapy))
		{
		    switch (i)
			{
			case  1:
			    ADAS.Transmap_to_data(mapx, mapy, 0, 0, ADAS.ex_calib_space.front_row, ADAS.ex_calib_space.front_col, ADAS.caramdata.fornt_data);
			    break;
			    
			case  2:
			    ADAS.Transmap_to_data(mapx, mapy, 0, 0, ADAS.ex_calib_space.left_row, ADAS.ex_calib_space.left_col, ADAS.caramdata.left_data);
				break;
				
			case  3:
			    ADAS.Transmap_to_data(mapx, mapy, ADAS.ImageSize.height - ADAS.ex_calib_space.rear_row, 0,
							ADAS.ImageSize.height, ADAS.ImageSize.width, ADAS.caramdata.rear_data);
				break;
				
			case 4:
			    ADAS.Transmap_to_data(mapx, mapy, 0, ADAS.ImageSize.width - ADAS.ex_calib_space.right_col,
							ADAS.ImageSize.height, ADAS.ImageSize.width, ADAS.caramdata.right_data);
				succed = true;
				break;
				
			default:
				break;
			}


			ADAS.Image_remap(1, i, imageun, mapx, mapy, udist_transed);
			ADAS.Image_fusion(0, udist_transed, registim);
			udist_transed.release();					
        }		
        else
		{
		    printf("process fail\n");
		    return -1;
		}
    }

	number = 2 * mapx.rows*mapx.cols;
	if (succed)
	{
        if (ADAS.Write_stitch_calibdata(1, output_file, number, ADAS.caramdata))
        {
            printf("save camera.dat ok\n");
        }
        else
        {
            printf("save camera.dat fail\n");
            return -1;
        }		
    }
    
    return 0;
}

