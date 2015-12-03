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


void fillChessboarLine(std::vector<cv::Point> *points,
	std::vector<cv::Point> *rowL,
	std::vector<cv::Point> *colL,
	std::vector<cv::Point2f> corners,
	cv::Size size)
{
    int n = 0;

    for (auto point : *points)
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
	    }
	}

	//center
	if (col != 0 && col != (size.width -1) && row != 0 && row != (size.height -1))
	{
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
	}

	//left
	else if (col == (size.width -1) && row != 0 && row != (size.height - 1))
	{
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
	}

	//right
	else if (col == 0 && row != 0 && row != (size.height - 1))
	{
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
	}

	//botton
	else if (row == 0 && col != 0 && col != (size.width -1))
	{
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
	}

	//top
	else if (row == (size.height - 1) && col != 0 && col != (size.width -1))
	{
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
	}

	//right-botton
	else if (row == 0 && col == 0)
	{
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
	}

	//left-botton
	else if (row == 0 && col == (size.width - 1))
	{
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
	}

	//right-top
	else if (row == (size.height - 1) && col == 0)
	{
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
	}

	//left-top
	else if (row == (size.height - 1) && col == (size.width - 1))
	{
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
	}
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

void getChessboardGrid(cv::Mat dst, cv::Point locate, int side, cv::Mat src,
	std::vector<cv::Point> top, std::vector<cv::Point> botton,
	std::vector<cv::Point> left, std::vector<cv::Point> right)
{
    double side_f = side - 1;

    std::vector<cv::Point> *mask_points = new std::vector<cv::Point>();
    cv::Mat mask;
    mask.create(src.rows, src.cols, CV_8UC1);
    mask.setTo(cv::Scalar(255));

    std::cout << "top size:" << top.size() << std::endl;
    for (int i = 1; i < top.size(); i++)
    {
        cv::line(mask, top.at(i), top.at(i-1), cv::Scalar(0));
	std::cout << "(" << top.at(i).x << "," << top.at(i).y << ") ";
    }
    std::cout << std::endl;

    std::cout << "botton size:" << botton.size() << std::endl;
    for (int i = 1; i < botton.size(); i++)
    {
        cv::line(mask, botton.at(i), botton.at(i-1), cv::Scalar(0));
	std::cout << "(" << botton.at(i).x << "," << botton.at(i).y << ") "; 
    }
    std::cout << std::endl;

    std::cout << "left size:" << left.size() << std::endl;
    for (int i = 1; i < left.size(); i++)
    {
        cv::line(mask, left.at(i), left.at(i-1), cv::Scalar(0));
	std::cout << "(" << left.at(i).x << "," << left.at(i).y << ") ";
    }
    std::cout << std::endl;

    std::cout << "right size:" << right.size() << std::endl;
    for (int i = 1; i < right.size(); i++)
    {
        cv::line(mask, right.at(i), right.at(i-1), cv::Scalar(0));
	std::cout << "(" << right.at(i).x << "," << right.at(i).y << ") ";
    }
    std::cout << std::endl;

    cv::floodFill(mask, cv::Point(1, 1), cv::Scalar::all(0));

    for (int r = 0; r < mask.rows; r++)
    {
	for (int c = 0; c < mask.cols; c++)
	{
	    if (mask.at<uchar>(r,c) != 0)
	    {
		mask_points->push_back({c, r});
	    }
	}
    }

    for (auto point : *mask_points)
    {
    }

    delete mask_points;


#if 0
    std::vector<cv::Point> *top_line = new std::vector<cv::Point>(top);
    std::vector<cv::Point> *botton_line = new std::vector<cv::Point>(botton);
    std::vector<cv::Point> *left_line = new std::vector<cv::Point>(left);
    std::vector<cv::Point> *right_line = new std::vector<cv::Point>(right);

    cv::Point point_src;
    cv::Point point_dst;
    
    cv::Point2f top_start = {0, 0};
    cv::Point2f top_end = {(float)side - 1, 0};
    cv::Point2f botton_start = {0, (float)side - 1};
    cv::Point2f botton_end = {(float)side - 1, (float)side - 1};
    cv::Point2f left_start = {0, 0};
    cv::Point2f left_end = {0, (float)side - 1};
    cv::Point2f right_start = {(float)side - 1, 0};
    cv::Point2f right_end = {(float)side - 1, (float)side - 1};

    float top_dist = (float)side/(float)top_line->size();
    float botton_dist = (float)side/(float)botton_line->size();
    float left_dist = (float)side/(float)left_line->size();
    float right_dist = (float)side/(float)right_line->size();
    
    while (top_line->size() > 0
	    || botton_line->size() > 0
	    || left_line->size() > 0
	    || right_line->size() > 0)
    {
	std::cout << "size" << "(" <<  top_line->size() << "," <<  botton_line->size() << "," << left_line->size() << "," << right_line->size() << ")" << std::endl;
	
	//top
	for (int i = 0; i < top_line->size(); i++)
	{
	    point_src = top_line->at(i);
	    point_dst.y = (int)round(top_start.y);

	    if (top_line->size() > 1)
	    {
		point_dst.x = (int)round(top_start.x + (top_end.x-top_start.x)*i/(top_line->size()-1));
	    }
	    else
	    {
		point_dst.x = (int)round((top_start.x + top_end.x)/2);
	    }

	    std::cout << "top" << "(" <<  point_dst.x << "," <<  point_dst.y << ")" << std::endl;
	    dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    top_line->at(i).y += 1;
	}
		
	//botton
	for (int i = 0; i < botton_line->size(); i++)
	{
	    point_src = botton_line->at(i);
	    point_dst.y = (int)round(botton_start.y);

	    if (botton_line->size() > 1)
	    {
		point_dst.x = (int)round(botton_start.x + (botton_end.x-botton_start.x)*i/(botton_line->size()-1));
	    }
	    else
	    {
		point_dst.x = (int)round((botton_start.x + botton_end.x)/2);
	    }

	    std::cout << "botton" << "(" <<  point_dst.x << "," <<  point_dst.y << ")" << std::endl;
	    dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    botton_line->at(i).y -= 1;
	}

	//left
	for (int i = 0; i < left_line->size(); i++)
	{
	    point_src = left_line->at(i);
	    point_dst.x = (int)round(left_start.x);

	    if (left_line->size() > 1)
	    {
		point_dst.y = (int)round(left_start.y + (left_end.y-left_start.y)*i/(left_line->size()-1));
	    }
	    else
	    {
		point_dst.y = (int)round((left_start.y + left_end.y)/2);
	    }

	    std::cout << "left" << "(" <<  point_dst.x << "," <<  point_dst.y << ")" << std::endl;
	    dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    left_line->at(i).x += 1;
	}

	//right
	for (int i = 0; i < right_line->size(); i++)
	{
	    point_src = right_line->at(i);
	    point_dst.x = (int)round(right_start.x);

	    if (right_line->size() > 1)
	    {
		point_dst.y = (int)round(right_start.y + (right_end.y-right_start.y)*i/(right_line->size()-1));
	    }
	    else
	    {
		point_dst.y = (int)round((right_start.y + right_end.y)/2);
	    }

	    std::cout << "right" << "(" <<  point_dst.x << "," <<  point_dst.y << ")" << std::endl;
	    dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    right_line->at(i).x -= 1;
	}
#if 0
	if (top_line->size() > 0 && left_line->size() > 0 && top_line->at(0) != left_line->at(0))
	{
	    if (top_line->at(0).y > left_line->at(0).y)
	    {
		top_line->insert(top_line->begin(), left_line->at(0));
		top_start.x -= top_dist;
	    }
	    else
	    {
                left_line->insert(left_line->begin(), top_line->at(0));
		left_start.y -= left_dist;
	    }
	}

        if (top_line->size() > 0 && left_line->size() > 0 && top_line->at(top_line->size()-1) != right_line->at(0))
	{
	    if (top_line->at(top_line->size()-1).y > right_line->at(0).y)
	    {
		top_line->push_back(right_line->at(0));
		top_end.x += top_dist;
	    }
	    else
	    {
                right_line->insert(right_line->begin(), top_line->at(top_line->size()-1));
		right_start.y -= right_dist;
	    }
	}
#endif
        if (top_line->size() > 0)
	{
	    top_start.x += top_dist;
	    top_end.x -= top_dist;
	    
	    top_line->erase(top_line->begin());
	    if (top_line->size() > 0)
	    {
		top_line->erase(top_line->begin()+(top_line->size()-1));
	    }
	}
	if (botton_line->size() > 0)
	{
	    botton_start.x += botton_dist;
	    botton_end.x -= botton_dist;

	    botton_line->erase(botton_line->begin());
	    if (botton_line->size() > 0)
	    {
		botton_line->erase(botton_line->begin()+(botton_line->size()-1));
	    }
	}

	if (left_line->size() > 0)
	{
            left_start.y += left_dist;
	    left_end.y -= left_dist;
	    
	    left_line->erase(left_line->begin());
	    if (left_line->size() > 0)
	    {
		left_line->erase(left_line->begin()+(left_line->size()-1));
	    }
	}

	if (right_line->size() > 0)
	{
	    right_start.y += right_dist;
	    right_end.y -= right_dist;
	    
	    right_line->erase(right_line->begin());
	    if (right_line->size() > 0)
	    {
		right_line->erase(right_line->begin()+(right_line->size()-1));
	    }
	}

	top_start.y = left_start.y;
	top_end.y = right_start.y;
	botton_start.y = left_end.y;
	botton_end.y = right_end.y;
	left_start.x = top_start.x;
	left_end.x = botton_start.x;
	right_start.x = top_end.x;
	right_end.x = botton_end.x;	
    }

    delete top_line;
    delete botton_line;
    delete left_line;
    delete right_line;

#endif

#if 0
    cv::Point2f start;
    cv::Point2f end;
    float h;
    float v;

    std::vector<cv::Point> *top_points = new std::vector<cv::Point>(top);
    std::vector<cv::Point> *botton_points = new std::vector<cv::Point>(botton);
    std::vector<cv::Point> *left_points = new std::vector<cv::Point>(left);
    std::vector<cv::Point> *right_points = new std::vector<cv::Point>(right);
    
    cv::Point point_src;
    cv::Point point_dst;
    
    //top
    start = {0,0};
    end = {(float)side - 1, 0};
    h = (float)side/((float)top.size()+(float)botton.size())*2;
    v = (float)side/((float)top.size()+(float)botton.size())*2;

    while (top_points->size() > 0)
    {
	for (int i = 0; i < top_points->size(); i++)
	{
	    point_src = top_points->at(i);
	    point_dst.y = (int)round(start.y);
	    if (top_points->size() > 1)
	    {
	        point_dst.x = (int)round(start.x + (end.x-start.x)*i/(top_points->size()-1));
	    }
	    else
	    {
		point_dst.x = (int)round((start.x+end.x)/2);
	    }

	    if (top_points->size() == top.size())
	    {
		dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y+1, point_src.x);
	    }
	    else
	    {
                dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    }

	    top_points->at(i).y += 1;
	}
	
	top_points->erase(top_points->begin());
	if (top_points->size() > 1)
	{
	    top_points->erase(top_points->begin()+(top_points->size()-1));
	}

	start.x += h;
	start.y += v;
	end.x -= h;
	end.y += v;
    }

    //botton
    start = {0,(float)side - 1};
    end = {(float)side - 1, (float)side - 1};
    h = (float)side/((float)top.size()+(float)botton.size())*2;
    v = (float)side/((float)top.size()+(float)botton.size())*2;

    while (botton_points->size() > 0)
    {
	for (int i = 0; i < botton_points->size(); i++)
	{
	    point_src = botton_points->at(i);
	    point_dst.y = (int)round(start.y);
	    if (botton_points->size() > 1)
	    {
	        point_dst.x = (int)round(start.x + (end.x-start.x)*i/(botton_points->size()-1));
	    }
	    else
	    {
		point_dst.x = (int)round((start.x+end.x)/2);
	    }

	    if (botton_points->size() == botton.size())
	    {
		dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y -1, point_src.x);
	    }
	    else
	    {
                dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    }

	    botton_points->at(i).y -= 1;
	}
	
	botton_points->erase(botton_points->begin());
	if (botton_points->size() > 1)
	{
	    botton_points->erase(botton_points->begin()+(botton_points->size()-1));
	}

	start.x += h;
	start.y -= v;
	end.x -= h;
	end.y -= v;
    }
    
    //left
    start = {0, 0};
    end = {0, (float)side - 1};
    h = (float)side/((float)left.size()+(float)right.size())*2;
    v = (float)side/((float)left.size()+(float)right.size())*2;

    while (left_points->size() > 0)
    {
	for (int i = 0; i < left_points->size(); i++)
	{
	    point_src = left_points->at(i);
	    point_dst.x = (int)round(start.x);
	    if (left_points->size() > 1)
	    {
	        point_dst.y = (int)round(start.y + (end.y-start.y)*i/(left_points->size()-1));
	    }
	    else
	    {
		point_dst.y = (int)round((start.y+end.y)/2);
	    }
	    
	    if (left_points->size() == left.size())
	    {
		dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x + 1);
	    }
	    else
	    {
                dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    }
	    
	    left_points->at(i).x += 1;
	}
	
	left_points->erase(left_points->begin());
	if (left_points->size() > 1)
	{
	    left_points->erase(left_points->begin()+(left_points->size()-1));
	}

	start.x += h;
	start.y += v;
	end.x += h;
	end.y -= v;
    }

    //right
    start = {(float)side - 1, 0};
    end = {(float)side - 1, (float)side - 1};
    h = (float)side/((float)left.size()+(float)right.size())*2;
    v = (float)side/((float)left.size()+(float)right.size())*2;

    while (right_points->size() > 0)
    {
	for (int i = 0; i < right_points->size(); i++)
	{
	    point_src = right_points->at(i);
	    point_dst.x = (int)round(start.x);
	    if (right_points->size() > 1)
	    {
	        point_dst.y = (int)round(start.y + (end.y-start.y)*i/(right_points->size()-1));
	    }
	    else
	    {
		point_dst.y = (int)round((start.y+end.y)/2);
	    }
	    
	    if (right_points->size() == right.size())
	    {
		dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x - 1);
	    }
	    else
	    {
                dst.at<cv::Vec3b>(point_dst.y+locate.y, point_dst.x+locate.x) = src.at<cv::Vec3b>(point_src.y, point_src.x);
	    }
	    
	    right_points->at(i).x -= 1;
	}
	
	right_points->erase(right_points->begin());
	if (right_points->size() > 1)
	{
	    right_points->erase(right_points->begin()+(right_points->size()-1));
	}

	start.x -= h;
	start.y += v;
	end.x -= h;
	end.y -= v;
    }
    delete top_points;
    delete botton_points;
    delete left_points;
    delete right_points;
#endif

#if 0
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
#endif
}

void getChessboardGrids(cv::Mat dst, cv::Size size, int side, cv::Mat src, std::vector<cv::Point> *rowL, std::vector<cv::Point> *colL)
{
    for (int i = 0; i < 1/*size.height*/; i++)
    {
	for (int j = 0; j < 1/*size.width*/; j++)
	{
	    cv::Point locate = {side*(size.width-1-j), side*(size.height-1-i)};
	    //cv::Mat grid = cv::Mat::zeros(side, side, src.type());
	    getChessboardGrid(dst, locate, side, src, *(rowL+size.width*(i+1)+j), *(rowL+size.width*i+j), *(colL + (size.width+1)*i+j+1), *(colL+(size.width+1)*i+j));
	    //IplImage ipl_grid = IplImage(grid);
	    //IplImage ipl_grids = IplImage(dst);
	    //CvRect roi_grid =cvRect(0, 0, side, side);
	    //CvRect roi_grids =cvRect(side*(size.width-1-j), side*(size.height-1-i), side, side);
	    //cvSetImageROI(&ipl_grid, roi_grid);
	    //cvSetImageROI(&ipl_grids, roi_grids);
	    //cvCopy(&ipl_grid, &ipl_grids); 
	}
    }
}

void getChessboardSidePoints(std::vector<cv::Point> *allPoints, std::vector<cv::Point> *sidePoints, std::vector<cv::Point2f> *corners, cv::Size size)
{
    bool remove = false;
    int rows = size.height;
    int cols = size.width;
    
    for (auto point : *allPoints)
    {
	remove = false;

        //left
	for (int i = 0; i < rows; i++)
	{
            float k, k1, k2, b1, b2;
	    
	    k = (corners->at((i+1)*cols - 1).y - corners->at((i+1)*cols - 2).y)/(corners->at((i+1)*cols - 1).x - corners->at((i+1)*cols - 2).x);
	    
	    if ((1+k)/(1-k) > 0)
	    {
		k1 = (k + 1) / (1 - k);
		k2 = (k - 1) / (1 + k);
	    }
	    else
	    {
                k1 = (k - 1) / (1 + k);
		k2 = (k + 1) / (1 - k);
	    }

            b1 = corners->at((i+1)*cols - 1).y - k1 * corners->at((i+1)*cols - 1).x;
	    b2 = corners->at((i+1)*cols - 1).y - k2 * corners->at((i+1)*cols - 1).x;
	    
	    if (point.x < corners->at((i+1)*cols - 1).x 
		&& point.y >= (k1 * point.x + b1) && point.y <= (k2 * point.x + b2))
		{
		    remove = true;
		    goto out;
		}
	}

	//right
	for (int i = 0; i < rows; i++)
	{
	    float k, k1, k2, b1, b2;
	    
	    k = (corners->at(i*cols).y - corners->at(1+i*cols).y)/(corners->at(i*cols).x - corners->at(1+i*cols).x);
	    
	    if ((1+k)/(1-k) > 0)
	    {
		k1 = (k + 1) / (1 - k);
		k2 = (k - 1) / (1 + k);
	    }
	    else
	    {
                k1 = (k - 1) / (1 + k);
		k2 = (k + 1) / (1 - k);
	    }

	    b1 = corners->at(i*cols).y - k1 * corners->at(i*cols).x;
	    b2 = corners->at(i*cols).y - k2 * corners->at(i*cols).x;

	    
	    if (point.x > corners->at(i*cols).x 
		&& point.y >= (k2 * point.x + b2) && point.y <= (k1 * point.x + b1))
		{
		    remove = true;
		    goto out;
		} 
	}

	//botton
	for (int i = 0; i < cols; i++)
	{
            float k, k1, k2, b1, b2;
	    
	    if (corners->at(i).x == corners->at(cols+i).x)
	    {
		k1 = 1;
		k2 = -1;
	    }
	    else
	    {
		k = (corners->at(i).y - corners->at(cols+i).y)/(corners->at(i).x - corners->at(cols+i).x);
	        if ((1+k)/(1-k) > 0)
		{
		    k1 = (k + 1) / (1 - k);
		    k2 = (k - 1) / (1 + k);
		}
		else
		{
		    k1 = (k - 1) / (1 + k);
		    k2 = (k + 1) / (1 - k);
		}
	    }

            b1 = corners->at(i).y - k1 * corners->at(i).x; 
            b2 = corners->at(i).y - k2 * corners->at(i).x; 
	    
	    if (point.y > corners->at(i).y 
		&& point.x >= (point.y - b2) / k2 && point.x <= (point.y - b1) / k1)
		{
		    remove = true;
		    goto out;
		}
	}

	//top
        for (int i = 0; i < cols; i++)
	{
            float k, k1, k2, b1, b2;
	    if (corners->at(cols*(rows-1)+i).x == corners->at(cols*(rows-2)+i).x)
	    {
                k1 = 1;
		k2 = -1;
	    }
            else
	    {
		k = (corners->at(cols*(rows-1)+i).y - corners->at(cols*(rows-2)+i).y)/(corners->at(cols*(rows-1)+i).x - corners->at(cols*(rows-2)+i).x);
		 
		if ((1+k)/(1-k) > 0)
		{
		    k1 = (k + 1) / (1 - k);
		    k2 = (k - 1) / (1 + k);
		}
		else
		{
		    k1 = (k - 1) / (1 + k);
		    k2 = (k + 1) / (1 - k);
		}
	    }
	    
	    b1 = corners->at(cols*(rows-1)+i).y - k1 * corners->at(cols*(rows-1)+i).x;
            b2 = corners->at(cols*(rows-1)+i).y - k2 * corners->at(cols*(rows-1)+i).x;
	    
	    if (point.y < corners->at(cols*(rows-1)+i).y 
		&& point.x >= (point.y - b1) / k1 && point.x <= (point.y - b2) / k2)
		{
		    remove = true;
		    goto out;
		}
	}

out:
	if (!remove)
	{
	    sidePoints->push_back({point.x, point.y});
	}
    }
}

void getChessboardRectanglePoints(cv::Mat mat, std::vector<cv::Point2f> *corners, std::vector<cv::Point> *rectPoints)
{
    int x_min = mat.cols;
    int x_max = 0;
    int y_min = mat.cols;
    int y_max = 0;

    cv::Mat *img_gray = new cv::Mat(mat.rows, mat.cols, CV_8UC1);
    cv::Mat *img_dst = new cv::Mat(mat.rows, mat.cols, CV_8UC1);

    cv::cvtColor(mat, *img_gray, CV_BGR2GRAY);
    cv::Canny(*img_gray, *img_dst, 80, 160, 3);
    
    for (const auto point : *corners)
    {
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

    x_min -= 1;
    x_max += 1;
    y_min -= 1;
    y_max += 1;

    for (int i = y_min; i <= y_max; i++)
    {
        for (int j = x_min; j <= x_max; j++)
	{
	    int color = img_dst->at<uchar>(i,j);
	    if (color > 0)
	    {
		rectPoints->push_back({j,i});
	    }
	}
    }

    delete img_gray;
    delete img_dst;
}

void sortChessboardSidePoints(std::vector<cv::Point> *rowL, std::vector<cv::Point> *colL, cv::Size size)
{
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

    int rows = size.height;
    int cols = size.width;
    std::vector<cv::Point> *line = NULL;

    for (int r = 0; r < rows; r++)
    {
	for (int c = 0; c < cols - 1; c++)
	{
	    line = rowL+(cols-1)*r+c;
	    std::sort(line->begin(), line->end(), xLess);
	}
    }
    
    for (int r = 0; r < rows - 1; r++)
    {
	for (int c = 0; c < cols; c++)
	{
	    line = colL+cols*r+c;
	    std::sort(line->begin(), line->end(), yLess);
	}
    }
}

int main(int argc, char **argv)
{
    std::vector<cv::Point> *rowL = new std::vector<cv::Point>[48];
    std::vector<cv::Point> *colL = new std::vector<cv::Point>[45];
    char name[255];
    if (argc > 1)
    {
	//sprintf(name, "Chessboard/%s_undistorted.jpg", argv[1]);
	sprintf(name, "Chessboard/%s.jpg", argv[1]);
    }
    else
    {
	//sprintf(name, "Chessboard/1_undistorted.jpg");
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
	cv::Mat *img_gray = new cv::Mat(gray.rows, gray.cols, CV_8UC1);
	
	cv::cvtColor(gray, *img_gray, CV_BGR2GRAY);
	cv::cornerSubPix(*img_gray, corners, cv::Size(10, 10), cv::Size(-1, -1),
		cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.001));

	delete img_gray;
	
	initChessboarLine(rowL, colL, corners, patternsize);
    }
    else
    {
	std::cout << "cv::findChessboardCorners() reutrn false" << std::endl;
    }

    //cv::drawChessboardCorners(gray, patternsize, cv::Mat(corners), patternfound);
    //cv::namedWindow("gray");
    //cv::imshow("gray", gray);
    //cv::imwrite("tmp.jpg", gray);

#if 1
    cv::Mat img_mask;
    cv::Mat img_show;
    img_mask.create(gray.rows, gray.cols, CV_8UC1);
    img_mask.setTo(cv::Scalar(255));
    
    std::vector<cv::Point> points;
    std::vector<cv::Point> points_line;

    getChessboardRectanglePoints(gray, &corners, &points);
#if 0
    for (auto point : points)
    {
	gray.at<cv::Vec3b>(point.y,point.x)[0] = 0;
	gray.at<cv::Vec3b>(point.y,point.x)[1] = 255;
	gray.at<cv::Vec3b>(point.y,point.x)[2] = 0;

    }
#endif
    getChessboardSidePoints(&points, &points_line, &corners, patternsize);

#if 0
    for (auto point : points_line)
    {
	gray.at<cv::Vec3b>(point.y,point.x)[0] = 0;
	gray.at<cv::Vec3b>(point.y,point.x)[1] = 0;
	gray.at<cv::Vec3b>(point.y,point.x)[2] = 255;
    }
#endif

    fillChessboarLine(&points_line, rowL, colL, corners, patternsize); 

    sortChessboardSidePoints(rowL, colL, patternsize);

    fixChessboarLine(rowL, 48, colL, 45);

#if 0
    for (int j = 0; j < 48; j++)
    {
	std::vector<cv::Point> *line = rowL + j;
	CvScalar color = {{0,0,255}};
	
	gray.at<cv::Vec3b>(line->at(0).y,line->at(0).x)[0] = 0;
	gray.at<cv::Vec3b>(line->at(0).y,line->at(0).x)[1] = 0;
	gray.at<cv::Vec3b>(line->at(0).y,line->at(0).x)[2] = 255;
	
	for (int i = 1; i < line->size(); i++)
	{
	    //cv::line(gray, line->at(i-1), line->at(i), color, 1, 16, 0);
	    gray.at<cv::Vec3b>(line->at(i).y,line->at(i).x)[0] = 0;
	    gray.at<cv::Vec3b>(line->at(i).y,line->at(i).x)[1] = 0;
	    gray.at<cv::Vec3b>(line->at(i).y,line->at(i).x)[2] = 255;
	}
    }

    for (int j = 0; j < 45; j++)
    {
	std::vector<cv::Point> *line = colL + j;
	CvScalar color = {{0,0,255}};
	
	gray.at<cv::Vec3b>(line->at(0).y,line->at(0).x)[0] = 0;
	gray.at<cv::Vec3b>(line->at(0).y,line->at(0).x)[1] = 0;
	gray.at<cv::Vec3b>(line->at(0).y,line->at(0).x)[2] = 255;
	
	for (int i = 1; i < line->size(); i++)
	{
	    //cv::line(gray, line->at(i-1), line->at(i), color, 1, 16, 0);
	    gray.at<cv::Vec3b>(line->at(i).y,line->at(i).x)[0] = 0;
	    gray.at<cv::Vec3b>(line->at(i).y,line->at(i).x)[1] = 0;
	    gray.at<cv::Vec3b>(line->at(i).y,line->at(i).x)[2] = 255;
	}
    }
#endif
    //cv::imwrite("Chessboard/3_corner.jpg", img_dst);
    //cv::namedWindow("corner");
    //cv::imshow("corner", gray);

    //cv::floodFill(img_mask, cv::Point(1, 1), cv::Scalar::all(0));
    //gray.copyTo(img_show, img_mask);
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
    cv::putText(gray, "Hello world", {gray.cols/2 - 300, gray.rows/2 - 50 }, 0, 2, {0,0,255}, 8);
    cv::namedWindow("gray");
    cv::imshow("gray", gray);
    cv::Mat grids = cv::Mat::zeros(500, 800, gray.type());
    getChessboardGrids(grids, {8,5}, 100, gray, rowL, colL); 

    cv::namedWindow("grids");
    cv::imshow("grids", grids);

    cv::imwrite("Chessboard/show.jpg", img_show);

    cv::imwrite("Chessboard/grids.jpg", grids);

#endif
#endif
    
    cv::waitKey();
    
    return 0;
}
