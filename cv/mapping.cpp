#include "cv.h"

#include <iostream>
#include <vector>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
	std::cout << "usage: " << argv[0] << "fs_file" << std::endl;
	return 0;
    }

    cv::FileStorage fs(argv[1], cv::FileStorage::READ);
    if (!fs.isOpened())
    {
	std::cout << "can't open the file:" << argv[1] << std::endl;
        return 0;
    }

    int row = 0;
    int col = 0;
    double angle = 0.0;
    bool manual = false;
    int manual_i = 0;
    bool found = false;
    std::string mapping_data;
    int mapping_fd = 0;
    int len = 0;

    
    std::vector<std::string> cameras;
    cameras.push_back("front");
    cameras.push_back("rear");
    cameras.push_back("left");
    cameras.push_back("right");
    
    char camera_img[32];
    char camera_manual_img[32];
    char camera_row[32];
    char camera_col[32];
    char camera_angle[32];
    char camera_mapping[32];
    std::string file_img;
    std::string file_manual_img;

    uint16_t square_size = 100;
    int img_width = 1280;
    int img_height = 800;
    int grid_row = 0;
    int grid_col = 0;
    int width_cut = 0;
    int height_cut = 0;

    fs["img_width"] >> img_width;
    fs["img_height"] >> img_height;
    fs["square_size"] >> square_size;
    fs["grid_row"] >> grid_row;
    fs["grid_col"] >> grid_col;
    fs["manual"] >> manual_i;
    fs["data"] >> mapping_data;

    width_cut = grid_col * square_size - img_width;
    height_cut = grid_row * square_size - img_height;

    if (width_cut < 0 || height_cut < 0)
    {
	std::cout << "grid_row or grid_col or square_size is too small" << std::endl;
	std::cout << "grid_row:" << grid_row << ", grid_col:" << grid_col
	    << ", square_size:" << square_size << std::endl;
	std::cout << "img_width:" << img_width << ", img_height:" << img_height << std::endl;
	return 0;
    }

    mapping_fd = open(mapping_data.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (mapping_fd < 0)
    {
	std::cout << "open error:" << strerror(errno) << std::endl;
	return 0;
    }

    manual = manual_i == 0 ? false : true;

    len = write(mapping_fd, &square_size, sizeof(square_size));

    
    for (auto camera : cameras)
    {
	std::vector<std::vector<cv::Point2f>*> *corners;
	std::vector<std::vector<std::vector<cv::Point>*>*> *rowLs;
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLs;
	std::vector<std::vector<std::vector<cv::Point>*>*> *mapping;
	std::vector<cv::Point> *rectPoints;
	std::vector<cv::Point> *chessboardPoints;

	sprintf(camera_img, "%s_img", camera.c_str());
	sprintf(camera_manual_img, "%s_manual_img", camera.c_str());
	sprintf(camera_row, "%s_row", camera.c_str());
	sprintf(camera_col, "%s_col", camera.c_str());
	sprintf(camera_angle, "%s_angle", camera.c_str());
	sprintf(camera_mapping, "%s_mapping", camera.c_str());

	fs[camera_img] >> file_img;
	fs[camera_manual_img] >> file_manual_img;
	fs[camera_row] >> row;
	fs[camera_col] >> col;
	fs[camera_angle] >> angle;

	cv::Mat mat = cv::imread(file_img);
	cv::Mat manual_mat = cv::imread(file_manual_img);

	if (mat.cols == 0 || mat.rows == 0)
	{
	    std::cout << camera.c_str() << ": Bad file " << file_img << std::endl;
	    continue;
	}
	cv::Size patternSize(col, row);
	cv::Mat grids = cv::Mat::zeros((row-1)*square_size, (col-1)*square_size, mat.type());
	
	corners = new std::vector<std::vector<cv::Point2f>*>();
	rowLs = new std::vector<std::vector<std::vector<cv::Point>*>*>();
	colLs = new std::vector<std::vector<std::vector<cv::Point>*>*>();
	mapping = new std::vector<std::vector<std::vector<cv::Point>*>*>();
	rectPoints = new std::vector<cv::Point>();
	chessboardPoints = new std::vector<cv::Point>();

	for (int i = 0; i < row; i++)
	{
	    std::vector<cv::Point2f> *points = new std::vector<cv::Point2f>();
	    corners->push_back(points);
	}

	for (int i = 0; i < row; i++)
	{
	    std::vector<std::vector<cv::Point>*> *rows = 
		new std::vector<std::vector<cv::Point>*>();
	    
	    rowLs->push_back(rows);
	    for (int j = 0; j < col -1; j++)
	    {
		std::vector<cv::Point> *points = new std::vector<cv::Point>();
		rowLs->at(i)->push_back(points);
	    }
	}

	for (int i = 0; i < col; i++)
	{
	    std::vector<std::vector<cv::Point>*> *rows = 
		new std::vector<std::vector<cv::Point>*>();
	    
	    colLs->push_back(rows);
	    for (int j = 0; j < row -1; j++)
	    {
		std::vector<cv::Point> *points = new std::vector<cv::Point>();
		colLs->at(i)->push_back(points);
	    }
	}

	for (int i = 0; i < row - 1; i++)
	{
	    std::vector<std::vector<cv::Point>*> *rows =
		new std::vector<std::vector<cv::Point>*>();
	    
	    mapping->push_back(rows);
	    for (int j = 0; j < col -1; j++)
	    {
		std::vector<cv::Point> *points = new std::vector<cv::Point>();
		mapping->at(i)->push_back(points);
	    }
	}

	found = findChessboardCorners(manual ? manual_mat : mat, patternSize, corners, manual);
	if (!found)
	{
	    std::cout << camera.c_str() << ":findChessboardCorners fail..." << std::endl;
	    goto out;
	}
	std::cout << "findChessboardCorners OK" << std::endl; 

	getChessboardRectanglePoints(mat, corners, rectPoints);
	std::cout << "getChessboardRectanglePoints OK" << std::endl;

	getChessboardLinePoints(rectPoints, chessboardPoints, corners, patternSize);
	std::cout << "getChessboardLinePoints OK" << std::endl;

	fillChessboardLines(chessboardPoints, rowLs, colLs, corners, patternSize);
	std::cout << "fillChessboardLines OK" << std::endl;

	fixChessboardLines(rowLs, colLs);
	std::cout << "fixChessboardLines OK" << std::endl;

	getChessboardGrids(grids, {col-1, row-1}, square_size, mat, rowLs, colLs, mapping);
	std::cout << "getChessboardGrids OK" << std::endl;
	
	saveChessboardGridsMapping(mapping_fd, mapping, angle, width_cut, height_cut);

	std::cout << camera.c_str() << ": ok......" << std::endl;

out:
	for (int i = 0; i < row; i++)
	{
	    delete corners->at(i);
	}
	delete corners;

	for (int i = 0; i < row; i++)
	{
	    for (int j = 0; j < col -1; j++)
	    {
		delete rowLs->at(i)->at(j);
	    }

	    delete rowLs->at(i);
	}
	delete rowLs;

	for (int i = 0; i < col; i++)
	{
	    for (int j = 0; j < row -1; j++)
	    {
		delete colLs->at(i)->at(j);
	    }
	    delete colLs->at(i);
	}
	delete colLs;

	for (int i = 0; i < row - 1; i++)
	{
	    for (int j = 0; j < col -1; j++)
	    {
		delete mapping->at(i)->at(j);
	    }
	    delete mapping->at(i);
	}
	delete mapping;

	delete rectPoints;
	delete chessboardPoints;
    }

    return 0;
}
