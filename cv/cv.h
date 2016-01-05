#ifndef CV_H
#define CV_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>

struct
{
    bool operator()(cv::Point a, cv::Point b)
    {
	return a.x < b.x;
    }
} PointXLess;

struct
{
    bool operator()(cv::Point a, cv::Point b)
    {
	return a.y < b.y;
    }
} PointYLess;

bool findChessboardCorners(cv::Mat image, cv::Size patternSize,
	std::vector<std::vector<cv::Point2f>*> *corners, bool manual);

void getChessboardRectanglePoints(cv::Mat image,
	std::vector<std::vector<cv::Point2f>*> *corners,
	std::vector<cv::Point> *rectPoints);

void getChessboardLinePoints(std::vector<cv::Point> *rectPoints,
	std::vector<cv::Point> *linePoints,
	std::vector<std::vector<cv::Point2f>*> *corners,
	cv::Size patternSize);

void fixChessboardLines(std::vector<std::vector<std::vector<cv::Point>*>*> *rowLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLines);

void fillChessboardLines(std::vector<cv::Point> *points,
	std::vector<std::vector<std::vector<cv::Point>*>*> *rowLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLines,
	std::vector<std::vector<cv::Point2f>*> *corners,
	cv::Size patternSize);

void getChessboardGrid(cv::Mat dst, cv::Point locate, int side, cv::Mat src,
	std::vector<cv::Point> *top, std::vector<cv::Point> *botton,
	std::vector<cv::Point> *left, std::vector<cv::Point> *right,
	std::vector<cv::Point> *mapping);

void getChessboardGrids(cv::Mat dst, cv::Size patternSize, int side, cv::Mat src,
	std::vector<std::vector<std::vector<cv::Point>*>*> *rowLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *mapping);

void saveChessboardGridsMapping(int fd,
	std::vector<std::vector<std::vector<cv::Point>*>*> *mapping,
	double angle);

#endif
