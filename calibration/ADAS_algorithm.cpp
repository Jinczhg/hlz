#include "ADAS_algorithm.h"
#include<iostream> 
#include<string> 
#include<sstream> 

#define show_calib_bord (1)
#define save_value      (0)
#define show_image      (0)

static int s_init_camera_paraters = 0;

bool Surveying_system::ConerDetect(Size board_size, Mat im, vector<Point2d> &coners)
{
	if (im.empty())
	{
		return false;
	}
	vector<vector<Point2d>>  corners_Seq;
	vector<Mat>  image_Seq;
	int successImageNum = 0;
	Mat imageGray;
	if (im.channels() > 1)
	{
		cvtColor(im, imageGray, CV_RGB2GRAY);
	}
	else
	{
		imageGray = im.clone();
	}		
	vector<Point2f> corners1;
		
	if (cv::findChessboardCorners(im, board_size, corners1, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE))
	{
		cornerSubPix(imageGray, corners1, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
		for (int p = 0; p < corners1.size(); p++)
		{
			Point2d temp;
			temp.x = (double)corners1[p].x;
			temp.y = (double)corners1[p].y;
			coners.push_back(temp);
		}
		
		if (show_calib_bord)
		{
			for (int j = 0; j < coners.size(); j++)
			{
				circle(im, corners1[j], 4, Scalar(0, 0, 255), 1, 2, 0);

				char c[20];
				printf(c, "%d", j);
				String p;


				Point poi;
				poi.x = coners[j].x;
				poi.y = coners[j].y;
				putText(im, c, poi, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 1, 2);
			}
		}
		im.copyTo(forshow_coner);
		return true;

	}
	else
	{
		printf("Detect coner false\n");
		return false;
	}
}

bool Surveying_system::Image_fusion(int index, Mat im, Mat &regist_im)
{
	int flag =0;

	if (index==1)
	{
		im.copyTo(regist_im);
		return true;
	}
	
	double alpha = 0.5;
	double beta = 1 - alpha;
	double gamma = 0;
	int chns = regist_im.channels();
	
	for (int i = 0; i < im.rows; i++)
	{
		uchar* regdata = (uchar*)(regist_im.data + regist_im.step*i);
		uchar * imdata = (uchar*)(im.data + im.step*i);
		for (int j = 0; j < im.cols; j++)
		{
			int p = 0;			
			if (flag==1)
			{							
				if (regdata[j*chns + 0]>5 && imdata[j*chns + 0]>5)
				{
					float w = (1 - abs(j / im.rows ))*(1 - abs(j / im.cols - 0.5));
					alpha = 1-w;
					beta = 1 - alpha;

					double B = imdata[j*chns + 0] * alpha + regdata[j*chns + 0] * beta + gamma;
					double G= imdata[j*chns + 1] * alpha + regdata[j*chns + 1] * beta + gamma;
					double R = imdata[j*chns + 2] * alpha + regdata[j*chns + 2] * beta + gamma;

					regdata[j*chns + 0] = (uchar)B;
					regdata[j*chns + 1] = (uchar)G;
					regdata[j*chns + 2] = (uchar)R;
										
				}
				else if (imdata[j*chns + 0] >5 && regdata[j*chns + 0] == 0)
				{
					regdata[j*chns + 0] = imdata[j*chns + 0];
					regdata[j*chns + 1] = imdata[j*chns + 1];
					regdata[j*chns + 2] = imdata[j*chns + 2];
				}
			}
			
			 if (flag==0)				
			{
				regdata[j*chns + 0] = imdata[j*chns + 0] > regdata[j*chns + 0] ? imdata[j*chns + 0] : regdata[j*chns + 0];
				regdata[j*chns + 1] = imdata[j*chns + 1] > regdata[j*chns + 1] ? imdata[j*chns + 1] : regdata[j*chns + 1];
				regdata[j*chns + 2] = imdata[j*chns + 2] > regdata[j*chns + 2] ? imdata[j*chns + 2] : regdata[j*chns + 2];
			}

			 if (flag==2)
			 {
				 if (index==2)
				 {
					 Point2f p1, p2;
					 p1.x = 360, p1.y = 440;
					 p2.x = 360, p2.y = 440;
					 float k = p1.y/p1.x;
					 float D = sqrt((p1.x - 0)*(p1.x - 0) + (p1.y - 400)*(p1.y - 400));

					 if (regdata[j*chns + 0]>5 && imdata[j*chns + 0]>5)
					 {
						 float d = (j - k*i) / sqrt(1 + k*k);
						 if (d<0)
						 {
							 beta = abs(d) / D;
							 alpha = 1 - beta;
						 }
						 else
						 {
							 alpha = abs(d) / D;
							 beta = 1 - alpha;
						 }
						
						 double B = imdata[j*chns + 0] * alpha + regdata[j*chns + 0] * beta + gamma;
						 double G = imdata[j*chns + 1] * alpha + regdata[j*chns + 1] * beta + gamma;
						 double R = imdata[j*chns + 2] * alpha + regdata[j*chns + 2] * beta + gamma;

						 regdata[j*chns + 0] = (uchar)B;
						 regdata[j*chns + 1] = (uchar)G;
						 regdata[j*chns + 2] = (uchar)R;
					 }
					 else if (imdata[j*chns + 0] >5 && regdata[j*chns + 0] == 0)
					 {
						 regdata[j*chns + 0] = imdata[j*chns + 0];
						 regdata[j*chns + 1] = imdata[j*chns + 1];
						 regdata[j*chns + 2] = imdata[j*chns + 2];
					}
				 }
			}				
		}
	}
	
	return true;
}

bool Surveying_system::calibration_by_ployfit(int index, int flage, Mat im, vector<Point2d> Bconers, Mat &Mapx, Mat &Mapy)
{
	if (im.empty()||Bconers.size()<1)
	{
		return false;
	}
	Size image_size = im.size();
	Mapx.create(image_size, CV_32FC1);
	Mapy.create(image_size, CV_32FC1);
	Mat Umapx, Umapy, Tr_mapx, Tr_mapy,H;
	Mat warp_im;
	Mat UdistIm, UdistIm2;
	vector<Point2d> coners, coners1;
	
    if (!UdistImge_ployfit(4,1,im, UdistIm, Umapx, Umapy))//ploy fit to nudist image
	{
		return false;
	}
	UdistIm.copyTo(forshow_udist);
	//imwrite("udist.bmp", UdistIm);
	if (ConerDetect(board_size, UdistIm, coners)) //coners detection
	{
		H = findHomography(Bconers,coners, 0 /*CV_FM_RANSAC*/); //find homograph by opencv
	    Warp_tran(H, Tr_mapx, Tr_mapy);//warp image	 	 
	    Udist_warp_TransMap(index,Umapx, Umapy, Tr_mapx, Tr_mapy, Mapx, Mapy);// nudist and wrap image to mapx and mapy
	}
	else
	{
		printf("no code points\n");
		return false;
	}
	  
	return true;
}



bool Surveying_system::Warp_tran(Mat H, Mat &mapx, Mat& mapy)
{
	
	Size ssize = ImageSize, dsize = ImageSize;	
	mapx.create(ImageSize, CV_32FC1);
	mapy.create(ImageSize, CV_32FC1);

	double* tM = H.ptr<double>(0);
	for (int dy = 0; dy < dsize.height; ++dy)
	{
		for (int dx = 0; dx < dsize.width; ++dx)
		{
			double den = tM[6] * dx + tM[7] * dy + tM[8];
			den = den ? 1.0 / den : 0.0;
			float v_0 = saturate_cast<float>((tM[0] * dx + tM[1] * dy + tM[2]) * den);
			float v_1 = saturate_cast<float>((tM[3] * dx + tM[4] * dy + tM[5]) * den);
			mapx.ptr<float>(dy)[dx] = (float)v_0;
			mapy.ptr<float>(dy)[dx] = (float)v_1;
		}
	}
	
	return true;
}

bool Surveying_system::Udist_warp_TransMap(int index,Mat udist_mapx, Mat udist_mapy, Mat wrap_mapx, Mat wrap_mapy, Mat &mapx, Mat &mapy)
{
	Size size1 = udist_mapx.size();
	Size size2 = udist_mapx.size();
	if ((size1.width!=size2.width)||size1.height!=size2.height)
	{
		return false;
	}
	mapx = Mat::zeros(ImageSize.height, ImageSize.width, CV_32FC1);
	mapy = Mat::zeros(ImageSize.height, ImageSize.width, CV_32FC1);	
	Mat templ;
	for (int i = 0; i < wrap_mapx.rows; i++)
	{
		for (int j = 0; j < wrap_mapx.cols; j++)
		{
			int y = (int)wrap_mapx.at<float>(i, j);
			int x = (int)wrap_mapy.at<float>(i, j);	

			if (x>0 && y>0 && x < size1.height&&y < size1.width)
			{	
				float px = udist_mapx.at<float>(x, y);
				float py = udist_mapy.at<float>(x, y);
				mapx.at<float>(i, j) = px;
				mapy.at<float>(i, j) = py;
			}
			else
			{
				mapx.at<float>(i, j) = 0;
				mapy.at<float>(i, j) = 0;
			}
		}
	}
}

bool Surveying_system::Remap(Mat im, Mat mapx, Mat mapy,int flag, Mat &remap_iamge)
{
	if (im.empty() || mapx.empty() || mapy.empty())
	{
		return false;
	}
	Scalar s(0, 0, 0);
	remap_iamge.create(ImageSize, CV_8UC3);
	int chns = im.channels();

	if (flag)
	{
		for (int i = 0; i < mapx.rows; i++)
		{
			uchar* remap_data = (uchar*)(remap_iamge.data + remap_iamge.step*i);

			for (int j = 0; j < mapx.cols; j++)
			{
				
				int x = (int)(mapx.at<float>(i, j) + 0.5);
				int y = (int)(mapy.at<float>(i, j) + 0.5);
				if (x>0 && y>0 && x<im.rows&&y<im.cols)
				{
					uchar* distdata = (uchar*)(im.data + im.step*y);
					remap_data[j*chns + 0] = distdata[x*chns + 0];
					remap_data[j*chns + 1] = distdata[x*chns + 1];
					remap_data[j*chns + 2] = distdata[x*chns + 2];
				}
				else
				{
					uchar* distdata = (uchar*)(im.data + im.step*y);
					remap_data[j*chns + 0] = 0;
					remap_data[j*chns + 1] = 0;
					remap_data[j*chns + 2] = 0;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < mapx.rows; i++)
		{
			uchar* remap_data = (uchar*)(remap_iamge.data + remap_iamge.step*i);

			for (int j = 0; j < mapx.cols; j++)
			{				
				float X = mapx.at<float>(i, j);
				float Y = mapy.at<float>(i, j);
				int x1 = (int)(X);
				int y1 = (int)(Y);
				if (X > 0 && Y > 0 &&  x1< im.rows && y1 < im.cols)
				{
					float u = X - x1;
					float v = Y - y1;
					int x2 = x1, y2 = y1 + 1;
					int x3 = x1 + 1, y3 = y1;
					int x4 = x1 + 1, y4 = y1 + 1;

					uchar* distdata1 = (uchar*)(im.data + im.step*y1);
					uchar* distdata2 = (uchar*)(im.data + im.step*y2);
					uchar* distdata3 = (uchar*)(im.data + im.step*y3);
					uchar* distdata4 = (uchar*)(im.data + im.step*y4);

					remap_data[j*chns + 0] = (1 - u)*(1 - v)*distdata1[x1*chns + 0] + (1 - u)*v*distdata1[x2*chns + 0] + u*(1 - v)*distdata1[x3*chns + 0] + u*v*distdata1[x4*chns + 0];
					remap_data[j*chns + 1] = (1 - u)*(1 - v)*distdata1[x1*chns + 1] + (1 - u)*v*distdata1[x2*chns + 1] + u*(1 - v)*distdata1[x3*chns + 1] + u*v*distdata1[x4*chns + 1];
					remap_data[j*chns + 2] = (1 - u)*(1 - v)*distdata1[x1*chns + 2] + (1 - u)*v*distdata1[x2*chns + 2] + u*(1 - v)*distdata1[x3*chns + 2] + u*v*distdata1[x4*chns + 2];
				}
				else
				{
					remap_data[j*chns + 0] =0;
					remap_data[j*chns + 1] = 0;
					remap_data[j*chns + 2] = 0;
				}
			}
		}
	}
}

bool Surveying_system::Image_remap(int flag,int index,Mat distimage, Mat mapx, Mat mapy, Mat &remap_iamge)
{
	if (distimage.empty()||mapx.empty()||mapy.empty())
	{
		return false;
	}
	
	Scalar s(0, 0, 0);
	remap_iamge.create(ImageSize, CV_8UC3);

	int div_x = 1.0;
	int div_y = 1.0;
	int chns = distimage.channels();
	int beginx = 0;
	int beginy = 0;
	int rows, cols;
	if (index == 1)
	{
		beginx = 0;
		beginy = 0;
		rows = ex_calib_space.front_row;// mapx.rows / 3;
		cols = ex_calib_space.front_col;// mapx.cols;
		
	}else if (index == 2)
	{
		beginx = 0;
		beginy = 0;
		rows = ex_calib_space.left_row;// mapx.rows;
		cols = ex_calib_space.left_col;// 2 * mapx.cols / 5;
	}
	else if (index == 3)
	{
		beginx = mapx.rows - ex_calib_space.rear_row;// 2 * mapx.rows / 3;
		beginy = 0;
		rows = mapx.rows;// mapx.rows;
		cols = ex_calib_space.rear_col;// mapx.cols;
	}
	else if (index == 4)
	{
		beginx = 0;
		beginy = mapx.cols - ex_calib_space.right_col;// mapx.cols / 2;
		rows = ex_calib_space.right_row;// mapx.rows;
		cols = mapx.cols;// mapx.cols;
	}
	else
	{
		beginx = 0;
		beginy = 0;
		rows = mapx.rows;
		cols = mapx.cols;
	}
	if (flag)
	{
		for (int i = beginx; i < rows; i++)
		{
			uchar* remap_data = (uchar*)(remap_iamge.data + remap_iamge.step*i);			
			for (int j = beginy; j < cols; j++)
			{
				int x = (int)(mapx.at<float>(i, j) + 0.5);
				int y = (int)(mapy.at<float>(i, j) + 0.5) ;
				if (x > 0 && y > 0 && y<distimage.rows&&x<distimage.cols)
				{
					uchar* distdata = (uchar*)(distimage.data + distimage.step*y);
					remap_data[j*chns + 0] = distdata[x*chns + 0];
					remap_data[j*chns + 1] = distdata[x*chns + 1];
					remap_data[j*chns + 2] = distdata[x*chns + 2];
				}
				else
				{
					remap_data[j*chns + 0] = 0;
					remap_data[j*chns + 1] = 0;
					remap_data[j*chns + 2] = 0;
				}
			}
		}
	}
	else
	{
		for (int i = beginx; i < rows; i++)
		{
			uchar* remap_data = (uchar*)(remap_iamge.data + remap_iamge.step*i);

			for (int j = beginy; j < cols; j++)
			{
				float X = mapx.at<float>(i, j);
				float Y = mapy.at<float>(i, j);
				int x1 = (int)(X);
				int y1 = (int)(Y);
				if (X > 0 && Y > 0 && y1<distimage.rows&&x1<distimage.cols)
				{
					float u = X - x1;
					float v = Y - y1;
					int x2 = x1, y2 = y1 + 1;
					int x3 = x1 + 1, y3 = y1;
					int x4 = x1 + 1, y4 = y1 + 1;
					

					uchar* distdata1 = (uchar*)(distimage.data + distimage.step*y1);
					uchar* distdata2 = (uchar*)(distimage.data + distimage.step*y2);
					uchar* distdata3 = (uchar*)(distimage.data + distimage.step*y3);
					uchar* distdata4 = (uchar*)(distimage.data + distimage.step*y4);

					remap_data[j*chns + 0] = (1 - u)*(1 - v)*distdata1[x1*chns + 0] + (1 - u)*v*distdata2[x2*chns + 0] + u*(1 - v)*distdata3[x3*chns + 0] + u*v*distdata4[x4*chns + 0];
					remap_data[j*chns + 1] = (1 - u)*(1 - v)*distdata1[x1*chns + 1] + (1 - u)*v*distdata2[x2*chns + 1] + u*(1 - v)*distdata3[x3*chns + 1] + u*v*distdata4[x4*chns + 1];
					remap_data[j*chns + 2] = (1 - u)*(1 - v)*distdata1[x1*chns + 2] + (1 - u)*v*distdata2[x2*chns + 2] + u*(1 - v)*distdata3[x3*chns + 2] + u*v*distdata4[x4*chns + 2];

				}
				else
				{
					remap_data[j*chns + 0] = 0;
					remap_data[j*chns + 1] = 0;
					remap_data[j*chns + 2] = 0;
				}
			}
		}
		
		imshow("distimage", remap_iamge);
		cvWaitKey(0);
	}
}


uchar* Surveying_system::Imgae_remap(int flag, int index, int src_heigth, int src_width, int dst_heigth, int dst_width, Mat distiamge,Mat &remap_iamge)
{
	uchar* remap_imagep = 0;
	remap_iamge.create(ImageSize, CV_8UC3);

	int beginx = 0;
	int beginy = 0;
	int rows = 0;
	int cols = 0;
	short*data = 0;
	int chns = distiamge.channels();
	int H = 0;
	int W = 0;
	if (index == 1)
	{
		beginx = 0; 
		beginy = 0;
		rows = ex_calib_space.front_row;// mapx.rows / 3;
		cols = ex_calib_space.front_col;// mapx.cols;
		data = stich_data.front.s_data;
		H = stich_data.front.height;
		W = stich_data.front.width;

	}
	else if (index == 2)
	{
		beginx = 0;
		beginy = 0;
		rows = ex_calib_space.left_row;// mapx.rows;
		cols = ex_calib_space.left_col;// 2 * mapx.cols / 5;
		data= stich_data.left.s_data;
		H = stich_data.left.height;
		W = stich_data.left.width;
	}
	else if (index == 3)
	{
		beginx = dst_heigth - ex_calib_space.rear_row;// 2 * mapx.rows / 3;
		beginy = 0;
		rows = dst_heigth;// mapx.rows;
		cols = ex_calib_space.rear_col;// mapx.cols;
		data = stich_data.rear.s_data;
		H = stich_data.rear.height;
		W = stich_data.rear.width;
	}
	else if (index == 4)
	{
		beginx = 0;
		beginy = dst_width - ex_calib_space.right_col;// mapx.cols / 2;
		rows = ex_calib_space.right_row;// mapx.rows;
		cols = dst_width;// mapx.cols;
		data = stich_data.right.s_data;
		H = stich_data.right.height;
		W = stich_data.right.width;
	}
	else
	{
		beginx = 0;
		beginy = 0;
		rows = dst_heigth;
		cols = dst_width;
	}
	int dex = 0;
	vector<short>V_X, V_Y;
	for (int i = 0; i < H;i++)
	{
		for (int j = 0; j < W;j++)
		{
			int p = i*W + j;
			V_X.push_back(data[p * 2]);
			V_Y.push_back(data[p * 2+1]);
		}
	}
	if (flag)
	{
		for (int i = beginx; i < rows; i++)
		{
			uchar* remap_data = (uchar*)(remap_iamge.data + remap_iamge.step*i);
			for (int j = beginy; j < cols; j++)
			{				
					int x = (int)V_X[dex];
					int y = (int)V_Y[dex];
					if (x > 0 && y > 0 && y < src_heigth&&x < src_width)
					{
						uchar* distdata = (uchar*)(distiamge.data + distiamge.step*y);
						
						remap_data[j*chns + 0] = distdata[x*chns + 0];
						remap_data[j*chns + 1] = distdata[x*chns + 1];
						remap_data[j*chns + 2] = distdata[x*chns + 2];
					}
					else
					{
						remap_data[j*chns + 0] = 0;
						remap_data[j*chns + 1] = 0;
						remap_data[j*chns + 2] = 0;
					}

					dex++;
				
			}

		}
	}

	return remap_imagep;
}

bool Surveying_system::Creat_calib_model(vector<vector<Point2d>>&V_coord)
{
	int calib_bord_x = ex_calib_collect.calib_bord_x;
	int calib_bord_y = ex_calib_collect.calib_bord_y;
	float d1 =ex_calib_collect.d1;
	float d2 = ex_calib_collect.d2;
	float fort_rea = ex_calib_collect.fort_rea;
	float left_right = ex_calib_collect.left_right;
	float Length = ex_calib_collect.Length;
	float Width = ex_calib_collect.Width;

	vector<Point2d> calib_coord;
	
	Point2f temp;
	
	for (int n = 0; n < 4; n++)
	{
		if (n == 0)
		{					
			for (int i = 0; i < calib_bord_y; i++)
			{
				for (int j = 0; j < calib_bord_x; j++)
				{
					float x = fort_rea + (calib_bord_y + 1)*d2 + (j + 1) * d1;
					float y = (i + 1) * d2;
					temp.x = x;
					temp.y = y;
					calib_coord.push_back(temp);
				}
			}
			V_coord.push_back(calib_coord);
		}
		
		calib_coord.clear();
		if (n==1)
		{
			for (int i = 0; i < calib_bord_y; i++)
			{
				for (int j = calib_bord_x; j >0; j--)
				{
					float x = (i + 1) * d2;
					float y = left_right + j * d1 + (calib_bord_y + 1)*d2;
					temp.x = x;
					temp.y = y;
					calib_coord.push_back(temp);
				}
			}
			V_coord.push_back(calib_coord);
		}

		calib_coord.clear();
		if (n == 2)
		{
			for (int i = calib_bord_y; i >0; i--)
			{
				for (int j = calib_bord_x; j > 0; j--)
				{
					float x = fort_rea + (calib_bord_y + 1) * d2 + j*d1;
					float y = Length + (calib_bord_y + 1)*d2 + i * d2;
					temp.x = x;
					temp.y = y;
					calib_coord.push_back(temp);
				}
			}
			V_coord.push_back(calib_coord);
		}

		calib_coord.clear();
		if (n == 3)
		{
			for (int i = calib_bord_y; i > 0; i--)
			{
				for (int j = 0; j < calib_bord_x; j++)
				{
					float x = Width + (calib_bord_y + 1) * d2 + i*d2;
					float y = left_right + (j + 1) *d1 + (calib_bord_y + 1) * d2;
					temp.x = x;
					temp.y = y;
					calib_coord.push_back(temp);
				}
			}
			V_coord.push_back(calib_coord);
		}
	}
	
	float maxy = V_coord[2][0].y + ex_calib_collect.d2;
	float maxx = V_coord[3][0].x + ex_calib_collect.d1;
	float alp = ex_calib_collect.scale;
	for (int i = 0; i < V_coord.size(); i++)
	{
		for (int j = 0; j < V_coord[i].size(); j++)
		{
			V_coord[i][j].x = V_coord[i][j].x / alp + (ex_calib_space.width / 2.0 - maxx / (2.0 * alp));
			V_coord[i][j].y = V_coord[i][j].y / alp + (ex_calib_space.height / 2.0 - maxy / (2.0 * alp));
		}
	}

	return true;
}

bool Surveying_system::Write_stitch_calibdata(int flag,char*filename, int number,Caream_data caram_data)
{

	FILE *fp, p;

	if ((fp = fopen(filename, "wb")) == NULL)
	{          
		return false;
	}
	if (flag)
	{
		for (int i = 0; i <5;i++)
		{
			float *data = 0;
			if (i == 0)
			{
				short heigth = (short)caram_data.out_image_height;
				short width = (short)caram_data.out_image_width;
				fwrite(&width, sizeof(short), 1, fp);
				fwrite(&heigth, sizeof(short), 1, fp);

				short p = 0;
				fwrite(&p, sizeof(short), 1, fp);				
			}

			
			if (i==1)
		    {
			    data = caram_data.fornt_data;
				short heigth2 = (short)ex_calib_space.front_row;
				short width2 = (short)ex_calib_space.front_col;
				fwrite(&heigth2, sizeof(short), 1, fp);
				fwrite(&width2, sizeof(short), 1, fp);
				number = 2 * heigth2*width2;
				for (int i = 0; i < number; i++)
				{
					short p = (short)data[i];
					fwrite(&p, sizeof(short), 1, fp);
				}
		    }
			if (i == 2)
			{
				data = caram_data.rear_data;
				short heigth2 = (short)ex_calib_space.rear_row;
				short width2 = (short)ex_calib_space.rear_col;
				fwrite(&heigth2, sizeof(short), 1, fp);
				fwrite(&width2, sizeof(short), 1, fp);
				number = 2 * heigth2*width2;
						
				for (int i = 0; i < number; i++)
				{
					short p = (short)data[i];
					fwrite(&p, sizeof(short), 1, fp);
				}
			}
			if (i == 3)
			{
				data = caram_data.left_data;
				short heigth2 = (short)ex_calib_space.left_row;
				short width2 = (short)ex_calib_space.left_col;
				fwrite(&heigth2, sizeof(short), 1, fp);
				fwrite(&width2, sizeof(short), 1, fp);
				number = 2 * heigth2*width2;

				for (int i = 0; i < number; i++)
				{
					short p = (short)data[i];
					fwrite(&p, sizeof(short), 1, fp);
				}
			}
			if (i == 4)
			{
				data = caram_data.right_data;
				short heigth2 = (short)ex_calib_space.right_row;
				short width2 = (short)ex_calib_space.right_col;
				fwrite(&heigth2, sizeof(short), 1, fp);
				fwrite(&width2, sizeof(short), 1, fp);
				number = 2 * heigth2*width2;

				for (int i = 0; i < number; i++)
				{
					short p = (short)data[i];
					fwrite(&p, sizeof(short), 1, fp);
				}
			}

			
		}
	
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
			{
				short heigth = (short)caram_data.out_image_height;
				short width = (short)caram_data.out_image_width;
				fwrite(&heigth, sizeof(short), 1, fp);
				fwrite(&width, sizeof(short),1, fp);

				short p = 0;
				fwrite(&p, sizeof(short), 1, fp);
				short p2 = 0;
				fwrite(&p2, sizeof(short), 1, fp);
			}

			short heigth2 = (short)caram_data.in_image_height;
			short width2 = (short)caram_data.in_image_width;
			fwrite(&heigth2, sizeof(short), 1, fp);
			fwrite(&width2, sizeof(short), 1, fp);
			float *data = 0;
			if (i == 0)
			{
				data = caram_data.fornt_data;
			}
			if (i == 1)
			{
				data = caram_data.left_data;
			}
			if (i == 2)
			{
				data = caram_data.rear_data;
			}
			if (i == 3)
			{
				data = caram_data.right_data;
			}

			for (int i = 0; i < number; i++)
			{
				short p = (short)data[i];
				fwrite(&p, sizeof(short), 1, fp);
			}
		}

	}
	fclose(fp);

	return true;
}

bool Surveying_system::Read_stich_calibdata(int flag, char *filename)
{
	FILE*fp;
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		return false;
	}
	short para[3] = {0};	
	for (int i = 0; i < 3; i++)
	{
		fread(&para[i], sizeof(short), 1, fp);
	}
	stich_data.out_image_width = para[0];
	stich_data.out_image_height = para[1];
	stich_data.flag = flag;
	
	Init_stich_data();

	
	/*************************front image data***************/
	short data[2] = { 0};
	for (int i = 0; i <2;i++)
	{
		fread(&data[i], sizeof(short), 1, fp);
	}
	stich_data.front.height = data[0];
	stich_data.front.width = data[1];
	for (int i = 0; i < 2*stich_data.front.height*stich_data.front.width; i++)
	{
		fread(&stich_data.front.s_data[i], sizeof(short), 1, fp);
	}
	
	/*************************rear image data***************/
	for (int i = 0; i < 2; i++)
	{
		fread(&data[i], sizeof(short), 1, fp);
	}
	stich_data.rear.height = data[0];
	stich_data.rear.width = data[1];
	for (int i = 0; i < 2*stich_data.rear.height*stich_data.rear.width; i++)
	{
		fread(&stich_data.rear.s_data[i], sizeof(short), 1, fp);
	}
	/*************************left image data***************/
	for (int i = 0; i < 2; i++)
	{
		fread(&data[i], sizeof(short), 1, fp);
	}
	stich_data.left.height = data[0];
	stich_data.left.width = data[1];
	for (int i = 0; i < 2*stich_data.left.height*stich_data.left.width; i++)
	{
		fread(&stich_data.left.s_data[i], sizeof(short), 1, fp);
	}

	/*************************right image data***************/
	for (int i = 0; i < 2; i++)
	{
		fread(&data[i], sizeof(short), 1, fp);
	}
	stich_data.right.height = data[0];
	stich_data.right.width = data[1];
	for (int i = 0; i < 2*stich_data.right.height*stich_data.right.width; i++)
	{
		fread(&stich_data.right.s_data[i], sizeof(short), 1, fp);
	}

	stich_data.in_image_height = data[0];
	stich_data.in_image_width = data[1];

	return true;
}

bool Surveying_system::Read_data(char*filename, int number, short *data)
{
	FILE *fp;	
	data = (short *)calloc(number, sizeof(short));
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		return false;
	}
	for (int i = 0; i < number; i++)
	{
		fread(&data[i], sizeof(short), 1, fp);
	}
}
bool Surveying_system::Read_data(char*filename, int number, float *data)
{
	FILE *fp;	
	data = (float *)calloc(number, sizeof(float));	
	if ((fp = fopen(filename, "rb")) == NULL)
	{
		return false;
	}
	for (int i = 0; i < number; i++)
	{		
		fread(&data[i], sizeof(float), 1, fp);
	}
}

bool Surveying_system::Transmap_to_data(Mat mapx, Mat mapy, int bigny, int bignx, int height, int width, float*data)
{
	if (mapx.empty()||mapy.empty())
	{
		return false;
	}
	
	int number = 2 * height*width;

	vector<float>x,y;
	int index = 0;
	for (int i = bigny; i <height; i++)
	{
		for (int j = bignx; j < width; j++)
		{
		  data[index * 2] = mapx.at<float>(i, j);
		  index++;
		}
	}
	
	index = 0;
	for (int i = bigny; i <height; i++)
	{
		for (int j = bignx; j <width; j++)
		{
			data[index * 2+1] = mapy.at<float>(i, j);
			index++;
		}
	}
	
	return true;
}

bool Surveying_system::Init_Ex_calib_space()
{
	const char *filename = "calib_space.xml";
	cv::FileStorage fs(filename, cv::FileStorage::READ);

	if (fs.isOpened())
	{
	    fs["width"] >> ex_calib_space.width;
	    fs["height"] >> ex_calib_space.height;
	    fs["front_rear_row"] >> ex_calib_space.front_row;
	    fs["left_right_col"] >> ex_calib_space.left_col;
		
		ex_calib_space.front_col = ex_calib_space.width;
		ex_calib_space.rear_row = ex_calib_space.front_row;
		ex_calib_space.rear_col = ex_calib_space.front_col;
		ex_calib_space.left_row = ex_calib_space.height;
		ex_calib_space.right_row = ex_calib_space.left_row;
		ex_calib_space.right_col = ex_calib_space.left_col;
	}
	else
	{
		printf("read calib_config file %s false\n", filename);
		return false;
	}
	
	return true;
}

bool Surveying_system::Init_Ex_calib_collcet()
{
	const char *filename = "calib_collect.xml";
	cv::FileStorage fs(filename, cv::FileStorage::READ);

	if (fs.isOpened())
	{
	    fs["calib_board_x"] >> ex_calib_collect.calib_bord_x;
	    fs["calib_board_y"] >> ex_calib_collect.calib_bord_y;
	    fs["d1"] >> ex_calib_collect.d1;
	    fs["d2"] >> ex_calib_collect.d2;
	    fs["front_rear"] >> ex_calib_collect.fort_rea;
	    fs["left_right"] >> ex_calib_collect.left_right;
	    fs["car_length"] >> ex_calib_collect.Length;
	    fs["car_width"] >> ex_calib_collect.Width;
	    fs["scale"] >> ex_calib_collect.scale;
	}
	else
	{
		printf("read calib_config file %s false\n", filename);
		return false;
	}
	
	return true;
}

bool Surveying_system::Init_Ex_calib(char *file_name)
{
	cv::FileStorage fs(file_name, cv::FileStorage::READ);

	if (fs.isOpened())
	{
	    fs["calib_board_x"] >> ex_calib_collect.calib_bord_x;
	    fs["calib_board_y"] >> ex_calib_collect.calib_bord_y;
	    fs["d1"] >> ex_calib_collect.d1;
	    fs["d2"] >> ex_calib_collect.d2;
	    fs["front_rear"] >> ex_calib_collect.fort_rea;
	    fs["left_right"] >> ex_calib_collect.left_right;
	    fs["car_length"] >> ex_calib_collect.Length;
	    fs["car_width"] >> ex_calib_collect.Width;
	    fs["scale"] >> ex_calib_collect.scale;
	    
	    fs["calib_width"] >> ex_calib_space.width;
	    fs["calib_height"] >> ex_calib_space.height;
	    fs["front_rear_row"] >> ex_calib_space.front_row;
	    fs["left_right_col"] >> ex_calib_space.left_col;
		
		ex_calib_space.front_col = ex_calib_space.width;
		ex_calib_space.rear_row = ex_calib_space.front_row;
		ex_calib_space.rear_col = ex_calib_space.front_col;
		ex_calib_space.left_row = ex_calib_space.height;
		ex_calib_space.right_row = ex_calib_space.left_row;
		ex_calib_space.right_col = ex_calib_space.left_col;
		
		fs.release();
	}
	else
	{
		printf("read calib_config file %s false\n", file_name);
		return false;
	}
	
	return true;
}

bool Surveying_system::Init_camera_undistorted_config(char *file_name)
{
    cv::FileStorage fs(file_name, cv::FileStorage::READ);

    if (fs.isOpened())
    {
	memset(&camera_undistorted, 0, sizeof(Camera_undistorted));
	
	fs["image_width"] >> camera_undistorted.image_width;
	fs["image_height"] >> camera_undistorted.image_height;
	fs["undist_image_width"] >> camera_undistorted.undist_image_width;
	fs["undist_image_height"] >> camera_undistorted.undist_image_height;
        
	fs["scale"] >> camera_undistorted.scale;

	fs["front_width"] >> camera_undistorted.front.width;
	fs["front_height"] >> camera_undistorted.front.height;
	fs["front_offset_x"] >> camera_undistorted.front.offset_x;
	fs["front_offset_y"] >> camera_undistorted.front.offset_y;

	fs["rear_width"] >> camera_undistorted.rear.width;
	fs["rear_height"] >> camera_undistorted.rear.height;
	fs["rear_offset_x"] >> camera_undistorted.rear.offset_x;
	fs["rear_offset_y"] >> camera_undistorted.rear.offset_y;

	fs["left_width"] >> camera_undistorted.left_front.width;
	fs["left_height"] >> camera_undistorted.left_front.height;
	fs["left_front_offset_x"] >> camera_undistorted.left_front.offset_x;
	fs["left_front_offset_y"] >> camera_undistorted.left_front.offset_y;
        fs["left_rear_offset_x"] >> camera_undistorted.left_rear.offset_x;
	fs["left_rear_offset_y"] >> camera_undistorted.left_rear.offset_y;

	fs["right_width"] >> camera_undistorted.right_front.width;
	fs["right_height"] >> camera_undistorted.right_front.height;
	fs["right_front_offset_x"] >> camera_undistorted.right_front.offset_x;
	fs["right_front_offset_y"] >> camera_undistorted.right_front.offset_y;
        fs["right_rear_offset_x"] >> camera_undistorted.right_rear.offset_x;
	fs["right_rear_offset_y"] >> camera_undistorted.right_rear.offset_y;

	camera_undistorted.left_rear.width = camera_undistorted.left_front.width;
	camera_undistorted.left_rear.height = camera_undistorted.left_front.height;

	camera_undistorted.right_rear.width = camera_undistorted.right_front.width;
	camera_undistorted.right_rear.height = camera_undistorted.right_front.height;
	
	fs.release();
    }
    else
    {
	printf("read camera_undistorted_config file %s false\n", file_name);
	return false;
    }

    return true;
}

bool Surveying_system::Init_caram_data(int in_heigth, int in_width, int out_heigth, int out_width)
{
	caramdata.in_image_height = in_heigth;
	caramdata.in_image_width = in_width;
	caramdata.out_image_height = out_heigth;
	caramdata.out_image_width = out_width;
	int number = 2*out_heigth*out_heigth;

	caramdata.fornt_data = (float *)calloc(2 * ex_calib_space.front_row*ex_calib_space.front_col, sizeof(float));
	caramdata.rear_data = (float *)calloc(2 * ex_calib_space.rear_row*ex_calib_space.rear_col, sizeof(float));
	caramdata.left_data = (float *)calloc(2 * ex_calib_space.left_row*ex_calib_space.left_col, sizeof(float));
	caramdata.right_data = (float *)calloc(2 * ex_calib_space.right_row*ex_calib_space.right_col, sizeof(float));

	return true;
}

bool Surveying_system::Init_stich_data()
{
	if (stich_data.flag==1)
	{
		stich_data.front.f_data = (float *)calloc(2 * ex_calib_space.front_col*ex_calib_space.front_row, sizeof(float));
		stich_data.left.f_data = (float *)calloc(2 * ex_calib_space.left_col*ex_calib_space.left_row, sizeof(float));
		stich_data.rear.f_data = (float *)calloc(2 * ex_calib_space.rear_row*ex_calib_space.rear_col, sizeof(float));
		stich_data.right.f_data = (float *)calloc(2 * ex_calib_space.right_row*ex_calib_space.right_col, sizeof(float));
	}
	else
	{
		stich_data.front.s_data = (short *)calloc(2 * ex_calib_space.front_col*ex_calib_space.front_row, sizeof(short));
		stich_data.left.s_data = (short *)calloc(2 * ex_calib_space.left_col*ex_calib_space.left_row, sizeof(short));
		stich_data.rear.s_data = (short *)calloc(2 * ex_calib_space.rear_row*ex_calib_space.rear_col, sizeof(short));
		stich_data.right.s_data = (short *)calloc(2 * ex_calib_space.right_row*ex_calib_space.right_col, sizeof(short));
		
	}

	return true;

}

bool Surveying_system::Read_Caream_paraters()
{
    if (s_init_camera_paraters)
    {
        return true;
    }
    
	char *filename = "caream_paraters.txt";
	FILE *fp;
	std::vector<double> VcalIfo;
	double temp;
	std::ifstream file(filename);
	if (file)
	{
		while (file)
		{
			file >> temp;
			VcalIfo.push_back(temp);
			if (VcalIfo.size() >= 8)
			{
				caream_paraters.dx = VcalIfo[0];
				caream_paraters.dy = VcalIfo[1];
				caream_paraters.F  = VcalIfo[2];
				caream_paraters.k1 = VcalIfo[3];
				caream_paraters.k2 = VcalIfo[4];
				caream_paraters.k3 = VcalIfo[5];
				caream_paraters.k4 = VcalIfo[6];
				caream_paraters.k5 = VcalIfo[7];	

			}
		}
		file.close();
		return true;
	}
	else
	{
		printf("read caream_paraters file false\n");
		return false;
	}
}

bool Surveying_system::Read_Caream_paraters(char *file_name)
{
	std::vector<double> VcalIfo;
	double temp;
	std::ifstream file(file_name);
	if (file)
	{
		while (file)
		{
			file >> temp;
			VcalIfo.push_back(temp);
			if (VcalIfo.size() >= 8)
			{
				caream_paraters.dx = VcalIfo[0];
				caream_paraters.dy = VcalIfo[1];
				caream_paraters.F  = VcalIfo[2];
				caream_paraters.k1 = VcalIfo[3];
				caream_paraters.k2 = VcalIfo[4];
				caream_paraters.k3 = VcalIfo[5];
				caream_paraters.k4 = VcalIfo[6];
				caream_paraters.k5 = VcalIfo[7];	

			}
		}
		
		file.close();
		
		s_init_camera_paraters = 1;
		
		return true;
	}
	else
	{
		printf("read caream_paraters file false\n");
		return false;
	}
}

bool Surveying_system::UdistImge_ployfit(int  scale, int flage, Mat image, Mat &UdistImge, Mat &Umapx, Mat &Umapy)
{
	
	if (Read_Caream_paraters())
	{

		double dx = caream_paraters.dx;
		double dy = caream_paraters.dy;
		double F = caream_paraters.F;
		double f = F / dx;
		double pi = 3.1415926;
		double a1 = caream_paraters.k1;
		double a2 = caream_paraters.k2;
		double a3 = caream_paraters.k3;
		double a4 = caream_paraters.k4;
		double a5 = caream_paraters.k5;
		int height = image.rows;
		int width = image.cols * scale;
		int cx = width / 2;
		int cy = height / 2;
		int CX = width / (2 * scale);
		int CY = height / 2;
		vector<int>VX, VY;
		Umapx.create(height, width, CV_32FC1);
		Umapy.create(height, width, CV_32FC1);

		for (int y = 0; y < height; y++)
		{
			float* m1f = Umapx.ptr<float>(y);
			float* m2f = Umapy.ptr<float>(y);
			for (int x = 0; x < width; x++)
			{
				double r = sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
				double angle = 180 * atan(r / f) / pi;
				double R = a5 + a4*angle + a3*angle*angle + a2*angle*angle*angle + a1*angle*angle*angle*angle;
				double X = ((x - cx) / r)*(R / dx) + CX;
				double Y = ((y - cy) / r)*(R / dy) + CY;

				m1f[x] = (float)X;
				m2f[x] = (float)Y;
				VX.push_back((int)X);
				VY.push_back((int)Y);
			}
		}
		if (flage)
		{
			uchar *remap_image = (uchar *)calloc(3 * height*width, sizeof(uchar));
			uchar *imagdata = image.data;
			for (int i = 0; i < (int)VX.size(); i++)
			{

				int x = (int)(VX[i] + 0.5);
				int y = (int)(VY[i] + 0.5);
				if (x > 0 && y > 0)
				{
					int index_src = 3 * (y * /*width*/ image.cols + x);

					if (index_src < 3 * height * /*width*/ image.cols)
					{
						remap_image[i * 3 + 0] = imagdata[index_src + 0];
						remap_image[i * 3 + 1] = imagdata[index_src + 1];
						remap_image[i * 3 + 2] = imagdata[index_src + 2];
					}
				}
				else
				{
					remap_image[i * 3 + 0] = 0;
					remap_image[i * 3 + 1] = 0;
					remap_image[i * 3 + 2] = 0;
				}

			}

			UdistImge.create(height, width, CV_8UC3);
			Mat im(height, width, CV_8UC3, remap_image);
			im.copyTo(UdistImge);
			
			return true;
		}


	}
	else
	{
		return false;
	}
}


bool Surveying_system::Save_singleCam_udistfile(char*file, Mat image)
{
	FILE *fp, p;

	if ((fp = fopen(file, "wb")) == NULL)
	{
		return false;
	}
		
	int out_height = 600;//720
	int out_width = 800;//800
	Mat udist, udistout, Umapx, Umapy, image_res;
	udistout.create(out_height,out_width, CV_8UC3);
	
	if (Read_Caream_paraters())
	{

		double dx = caream_paraters.dx;
		double dy = caream_paraters.dy;
		double F = caream_paraters.F;
		double f = F /dx;
		double pi = 3.1415926;
		double a1 = caream_paraters.k1;
		double a2 = caream_paraters.k2;
		double a3 = caream_paraters.k3;
		double a4 = caream_paraters.k4;
		double a5 = caream_paraters.k5;
		int height = image.rows;
		int width = image.cols;
		int cx = out_width / 2;
		int cy = out_height / 2;
		int CX = width / 2;
		int CY = height / 2;
		vector<int>VX, VY;
		Umapx.create(out_height, out_width, CV_32FC1);
		Umapy.create(out_height, out_width, CV_32FC1);

		for (int y = 0; y < out_height; y++)
		{
			float* m1f = Umapx.ptr<float>(y);
			float* m2f = Umapy.ptr<float>(y);
			for (int x = 0; x < out_width; x++)
			{
				double r = sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
				double angle = 180 * atan(r / f) / pi;
				double R = a5 + a4*angle + a3*angle*angle + a2*angle*angle*angle + a1*angle*angle*angle*angle;
				double X = ((x - cx) / r)*(R / dx) + CX;
				double Y = ((y - cy) / r)*(R / dy) + CY;

				m1f[x] = (float)X;
				m2f[x] = (float)Y;
				VX.push_back((int)X);
				VY.push_back((int)Y);
			}
		}

		fwrite(&out_width, sizeof(short), 1, fp);
		fwrite(&out_height, sizeof(short), 1, fp);

		for (int i = 0; i < VX.size();i++)
		{
			short x = (short)VX[i];
			short y = (short)VY[i];
			fwrite(&x, sizeof(short), 1, fp);
			fwrite(&y, sizeof(short), 1, fp);
		}
	}

}

bool Surveying_system::Create_camera_calib_model(vector<Point2d> &coord, float scale, int width, int height)
{
    int calib_bord_x = ex_calib_collect.calib_bord_x;
    int calib_bord_y = ex_calib_collect.calib_bord_y;
    float d1 = ex_calib_collect.d1;
    float d2 = ex_calib_collect.d2;

    Point2d temp;

    float x = 0;
    float y = 0;

    for (int i = 0; i < calib_bord_y; i++)
    {
	for (int j = 0; j < calib_bord_x; j++)
	{
	    temp.x = j * d1 * scale;
	    temp.y = i * d2 * scale;
	    coord.push_back(temp);
	}
    }

    x = width / 2 - (coord[calib_bord_x - 1].x - coord[0].x) / 2;
    y = height / 2;

    for (int i = 0; i < calib_bord_y; i++)
    {
	for (int j = 0; j < calib_bord_x; j++)
	{
	    coord[i*calib_bord_x + j].x += x;
	    coord[i*calib_bord_x + j].y += y;
	}
    }
}

bool Surveying_system::Save_Camera_undistorted_file(char *file_name)
{
    FILE *fp;

    if ((fp = fopen(file_name, "wb")) == NULL)
    {
	return false;
    }

    double dx = caream_paraters.dx;
    double dy = caream_paraters.dy;
    double F = caream_paraters.F;
    double f = F /dx;
    double pi = 3.1415926;
    double a1 = caream_paraters.k1;
    double a2 = caream_paraters.k2;
    double a3 = caream_paraters.k3;
    double a4 = caream_paraters.k4;
    double a5 = caream_paraters.k5;
    int width = camera_undistorted.image_width;
    int height = camera_undistorted.image_height;
    int undist_width = camera_undistorted.undist_image_width;
    int undist_height = camera_undistorted.undist_image_height;
    int offset = 0;
    
    int cx = undist_width / 2;
    int cy = undist_height / 2;
    int CX = width / 2;
    int CY = height / 2;
    
    vector<short> vsx, vsy;
    vector<float> vfx, vfy;

    for (int y = 0; y < undist_height; y++)
    {
	for (int x = 0; x < undist_width; x++)
	{
	    double r = sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
	    double angle = 180 * atan(r / f) / pi;
	    double R = a5 + a4*angle + a3*angle*angle + a2*angle*angle*angle + a1*angle*angle*angle*angle;
	    double X = 0.0;
	    double Y = 0.0;
	    if (r != 0)
	    {
	        X = ((x - cx) / r)*(R / dx) + CX;
	        Y = ((y - cy) / r)*(R / dy) + CY;
	    }
	    else
	    {
		X = CX;
		Y = CY;
	    }

	    vfx.push_back((float)X);
	    vfy.push_back((float)Y);

	    vsx.push_back((short)(X + 0.5));
	    vsy.push_back((short)(Y + 0.5));
	}
    }

    //front
    fwrite(&camera_undistorted.front.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.front.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.front.height; r++)
    {
        for (int c = 0; c < camera_undistorted.front.width; c++)
	{
	    offset = (r + camera_undistorted.front.offset_y) * undist_width + (c + camera_undistorted.front.offset_x);
	    fwrite(&vsx[offset], sizeof(short), 1, fp);
            fwrite(&vsy[offset], sizeof(short), 1, fp);
	}
    }

    //rear
    fwrite(&camera_undistorted.rear.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.rear.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.rear.height; r++)
    {
        for (int c = 0; c < camera_undistorted.rear.width; c++)
	{
	    offset = (r + camera_undistorted.rear.offset_y) * undist_width + (c + camera_undistorted.rear.offset_x);
	    fwrite(&vsx[offset], sizeof(short), 1, fp);
            fwrite(&vsy[offset], sizeof(short), 1, fp);
	}
    }

#if 0
    //left_front
    fwrite(&camera_undistorted.left_front.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.left_front.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.left_front.height; r++)
    {
        for (int c = 0; c < camera_undistorted.left_front.width; c++)
	{
	    offset = (c + camera_undistorted.left_front.offset_x) * undist_width + (undist_width - 1 - (r + camera_undistorted.left_front.offset_y));
	    fwrite(&vsx[offset], sizeof(short), 1, fp);
            fwrite(&vsy[offset], sizeof(short), 1, fp);
	}
    }

    //right_front
    fwrite(&camera_undistorted.right_front.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.right_front.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.right_front.height; r++)
    {
        for (int c = 0; c < camera_undistorted.right_front.width; c++)
	{
	    offset = (undist_height - 1 - (c + camera_undistorted.right_front.offset_x)) * undist_width + (r + camera_undistorted.right_front.offset_y); 
	    fwrite(&vsx[offset], sizeof(short), 1, fp);
            fwrite(&vsy[offset], sizeof(short), 1, fp);
	}
    }

    //left_rear
    fwrite(&camera_undistorted.left_rear.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.left_rear.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.left_rear.height; r++)
    {
        for (int c = 0; c < camera_undistorted.left_rear.width; c++)
	{
	    offset = (c + camera_undistorted.left_rear.offset_x) * undist_width + (undist_width - 1 - (r + camera_undistorted.left_rear.offset_y));
	    fwrite(&vsx[offset], sizeof(short), 1, fp);
            fwrite(&vsy[offset], sizeof(short), 1, fp);
	}
    }

    //right_rear
    fwrite(&camera_undistorted.right_rear.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.right_rear.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.right_rear.height; r++)
    {
        for (int c = 0; c < camera_undistorted.right_rear.width; c++)
	{
	    offset = (undist_height - 1 - (c + camera_undistorted.right_rear.offset_x)) * undist_width + (r + camera_undistorted.right_rear.offset_y); 
	    fwrite(&vsx[offset], sizeof(short), 1, fp);
            fwrite(&vsy[offset], sizeof(short), 1, fp);
	}
    }
#else
    Mat umapx, umapy, tr_mapx, tr_mapy, H;
    Mat undistIm;
    Mat left_mapx, left_mapy;
    Mat right_mapx, right_mapy;
    vector<Point2d> corners, m_corners;
    
    Create_camera_calib_model(m_corners, camera_undistorted.scale, 1280 * 4, 720);
    

    if (!UdistImge_ployfit(4, 1, V_image[1], undistIm, umapx, umapy))
    {

	return false;
    }

    corners.clear();
    if (ConerDetect(board_size, undistIm, corners)) //coners detection
    {
	H = findHomography(m_corners, corners, 0 /*CV_FM_RANSAC*/);
        Warp_tran(H, tr_mapx, tr_mapy);
	Udist_warp_TransMap(0, umapx, umapy, tr_mapx, tr_mapy, left_mapx, left_mapy);
    }
    else
    {
	printf("no code points\n");
	return false;
    }

    if (!UdistImge_ployfit(4, 1, V_image[3], undistIm, umapx, umapy))
    {
	return false;
    }

    corners.clear();
    if (ConerDetect(board_size, undistIm, corners)) //coners detection
    {
	H = findHomography(m_corners, corners, 0 /*CV_FM_RANSAC*/);
        Warp_tran(H, tr_mapx, tr_mapy);
	Udist_warp_TransMap(0, umapx, umapy, tr_mapx, tr_mapy, right_mapx, right_mapy);
    }
    else
    {
	printf("no code points\n");
	return false;
    }

    //left_front
    fwrite(&camera_undistorted.left_front.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.left_front.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.left_front.height; r++)
    {
        for (int c = 0; c < camera_undistorted.left_front.width; c++)
	{
	    int x = (int)left_mapx.ptr<float>(c + camera_undistorted.left_front.offset_x)[1280 * 4 - 1 -  (r + camera_undistorted.left_front.offset_y)];
	    int y = (int)left_mapy.ptr<float>(c + camera_undistorted.left_front.offset_x)[1280 * 4 - 1 -  (r + camera_undistorted.left_front.offset_y)];

	    fwrite(&x, sizeof(short), 1, fp);
            fwrite(&y, sizeof(short), 1, fp);
	}
    }

    //right_front
    fwrite(&camera_undistorted.right_front.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.right_front.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.right_front.height; r++)
    {
        for (int c = 0; c < camera_undistorted.right_front.width; c++)
	{
	    int x = (int)right_mapx.ptr<float>(720 - 1 - (c + camera_undistorted.right_front.offset_x))[r + camera_undistorted.right_front.offset_y];
	    int y = (int)right_mapy.ptr<float>(720 - 1 - (c + camera_undistorted.right_front.offset_x))[r + camera_undistorted.right_front.offset_y];

	    fwrite(&x, sizeof(short), 1, fp);
            fwrite(&y, sizeof(short), 1, fp);
	}
    }

    //left_rear
    fwrite(&camera_undistorted.left_rear.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.left_rear.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.left_rear.height; r++)
    {
        for (int c = 0; c < camera_undistorted.left_rear.width; c++)
	{
            int x = (int)left_mapx.ptr<float>(c + camera_undistorted.left_rear.offset_x)[1280 * 4 - 1 -  (r + camera_undistorted.left_rear.offset_y)];
	    int y = (int)left_mapy.ptr<float>(c + camera_undistorted.left_rear.offset_x)[1280 * 4 - 1 -  (r + camera_undistorted.left_rear.offset_y)];

	    fwrite(&x, sizeof(short), 1, fp);
            fwrite(&y, sizeof(short), 1, fp);
	}
    }

    //right_rear
    fwrite(&camera_undistorted.right_rear.height, sizeof(short), 1, fp);
    fwrite(&camera_undistorted.right_rear.width, sizeof(short), 1, fp);

    for (int r = 0; r < camera_undistorted.right_rear.height; r++)
    {
        for (int c = 0; c < camera_undistorted.right_rear.width; c++)
	{
	    int x = (int)right_mapx.ptr<float>(720 - 1 - (c + camera_undistorted.right_rear.offset_x))[r + camera_undistorted.right_rear.offset_y];
	    int y = (int)right_mapy.ptr<float>(720 - 1 - (c + camera_undistorted.right_rear.offset_x))[r + camera_undistorted.right_rear.offset_y];

	    fwrite(&x, sizeof(short), 1, fp);
            fwrite(&y, sizeof(short), 1, fp);
	}
    }
#endif

    fclose(fp);

    return true;
}

bool Surveying_system::Read_singlecam(char*file,Mat image)
{
	FILE*fp;
	if ((fp = fopen(file, "rb")) == NULL)
	{
		return false;
	}

	short para[2] = { 0 };
	for (int i = 0; i < 2; i++)
	{
		fread(&para[i], sizeof(short), 1, fp);
	}
	int width = para[0];
	int height = para[1];
	short*data= (short *)calloc(2*height*width, sizeof(short));

	for (int i = 0; i < 2 * height*width;i++)
	{
		fread(&data[i], sizeof(short), 1, fp);
	}

	Mat remap_iamge;
	uchar* remap_imagep = 0;
	remap_iamge.create(height,width, CV_8UC3);

	int chns = image.channels();
	for (int i = 0; i < height; i++)
	{
		uchar* remap_data = (uchar*)(remap_iamge.data + remap_iamge.step*i);
		for (int j = 0; j < width; j++)
		{
			int index = i*width + j;
			int x = (int)data[2*index];
			int y = (int)data[2*index+1];
			if (x > 0 && y > 0 && y < image.rows&&x < image.cols)
			{
				uchar* distdata = (uchar*)(image.data + image.step*y);
				remap_data[j*chns + 0] = distdata[x*chns + 0];
				remap_data[j*chns + 1] = distdata[x*chns + 1];
				remap_data[j*chns + 2] = distdata[x*chns + 2];
			}
			else
			{
				remap_data[j*chns + 0] = 0;
				remap_data[j*chns + 1] = 0;
				remap_data[j*chns + 2] = 0;
			}
		}

	}

	imshow("image", remap_iamge);
	cvWaitKey(10);
}
