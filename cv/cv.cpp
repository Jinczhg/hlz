#include "cv.h"

#include <iostream>
#include <unistd.h>

bool findChessboardCorners(cv::Mat image, cv::Size patternSize,
	std::vector<std::vector<cv::Point2f>*> *corners, bool manual)
{
    bool ret = true;

    if (!corners || corners->size() < patternSize.height)
    {
	return false;
    }
	
    int row = patternSize.height;
    int col  = patternSize.width;
    std::vector<cv::Point2f> corners_tmp;

    if (manual)
    {
        int c1, c2, c3;

	for (int r = 0; r < image.rows; r++)
	{
	    for (int c = 0; c < image.cols; c++)
	    {
		c1 = image.at<cv::Vec3b>(r, c)[0]; //blue
		c2 = image.at<cv::Vec3b>(r, c)[1]; //green
		c3 = image.at<cv::Vec3b>(r, c)[2]; //red
		if (c1 < 5 && c2 < 5)
		{
		    for (int i = 0; i < row; i++)
		    {
			if (c3 > 250 - 10 * i)
			{
			    corners->at(i)->push_back(cv::Point(c,r));
			    break;
			}
		    }
		}
	    }
	}

	for (auto points : *corners)
	{
	    if (points->size() != col)
	    {
		std::cout << "points->size() != col..." << points->size() << std::endl; 
		ret = false;
		break;
	    }
            
	    std::sort(points->begin(), points->end(), PointXLess);
	    
	    for (auto point : *points)
	    {
		corners_tmp.push_back(point);
	    }

	    points->clear();
	}
    }
    else
    {
	ret = cv::findChessboardCorners(image, patternSize, corners_tmp,
		cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE
		/*+ cv::CALIB_CB_FILTER_QUADS + cv::CALIB_CB_FAST_CHECK*/);	
    }

    if (ret)
    {
	cv::Mat *img_gray = new cv::Mat(image.rows, image.cols, CV_8UC1);
    
	cv::cvtColor(image, *img_gray, CV_BGR2GRAY);
	cv::cornerSubPix(*img_gray, corners_tmp, cv::Size(5, 5), cv::Size(-1, -1),
		cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 500, 0.0001));

	delete img_gray;

	if (corners_tmp.at(0).x > corners_tmp.at(1).x
		&& corners_tmp.at(0).y > corners_tmp.at(col).y)
	{
	    for (int r = 0; r < row; r++)
	    {
		for (int c = 0; c < col; c++)
		{
		    corners->at(r)->push_back(corners_tmp.at((row - r) * col - c - 1));
		}
	    }
	}
	else if (corners_tmp.at(0).x < corners_tmp.at(1).x
		&& corners_tmp.at(0).y < corners_tmp.at(col).y)
	{
	    for (int r = 0; r < row; r++)
	    {
		for (int c = 0; c < col; c++)
		{
		    corners->at(r)->push_back(corners_tmp.at(r * col + c));
		}
	    }
	}
	else
	{
	    ret = false;
	    std::cout << "not match" << std::endl;
	}
    }

    return ret;
}

void getChessboardRectanglePoints(cv::Mat image,
	std::vector<std::vector<cv::Point2f>*> *corners,
	std::vector<cv::Point> *rectPoints)
{
    int x_min = image.cols;
    int x_max = 0;
    int y_min = image.rows;
    int y_max = 0;

    cv::Mat *img_gray = new cv::Mat(image.rows, image.cols, CV_8UC1);
    cv::Mat *img_dst = new cv::Mat(image.rows, image.cols, CV_8UC1);
    cv::Mat *img_blur = new cv::Mat(image.rows, image.cols, CV_8UC1);
    cv::cvtColor(image, *img_gray, CV_BGR2GRAY);

    cv::GaussianBlur(*img_gray, *img_blur, cv::Size(1, 1), 0, 0); 
    cv::Canny(*img_blur, *img_dst, 100, 300, 3, true);

    for (const auto points : *corners)
    {
	for (const auto point : *points)
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
    delete img_blur;
}

void getChessboardLinePoints(std::vector<cv::Point> *rectPoints,
	std::vector<cv::Point> *linePoints,
	std::vector<std::vector<cv::Point2f>*> *corners,
	cv::Size patternSize)
{
    bool remove = false;
    int rows = patternSize.height;
    int cols = patternSize.width;
    
    for (auto point : *rectPoints)
    {
	remove = false;

        //left
	for (int i = 0; i < rows; i++)
	{
            float k1, k2, b1, b2;
	    if (i < rows - 1)
	    {
		if (corners->at(i)->at(0).x == corners->at(i+1)->at(1).x)
		{
		    k1 = tan(70);
		}
		else
		{
		    k1 = (corners->at(i+1)->at(1).y - corners->at(i)->at(0).y)
			/ (corners->at(i+1)->at(1).x - corners->at(i)->at(0).x);
		}
	    }
	    else
	    {
		if (corners->at(i-1)->at(0).x == corners->at(i)->at(1).x)
		{
		    k1 = tan(70);
		}
		else
		{
		    k1 = (corners->at(i)->at(1).y - corners->at(i-1)->at(0).y)
		        / (corners->at(i)->at(1).x - corners->at(i-1)->at(0).x);
		}
	    }

	    if (i > 0)
	    {
		if (corners->at(i)->at(0).x == corners->at(i-1)->at(1).x)
		{
		    k2 = tan(110);
		}
		else
		{
		    k2 = (corners->at(i-1)->at(1).y - corners->at(i)->at(0).y)
			/ (corners->at(i-1)->at(1).x - corners->at(i)->at(0).x);
		}
	    }
	    else
	    {
                if (corners->at(i+1)->at(0).x == corners->at(i)->at(1).x)
		{
		    k2 = tan(110);
		}
		else
		{
		    k2 = (corners->at(i)->at(1).y - corners->at(i+1)->at(0).y)
		        / (corners->at(i)->at(1).x - corners->at(i+1)->at(0).x);
		}
	    }

	    b1 = corners->at(i)->at(0).y - k1 * corners->at(i)->at(0).x;
	    b2 = corners->at(i)->at(0).y - k2 * corners->at(i)->at(0).x;
            
	    if (point.x < corners->at(i)->at(0).x)
	    {
		    if (point.x <= (point.y - b1) / k1 && point.x <= (point.y - b2) / k2)
		    {
			remove = true;
			goto out;
		    }
	    }
	}
	
	//right
	for (int i = 0; i < rows; i++)
	{
	    float k1, k2, b1, b2;
            
	    if (i < rows - 1)
	    {
		if (corners->at(i)->at(cols-1).x == corners->at(i+1)->at(cols-2).x)
		{
		    k1 = tan(110);
		}
		else
		{
		    k1 = (corners->at(i+1)->at(cols-2).y - corners->at(i)->at(cols-1).y)
			/ (corners->at(i+1)->at(cols-2).x - corners->at(i)->at(cols-1).x);
		}
	    }
	    else
	    {
		k1 = (corners->at(i)->at(cols-2).y - corners->at(i-1)->at(cols-1).y)
		    / (corners->at(i)->at(cols-2).x - corners->at(i-1)->at(cols-1).x);
	    }

	    if (i > 0)
	    {
		if (corners->at(i)->at(cols-1).x == corners->at(i-1)->at(cols-2).x)
		{
		    k2 = tan(70);
		}
		else
		{
		    k2 = (corners->at(i-1)->at(cols-2).y - corners->at(i)->at(cols-1).y)
			/ (corners->at(i-1)->at(cols-2).x - corners->at(i)->at(cols-1).x);
		}
	    }
	    else
	    {
		k2 = (corners->at(i)->at(cols-2).y - corners->at(i+1)->at(cols-1).y)
		    / (corners->at(i)->at(cols-2).x - corners->at(i+1)->at(cols-1).x);
	    }

	    b1 = corners->at(i)->at(cols-1).y - k1 * corners->at(i)->at(cols-1).x;
	    b2 = corners->at(i)->at(cols-1).y - k2 * corners->at(i)->at(cols-1).x;
            
	    if (point.x > corners->at(i)->at(cols-1).x)
	    {
		    if (point.x >= (point.y - b1) / k1 && point.x >= (point.y - b2) / k2)
		    {
			remove = true;
			goto out;
		    }
	    }
	}

	//botton
	for (int i = 0; i < cols; i++)
	{
            float k1, k2, b1, b2;
            
	    if (i < cols -1)
	    {
		if (corners->at(rows-1)->at(i).x == corners->at(rows-2)->at(i+1).x)
		{
		    k1 = tan(110);
		}
		else
		{
		    k1 = (corners->at(rows-2)->at(i+1).y - corners->at(rows-1)->at(i).y)
			/ (corners->at(rows-2)->at(i+1).x - corners->at(rows-1)->at(i).x);
		}
	    }
	    else
	    {
		k1 = (corners->at(rows-2)->at(i).y - corners->at(rows-1)->at(i-1).y)
		    / (corners->at(rows-2)->at(i).x - corners->at(rows-1)->at(i-1).x);
	    }

	    if (i > 0)
	    {
		if (corners->at(rows-1)->at(i).x == corners->at(rows-2)->at(i-1).x)
		{
		    k2 = tan(70);
		}
		else
		{
		    k2 = (corners->at(rows-2)->at(i-1).y - corners->at(rows-1)->at(i).y)
			/ (corners->at(rows-2)->at(i-1).x - corners->at(rows-1)->at(i).x);
		}
	    }
	    else
	    {
		k2 = (corners->at(rows-2)->at(i).y - corners->at(rows-1)->at(i+1).y)
		    / (corners->at(rows-2)->at(i).x - corners->at(rows-1)->at(i+1).x);
	    }

	    b1 = corners->at(rows-1)->at(i).y - k1 * corners->at(rows-1)->at(i).x;
	    b2 = corners->at(rows-1)->at(i).y - k2 * corners->at(rows-1)->at(i).x;
            
	    if (point.y > corners->at(rows-1)->at(i).y
		    && point.x >= (point.y - b1) / k1 && point.x <= (point.y - b2) / k2)
	    {
		remove = true;
		goto out;
	    }
	}

	//top
        for (int i = 0; i < cols; i++)
	{
            float k1, k2, b1, b2;

	    if (i < cols -1)
	    {
		if (corners->at(0)->at(i).x == corners->at(1)->at(i+1).x)
		{
		    k1 = tan(70);
		}
		else
		{
		    k1 = (corners->at(1)->at(i+1).y - corners->at(0)->at(i).y)
			/ (corners->at(1)->at(i+1).x - corners->at(0)->at(i).x);
		}
	    }
	    else
	    {
		k1 = (corners->at(1)->at(i).y - corners->at(0)->at(i-1).y)
		    / (corners->at(1)->at(i).x - corners->at(0)->at(i-1).x);
	    }

	    if (i > 0)
	    {
		if (corners->at(0)->at(i).x == corners->at(1)->at(i-1).x)
		{
		    k2 = tan(110);
		}
		else
		{
		    k2 = (corners->at(1)->at(i-1).y - corners->at(0)->at(i).y)
			/ (corners->at(1)->at(i-1).x - corners->at(0)->at(i).x);
		}
	    }
	    else
	    {
		k2 = (corners->at(1)->at(i).y - corners->at(0)->at(i+1).y)
		    / (corners->at(1)->at(i).x - corners->at(0)->at(i+1).x);
	    }

	    b1 = corners->at(0)->at(i).y - k1 * corners->at(0)->at(i).x;
	    b2 = corners->at(0)->at(i).y - k2 * corners->at(0)->at(i).x;
            
	    if (point.y < corners->at(0)->at(i).y
		    && point.x >= (point.y - b1) / k1 && point.x <= (point.y - b2) / k2)
	    {
		remove = true;
		goto out;
	    }
	}

out:
	if (!remove)
	{
	    linePoints->push_back({point.x, point.y});
	}
    }

}

void fixChessboardLines(std::vector<std::vector<std::vector<cv::Point>*>*> *rowLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLines)
{
    std::vector<cv::Point> *side;
    std::vector<cv::Point> *points = new std::vector<cv::Point>();
    std::vector<cv::Point> *tmp_points = new std::vector<cv::Point>();
    cv::Point p1, p2;
    float xDistance, yDistance;
    int xStart, xEnd, yStart, yEnd;

    for (int i = 0; i < rowLines->size(); i++)
    {
	for (auto row : *rowLines->at(i))
	{
	    side = row;

	    points->clear();
	    tmp_points->clear();

	    points->push_back(side->at(0));
	    tmp_points->push_back(side->at(0));
	    xStart = side->at(0).x;
	    xEnd = side->at(side->size()-1).x;

	    for (int j = 1; j < side->size() -1; j++)
	    {
		p1 = side->at(j);
		p2 = tmp_points->at(tmp_points->size()-1);

		if (abs(p1.y - p2.y) > 3)
		{
		    continue;
		}

		if (p1.x <= xStart)
		{
		    continue;
		}
		
		if (p1.x >= xEnd)
		{
		    break;
		}
		
		if (p1.x == p2.x && tmp_points->size() > 1)
		{
		    tmp_points->erase(tmp_points->begin()+tmp_points->size()-1);
		}
		else
		{
		    tmp_points->push_back(p1);
		}
	    }

	    tmp_points->push_back(side->at(side->size()-1));

	    for (int j = 1; j < tmp_points->size(); j++)
	    {
		p1 = tmp_points->at(j);
		p2 = points->at(points->size()-1);

		xDistance = p1.x - p2.x;
		yDistance = p1.y - p2.y;

		for (int k = 1; k < xDistance; k++)
		{
		    points->push_back({p2.x + k, p2.y + (int)round(k * yDistance / xDistance)});
		}

		if (xDistance == 0 && j == tmp_points->size() - 1)
		{
		    points->at(points->size()-1).y = p1.y;
		}
		else
		{
		    points->push_back(p1);
		}
	    }

	    *side = *points;
	}
    }

    for (int i = 0; i < colLines->size(); i++)
    {
	for (auto col : *colLines->at(i))
	{
	    side = col;
	    
	    points->clear();
	    tmp_points->clear();

	    points->push_back(side->at(0));
	    tmp_points->push_back(side->at(0));
	    yStart = side->at(0).y;
	    yEnd = side->at(side->size()-1).y;

	    for (int j = 1; j < side->size() -1; j++)
	    {
		p1 = side->at(j);
		p2 = tmp_points->at(tmp_points->size()-1);

		if (abs(p1.x - p2.x) > 3)
		{
		    continue;
		}

		if (p1.y <= yStart)
		{
		    continue;
		}

		if (p1.y > yEnd)
		{
		    break;
		}
		
		if (p1.y == p2.y && tmp_points->size() > 1)
		{
		    tmp_points->erase(tmp_points->begin()+tmp_points->size()-1);
		}
		else
		{
		    tmp_points->push_back(p1);
		}
	    }

	    tmp_points->push_back(side->at(side->size()-1));
	    
	    for (int j = 1; j < tmp_points->size(); j++)
	    {
		p1 = tmp_points->at(j);
		p2 = points->at(points->size()-1);

		xDistance = p1.x - p2.x;
		yDistance = p1.y - p2.y;

		for (int k = 1; k < yDistance; k++)
		{
		    points->push_back({p2.x + (int)round(k * xDistance / yDistance), p2.y + k});
		}

		if (yDistance == 0 && j == tmp_points->size() - 1)
		{
		    points->at(points->size()-1).x = p1.x;
		}
		else
		{
		    points->push_back(p1);
		}
	    }

	    *side = *points;
	}
    }

    delete points;
    delete tmp_points;
}

void fillChessboardLines(std::vector<cv::Point> *points,
	std::vector<std::vector<std::vector<cv::Point>*>*> *rowLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLines,
	std::vector<std::vector<cv::Point2f>*> *corners,
	cv::Size patternSize)
{
    float distance = std::numeric_limits<float>::max();
    int row = 0;
    int col = 0;
    float tmp = 0; 
    float a1 = 0;
    float a2 = 0;
    float b1 = 0;
    float b2 = 0;
    float k, b;
    float d;
    const float d_max = 5; 
    
    //fill
    for (auto point : *points)
    {
	distance = std::numeric_limits<float>::max();
	row = 0;
	col = 0;
	tmp = 0; 
	a1 = 0;
	a2 = 0;
	b1 = 0;
	b2 = 0;

	for (int i = 0; i < patternSize.height; i++)
	{
	    for (int j = 0; j < patternSize.width; j++)
	    {
                tmp = std::pow(point.x - corners->at(i)->at(j).x, 2)
		    + std::pow(point.y - corners->at(i)->at(j).y, 2);
		
		if (distance > tmp)
		{
		    //the distance of point to line
		    if (i > 0)
		    {
			if (corners->at(i-1)->at(j).x == corners->at(i)->at(j).x)
			{
			    d = abs(point.x - corners->at(i)->at(j).x);
			}
			else
			{
			    k = (corners->at(i-1)->at(j).y - corners->at(i)->at(j).y)
			        / (corners->at(i-1)->at(j).x - corners->at(i)->at(j).x);
			    
			    b = corners->at(i)->at(j).y - k * corners->at(i)->at(j).x;
                            
			    d = abs((k * point.x - point.y + b)/sqrt(k * k + 1));
			}
		    
			if (d < d_max)
			{
			    distance = tmp;
			    row = i;
			    col = j;
			    continue;
			}
		    }

		    if (i < patternSize.height - 1)
		    {
			if (corners->at(i+1)->at(j).x == corners->at(i)->at(j).x)
			{
			    d = abs(point.x - corners->at(i)->at(j).x);
			}
			else
			{
			    k = (corners->at(i+1)->at(j).y - corners->at(i)->at(j).y)
			        / (corners->at(i+1)->at(j).x - corners->at(i)->at(j).x);
			    
			    b = corners->at(i)->at(j).y - k * corners->at(i)->at(j).x;
                            
			    d = abs((k * point.x - point.y + b)/sqrt(k * k + 1));
			}
		    
			if (d < d_max)
			{
			    distance = tmp;
			    row = i;
			    col = j;
			    continue;
			}
		    }

                    if (j > 0)
		    {
			if (corners->at(i)->at(j-1).y == corners->at(i)->at(j).y)
			{
			    d = abs(point.y - corners->at(i)->at(j).y);
			}
			else
			{
			    k = (corners->at(i)->at(j-1).y - corners->at(i)->at(j).y)
			        / (corners->at(i)->at(j-1).x - corners->at(i)->at(j).x);
			    
			    b = corners->at(i)->at(j).y - k * corners->at(i)->at(j).x;
                            
			    d = abs((k * point.x - point.y + b)/sqrt(k * k + 1));
			}
		    
			if (d < d_max)
			{
			    distance = tmp;
			    row = i;
			    col = j;
			    continue;
			}
		    }

                    if (j < patternSize.width - 1)
		    {
			if (corners->at(i)->at(j+1).y == corners->at(i)->at(j).y)
			{
			    d = abs(point.y - corners->at(i)->at(j).y);
			}
			else
			{
			    k = (corners->at(i)->at(j+1).y - corners->at(i)->at(j).y)
			        / (corners->at(i)->at(j+1).x - corners->at(i)->at(j).x);
			    
			    b = corners->at(i)->at(j).y - k * corners->at(i)->at(j).x;
                            
			    d = abs((k * point.x - point.y + b)/sqrt(k * k + 1));
			}
		    
			if (d < d_max)
			{
			    distance = tmp;
			    row = i;
			    col = j;
			    continue;
			}
		    }
	        }
	    }
	}

	if (distance == std::numeric_limits<float>::max())
	{
	    continue;
	}

	//center
	if (col != 0 && col != (patternSize.width -1)
		&& row != 0 && row != (patternSize.height -1))
	{
            a1 = (corners->at(row-1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col-1).x - corners->at(row)->at(col).x);
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row+1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col-1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;
            
	    if (a1 > 0 && a2 < 0)
	    {
		if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			colLines->at(col)->at(row)->push_back(point);
		    }
		    else
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		}
		else
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row-1)->push_back(point);
		    }
		}
	    }
	    else if (a1 < 0 && a2 < 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row)->push_back(point);
		    }
		}
		else
		{
		    if (point.y > a2*point.x + b2)
		    {
			colLines->at(col)->at(row-1)->push_back(point);
		    }
		    else
		    {
			rowLines->at(row)->at(col)->push_back(point);
		    }
		}
	    }
	    else if (a1 > 0 && a2 > 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row-1)->push_back(point);
		    }
		}
		else
		{
		    if (point.y > a2*point.x + b2)
		    {
			colLines->at(col)->at(row)->push_back(point); 
		    }
		    else
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		}
	    }
	}

	//left
	else if (col == 0 && row != 0 && row != (patternSize.height - 1))
	{
	    a1 = (corners->at(row+1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col+1).x - corners->at(row)->at(col).x);
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row-1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col+1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

	    if (a1 > 0 && a2 < 0)
	    {
		if (point.y > a1*point.x + b1)
		{
		    colLines->at(col)->at(row)->push_back(point);
		}
		else
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row-1)->push_back(point);
		    }
		}
	    }
	    else if (a1 < 0 && a2 < 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row-1)->push_back(point);
		    }
		}
		else
		{
		    colLines->at(col)->at(row)->push_back(point);
		}
		
	    }
	    else if (a1 > 0 && a2 > 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row-1)->push_back(point);
		    }
		}
		else
		{
		    colLines->at(col)->at(row)->push_back(point);
		}
	    }
	}

	//right
	else if (col == (patternSize.width - 1) &&
		row != 0 && row != (patternSize.height - 1))
	{
            a1 = (corners->at(row-1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col-1).x - corners->at(row)->at(col).x );
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row+1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col-1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

	    if (a1 > 0 && a2 < 0)
	    {
		if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			colLines->at(col)->at(row)->push_back(point);
		    }
		    else
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		}
		else
		{
		    colLines->at(col)->at(row-1)->push_back(point);
		}
	    }
	    else if (a1 > 0 && a2 > 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row)->push_back(point);
		    }
		}
		else
		{
		    colLines->at(col)->at(row-1)->push_back(point);
		}
	    }
	    else if (a1 < 0 && a2 < 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    colLines->at(col)->at(row-1)->push_back(point);
		}
		else
                {
		    if (point.y > a2*point.x + b2)
		    {
			colLines->at(col)->at(row)->push_back(point);
		    }
		    else
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		}
	    }
	}
	
	//botton
	else if (row == (patternSize.height - 1) && col != 0 && col != (patternSize.width -1))
	{
            a1 = (corners->at(row-1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col-1).x - corners->at(row)->at(col).x );
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row-1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col+1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

	    if (a1 > 0 && a2 < 0)
	    {
		if (point.y > a1*point.x + b1)
		{
		    rowLines->at(row)->at(col-1)->push_back(point);
		}
		else
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col)->push_back(point);
		    }
		    else
		    {
			colLines->at(col)->at(row-1)->push_back(point);
		    }
		}
	    }
	    else if (a1 > 0 && a2 > 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    rowLines->at(row)->at(col-1)->push_back(point);
		}
		else
		{
		    if (point.y > a2*point.x + b2)
		    {
                        colLines->at(col)->at(row-1)->push_back(point);
		    }
		    else
		    {	
                        rowLines->at(row)->at(col)->push_back(point);
		    }
		}
	    }
	    else if (a1 < 0 && a2 < 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
                        rowLines->at(row)->at(col)->push_back(point);
		    }
		    else
		    {	
                        colLines->at(col)->at(row-1)->push_back(point);
		    }
		}
		else
                {
		    rowLines->at(row)->at(col-1)->push_back(point);
		}
	    }
	}
	
	//top
	else if (row == 0 && col != 0 && col != (patternSize.width -1))
	{
            a1 = (corners->at(row+1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col+1).x - corners->at(row)->at(col).x);
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row+1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col-1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

	    if (a1 > 0 && a2 < 0)
	    {
		if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			colLines->at(col)->at(row)->push_back(point);
		    }
		    else
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		}
		else
		{
		    rowLines->at(row)->at(col)->push_back(point);
		}
	    }
	    else if (a1 > 0 && a2 > 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    if (point.y > a2*point.x + b2)
		    {
			rowLines->at(row)->at(col-1)->push_back(point);
		    }
		    else
		    {	
                        colLines->at(col)->at(row)->push_back(point);
		    }
		}
		else
		{
		    rowLines->at(row)->at(col)->push_back(point);
		}
	    }
	    else if (a1 < 0 && a2 < 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    rowLines->at(row)->at(col)->push_back(point);   
		}
		else
		{
		    if (point.y > a2*point.x + b2)
		    {
			colLines->at(col)->at(row)->push_back(point);
		    }
		    else
		    {	
                        rowLines->at(row)->at(col-1)->push_back(point);
		    }
		}
	    }
	}

	//right-botton
	else if (row == (patternSize.height - 1) && col == (patternSize.width - 1))
	{
            a1 = (corners->at(row-1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col-1).x - corners->at(row)->at(col).x );
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;

	    if (a1 > 0)
	    {
		if (point.y > a1*point.x + b1)
		{
		    rowLines->at(row)->at(col-1)->push_back(point);
		}
		else
		{
		    colLines->at(col)->at(row-1)->push_back(point);
		}
	    }
	    else if (a1 < 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    colLines->at(col)->at(row-1)->push_back(point);
		}
		else
		{
		    rowLines->at(row)->at(col-1)->push_back(point);
		}
	    }
	}

	//left-botton
	else if (row == (patternSize.height - 1) && col == 0)
	{
            a2 = (corners->at(row-1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col+1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

	    if (a2 < 0)
	    {
		if (point.y > a2*point.x + b2)
		{
		    rowLines->at(row)->at(col)->push_back(point);
		}
		else
		{
		    colLines->at(col)->at(row-1)->push_back(point);
		}
	    }
	    else if (a2 > 0)
	    {
                if (point.y > a2*point.x + b2)
		{
		    colLines->at(col)->at(row-1)->push_back(point);
		}
		else
		{
		    rowLines->at(row)->at(col)->push_back(point);
		}
	    }
	}

	//right-top
	else if (row == 0 && col == (patternSize.width - 1))
	{
            a2 = (corners->at(row+1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col-1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;
	    
	    if (a2 < 0)
	    {
		if (point.y > a2*point.x + b2)
		{
		    colLines->at(col)->at(row)->push_back(point);
		}
		else
		{
		    rowLines->at(row)->at(col-1)->push_back(point);
		}
	    }
	    else if (a2 > 0)
	    {
                if (point.y > a2*point.x + b2)
		{
		    rowLines->at(row)->at(col-1)->push_back(point);
		}
		else
		{
		    colLines->at(col)->at(row)->push_back(point);
		}
	    }
	}

	//left-top
	else if (row == 0 && col == 0)
	{
            a1 = (corners->at(row+1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col+1).x - corners->at(row)->at(col).x );
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    if (a1 > 0)
	    {
		if (point.y > a1*point.x + b1)
		{
		    colLines->at(col)->at(row)->push_back(point);
		}
		else
		{
		    rowLines->at(row)->at(col)->push_back(point);
		}
	    }
	    else if (a1 < 0)
	    {
                if (point.y > a1*point.x + b1)
		{
		    rowLines->at(row)->at(col)->push_back(point);
		}
		else
		{
		    colLines->at(col)->at(row)->push_back(point);
		}
	    }
	}
    }

    //sort
    for (auto lines : *rowLines)
    {
	for (auto line : *lines)
	{
            std::sort(line->begin(), line->end(), PointXLess);
	}
    }

    for (auto lines : *colLines)
    {
	for (auto line : *lines)
	{
	    std::sort(line->begin(), line->end(), PointYLess);
	}
    }
    
    //insert corners
    for (int r = 0; r < patternSize.height; r++)
    {
	for (int c = 0; c < patternSize.width - 1; c++)
	{
	    rowLines->at(r)->at(c)->insert(rowLines->at(r)->at(c)->begin(),
		    {(int)round(corners->at(r)->at(c).x),
		    (int)round(corners->at(r)->at(c).y)});
	    
	    rowLines->at(r)->at(c)->insert(rowLines->at(r)->at(c)->begin()
		    + rowLines->at(r)->at(c)->size(),
		    {(int)round(corners->at(r)->at(c + 1).x),
		    (int)round(corners->at(r)->at(c + 1).y)});
	}
    }

    for (int r = 0; r < patternSize.height - 1; r++)
    {
	for (int c = 0; c < patternSize.width; c++)
	{
	    colLines->at(c)->at(r)->insert(colLines->at(c)->at(r)->begin(),
		    {(int)round(corners->at(r)->at(c).x),
		    (int)round(corners->at(r)->at(c).y)});
	    
	    colLines->at(c)->at(r)->insert(colLines->at(c)->at(r)->begin()
		    + colLines->at(c)->at(r)->size(),
		    {(int)round(corners->at(r + 1)->at(c).x),
		    (int)round(corners->at(r + 1)->at(c).y)});
	}
    }
}

void getChessboardGrid(cv::Mat dst, cv::Point locate, int side, cv::Mat src,
	std::vector<cv::Point> *top, std::vector<cv::Point> *botton,
	std::vector<cv::Point> *left, std::vector<cv::Point> *right,
	std::vector<cv::Point> *mapping)
{
    cv::Point point;
    cv::Point top_point;
    cv::Point botton_point;
    cv::Point left_point;
    cv::Point right_point;
    float k_h, k_v, b_h, b_v;

    float h, v, x, y;
    float side_f = side - 1;
    int h_size, v_size;
    int i_top, i_botton, i_left, i_right;

    cv::Mat *mask = new cv::Mat(src.rows, src.cols, CV_8UC1);
    std::vector<cv::Point> *mask_points = new std::vector<cv::Point>();
    std::vector<cv::Point> *top_line = new std::vector<cv::Point>();
    std::vector<cv::Point> *botton_line = new std::vector<cv::Point>();
    std::vector<cv::Point> *left_line = new std::vector<cv::Point>();
    std::vector<cv::Point> *right_line = new std::vector<cv::Point>();

    mask->setTo(cv::Scalar(255));

    cv::namedWindow("mask");

    for (int i = 1; i < top->size(); i++)
    {
        cv::line(*mask, top->at(i), top->at(i-1), cv::Scalar(0));
    }
    //cv::imshow("mask", *mask);
    //cv::waitKey();

    for (int i = 1; i < botton->size(); i++)
    {
        cv::line(*mask, botton->at(i), botton->at(i-1), cv::Scalar(0));
    }
    //cv::imshow("mask", *mask);
    //cv::waitKey();

    for (int i = 1; i < left->size(); i++)
    {
        cv::line(*mask, left->at(i), left->at(i-1), cv::Scalar(0));
    }
    //cv::imshow("mask", *mask);
    //cv::waitKey();

    for (int i = 1; i < right->size(); i++)
    {
        cv::line(*mask, right->at(i), right->at(i-1), cv::Scalar(0));
    }
    //cv::imshow("mask", *mask);
    //cv::waitKey();

    cv::destroyWindow("mask");

    cv::floodFill(*mask, cv::Point(1, 1), cv::Scalar::all(0));

    for (int r = 0; r < mask->rows; r++)
    {
	for (int c = 0; c < mask->cols; c++)
	{
	    if (mask->at<uchar>(r,c) != 0)
	    {
		mask_points->push_back({c, r});
	    }
	}
    }

    //top_line
    for (auto point : *top)
    {
	mask->at<uchar>(point.y, point.x) = 125;
    }

    for (auto point : *mask_points)
    {
	if (mask->at<uchar>(point.y-1, point.x) == 125
		|| mask->at<uchar>(point.y-1, point.x-1) == 125
		|| mask->at<uchar>(point.y-1, point.x+1) == 125)
	{
	    top_line->push_back(point);
	}
    }
    std::sort(top_line->begin(), top_line->end(), PointXLess);

    for (auto point : *top)
    {
	mask->at<uchar>(point.y, point.x) = 0;
    }

    //botton_line
    for (auto point : *botton)
    {
	mask->at<uchar>(point.y, point.x) = 125;
    }

    for (auto point : *mask_points)
    {
	if (mask->at<uchar>(point.y+1, point.x) == 125
		|| mask->at<uchar>(point.y+1, point.x-1) == 125
		|| mask->at<uchar>(point.y+1, point.x+1) == 125)
	{
	    botton_line->push_back(point);
	}
    }
    std::sort(botton_line->begin(), botton_line->end(), PointXLess);
    
    for (auto point : *botton)
    {
	mask->at<uchar>(point.y, point.x) = 0;
    }

    //left_line
    for (auto point : *left)
    {
	mask->at<uchar>(point.y, point.x) = 125;
    }

    for (auto point : *mask_points)
    {
	if (mask->at<uchar>(point.y, point.x-1) == 125
		|| mask->at<uchar>(point.y-1, point.x-1) == 125
		|| mask->at<uchar>(point.y+1, point.x-1) == 125)
	{
	    left_line->push_back(point);
	}
    }
    std::sort(left_line->begin(), left_line->end(), PointYLess);
    
    for (auto point : *left)
    {
	mask->at<uchar>(point.y, point.x) = 0;
    }

    //right_line
    for (auto point : *right)
    {
	mask->at<uchar>(point.y, point.x) = 125;
    }

    for (auto point : *mask_points)
    {
	if (mask->at<uchar>(point.y, point.x+1) == 125
		|| mask->at<uchar>(point.y-1, point.x+1) == 125
		|| mask->at<uchar>(point.y+1, point.x+1) == 125)
	{
	    right_line->push_back(point);
	}
    }
    std::sort(right_line->begin(), right_line->end(), PointYLess);

    for (auto point : *left)
    {
	mask->at<uchar>(point.y, point.x) = 0;
    }
    
    h_size = (top_line->size()-1 + botton_line->size()-1) / 2;
    v_size = (left_line->size()-1 + right_line->size()-1) / 2;

    //fill grid
    for (int r = 0; r < side; r++)
    {
        for (int c = 0; c < side; c++)
	{
            h = h_size * c / side_f;
	    v = v_size * r / side_f;

	    if (r == 0)
	    {
                i_top = (top_line->size()-1)*h/h_size;
                point = top_line->at(i_top);
	    }
	    else if (r == side_f)
	    {
		i_botton = (botton_line->size()-1)*h/h_size;
		point = botton_line->at(i_botton);
	    }
	    else if (c == 0)
	    {
		i_left = (left_line->size()-1)*v/v_size;
		point = left_line->at(i_left);
	    }
	    else if (c == side_f)
	    {
		i_right = (right_line->size()-1)*v/v_size;
                point = right_line->at(i_right);
	    }
	    else
	    {
		i_top = (int)round((top_line->size()-1)*h/h_size);
		i_botton = (int)round((botton_line->size()-1)*h/h_size);
		i_left = (int)round((left_line->size()-1)*v/v_size);
		i_right = (int)round((right_line->size()-1)*v/v_size);
		top_point = top_line->at(i_top);
		botton_point = botton_line->at(i_botton);
		left_point = left_line->at(i_left);
		right_point = right_line->at(i_right);

                if (top_point.x == botton_point.x)
		{
                    point.x = top_point.x;
		    point.y = top_point.y + (botton_point.y - top_point.y) * v / v_size;
		}
		else
		{
		    k_h = ((float)(top_point.y-botton_point.y))
			/ ((float)(top_point.x-botton_point.x));
		    k_v = ((float)(left_point.y-right_point.y))
			/ ((float)(left_point.x-right_point.x));
		    b_h = top_point.y - k_h*top_point.x;
		    b_v = left_point.y - k_v*left_point.x;

		    if (k_h != k_v)
		    {
                        x = (b_v - b_h) / (k_h - k_v);
		        y = (b_h * k_v - b_v * k_h) / (k_v - k_h);
		        point.x = (int)round(x);
		        point.y = (int)round(y);

		    }
		    else
		    {
			point = mapping->at(mapping->size() - 1);
		    }

		}

		if (mask->at<uchar>(point.y, point.x) != 255)
		{
		    if (point.x > right_point.x)
		    {
                        point.x = right_point.x - sqrt((h_size-h)*(h_size-h) / (k_v*k_v + 1));
			point.y = right_point.y + (k_v > 0 ? 1.0 : -1.0)
			    * sqrt((h_size-h)*(h_size-h)*k_v*k_v/(1 + k_v*k_v));
			
			if (mask->at<uchar>(point.y, point.x) != 255)
			{
                            point.x = right_point.x;
			    point.y = right_point.y;
			}
		    }
		    else if (point.x < left_point.x)
		    {
			point.x = left_point.x + sqrt(h*h / (k_v*k_v + 1));
			point.y = left_point.y + (k_v > 0 ? 1.0 : -1.0)
			    * sqrt((h_size-h)*(h_size-h)*k_v*k_v/(1 + k_v*k_v));
                        
			if (mask->at<uchar>(point.y, point.x) != 255)
			{
                            point.x = left_point.x;
			    point.y = left_point.y;
			}
		    }
		    else if (point.y > botton_point.y)
		    {
			point.x = botton_point.x + (k_h > 0 ? 1.0 : -1.0) 
			    * sqrt((v_size-v)*(v_size-v) / (k_h*k_h + 1));
			point.y = botton_point.y - sqrt((v_size-v) 
				* (v_size-v)*k_h*k_h/(1 + k_h*k_h));
                        
			if (mask->at<uchar>(point.y, point.x) != 255)
			{
                            point.x = botton_point.x;
			    point.y = botton_point.y;
			}
		    }
		    else if (point.y < top_point.y)
		    {
			point.x = top_point.x + (k_h > 0 ? 1.0 : -1.0)
			    * sqrt(v*v / (k_h*k_h + 1));
			point.y = top_point.y + sqrt((v_size-v)*(v_size-v)
				* k_h*k_h/(1 + k_h*k_h));
                        
			if (mask->at<uchar>(point.y, point.x) != 255)
			{
                            point.x = top_point.x;
			    point.y = top_point.y;
			}
		    }
		    else
		    {
			point = mapping->at(mapping->size() - 1);
		    }
		}
	    }

            dst.at<cv::Vec3b>(r+locate.y, c+locate.x) = src.at<cv::Vec3b>(point.y, point.x);
	    if (mapping)
	    {
	        mapping->push_back(point);
	    }
	}
    }

    delete mask_points;
    delete top_line;
    delete botton_line;
    delete left_line;
    delete right_line;
    delete mask;

}

void getChessboardGrids(cv::Mat dst, cv::Size patternSize, int side, cv::Mat src,
	std::vector<std::vector<std::vector<cv::Point>*>*> *rowLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *mapping) 
{
    for (int i = 0; i < patternSize.height; i++)
    {
	for (int j = 0; j < patternSize.width; j++)
	{
            cv::Point locate = {side*j, side*i};
	    getChessboardGrid(dst, locate, side, src,
		    rowLines->at(i)->at(j),
		    rowLines->at(i+1)->at(j),
		    colLines->at(j)->at(i),
		    colLines->at(j+1)->at(i),
		    mapping->at(i)->at(j));
	}
    }
}

void saveChessboardGridsMapping(cv::FileStorage fs,
	std::vector<std::vector<std::vector<cv::Point>*>*> *mapping,
	double angle)
{
    int row = 0;
    int col = 0;
    int size = 0;
    
    if (mapping)
    {
        row = mapping->size();
	if (mapping->at(0))
	{
            col = mapping->at(0)->size();
	    if (mapping->at(0)->at(0))
	    {
                size = sqrt(mapping->at(0)->at(0)->size());
	    }
	}
    }

    cv::Mat mapping_mat(size*row,size*col, CV_32SC2);
    cv::Mat mapping_mat_dst;
    int rows = row * size;
    int cols = col * size;


    if (angle == 0.0)
    {
	mapping_mat_dst.create(size*row, size*col, CV_32SC2);
        rows = mapping_mat_dst.rows;
	cols = mapping_mat_dst.cols;

	for (int r_mat = 0; r_mat < mapping_mat_dst.rows; r_mat++)
	{
	    int r = r_mat / size;
	    int r_m = r_mat % size * size;
	    for (int c = 0; c < col; c++)
	    {
		for (int i = 0; i < size; i++)
		{
		    int c_mat = size * c + i;
		    
		    mapping_mat_dst.at<cv::Vec2i>(r_mat,c_mat)[0] = 
			mapping->at(r)->at(c)->at(r_m+i).x;
		    
		    mapping_mat_dst.at<cv::Vec2i>(r_mat,c_mat)[1] = 
			mapping->at(r)->at(c)->at(r_m+i).y;
		}
	    }
	}
    }
    else
    {
	for (int r_mat = 0; r_mat < size*row; r_mat++)
	{
	    int r = r_mat / size;
	    int r_m = r_mat % size * size;
	    for (int c = 0; c < col; c++)
	    {
		for (int i = 0; i < size; i++)
		{
		    int c_mat = size * c + i;
		    
		    mapping_mat.at<cv::Vec2i>(r_mat,c_mat)[0] = 
			mapping->at(r)->at(c)->at(r_m+i).x;
		    
		    mapping_mat.at<cv::Vec2i>(r_mat,c_mat)[1] = 
			mapping->at(r)->at(c)->at(r_m+i).y;
		}
	    }
	}

	if (angle == 90.0)
	{
	    mapping_mat_dst.create(size*col, size*row, CV_32SC2);
	    rows = mapping_mat_dst.rows;
	    cols = mapping_mat_dst.cols;

	    for (int r = 0; r < rows; r++)
	    {
		for (int c = 0; c < cols; c++)
		{
		    mapping_mat_dst.at<cv::Vec2i>(r,c)[0] = 
			mapping_mat.at<cv::Vec2i>(c,rows-r-1)[0];
		    
		    mapping_mat_dst.at<cv::Vec2i>(r,c)[1] = 
			mapping_mat.at<cv::Vec2i>(c,rows-r-1)[1];
		}
	    }
	}
	else if (angle == 180.0)
	{
            mapping_mat_dst.create(size*row, size*col, CV_32SC2);
            rows = mapping_mat_dst.rows;
	    cols = mapping_mat_dst.cols;
	    
	    for (int r = 0; r < rows; r++)
	    {
		for (int c = 0; c < cols; c++)
		{
		    mapping_mat_dst.at<cv::Vec2i>(r,c)[0] = 
			mapping_mat.at<cv::Vec2i>(rows-1-r,cols-1-c)[0];
		    
		    mapping_mat_dst.at<cv::Vec2i>(r,c)[1] = 
			mapping_mat.at<cv::Vec2i>(rows-1-r,cols-1-c)[1];    
		}
	    }
	}
	else if (angle == 270.0)
	{
            mapping_mat_dst.create(size*col, size*row, CV_32SC2);
            rows = mapping_mat_dst.rows;
	    cols = mapping_mat_dst.cols;

	    for (int r = 0; r < rows; r++)
	    {
		for (int c = 0; c < cols; c++)
		{
                    mapping_mat_dst.at<cv::Vec2i>(r,c)[0] = 
			mapping_mat.at<cv::Vec2i>(cols-1-c,r)[0];
		    
		    mapping_mat_dst.at<cv::Vec2i>(r,c)[1] = 
			mapping_mat.at<cv::Vec2i>(cols-1-c,r)[1];
		}
	    }
	}
    }

    fs << "square_size" << size;
    fs << "grid_row" << rows / size;
    fs << "grid_col" << cols / size;
    fs << "grid_mapping" << mapping_mat_dst;
}

void saveChessboardGridsMapping(int fd,
	std::vector<std::vector<std::vector<cv::Point>*>*> *mapping,
	double angle)
{
    uint8_t row = 0;
    uint8_t col = 0;
    int size = 0;
    uint16_t x, y;
    int len = 0;
    
    if (mapping)
    {
        row = mapping->size();
	if (mapping->at(0))
	{
            col = mapping->at(0)->size();
	    if (mapping->at(0)->at(0))
	    {
                size = sqrt(mapping->at(0)->at(0)->size());
	    }
	}
    }

    cv::Mat mapping_mat(size*row,size*col, CV_32SC2);
    cv::Mat mapping_mat_dst;
    uint16_t rows = row * size;
    uint16_t cols = col * size;


    if (angle == 0.0)
    {
	mapping_mat_dst.create(size*row, size*col, CV_32SC2);
        rows = mapping_mat_dst.rows;
	cols = mapping_mat_dst.cols;
        
	len = write(fd, &rows, sizeof(rows));
        len = write(fd, &cols, sizeof(cols));
	
	for (int r_mat = 0; r_mat < mapping_mat_dst.rows; r_mat++)
	{
	    int r = r_mat / size;
	    int r_m = r_mat % size * size;
	    for (int c = 0; c < col; c++)
	    {
		for (int i = 0; i < size; i++)
		{
		    int c_mat = size * c + i;

                    x = mapping->at(r)->at(c)->at(r_m+i).x;
		    y = mapping->at(r)->at(c)->at(r_m+i).y;
		    
		    len = write(fd, &x, 2);
                    len = write(fd, &y, 2);
		}
	    }
	}
    }
    else
    {
	for (int r_mat = 0; r_mat < size*row; r_mat++)
	{
	    int r = r_mat / size;
	    int r_m = r_mat % size * size;
	    for (int c = 0; c < col; c++)
	    {
		for (int i = 0; i < size; i++)
		{
		    int c_mat = size * c + i;
		    
		    mapping_mat.at<cv::Vec2i>(r_mat,c_mat)[0] = 
			mapping->at(r)->at(c)->at(r_m+i).x;
		    
		    mapping_mat.at<cv::Vec2i>(r_mat,c_mat)[1] = 
			mapping->at(r)->at(c)->at(r_m+i).y;
		}
	    }
	}

	if (angle == 90.0)
	{
	    mapping_mat_dst.create(size*col, size*row, CV_32SC2);
	    rows = mapping_mat_dst.rows;
	    cols = mapping_mat_dst.cols;
            
	    len = write(fd, &rows, sizeof(rows));
            len = write(fd, &cols, sizeof(cols));
	    
	    for (int r = 0; r < rows; r++)
	    {
		for (int c = 0; c < cols; c++)
		{
		    x = mapping_mat.at<cv::Vec2i>(c,rows-r-1)[0];
		    y = mapping_mat.at<cv::Vec2i>(c,rows-r-1)[1];

	            len = write(fd, &x, 2);
                    len = write(fd, &y, 2);
		}
	    }
	}
	else if (angle == 180.0)
	{
            mapping_mat_dst.create(size*row, size*col, CV_32SC2);
            rows = mapping_mat_dst.rows;
	    cols = mapping_mat_dst.cols;
	    
	    len = write(fd, &rows, sizeof(rows));
            len = write(fd, &cols, sizeof(cols));    
	    
	    for (int r = 0; r < rows; r++)
	    {
		for (int c = 0; c < cols; c++)
		{
		    x = mapping_mat.at<cv::Vec2i>(rows-1-r,cols-1-c)[0];
		    y = mapping_mat.at<cv::Vec2i>(rows-1-r,cols-1-c)[1];

	            len = write(fd, &x, 2);
                    len = write(fd, &y, 2);
		}
	    }
	}
	else if (angle == 270.0)
	{
            mapping_mat_dst.create(size*col, size*row, CV_32SC2);
            rows = mapping_mat_dst.rows;
	    cols = mapping_mat_dst.cols;
            
	    len = write(fd, &rows, sizeof(rows));
            len = write(fd, &cols, sizeof(cols));

	    for (int r = 0; r < rows; r++)
	    {
		for (int c = 0; c < cols; c++)
		{
		    x = mapping_mat.at<cv::Vec2i>(cols-1-c,r)[0];
		    y = mapping_mat.at<cv::Vec2i>(cols-1-c,r)[1];

	            len = write(fd, &x, 2);
                    len = write(fd, &y, 2);
		}
	    }
	}
    }
}
