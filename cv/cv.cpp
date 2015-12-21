#include "cv.h"

#include <iostream>

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
		ret = false;
		break;
	    }
            
	    std::sort(points->begin(), points->end(), PointXLess);
	}
    }
    else
    {
	std::vector<cv::Point2f> corners_tmp;
	ret = cv::findChessboardCorners(image, patternSize, corners_tmp,
		cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE
		/*+ cv::CALIB_CB_FILTER_QUADS + cv::CALIB_CB_FAST_CHECK*/);
	if (ret)
	{
            cv::Mat *img_gray = new cv::Mat(image.rows, image.cols, CV_8UC1);
	
	    cv::cvtColor(image, *img_gray, CV_BGR2GRAY);
	    cv::cornerSubPix(*img_gray, corners_tmp, cv::Size(10, 10), cv::Size(-1, -1),
		    cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.001));

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
            float k, k1, k2, b1, b2;
	    
	    k = (corners->at(i)->at(0).y - corners->at(i)->at(1).y)
		/ (corners->at(i)->at(0).x - corners->at(i)->at(1).x);
	    
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

            b1 = corners->at(i)->at(0).y - k1 * corners->at(i)->at(0).x;
	    b2 = corners->at(i)->at(0).y - k2 * corners->at(i)->at(0).x;
	    
	    if (point.x < corners->at(i)->at(0).x 
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
	    
	    k = (corners->at(i)->at(cols-1).y - corners->at(i)->at(cols-2).y)
		/ (corners->at(i)->at(cols-1).x - corners->at(i)->at(cols-2).x);
	    
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

	    b1 = corners->at(i)->at(cols-1).y - k1 * corners->at(i)->at(cols-1).x;
	    b2 = corners->at(i)->at(cols-1).y - k2 * corners->at(i)->at(cols-1).x;

	    
	    if (point.x > corners->at(i)->at(cols-1).x 
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
	    
	    if (corners->at(rows-1)->at(i).x == corners->at(rows-2)->at(i).x)
	    {
		k1 = 1;
		k2 = -1;
	    }
	    else
	    {
		k = (corners->at(rows-1)->at(i).y - corners->at(rows-2)->at(i).y)
		    / (corners->at(rows-1)->at(i).x - corners->at(rows-2)->at(i).x);
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

            b1 = corners->at(rows-1)->at(i).y - k1 * corners->at(rows-1)->at(i).x; 
            b2 = corners->at(rows-1)->at(i).y - k2 * corners->at(rows-1)->at(i).x; 
	    
	    if (point.y > corners->at(rows-1)->at(i).y 
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
	    if (corners->at(0)->at(i).x == corners->at(1)->at(i).x)
	    {
                k1 = 1;
		k2 = -1;
	    }
            else
	    {
		k = (corners->at(0)->at(i).y - corners->at(1)->at(i).y)
		    / (corners->at(0)->at(i).x - corners->at(1)->at(i).x);
		 
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

void initChessboardLines(std::vector<std::vector<std::vector<cv::Point>*>*> *rowLines,
	std::vector<std::vector<std::vector<cv::Point>*>*> *colLines,
	std::vector<std::vector<cv::Point2f>*> *corners,
	cv::Size patternSize)
{
    for (int r = 0; r < patternSize.height; r++)
    {
	for (int c = 0; c < patternSize.width - 1; c++)
	{
	    rowLines->at(r)->at(c)->push_back({(int)round(corners->at(r)->at(c).x),
		    (int)round(corners->at(r)->at(c).y)});
	    rowLines->at(r)->at(c)->push_back({(int)round(corners->at(r)->at(c + 1).x),
		    (int)round(corners->at(r)->at(c + 1).y)});
	}
    }

    for (int r = 0; r < patternSize.height - 1; r++)
    {
	for (int c = 0; c < patternSize.width; c++)
	{
	    colLines->at(c)->at(r)->push_back({(int)round(corners->at(r)->at(c).x),
		    (int)round(corners->at(r)->at(c).y)});
	    colLines->at(c)->at(r)->push_back({(int)round(corners->at(r + 1)->at(c).x),
		    (int)round(corners->at(r + 1)->at(c).y)});
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

    for (int i = 0; i < rowLines->size(); i++)
    {
	for (auto row : *rowLines->at(i))
	{
	    side = row;

	    points->clear();
	    tmp_points->clear();

	    points->push_back(side->at(0));
	    tmp_points->push_back(side->at(0));

	    for (int j = 1; j < side->size() -1; j++)
	    {
		p1 = side->at(j);
		p2 = tmp_points->at(tmp_points->size()-1);
		
		if (p1.x == p2.x && tmp_points->size() > 1)
		{
		    tmp_points->erase(tmp_points->begin()+tmp_points->size()-1);
		}
		else
		{
		    tmp_points->push_back(p1);
		}
	    }

	    tmp_points->push_back(*side->rbegin());

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

	    for (int j = 1; j < side->size() -1; j++)
	    {
		p1 = side->at(j);
		p2 = tmp_points->at(tmp_points->size()-1);
		
		if (p1.y == p2.y && tmp_points->size() > 1)
		{
		    tmp_points->erase(tmp_points->begin()+tmp_points->size()-1);
		}
		else
		{
		    tmp_points->push_back(p1);
		}
	    }

	    tmp_points->push_back(*side->rbegin());
	    
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
    //fill
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

	for (int i = 0; i < patternSize.height; i++)
	{
	    for (int j = 0; j < patternSize.width; j++)
	    {
                tmp = std::pow(point.x - corners->at(i)->at(j).x, 2)
		    + std::pow(point.y - corners->at(i)->at(j).y, 2);
		if (distance > tmp)
		{
		    distance = tmp;
		    row = i;
		    col = j;
	        }
	    }
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

	//left
	else if (col == 0 && row != 0 && row != (patternSize.height - 1))
	{
	    a1 = (corners->at(row+1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col+1).x - corners->at(row)->at(col).x);
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row-1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col+1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

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
	
	//botton
	else if (row == (patternSize.height - 1) && col != 0 && col != (patternSize.width -1))
	{
            a1 = (corners->at(row-1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col-1).x - corners->at(row)->at(col).x );
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row-1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col+1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

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
	
	//top
	else if (row == 0 && col != 0 && col != (patternSize.width -1))
	{
            a1 = (corners->at(row+1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col+1).x - corners->at(row)->at(col).x);
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    a2 = (corners->at(row+1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col-1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

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

	//right-botton
	else if (row == (patternSize.height - 1) && col == (patternSize.width - 1))
	{
            a1 = (corners->at(row-1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col-1).x - corners->at(row)->at(col).x );
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;

	    if (point.y > a1*point.x + b1)
	    {
		rowLines->at(row)->at(col-1)->push_back(point);
	    }
	    else
	    {
		colLines->at(col)->at(row-1)->push_back(point);
	    }
	}

	//left-botton
	else if (row == (patternSize.height - 1) && col == 0)
	{
            a2 = (corners->at(row-1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row-1)->at(col+1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;

	    if (point.y > a2*point.x + b2)
	    {
		rowLines->at(row)->at(col)->push_back(point);
	    }
	    else
	    {
		colLines->at(col)->at(row-1)->push_back(point);
	    }
	}

	//right-top
	else if (row == 0 && col == (patternSize.width - 1))
	{
            a2 = (corners->at(row+1)->at(col-1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col-1).x - corners->at(row)->at(col).x);
            b2 = corners->at(row)->at(col).y - a2 * corners->at(row)->at(col).x;
	    
	    if (point.y > a2*point.x + b2)
	    {
		colLines->at(col)->at(row)->push_back(point);
	    }
	    else
	    {
		rowLines->at(row)->at(col-1)->push_back(point);
	    }
	}

	//left-top
	else if (row == 0 && col == 0)
	{
            a1 = (corners->at(row+1)->at(col+1).y - corners->at(row)->at(col).y)
		/ (corners->at(row+1)->at(col+1).x - corners->at(row)->at(col).x );
	    b1 = corners->at(row)->at(col).y - a1 * corners->at(row)->at(col).x;
	    
	    if (point.y > a1*point.x + b1)
	    {
		colLines->at(col)->at(row)->push_back(point);
	    }
	    else
	    {
		rowLines->at(row)->at(col)->push_back(point);
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

