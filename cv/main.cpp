#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <vector>
#include <limits>

void initChessboarLine(std::vector<cv::Point> *rowL,
	std::vector<cv::Point> *colL,
	std::vector<cv::Point2f> corners,
	cv::Size size)
{
    for (int r = 0; r < size.height; r++)
    {
	for (int c = 0; c < size.width - 1; c++)
	{
	    (rowL + (size.width-1)*r + c)->push_back({(int)round(corners[size.width*r+c].x), (int)round(corners[size.width*r+c].y)});
	    (rowL + (size.width-1)*r + c)->push_back({(int)round(corners[size.width*r+c+1].x), (int)round(corners[size.width*r+c+1].y)});
	}
    }

    for (int r = 0; r < size.height - 1; r++)
    {
	for (int c = 0; c < size.width; c++)
	{
	    (colL + size.width*r + c)->push_back({(int)round(corners[size.width*r+c].x), (int)round(corners[size.width*r+c].y)});
	    (colL + size.width*r + c)->push_back({(int)round(corners[size.width*(r+1)+c].x), (int)round(corners[size.width*(r+1)+c].y)});
	}
    }
}

void fixChessboarLine(std::vector<cv::Point> *rowL,
	int rows,
	std::vector<cv::Point> *colL,
	int cols)
{
    for (int i = 0; i < rows; i++)
    {
	std::vector<cv::Point> points;
	points.push_back((rowL + i)->at(0));
	
	for (int j = 1; j < (rowL + i)->size(); j++)
	{
	    int xDistance = (rowL + i)->at(j).x - (rowL + i)->at(j-1).x;
	    int yDistance = (rowL + i)->at(j).y - (rowL + i)->at(j-1).y;
	    for (int k = 1; k < xDistance; k++)
	    {
		 points.push_back({(rowL + i)->at(j-1).x + k, (rowL + i)->at(j-1).y + k * yDistance / xDistance});
	    }
	    points.push_back((rowL + i)->at(j));
	}

	*(rowL + i) = points;
    }

    for (int i = 0; i < cols; i++)
    {
	std::vector<cv::Point> points;
	points.push_back((colL + i)->at(0));
	
	for (int j = 1; j < (colL + i)->size(); j++)
	{
	    int xDistance = (colL + i)->at(j).x - (colL + i)->at(j-1).x;
	    int yDistance = (colL + i)->at(j).y - (colL + i)->at(j-1).y;
	    for (int k = 1; k < yDistance; k++)
	    {
		 points.push_back({(colL + i)->at(j-1).x + k * xDistance / yDistance, (colL + i)->at(j-1).y + k});
	    }
	    points.push_back((colL + i)->at(j));
	}

	*(colL + i) = points;
    }
}


void fillChessboarLine(std::vector<cv::Point> points,
	std::vector<cv::Point> *rowL,
	std::vector<cv::Point> *colL,
	std::vector<cv::Point2f> corners,
	cv::Size size)
{
    std::cout << "points.size()=" << points.size() << std::endl;
    int n = 0;

    for (auto point : points)
    {
	float distance = std::numeric_limits<float>::max();
	int row = 0;
	int col = 0;
	float tmp = 0; 
	float a1 = 0;
	float a2 = 0;
	float b1 = 0;
	float b2 = 0;
	n++;
	//std::cout << n << std::endl;

	for (int i = 0; i < size.height; i++)
	{
	    for (int j = 0; j < size.width; j++)
	    {
                tmp = std::pow(point.x - corners[size.width*i+j].x, 2) + std::pow(point.y - corners[size.width*i+j].y, 2);
		if (distance > tmp)
		{
		    distance = tmp;
		    row = i;
		    col = j;
	        }
		//std::cout << "tmp:" << tmp << "    ";
	    }
	    //std::cout << std::endl;
	}

	//std::cout << "(" << point.x << "," << point.y << ")" << std::endl;
        //std::cout << "row = " << row << ", col = " << col << std::endl;
#if 1
	//center
	if (col != 0 && col != (size.width -1) && row != 0 && row != (size.height -1))
	{
#if 1
            //std::cout << "center" << std::endl;

            a1 = (corners[size.width*(row+1)+col+1].y - corners[size.width*row+col].y) / (corners[size.width*(row+1)+col+1].x - corners[size.width*row+col].x);
	    b1 = corners[size.width*row+col].y - a1 * corners[size.width*row+col].x;
	    
	    a2 = (corners[size.width*(row-1)+col+1].y - corners[size.width*row+col].y) / (corners[size.width*(row-1)+col+1].x - corners[size.width*row+col].x);
            b2 = corners[size.width*row+col].y - a2 * corners[size.width*row+col].x;
            
	    if (point.y > a1*point.x + b1)
	    {
		if (point.y > a2*point.x + b2)
		{
		    (colL + size.width*(row-1) + col)->push_back(point);
		}
		else
		{
		    (rowL + (size.width-1)*row + col)->push_back(point);
		}
	    }
	    else
	    {
		if (point.y > a2*point.x + b2)
		{
		   (rowL + ((size.width-1)*row + col - 1))->push_back(point);
		}
		else
		{
                   (colL + size.width*row + col)->push_back(point);
		}
	    }
#endif
	}
	//left
	else if (col == (size.width -1) && row != 0 && row != (size.height - 1))
	{
#if 1
	    //std::cout << "left" << std::endl;

	    a1 = (corners[size.width*(row-1)+col-1].y - corners[size.width*row+col].y) / (corners[size.width*(row-1)+col-1].x - corners[size.width*row+col].x);
	    b1 = corners[size.width*row+col].y - a1 * corners[size.width*row+col].x;
	    
	    a2 = (corners[size.width*(row+1)+col-1].y - corners[size.width*row+col].y) / (corners[size.width*(row+1)+col-1].x - corners[size.width*row+col].x);
            b2 = corners[size.width*row+col].y - a2 * corners[size.width*row+col].x;

	    if (point.y > a1*point.x + b1)
	    {
                (colL + size.width*(row-1) + col)->push_back(point);
	    }
	    else
	    {
                if (point.y > a2*point.x + b2)
		{
		    (rowL + (size.width-1)*row + col - 1)->push_back(point);
		}
		else
		{
		    (colL + (size.width)*row + col)->push_back(point);
		}
	    }
#endif
	}
	//right
	else if (col == 0 && row != 0 && row != (size.height - 1))
	{
#if 1
	    //std::cout << "right" << std::endl;

            a1 = (corners[size.width*(row+1)+1].y - corners[size.width*row].y) / (corners[size.width*(row+1)+1].x - corners[size.width*row].x );
	    b1 = corners[size.width*row+col].y - a1 * corners[size.width*row+col].x;
	    
	    a2 = (corners[size.width*(row-1)+1].y - corners[size.width*row].y) / (corners[size.width*(row-1)+1].x - corners[size.width*row].x);
            b2 = corners[size.width*row].y - a2 * corners[size.width*row].x;

	    if (point.y > a1*point.x + b1)
	    {
		if (point.y > a2*point.x + b2)
		{
		    (colL + size.width*(row-1))->push_back(point);
		}
		else
		{
		    (rowL + (size.width-1)*row)->push_back(point);
		}
	    }
	    else
	    {
		(colL + size.width*row)->push_back(point);
	    }
#endif
	}
	//botton
	else if (row == 0 && col != 0 && col != (size.width -1))
	{
#if 1
	    //std::cout << "botton" << std::endl;

            a1 = (corners[size.width+col+1].y - corners[col].y) / (corners[size.width+col+1].x - corners[col].x );
	    b1 = corners[col].y - a1 * corners[col].x;
	    
	    a2 = (corners[size.width+col-1].y - corners[col].y) / (corners[size.width+col-1].x - corners[col].x);
            b2 = corners[col].y - a2 * corners[col].x;

	    if (point.y > a1*point.x + b1)
	    {
		(rowL+ col)->push_back(point);
	    }
	    else
	    {
		if (point.y > a2*point.x + b2)
		{
                    (rowL + (col - 1))->push_back(point);
		}
		else
		{
		    (colL + col)->push_back(point);
		}
	    }
#endif
	}
	//top
	else if (row == (size.height - 1) && col != 0 && col != (size.width -1))
	{
#if 1
	    //std::cout << "top" << std::endl;

            a1 = (corners[size.width*(row-1)+col-1].y - corners[size.width*row+col].y) / (corners[size.width*(row-1)+col-1].x - corners[size.width*row+col].x );
	    b1 = corners[size.width*row+col].y - a1 * corners[size.width*row+col].x;
	    
	    a2 = (corners[size.width*(row-1)+col+1].y - corners[size.width*row+col].y) / (corners[size.width*(row-1)+col+1].x - corners[size.width*row+col].x);
            b2 = corners[size.width*row+col].y - a2 * corners[size.width*row+col].x;

	    if (point.y > a1*point.x + b1)
	    {
		if (point.y > a2*point.x + b2)
		{
                    (colL + size.width*(row-1) + col)->push_back(point);
		}
		else
		{
		    (rowL + (size.width-1)*row + col)->push_back(point);
		}
	    }
	    else
	    {
		(rowL + (size.width-1)*row + col - 1)->push_back(point);
	    }
#endif
	}
	//right-botton
	else if (row == 0 && col == 0)
	{
#if 1
	    //std::cout << "right-botton" << std::endl;

            a1 = (corners[size.width+1].y - corners[0].y) / (corners[size.width+1].x - corners[0].x );
	    b1 = corners[0].y - a1 * corners[0].x;
	    if (point.y > a1*point.x + b1)
	    {
		rowL->push_back(point);
	    }
	    else
	    {
		colL->push_back(point);
	    }
#endif
	}
	//left-botton
	else if (row == 0 && col == (size.width - 1))
	{
#if 1
	    //std::cout << "left-botton" << std::endl;

            a2 = (corners[size.width+col-1].y - corners[col].y) / (corners[size.width+col-1].x - corners[col].x);
            b2 = corners[col].y - a2 * corners[col].x;
	    if (point.y > a2*point.x + b2)
	    {
		(rowL + col - 1)->push_back(point);
	    }
	    else
	    {
		(colL + col)->push_back(point);
	    }
#endif
	}
	//right-top
	else if (row == (size.height - 1) && col == 0)
	{
#if 1
	    //std::cout << "right-top" << std::endl;

            a2 = (corners[size.width*(row-1)+1].y - corners[size.width*row].y) / (corners[size.width*(row-1)+1].x - corners[size.width*row].x);
            b2 = corners[size.width*row].y - a2 * corners[size.width*row].x;
	    if (point.y > a2*point.x + b2)
	    {
		(colL + size.width*(row-1))->push_back(point);
	    }
	    else
	    {
		(rowL + (size.width-1)*row)->push_back(point);
	    }
#endif
	}
	//left-top
	else if (row == (size.height - 1) && col == (size.width - 1))
	{
#if 1
	    //std::cout << "left-top" << std::endl;

            a1 = (corners[size.width*(row-1)+col-1].y - corners[size.width*row+col].y) / (corners[size.width*(row-1)+col-1].x - corners[size.width*row+col].x );
	    b1 = corners[size.width*row+col].y - a1 * corners[size.width*row+col].x;
	    if (point.y > a1*point.x + b1)
	    {
		(colL + size.width*(row-1) + col)->push_back(point);
	    }
	    else
	    {
		(rowL + (size.width-1)*row + (col-1))->push_back(point);
	    }
#endif
	}
#endif
    }
}

void getChessboardGridSidePoints(std::vector<cv::Point> *points, cv::Point start, int side, int direction, int reference)
{
    for (int i = 0; i < reference; i++)
    {
	if (direction == 0)
	{
            points->push_back({start.x + (int)round((float)(i * (side - 1)) / (reference - 1)), start.y});
	}
	else
	{
            points->push_back({start.x, start.y + (int)round((float)(i * (side - 1)) / (reference - 1))});
	}
    }
}

void getLinePoints(std::vector<cv::Point> *points, cv::Point start, cv::Point end, int direction)
{
    float x, y;

    if (direction == 0)
    {
	if (start.x == end.x)
	{
	    for (int i = 0; i <= end.y - start.y; i++)
	    {
		x = start.x;
		y = start.y + i;
		points->push_back({(int)x, (int)y});
	    }
	}
	else
	{
	    float a = (float)(start.y - end.y) / (float)(start.x - end.x);
	    float b = start.y - a * start.x;
	    
	    for (int i = 0; i <= end.y - start.y; i++)
	    {
		y = start.y + i;
		x = round((y - b) / a);
		points->push_back({(int)x, (int)y});
	    }
	}
    }
    else
    {
        if (start.y == end.y)
	{
	    for (int i = 0; i <= end.x - start.x; i++)
	    {
		x = start.x + i;
		y = start.y;
		points->push_back({(int)x, (int)y});
	    }
	}
	else
	{
	    float a = (float)(start.y - end.y) / (float)(start.x - end.x);
	    float b = start.y - a * start.x;
	    
	    for (int i = 0; i <= end.x - start.x; i++)
	    {
		x = start.x + i;
		y = round(a * x + b);
		points->push_back({(int)x, (int)y});
	    }
	}
    }
}

void getLinePoints(std::vector<cv::Point> *points, cv::Point start, cv::Point end, int direction, int reference)
{
    float x, y;

    if (direction == 0)
    {
	if (start.x == end.x)
	{
	    for (int i = 0; i < reference; i++)
	    {
		x = start.x;
		y = round((float)(start.y + i * (end.y - start.y)) / (float)reference);
		points->push_back({(int)x, (int)y});
	    }
	}
	else
	{
	    float a = (float)(start.y - end.y) / (float)(start.x - end.x);
	    float b = start.y - a * start.x;
	    
	    for (int i = 0; i < reference; i++)
	    {
		y = start.y + round((float)(i * (end.y - start.y)) / (reference - 1));
		x = round((y - b) / a);

		points->push_back({(int)x, (int)y});
	    }
	}
    }
    else
    {
        if (start.y == end.y)
	{
	    for (int i = 0; i < reference; i++)
	    {
		y = start.y;
		x = round((float)(start.x + i * (end.x - start.x)) / (float)reference);
		points->push_back({(int)x, (int)y});
	    }
	}
	else
	{
	    float a = (float)(start.y - end.y) / (float)(start.x - end.x);
	    float b = start.y - a * start.x;
	    
	    for (int i = 0; i < reference; i++)
	    {
		x = start.x + round((float)(i * (end.x - start.x)) / (reference - 1));
		y = round (a * x + b);

		points->push_back({(int)x, (int)y});
	    }
	}
    }
}

void getChessboardGrid(cv::Mat dst, cv::Mat src, std::vector<cv::Point> top, std::vector<cv::Point> botton, std::vector<cv::Point> left, std::vector<cv::Point> right)
{
    cv::Point center_dst = {dst.cols/2, dst.rows/2};
    cv::Point center_src = {(top[top.size() / 2].x + botton[botton.size() / 2].x) / 2,
                           (left[left.size() / 2].y + right[right.size() / 2].y) / 2};

    cv::Point2f srcTri[3];
    cv::Point2f dstTri[3];
    cv::Mat warp_mat;
    warp_mat.create(2, 3, CV_32FC1);
    cv::Mat mask;
    mask.create(dst.rows, dst.cols, CV_8UC1);
    mask.setTo(cv::Scalar(255));

    //top
    dst.at<cv::Vec3b>(0,0) = src.at<cv::Vec3b>(top[0].y,top[0].x);
    dst.at<cv::Vec3b>(0,99) = src.at<cv::Vec3b>(top[top.size()-1].y,top[top.size()-1].x);
    dst.at<cv::Vec3b>(center_dst.y,center_dst.x) = src.at<cv::Vec3b>(center_src.y,center_src.x);
    
    dst.at<cv::Vec3b>(0,50) = src.at<cv::Vec3b>(top[(top.size()-1)/2].y,top[(top.size()-1)/2].x);

    std::vector<cv::Point> *top_points = new std::vector<cv::Point>();
    getChessboardGridSidePoints(top_points, {0,0}, dst.cols, 0, top.size());

    for (int i = 0; i < top.size(); i++)
    {
	std::vector<cv::Point> *line_points_src = new std::vector<cv::Point>();
	std::vector<cv::Point> *line_points_dst = new std::vector<cv::Point>();
	
	getLinePoints(line_points_src, top[i], center_src, 0);
	getLinePoints(line_points_dst, top_points->at(i), center_dst, 0, line_points_src->size());

	for (int j = 0; j < line_points_src->size(); j++)
	{
	    dst.at<cv::Vec3b>(line_points_dst->at(j).y, line_points_dst->at(j).x) = src.at<cv::Vec3b>(line_points_src->at(j).y, line_points_src->at(j).x);
	}

	delete line_points_src;
	delete line_points_dst;
    }
    delete top_points;
    
    //bottton
    dst.at<cv::Vec3b>(0,99) = src.at<cv::Vec3b>(top[0].y,top[0].x);
    dst.at<cv::Vec3b>(99,99) = src.at<cv::Vec3b>(top[top.size()-1].y,top[top.size()-1].x);
    dst.at<cv::Vec3b>(center_dst.y,center_dst.x) = src.at<cv::Vec3b>(center_src.y,center_src.x);
    
    dst.at<cv::Vec3b>(99,50) = src.at<cv::Vec3b>(top[(top.size()-1)/2].y,top[(top.size()-1)/2].x);
    
    std::vector<cv::Point> *botton_points = new std::vector<cv::Point>();
    getChessboardGridSidePoints(botton_points, {0, 99}, dst.cols, 0, botton.size());
    
    for (int i = 1; i < botton.size(); i++)
    {
        std::vector<cv::Point> *line_points_src = new std::vector<cv::Point>();
	std::vector<cv::Point> *line_points_dst = new std::vector<cv::Point>();
	
	getLinePoints(line_points_src, center_src, botton[i], 0);
	getLinePoints(line_points_dst, center_dst, botton_points->at(i), 0, line_points_src->size());

	for (int j = 0; j < line_points_src->size(); j++)
	{
	    dst.at<cv::Vec3b>(line_points_dst->at(j).y, line_points_dst->at(j).x) = src.at<cv::Vec3b>(line_points_src->at(j).y, line_points_src->at(j).x);
	}

	delete line_points_src;
	delete line_points_dst;
    }
    delete botton_points;
 
    //left
    dst.at<cv::Vec3b>(0,0) = src.at<cv::Vec3b>(top[0].y,top[0].x);
    dst.at<cv::Vec3b>(99,0) = src.at<cv::Vec3b>(top[top.size()-1].y,top[top.size()-1].x);
    dst.at<cv::Vec3b>(center_dst.y,center_dst.x) = src.at<cv::Vec3b>(center_src.y,center_src.x);
    
    dst.at<cv::Vec3b>(50,0) = src.at<cv::Vec3b>(top[(top.size()-1)/2].y,top[(top.size()-1)/2].x);
    
    std::vector<cv::Point> *left_points = new std::vector<cv::Point>();
    getChessboardGridSidePoints(left_points, {0, 0}, dst.cols, 1, left.size());

    for (int i = 1; i < left.size(); i++)
    {
        std::vector<cv::Point> *line_points_src = new std::vector<cv::Point>();
	std::vector<cv::Point> *line_points_dst = new std::vector<cv::Point>();
	
	getLinePoints(line_points_src, left[i], center_src, 1);
	getLinePoints(line_points_dst, left_points->at(i), center_dst, 1, line_points_src->size());

	for (int j = 0; j < line_points_src->size(); j++)
	{
	    dst.at<cv::Vec3b>(line_points_dst->at(j).y, line_points_dst->at(j).x) = src.at<cv::Vec3b>(line_points_src->at(j).y, line_points_src->at(j).x);
	}

	delete line_points_src;
	delete line_points_dst;
    }
    delete left_points;

    //right
    dst.at<cv::Vec3b>(0,99) = src.at<cv::Vec3b>(top[0].y,top[0].x);
    dst.at<cv::Vec3b>(99,99) = src.at<cv::Vec3b>(top[top.size()-1].y,top[top.size()-1].x);
    dst.at<cv::Vec3b>(center_dst.y,center_dst.x) = src.at<cv::Vec3b>(center_src.y,center_src.x);
    
    dst.at<cv::Vec3b>(50,99) = src.at<cv::Vec3b>(top[(top.size()-1)/2].y,top[(top.size()-1)/2].x);
    
    std::vector<cv::Point> *right_points = new std::vector<cv::Point>();
    getChessboardGridSidePoints(right_points, {99, 0}, dst.cols, 1, right.size());
    
    for (int i = 1; i < right.size(); i++)
    {
        std::vector<cv::Point> *line_points_src = new std::vector<cv::Point>();
	std::vector<cv::Point> *line_points_dst = new std::vector<cv::Point>();
	
	getLinePoints(line_points_src, center_src, right[i], 1);
	getLinePoints(line_points_dst, center_dst, right_points->at(i), 1, line_points_src->size());

	for (int j = 0; j < line_points_src->size(); j++)
	{
	    dst.at<cv::Vec3b>(line_points_dst->at(j).y, line_points_dst->at(j).x) = src.at<cv::Vec3b>(line_points_src->at(j).y, line_points_src->at(j).x);
	}

	delete line_points_src;
	delete line_points_dst;
    }
    delete right_points;

    for (int i = 0; i < dst.rows; i++)
    {
	for (int j = 0; j < dst.cols; j++)
	{
	    if (dst.at<cv::Vec3b>(i, j) == cv::Vec3b(0,0,0))
	    {
		//dst.at<cv::Vec3b>(i, j) = dst.at<cv::Vec3b>(center_dst.y, center_dst.x);
	    }
	    //dst.at<cv::Vec3b>(i, j) = dst.at<cv::Vec3b>(center_dst.y, center_dst.x);
	}
    }
}

void getChessboardGrids(cv::Mat dst, cv::Size size, int side, cv::Mat src, std::vector<cv::Point> *rowL, std::vector<cv::Point> *colL)
{
    for (int i = 0; i < size.height; i++)
    {
	for (int j = 0; j < size.width; j++)
	{
	    cv::Mat grid = cv::Mat::zeros(side, side, src.type());
	    getChessboardGrid(grid, src, *(rowL+size.width*(i+1)+j), *(rowL+size.width*i+j), *(colL + (size.width+1)*i+j+1), *(colL+(size.width+1)*i+j));
	    IplImage ipl_grid = IplImage(grid);
	    IplImage ipl_grids = IplImage(dst);
	    CvRect roi_grid =cvRect(0, 0, side, side);
	    CvRect roi_grids =cvRect(side*(size.width-1-j), side*(size.height-1-i), side, side);
	    cvSetImageROI(&ipl_grid, roi_grid);
	    cvSetImageROI(&ipl_grids, roi_grids);
	    cvCopy(&ipl_grid, &ipl_grids); 
	}
    }
}

int main(int argc, char **argv)
{
    //std::vector<cv::Point> rowL[8][6];
    //std::vector<cv::Point> colL[9][5];
    std::vector<cv::Point> *rowL = new std::vector<cv::Point>[48];
    std::vector<cv::Point> *colL = new std::vector<cv::Point>[45];
    char name[255];
    if (argc > 1)
    {
	sprintf(name, "Chessboard/%s.jpg", argv[1]);
    }
    else
    {
	sprintf(name, "Chessboard/1.jpg");
    }
    cv::Mat gray = cv::imread(name);
#if 1
    cv::Size patternsize(9, 6);
    std::vector<cv::Point2f> corners;

    bool patternfound = cv::findChessboardCorners(gray, patternsize, corners,
	    cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE
	    /*+ cv::CALIB_CB_FAST_CHECK*/);

    if (patternfound)
    {
	initChessboarLine(rowL, colL, corners, patternsize);
	//cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
	//	cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
    }
    else
    {
	std::cout << "cv::findChessboardCorners() reutrn false" << std::endl;
    }

    //cv::drawChessboardCorners(gray, patternsize, cv::Mat(corners), patternfound);

    cv::Mat img;
    cv::Mat img_dst;
    cv::Mat img_mask;
    cv::Mat img_show;
    img.create(gray.rows, gray.cols, CV_8UC1);
    img_dst.create(gray.rows, gray.cols, CV_8UC1);
    img_mask.create(gray.rows, gray.cols, CV_8UC1);
    img_mask.setTo(cv::Scalar(255));
    //gray.copyTo(img_show);
    

    cv::cvtColor(gray, img, CV_BGR2GRAY);
    cv::Canny(img, img_dst, 50, 150, 3);
    int x_min = gray.cols;
    int x_max = 0;
    int y_min = gray.cols;
    int y_max = 0;
    std::vector<cv::Point> points;
    std::vector<cv::Point> points_line;

    std::cout << img_dst.channels() << std::endl;

#if 1
    for (const auto point : corners)
    {
	//std::cout << point.x << ", " << point.y << std::endl;
	if (x_min > point.x)
	{
	    x_min = point.x;
	}

	if (x_max < point.x)
	{
	    x_max = point.x;
	}

	if (y_min > point.y)
	{
	    y_min = point.y;
	}

	if (y_max < point.y)
	{
	    y_max = point.y;
	}
    }

    std::cout << x_min << " " << x_max << " " << y_min << " " <<  y_max << std::endl;
    int width = (x_max - x_min)/16;
    int hight = (y_max - y_min)/10;
    x_min -= width;
    x_max += width;
    y_min -= hight;
    y_max += hight;

    for (int i = y_min; i <= y_max; i++)
    {
        for (int j = x_min; j <= x_max; j++)
	{
	    int color = img_dst.at<uchar>(i,j);
	    //std::cout << i << "," << j << "=" << color << std::endl;
	    if (color > 0)
	    {
		*(img_dst.data + img_dst.cols * i + j) = 100;
		points.push_back({i,j});
	    }
	}
    }

    bool remove = false;
    for (auto point : points)
    {
	remove = false;

        //left
	for (int i = 0; i < 6; i++)
	{
	    float fix = std::fabs(corners[8+i*9].y - corners[7+i*9].y)/std::fabs(corners[8+i*9].x - corners[7+i*9].x);
	    fix *= 2;
	    if(fix < 0.9)
	    {
		fix = 0.9;
	    }
	    else if (fix > 2.0)
	    {
		fix = 2.0;
	    }

	    if (point.y < corners[8+i*9].x 
		&& (point.x >= (corners[8+i*9].y - (corners[8+i*9].x - point.y)*fix) && point.x <= (corners[8+i*9].y + (corners[8+i*9].x - point.y)*fix)))
		{
		    remove = true;
		    goto out;
		}
	}

	//right
	for (int i = 0; i < 6; i++)
	{
	    float fix = std::fabs(corners[i*9].y - corners[1+i*9].y)/std::fabs(corners[i*9].x - corners[1+i*9].x);
	    fix *= 2;
	    if (fix < 0.9)
	    {
		fix = 0.9;
	    }
	    else if (fix > 2.0)
	    {
		fix = 2.0;
	    }

	    if (point.y > corners[i*9].x 
		&& (point.x >= (corners[i*9].y - (point.y - corners[i*9].x)*fix) && point.x <= (corners[i*9].y + (point.y - corners[i*9].x)*fix)))
		{
		    remove = true;
		    goto out;
		}
	}

	//botton
	for (int i = 0; i < 9; i++)
	{
	    float fix = std::fabs(corners[i].x - corners[9+i].x)/std::fabs(corners[i].y - corners[9+i].y);
	    fix *= 2;
	    if (fix < 0.9)
	    {
		fix = 0.9;
	    }
	    else if (fix > 2.0)
	    {
		fix = 2.0;
	    }

            if (point.x > corners[i].y
		&& (point.y >= (corners[i].x - (point.x - corners[i].y)*fix) && point.y <= (corners[i].x + (point.x - corners[i].y)*fix)))
		{
		    remove = true;
		    goto out;
		}
	}

	//top
        for (int i = 0; i < 9; i++)
	{
	    float fix = std::fabs(corners[45+i].y - corners[36+i].y)/std::fabs(corners[45+i].x - corners[36+i].x);
	    fix *= 2;
	    if (fix < 0.9)
	    {
		fix = 0.9;
	    }
	    else if (fix > 2.0)
	    {
		fix = 2;
	    }

            if (point.x < corners[45+i].y
		&& (point.y >= (corners[45+i].x - (corners[45+i].y - point.x)*fix) && point.y <= (corners[45+i].x + (corners[45+i].y - point.x)*fix)))
		{
		    remove = true;
		    goto out;
		}
	}

out:
	if (!remove)
	{
	    //gray.at<cv::Vec3b>(point.x,point.y)[0] = 0;
	    //gray.at<cv::Vec3b>(point.x,point.y)[1] = 0;
	    //gray.at<cv::Vec3b>(point.x,point.y)[2] = 255;
	    points_line.push_back({point.y, point.x});
	}
    }
    
    fillChessboarLine(points_line, (std::vector<cv::Point>*)rowL, (std::vector<cv::Point>*)colL, corners, patternsize); 

    struct 
    {
	bool operator()(cv::Point a, cv::Point b)
	{
	    return a.x < b.x;
	}
    } xLess;

    struct 
    {
	bool operator()(cv::Point a, cv::Point b)
	{
	    return a.y < b.y;
	}
    } yLess;

    for (int r = 0; r < 6; r++)
    {
	for (int c = 0; c < 8; c++)
	{
	    std::sort((rowL+8*r+c)->begin(), (rowL+8*r+c)->end(), xLess);

	    for (int i = 1; i < (rowL+8*r+c)->size(); i++)
	    {
		CvScalar color = {{0,0,255}};
		//cv::line(gray, (rowL+8*r+c)->at(i-1), (rowL+8*r+c)->at(i), color, 1, 16, 0);

		if (r == 0 || r == 5)
		{
		    cv::line(img_mask, (rowL+8*r+c)->at(i-1), (rowL+8*r+c)->at(i), cv::Scalar::all(0), 1, 16, 0);
		}
	    }
	}
    }
    
    for (int r = 0; r < 5; r++)
    {
	for (int c = 0; c < 9; c++)
	{
	    std::sort((colL+9*r+c)->begin(), (colL+9*r+c)->end(), yLess);

	    for (int i = 1; i < (colL+9*r+c)->size(); i++)
	    {
		CvScalar color = {{0,0,255}};
		//cv::line(gray, (colL+9*r+c)->at(i-1), (colL+9*r+c)->at(i), color, 1, 16, 0);
		if (c == 0 || c == 8)
		{
		    cv::line(img_mask, (colL+9*r+c)->at(i-1), (colL+9*r+c)->at(i), cv::Scalar::all(0), 1, 16, 0);
		}
	    }
	}
    }

    //fixChessboarLine(rowL, 48, colL, 45);
#if 0
    for (int r = 0; r < 6; r++)
    {
	for (int c = 0; c < 8; c++)
	{

	    for (int i = 1; i < (rowL+8*r+c)->size(); i++)
	    {
		CvScalar color = {{0,0,255}};
		cv::line(gray, (rowL+8*r+c)->at(i-1), (rowL+8*r+c)->at(i), color, 1, 16, 0);
	    }
	}
    }
    
    for (int r = 0; r < 5; r++)
    {
	for (int c = 0; c < 9; c++)
	{
	    for (int i = 1; i < (colL+9*r+c)->size(); i++)
	    {
		CvScalar color = {{0,0,255}};
		cv::line(gray, (colL+9*r+c)->at(i-1), (colL+9*r+c)->at(i), color, 1, 16, 0);
	    }
	}
    }
#endif

#endif

#if 1
    //CvScalar color = {{0,0,255}};
    //cv::line(gray, corners[0], corners[1], color, 1, 16, 0);
    //cv::line(gray, corners[1], corners[2], color, 1, 16, 0);
    //cv::line(gray, corners[9], corners[10], color, 1, 16, 0);
    //cv::line(gray, corners[10], corners[11], color, 1, 16, 0);

#endif

    //cv::imwrite("Chessboard/3_corner.jpg", img_dst);
    cv::namedWindow("corner");
    cv::imshow("corner", gray);

    cv::floodFill(img_mask, cv::Point(1, 1), cv::Scalar::all(0));
    gray.copyTo(img_show, img_mask);
    //cv::namedWindow("mask");
    //cv::imshow("mask", img_mask);
    //cv::namedWindow("show");
    //cv::imshow("show", img_show);

#if 0
    cv::Point2f srcTri[3] = {{corners[53].x,corners[53].y},{corners[45].x, corners[45].y},{corners[0].x, corners[0].y}};
    cv::Point2f dstTri[3] = {{corners[8].x,corners[45].y},{corners[0].x, corners[45].y},{corners[0].x, corners[0].y}};
    cv::Mat warp_mat;
    warp_mat.create(2, 3, CV_32FC1);
    warp_mat = cv::getAffineTransform(srcTri, dstTri);
    cv::Mat warp = cv::Mat::zeros(gray.rows, gray.cols, gray.type());
    cv::warpAffine(gray, warp, warp_mat, warp.size());
    cv::namedWindow("warp");
    cv::imshow("warp", warp);
#endif
    fixChessboarLine(rowL, 48, colL, 45);
    cv::Mat grids = cv::Mat::zeros(500, 800, gray.type());
    getChessboardGrids(grids, {8,5}, 100, gray, rowL, colL); 

    cv::namedWindow("grids");
    cv::imshow("grids", grids);

    cv::imwrite("Chessboard/show.jpg", img_show);

    cv::imwrite("Chessboard/grids.jpg", grids);

    cv::waitKey();
#endif

    return 0;
}
