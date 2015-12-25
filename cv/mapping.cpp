#include "cv.h"

#include <iostream>
#include <vector>

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
    bool found = false;

    
    std::vector<std::string> cameras;
    cameras.push_back("front");
    cameras.push_back("rear");
    cameras.push_back("left");
    cameras.push_back("right");
    
    char camera_img[32];
    char camera_row[32];
    char camera_col[32];
    char camera_angle[32];
    char camera_mapping[32];
    std::string file_img;
    std::string file_fs;
    int square_size = 100;

    fs["square_size"] >> square_size;
    
    for (auto camera : cameras)
    {
	std::vector<std::vector<cv::Point2f>*> *corners;
	std::vector<std::vector<std::vector<cv::Point>*>*> *rowLs;
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLs;
	std::vector<std::vector<std::vector<cv::Point>*>*> *mapping;
	std::vector<cv::Point> *rectPoints;
	std::vector<cv::Point> *chessboardPoints;

	sprintf(camera_img, "%s_img", camera.c_str());
	sprintf(camera_row, "%s_row", camera.c_str());
	sprintf(camera_col, "%s_col", camera.c_str());
	sprintf(camera_angle, "%s_angle", camera.c_str());
	sprintf(camera_mapping, "%s_mapping", camera.c_str());

	fs[camera_img] >> file_img;
	fs[camera_row] >> row;
	fs[camera_col] >> col;
	fs[camera_angle] >> angle;
	fs[camera_mapping] >> file_fs;

	cv::Mat mat = cv::imread(file_img);
	if (mat.cols == 0 || mat.rows == 0)
	{
	    std::cout << camera.c_str() << ": Bad file " << file_img << std::endl;
	    continue;
	}
	cv::FileStorage camera_fs(file_fs, cv::FileStorage::WRITE);
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

	found = findChessboardCorners(mat, patternSize, corners, manual);
	if (!found)
	{
	    std::cout << camera.c_str() << ":findChessboardCorners fail..." << std::endl;
	    goto out;
	}

	getChessboardRectanglePoints(mat, corners, rectPoints);


	getChessboardLinePoints(rectPoints, chessboardPoints, corners, patternSize);

	fillChessboardLines(chessboardPoints, rowLs, colLs, corners, patternSize);
	fixChessboardLines(rowLs, colLs);

	getChessboardGrids(grids, {col-1, row-1}, square_size, mat, rowLs, colLs, mapping);
	
	saveChessboardGridsMapping(camera_fs, mapping, angle);

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
