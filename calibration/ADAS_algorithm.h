/*-----------------------------
function:Surveying system 
author  £ºhuanghuiming
method  : camera  internal parameters  calibration  by  polynomial fitting.
          external parameter calibration by 4 Checkerboard patterns. 
time£º217-2-23
----------------------------------*/

#ifndef __ADAS_ALGORITHM_H
#define __ADAS_ALGORITHM_H

#include <iostream>
#include <vector>    
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

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
{
    short height;
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
    float scene_width;
    float scene_height;
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


class Surveying_system
{
public:
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
    vector<Mat> V_image;
    Mat forshow_udist;
    Mat forshow_coner;
	
	
    bool Detect_coner(Size board_size,Mat image,vector<Point2d> &coners);	
    bool Image_calibration(int index,int flage,Mat im, Mat intrinsic_matrix, Mat distortion_coeffs, vector<Point2d> Bconers, Mat &Mapx, Mat &Mapy);
    bool Image_fusion(int index,  Mat im,  Mat &regist_im);
    bool Remap(Mat im, Mat mapx, Mat mapy, int flag,Mat &remap_iamge);
    bool Image_remap(int flag,int index, Mat distimage, Mat mapx, Mat mapy, Mat &remap_iamge);
    bool Udist_warp_TransMap(int index, Mat udist_mapx, Mat udist_mapy, Mat wrap_mapx, Mat wrap_mapy, Mat &mapx, Mat &mapy);
    bool ReadMap(char *file, Size size,Mat &mapx);
    int  Im_point_locad(Point2d impoint, Point2d S, Point2d E);		
    bool Creat_calib_model(vector<vector<Point2d>>&V_coord);
    bool Init_caram_data(int in_heigth,int in_width,int out_heigth,int out_width);
    bool Init_stich_data();
    bool Transmap_to_data(Mat mapx, Mat mapy,int bignx,int bigny, int height,int width,float*data);
    bool Write_stitch_calibdata(int flag, char*filename, int number, Caream_data caram_data);
    bool Read_stich_calibdata(int flag, char *filename);
    bool Read_data(char*filename, int number, short *data);
    bool Read_data(char*filename, int number, float *data);
    bool Init_Ex_calib_space();
    bool Init_Ex_calib_collcet();		
    unsigned char*Image_fusion(unsigned char*sur, unsigned char*dst, int height, int width);
    unsigned char*Imgae_remap(int flag, int index, int src_heigth, int src_width, int dst_heigth, int dst_width,Mat distiamge, Mat &imagedata);
    void Undistort_image(Mat input_im, Mat& out_im, Mat intrinsic_matrix, Mat distortion_coeffs);	
    void Trans_Image(int index,vector<Point2d> Bconers, vector<Point2d> coners, Mat im, Mat &outim);
    bool ConerDetect(Size board_size, Mat im, vector<Point2d> &coners);	
    bool UdistIm_hhm(Mat K, Mat D, Mat R, Mat P, cv::Size& size, int m1type, Mat &map1, Mat &map2);	
    bool Warp_tran( Mat H, Mat &mapx,Mat& mapy);	
    bool Saveimage_value(int flag, char *filename, Mat im);		
    bool Read_Caream_paraters();
    bool UdistImge_ployfit(Mat image, Mat &udistimage, Mat &Umapx, Mat &Umapy);	 //camera  Internal parameters  calibration  by  Polynomial fitting

private:

};

#endif //__ADAS_ALGORITHM_H
