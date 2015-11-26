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
    for (int i = 1; i < top.size(); i++)
    {
	srcTri[0] = {top[i - 1].x, top[i - 1].y};
	srcTri[1] = {top[i].x, top[i].y};
	srcTri[2] = center_src;
	dstTri[0] = {(dst.cols - 1) * (i - 1) / (top.size() - 1), 1};
	dstTri[1] = {(dst.cols - 1) * i / (top.size() - 1), 1};
	dstTri[2] = center_dst;

	cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(0), 1, 16, 0);

	warp_mat = cv::getAffineTransform(srcTri, dstTri);	
	cv::Mat warp = cv::Mat::zeros(dst.rows, dst.cols, dst.type());
	cv::warpAffine(src, warp, warp_mat, warp.size());
	cv::floodFill(mask, cv::Point(0, mask.rows - 1), cv::Scalar::all(0));

        cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(255), 1, 16, 0);

	warp.copyTo(dst, mask);
	mask.setTo(cv::Scalar(255));
    }
    
    //bottton
    for (int i = 1; i < botton.size(); i++)
    {
	srcTri[0] = {botton[i - 1].x, botton[i - 1].y};
	srcTri[1] = center_src;
	srcTri[2] = {botton[i].x, botton[i].y};
	dstTri[0] = {(dst.cols - 1) * (i - 1) / (botton.size() - 1)+1, dst.rows - 2};
	dstTri[1] = center_dst;
	dstTri[2] = {(dst.cols - 1) * i / (botton.size() - 1)+1, dst.rows - 2};

	cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(0), 1, 16, 0);

	warp_mat = cv::getAffineTransform(srcTri, dstTri);	
	cv::Mat warp = cv::Mat::zeros(dst.rows, dst.cols, dst.type());
	cv::warpAffine(src, warp, warp_mat, warp.size());
	cv::floodFill(mask, cv::Point(0, 0), cv::Scalar::all(0));

        cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(255), 1, 16, 0);

	warp.copyTo(dst, mask);
	mask.setTo(cv::Scalar(255));
    } 
 
    //left
    for (int i = 1; i < left.size(); i++)
    {
	srcTri[0] = {left[i - 1].x, left[i - 1].y};
	srcTri[1] = center_src;
	srcTri[2] = {left[i].x, left[i].y};
	dstTri[0] = {1, (dst.rows - 1) * (i - 1) / (left.size() - 1)+1};
	dstTri[1] = center_dst;
	dstTri[2] = {1, (dst.rows - 1) * i / (left.size() - 1)+1};

	cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(0), 1, 16, 0);

	warp_mat = cv::getAffineTransform(srcTri, dstTri);	
	cv::Mat warp = cv::Mat::zeros(dst.rows, dst.cols, dst.type());
	cv::warpAffine(src, warp, warp_mat, warp.size());
	cv::floodFill(mask, cv::Point(dst.cols - 1, 0), cv::Scalar::all(0));
        
	cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(255), 1, 16, 0);

	warp.copyTo(dst, mask);
	mask.setTo(cv::Scalar(255));
    }

    //right
    for (int i = 1; i < right.size(); i++)
    {
	srcTri[0] = center_src;
	srcTri[1] = {right[i - 1].x, right[i - 1].y};
	srcTri[2] = {right[i].x, right[i].y};
	dstTri[0] = center_dst;
	dstTri[1] = {dst.cols - 2, (dst.rows - 1) * (i - 1) / (right.size() - 1)+1};
	dstTri[2] = {dst.cols - 2, (dst.rows - 1) * i / (right.size() - 1)+1};

	cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(0), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(0), 1, 16, 0);

	warp_mat = cv::getAffineTransform(srcTri, dstTri);	
	cv::Mat warp = cv::Mat::zeros(dst.rows, dst.cols, dst.type());
	cv::warpAffine(src, warp, warp_mat, warp.size());
	cv::floodFill(mask, cv::Point(0, 0), cv::Scalar::all(0));
        
	cv::line(mask, dstTri[0], dstTri[1], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[1], dstTri[2], cv::Scalar::all(255), 1, 16, 0);
	cv::line(mask, dstTri[2], dstTri[2], cv::Scalar::all(255), 1, 16, 0);
	
	warp.copyTo(dst, mask);
	mask.setTo(cv::Scalar(255));
    }
}

void getChessboardGrids(cv::Mat dst, cv::Size size, int side, cv::Mat src, std::vector<cv::Point> *rowL, std::vector<cv::Point> *colL)
{
    for (int i = 0; i < size.height; i++)
    {
	for (int j = 0; j < size.width; j++)
	{
	    cv::Mat grid = cv::Mat::zeros(side+2, side+2, src.type());
	    getChessboardGrid(grid, src, *(rowL+size.width*(i+1)+j), *(rowL+size.width*i+j), *(colL + (size.width+1)*i+j+1), *(colL+(size.width+1)*i+j));
	    IplImage ipl_grid = IplImage(grid);
	    IplImage ipl_grids = IplImage(dst);
	    CvRect roi_grid =cvRect(1, 1, side, side);
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
	    //int x_color = img.at<cv::Vec3b>(i+1,j)[0] + img.at<cv::Vec3b>(i+1,j)[1] + img.at<cv::Vec3b>(i+1,j)[2];
	    //int y_color = img.at<cv::Vec3b>(i,j+1)[0] + img.at<cv::Vec3b>(i,j+1)[1] + img.at<cv::Vec3b>(i,j+1)[2];
	    //if (color < (x_color + 255))
	    //{
	//	points.push_back({i,j});
	  //  }
	    //else if (color < (y_color + 255))
	    //{
	//	points.push_back({i,j});
	  //  }
	}
    }

#if 1
    bool remove = false;
    for (auto point : points)
    {
	remove = false;
#if 0
	//top
        for (int i = 0; i < 9; i++)
	{
	    if (point.x < corners[45+i].y && point.y < corners[45+i].x)
	    {
		std::cout << "remove " << point.y << " " << point.x << std::endl;
		remove = true;
		break;
	    }
	}

	//botton
	for (int i = 0; i < 9; i++)
	{
	    if (point.y > corners[i].x && point.x > corners[i].y)
	    {
		std::cout << "remove " << point.y << " " << point.x << std::endl;
		remove = true;
		break;
	    }
	}

	//right
	for (int i = 0; i < 6; i++)
	{
            if (point.y > corners[i*9].x && point.x < corners[i*9].y)
	    {
		std::cout << "remove " << point.y << " " << point.x << std::endl;
		remove = true;
		break;
	    }
	}

        //left
	for (int i = 0; i < 6; i++)
	{
            if (point.y > corners[i*9+8].x && point.x < corners[i*9+8].y)
	    {
		std::cout << "remove " << point.y << " " << point.x << std::endl;
		remove = true;
		break;
	    }
	}
#endif

        //left
	for (int i = 0; i < 6; i++)
	{
	    float fix = std::fabs(corners[8+i*9].y - corners[7+i*9].y)/std::fabs(corners[8+i*9].x - corners[7+i*9].x);
	    fix *= 2;
	    if (fix < 0.9)
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
#endif
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
    cv::Mat grids = cv::Mat::zeros(500, 800, gray.type());
    getChessboardGrids(grids, {8,5}, 100, gray, rowL, colL); 

#if 0
    for (int i = 0; i < 5; i++)
    {
	for (int j = 0; j < 8; j++)
	{
	    cv::Mat grid = cv::Mat::zeros(100, 100, gray.type());
	    getChessboardGrid(grid, gray, *(rowL+8*(i+1)+j), *(rowL+8*i+j), *(colL + 9*i+j+1), *(colL+9*i+j));
	    IplImage ipl_grid = IplImage(grid);
	    IplImage ipl_grids = IplImage(grids);
	    CvRect roi_grid =cvRect(0, 0, 100, 100);
	    CvRect roi_grids =cvRect(100*(7-j), 100*(4-i), 100, 100);
	    cvSetImageROI(&ipl_grid, roi_grid);
	    cvSetImageROI(&ipl_grids, roi_grids);
	    cvCopy(&ipl_grid, &ipl_grids);  
	}
    }
#endif
    cv::namedWindow("grids");
    cv::imshow("grids", grids);

    cv::imwrite("Chessboard/show.jpg", img_show);

    cv::imwrite("Chessboard/grids.jpg", grids);

    cv::waitKey();
#endif

    return 0;
}
