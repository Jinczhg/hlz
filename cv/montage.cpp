#include "cv.h"

#include <iostream>
#include <vector>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
	std::cout << "usage: " << argv[0] << " fs_file" << std::endl;
	return 0;
    }

    int mode = 0;
    std::string front_file;
    std::string rear_file;
    std::string left_file;
    std::string right_file;
    std::string front_img;
    std::string rear_img;
    std::string left_img;
    std::string right_img;

    int row_front = 0;
    int col_front = 0;
    int row_rear = 0;
    int col_rear = 0;
    int row_left = 0;
    int col_left = 0;
    int row_right = 0;
    int col_right = 0;
    int square_size = 0;

    cv::FileStorage fs_s(argv[1], cv::FileStorage::READ);

    fs_s["mode"] >> mode;
    fs_s["front_mapping"] >> front_file;
    fs_s["front_img"] >> front_img;

    fs_s["rear_mapping"] >> rear_file;
    fs_s["rear_img"] >> rear_img;

    fs_s["left_mapping"] >> left_file;
    fs_s["left_img"] >> left_img;

    fs_s["right_mapping"] >> right_file;
    fs_s["right_img"] >> right_img;

    cv::FileStorage fs_front(front_file, cv::FileStorage::READ);
    cv::FileStorage fs_rear(rear_file, cv::FileStorage::READ);
    cv::FileStorage fs_left(left_file, cv::FileStorage::READ);
    cv::FileStorage fs_right(right_file, cv::FileStorage::READ);

    fs_front["square_size"] >> square_size;

    fs_front["grid_row"] >> row_front;
    fs_front["grid_col"] >> col_front;
    
    fs_rear["grid_row"] >> row_rear;
    fs_rear["grid_col"] >> col_rear;
    
    fs_left["grid_row"] >> row_left;
    fs_left["grid_col"] >> col_left;

    fs_right["grid_row"] >> row_right;
    fs_right["grid_col"] >> col_right;

    int r_l = 0;
    int c_l = 0;

    cv::Mat montage_mat(square_size*(row_front+row_left+row_rear),
	    square_size*col_front,
	    CV_8UC3);

    cv::Mat front_mapping(square_size*row_front, square_size*col_front, CV_32SC2);
    fs_front["grid_mapping"] >> front_mapping;
    
    cv::Mat left_mapping(square_size*row_left, square_size*col_left, CV_32SC2);
    fs_left["grid_mapping"] >> left_mapping;

    cv::Mat rear_mapping(square_size*row_rear, square_size*col_rear, CV_32SC2);
    fs_rear["grid_mapping"] >> rear_mapping;

    cv::Mat right_mapping(square_size*row_right, square_size*col_right, CV_32SC2);
    fs_right["grid_mapping"] >> right_mapping;
    
    if (mode == 0) //offline
    {
        cv::Mat front_mat = cv::imread(front_img);
        cv::Mat rear_mat = cv::imread(rear_img);
        cv::Mat left_mat = cv::imread(left_img);
        cv::Mat right_mat = cv::imread(right_img);

	//front
	for (int r = 0; r < front_mapping.rows; r++)
	{
	    for (int c = 0; c < front_mapping.cols; c++)
	    {
		cv::Point p;
		p.x = front_mapping.at<cv::Vec2i>(r,c)[0];
		p.y = front_mapping.at<cv::Vec2i>(r,c)[1];

		montage_mat.at<cv::Vec3b>(r, c) = front_mat.at<cv::Vec3b>(p.y, p.x);
	    }
	}

	//left
	for (int r = 0; r < left_mapping.rows; r++)
	{
	    r_l = r + front_mapping.rows;
	    for (int c = 0; c < left_mapping.cols; c++)
	    {
		cv::Point p;
		p.x = left_mapping.at<cv::Vec2i>(r,c)[0];
		p.y = left_mapping.at<cv::Vec2i>(r,c)[1];

		montage_mat.at<cv::Vec3b>(r_l, c) = left_mat.at<cv::Vec3b>(p.y, p.x);
	    }
	}

	//rear
	for (int r = 0; r < rear_mapping.rows; r++)
	{
	    r_l = r + front_mapping.rows + left_mapping.rows;
	    for (int c = 0; c < rear_mapping.cols; c++)
	    {
		cv::Point p;
		p.x = rear_mapping.at<cv::Vec2i>(r,c)[0];
		p.y = rear_mapping.at<cv::Vec2i>(r,c)[1];

		montage_mat.at<cv::Vec3b>(r_l, c) = rear_mat.at<cv::Vec3b>(p.y, p.x);
	    }
	}

	//right
	for (int r = 0; r < right_mapping.rows; r++)
	{
	    r_l = r + front_mapping.rows;
	    c_l = montage_mat.cols - right_mapping.cols;
	    for (int c = 0; c < right_mapping.cols; c++)
	    {
		cv::Point p;
		p.x = right_mapping.at<cv::Vec2i>(r,c)[0];
		p.y = right_mapping.at<cv::Vec2i>(r,c)[1];

		montage_mat.at<cv::Vec3b>(r_l, c_l + c) = right_mat.at<cv::Vec3b>(p.y, p.x);
	    }
	}

        cv::namedWindow("montage");
        cv::imshow("montage", montage_mat);
        cv::waitKey();

        cv::imwrite("img/montage.jpg", montage_mat);
    }
    else //online
    {
    }
    
    return 0;
}
