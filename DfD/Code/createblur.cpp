//#include "stdafx.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <omp.h>
#include "allocate.h"
#include <iostream>
#include <fstream>
#include <Windows.h>

#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  

using namespace std;
using namespace cv;

#define MAX_CLASSES 256.0
#define MaxSigma 5.0


//void createblur(int col, int row, IplImage* ImageInFocus, int classes,double **y[],double **xt[], double **atlas[], int ATLAS, IplImage*ImageOutOfFocus, IplImage * SyntheticDefocus, IplImage* GauBlur[],unsigned char **xttemp[], IplImage *groundtruth)

void createblur(Mat ImageInFocus, int classes, double **y[], double **xt[], double **atlas[], int ATLAS, Mat ImageOutOfFocus, Mat SyntheticDefocus, unsigned char **xttemp[], Mat groundtruth)
{
	int idx, jdx, kdx, dd, kk, step, channels;
	double x = rand() % 1001;
	double flag, BlurStep, para;
	CvScalar r, s, t, a;
	int col = ImageInFocus.cols;
	int row = ImageInFocus.rows;

	vector<Mat> GauBlur(260);

	///// Initialize image data structure ////////////////////////////////////////////////////////////////////////
	
	for (dd = 0; dd <= classes + 1; dd++)
	{
		xt[dd] = (double **)get_img(col, row, sizeof(double));
	}

	y[1] = (double **)get_img(col, row, sizeof(double));
	atlas[0] = (double **)get_img(col, row, sizeof(double));

	for (idx = 0; idx < row; idx++)
	{	
		for (jdx = 0; jdx < col; jdx++)
		{
			atlas[0][idx][jdx] = 0;
		}
	}


 ///// Loading the atlas for Gamma ////////////////////////////////////////////////////////////////////////////
	if(ATLAS==1)
	{
		//IplImage* inputdepth = cvLoadImage("TrueBlurrMap000_remake.png",1);
		//IplImage* graydepth = cvCreateImage(cvGetSize(inputdepth), 8, 1);

		Mat inputdepth = imread("TrueBlurrMap000_remake.png", CV_LOAD_IMAGE_COLOR);
		Mat graydepth = Mat(inputdepth.size(), CV_8UC1);
			
		for (idx = 0; idx < row; idx++)
		{
			for (jdx = 0; jdx < col; jdx++)
			{
				//a = cvGet2D(graydepth, i, j);
				a = graydepth.at<UINT8>(idx, jdx);
				atlas[0][idx][jdx] = a.val[0];
			}
		}
	}


  /////  Initialize Markov Random Field ///////////////////////////////////////////////////////////////////////
	for (idx = 0; idx < row; idx++)
	{
		for (jdx = 0; jdx < col; jdx++)
		{
			x = rand() % 1001;
			flag = x / 500;
			xt[0][idx][jdx] = classes / 2;
			for (kk = 1; kk <= classes; kk++)
			{
				if (((double)(kk) / (double)(classes) < flag) && (flag <= (double)(kk + 1) / (double)(classes)))
				{
					xt[0][idx][jdx] = kk;
					break;
				}
			}
		}
	}

	for (idx = 0; idx < row; idx++)
	{
		for (jdx = 0; jdx < col; jdx++)
		{
			//r = cvGet2D(ImageInFocus, idx, jdx);
			r = ImageInFocus.at<double>(idx, jdx);
			xt[1][idx][jdx] = r.val[0];
		}
	}

 /////  Initialize input blur array ////////////////////////////////////////////////////////////////////////////
	for (idx = 0; idx < row; idx++)
	{
		for (jdx = 0; jdx < col; jdx++)
		{
			//s = cvGet2D(ImageOutOfFocus, i, j);
			s = ImageOutOfFocus.at<double>(idx, jdx);
			y[1][idx][jdx] = s.val[0];
		}
	}
			
 /////// Create Multi-level blur image //////////////////////////////////////////////////////////////////////////
	for (kdx = 1; kdx < classes + 1; kdx++)
	{
		BlurStep =MaxSigma/(double(MAX_CLASSES));
		para = BlurStep * kdx;

		//cvSmooth( ImageInFocus , SyntheticDefocus , CV_GAUSSIAN, 0, 0, para, para);
		GaussianBlur(ImageInFocus, SyntheticDefocus, Size(0, 0), para, para, BORDER_REPLICATE);

		GauBlur[kdx - 1] = Mat(ImageInFocus.size(), CV_64F);

		//GauBlur[kdx - 1] = SyntheticDefocus;
		SyntheticDefocus.copyTo(GauBlur[kdx - 1]);

		for (idx = 0; idx < row; idx++)
		{
			for (jdx = 0; jdx < col; jdx++)
			{
				//t = cvGet2D(GauBlur[kdx - 1], idx, jdx);
				//t = GauBlur._Myfirst[kdx - 1].data(jdx+idx*col);
				t = GauBlur[kdx - 1].at<double>(idx,jdx);
				xt[kdx][idx][jdx] = t.val[0];
			}
		}
	}
    

///// Use initial depth map as starting point, here "groundtruth" is initial depth map //////////////////////////
	xttemp[0] = (unsigned char **)get_img(col,row,sizeof(double));
	for (idx = 0; idx < row; idx++)
	{
		for (jdx = 0; jdx < col; jdx++)	
		{
			//{xttemp[0][i][j] = xt[0][i][j];}
			//xttemp[0][i][j] = cvGetReal2D(groundtruth, i, j);
			xttemp[0][idx][jdx] = groundtruth.at<double>(idx, jdx);
			xttemp[0][idx][jdx] = unsigned(255.0 - ((256.0 / MAX_CLASSES)*(xttemp[0][idx][jdx] - 1.0)));
		}
	}


}