

/*-----------------------------
function:Surveying system 
author  £ºhuanghuiming
method  : camera  internal parameters  calibration  by  polynomial fitting.
          external parameter calibration by 4 Checkerboard patterns. 
time£º217-2-23
----------------------------------*/

#include <iostream>
#include <vector>    
#include<fstream>
#include "opencv2/core/core.hpp"
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#ifdef _DEBUG
#pragma comment(lib,"opencv_features2d244d.lib")
#pragma comment(lib,"opencv_core244d.lib")
#pragma comment(lib,"opencv_highgui244d.lib")
#pragma comment(lib,"opencv_calib3d244d.lib")
#pragma comment(lib,"opencv_flann244d.lib")
#pragma comment(lib,"opencv_contrib244d.lib")
#pragma comment(lib,"opencv_imgproc244d.lib")
#else
#pragma comment(lib,"opencv_features2d244.lib")
#pragma comment(lib,"opencv_core244.lib")
#pragma comment(lib,"opencv_highgui244.lib")
#pragma comment(lib,"opencv_calib3d244.lib")
#pragma comment(lib,"opencv_flann244.lib")
#pragma comment(lib,"opencv_contrib244.lib")
#pragma comment(lib,"opencv_imgproc244.lib")

#endif


struct Caream_paraters
{
	double dx;
	double dy;
	double F;
	double k1;
	double k2;
	double k3;
	double k4;
	double k5;
};

struct Caream_data
{	
	short out_image_height;
	short out_image_width;
	short in_image_height;
	short in_image_width;	
	float *fornt_data;
	float *left_data;
	float *rear_data;
	float *right_data;
};
  
struct Image_data
{	short height;
	short width;
	float *f_data;
	short *s_data;
};
struct Stich_calib_data
{
	short flag;
	short out_image_height;
	short out_image_width;
	short in_image_height;
	short in_image_width;
	Image_data front;
	Image_data left;
	Image_data rear;
	Image_data right;

};

struct Ex_calib_space
{
	int width;
	int height;
	int front_row;
	int front_col;
	int rear_row;
	int rear_col;
	int left_row;
	int left_col;
	int right_row;
	int right_col;
};

struct Ex_calib_collect
{
	int calib_bord_x;
	int calib_bord_y;	
	float d1;
	float d2;
	float fort_rea;
	float left_right;
	float Length;
	float Width;
	float scale;
};

struct Camera_undistorted_area
{
    int width;
    int height;
    int offset_x;
    int offset_y;
    int rotation_angle;
};

struct Camera_undistorted
{
    int image_width;
    int image_height;
    int undist_image_width;
    int undist_image_height;
    float scale;

    Camera_undistorted_area front;
    Camera_undistorted_area rear;
    Camera_undistorted_area left_front;
    Camera_undistorted_area right_front;
    Camera_undistorted_area left_rear;
    Camera_undistorted_area right_rear;
};

class Surveying_system
{
public:
	//suface_match();
	//~suface_match();
	Mat registim;
	Size ImageSize;
	Size board_size;
	vector<Point2d> points;
	double x ;
	double y ;
	double beam_value;
	Caream_data caramdata;
	Stich_calib_data  stich_data;
	Ex_calib_space    ex_calib_space;
	Ex_calib_collect  ex_calib_collect;
	Caream_paraters   caream_paraters;
	Camera_undistorted camera_undistorted;
	vector<Mat> V_image;
	Mat forshow_udist;
	Mat forshow_coner;
	
	bool Init_stich_data();
	bool Init_Ex_calib_space();
	bool Init_Ex_calib_collcet();
	bool Init_Ex_calib(char *file_name);
	bool Read_Caream_paraters();
	bool Read_Caream_paraters(char *file_name);
	bool Init_caram_data(int in_heigth, int in_width, int out_heigth, int out_width);
	bool Init_camera_undistorted_config(char *file_name);
	bool Creat_calib_model(vector<vector<Point2d> >&V_coord);
	bool ConerDetect(Size board_size, Mat im, vector<Point2d> &coners);
	bool calibration_by_ployfit(int index,int flage,Mat im, vector<Point2d> Bconers, Mat &Mapx, Mat &Mapy);
	bool UdistImge_ployfit(int  scale, int flage,Mat image, Mat &udistimage, Mat &Umapx, Mat &Umapy);	 //camera  Internal parameters  calibration  by  Polynomial fitting
	bool Udist_warp_TransMap(int index, Mat udist_mapx, Mat udist_mapy, Mat wrap_mapx, Mat wrap_mapy, Mat &mapx, Mat &mapy);
	bool Warp_tran(Mat H, Mat &mapx, Mat& mapy);
	bool Transmap_to_data(Mat mapx, Mat mapy, int bignx, int bigny, int height, int width, float*data);
	bool Write_stitch_calibdata(int flag, char*filename, int number, Caream_data caram_data);
	bool Save_singleCam_udistfile(char*file,Mat image);
	bool Save_Camera_undistorted_file(char *file_name);
	bool Create_camera_calib_model(vector<Point2d> &coord, float scale, int width, int height);

	bool Image_fusion(int index,  Mat im,  Mat &regist_im);
	bool Remap(Mat im, Mat mapx, Mat mapy, int flag,Mat &remap_iamge);
	bool Image_remap(int flag,int index, Mat distimage, Mat mapx, Mat mapy, Mat &remap_iamge);
	bool Read_stich_calibdata(int flag, char *filename);
	bool Read_data(char*filename, int number, short *data);
    bool Read_data(char*filename, int number, float *data);			
	unsigned char*Imgae_remap(int flag, int index, int src_heigth, int src_width, int dst_heigth, int dst_width,Mat distiamge, Mat &imagedata);						
	bool Read_singlecam(char*file,Mat image);

	/*3D view*/
	bool Calcte_cam_extrinicPara(Mat image);

private:

};
