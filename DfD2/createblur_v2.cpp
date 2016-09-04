//#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <omp.h>
#include "allocate.h"
#include <iostream>
#include <fstream>
//#include "cv.h"
//#include "highgui.h"

#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>
#define MAX_CLASSES 256.0
#define MaxSigma 2.5

using namespace std;
using namespace cv;

///////// Space Varying 2D filter ///////////////////////////////////////////////////////
void SpaceVaryingfilter2D (IplImage * image, int kernel_size,double min_sigma, double max_sigma, IplImage * filter);


//void createblur(int col, int row, IplImage* ImageInFocus, int classes,double **y[],double **xt[], double **atlas[], int ATLAS, IplImage*ImageOutOfFocus, IplImage * SyntheticDefocus, IplImage* GauBlur[],unsigned char **xttemp[], IplImage *groundtruth)
//void createblur(int col, int row, IplImage* ImageInFocus_I, int classes, double **y[], double **xt[], double **atlas[], int ATLAS, IplImage* ImageOutOfFocus_I, IplImage* SyntheticDefocus_I, IplImage* GauBlur[], Mat &xttemp)
//void createblur(int col, int row, Mat ImageInFocus, int classes, double **xt[], double **atlas[], int ATLAS, Mat ImageOutOfFocus, Mat SyntheticDefocus, Mat &xttemp)
void createblur(int col, int row, Mat ImageInFocus, int classes, vector<Mat> &xt, double **atlas[], int ATLAS, Mat ImageOutOfFocus, Mat SyntheticDefocus, Mat &xttemp)
{
	int i, j, dd, kk, step,channels;
	int idx, jdx, kdx;
	double x = rand()%1001;
	double flag, BlurStep, para;
	CvScalar r, s, t, a;
    int kernel_size;

	//Mat ImageOutOfFocus = Mat(ImageOutOfFocus_I);
	//Mat SyntheticDefocus = Mat(SyntheticDefocus_I);
	//Mat ImageInFocus = Mat(ImageInFocus_I);


  ///// Initialize image data structure ////////////////////////////////////////////////////////////////////////
	//col = ImageInFocus->width , row = ImageInFocus->height;
	//for ( dd = 0; dd <= classes+1; dd++)
	//{
	//	xt[dd] = (double **)get_img(col,row,sizeof(double));
	//}
	
	//y[0] = (double **)get_img(col,row,sizeof(double));
	//atlas[0] = (double **)get_img(col, row, sizeof(double));
	//
	//for (i = 0; i < row; i++)
	//{
	//	for (j = 0; j < col; j++)
	//	{
	//		atlas[0][i][j] = 0;
	//	}
	//}

 ///// Loading the atlas for Gamma ////////////////////////////////////////////////////////////////////////////
	if(ATLAS==1)
	{
	    IplImage* inputdepth = cvLoadImage("TrueBlurrMap000_remake.png",1);
	    IplImage* graydepth = cvCreateImage(cvGetSize(inputdepth), 8, 1);
		for (i = 0; i < row; i++)
		{
			for (j = 0; j < col; j++)
			{
				a = cvGet2D(graydepth, i, j);
				atlas[0][i][j] = a.val[0];
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
			//xt[0][idx][jdx] = classes / 2;
			xt[0].at<double>(idx, jdx) = classes / 2.0;
			for (kdx = 0; kdx <= classes; kdx++)
			{
				if (((double)(kdx) / (double)(classes) < flag) && (flag <= (double)(kdx + 1) / (double)(classes)))
				{
					//xt[0][idx][jdx] = kdx;
					xt[0].at<double>(idx, jdx) = (double)kdx;
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
			//xt[1][idx][jdx] = r.val[0];
			//xt[1][idx][jdx] = ImageInFocus.at<double>(idx,jdx);
			xt[1].at<double>(idx, jdx) = ImageInFocus.at<double>(idx, jdx);
		}
	}

 /////  Initialize input blur array ////////////////////////////////////////////////////////////////////////////
	// y is the same as ImageOutOfFocus
	//for (i = 0; i < row; i++)
	//{
	//	for (j = 0; j < col; j++)
	//	{
	//		//s = cvGet2D(ImageOutOfFocus, i, j);
	//		//y[0][i][j] = s.val[0];
	//		y[0][i][j] = ImageOutOfFocus.at<double>(i, j);

	//	}
	//}

 /////// Create Multi-level blur image //////////////////////////////////////////////////////////////////////////

	for( int l = 0 ;l < classes ; l = ++l )
	{
		BlurStep = MaxSigma/(double(MAX_CLASSES));
		//para = BlurStep * l;
		para = BlurStep * (l+1);

		//cvSmooth( ImageInFocus , SyntheticDefocus , CV_GAUSSIAN, 0, 0, para, para);
		GaussianBlur(ImageInFocus, SyntheticDefocus, Size(0, 0), para, para, BORDER_REPLICATE);
		
		//kernel_size = cvRound(para*8 + 1)|1;
		//SpaceVaryingfilter2D (ImageInFocus, kernel_size, para, 0.5*BlurStep, SyntheticDefocus);
		//GauBlur[l] = new IplImage(SyntheticDefocus);

		for (idx = 0; idx < row; idx++)
		{
			for (jdx = 0; jdx < col; jdx++)
			{
				//t = cvGet2D(GauBlur[l], idx, jdx);
				//xt[l + 1][idx][jdx] = t.val[0];
				//xt[l + 1][idx][jdx] = SyntheticDefocus.at<double>(idx, jdx);
				xt[l + 1].at<double>(idx, jdx) = SyntheticDefocus.at<double>(idx, jdx);
			}
		}
	}
    

///// Use initial depth map as starting point, here "groundtruth" is initial depth map //////////////////////////
	//xttemp[0] = (unsigned char **)get_img(col,row,sizeof(double));

	//Mat temp = Mat(Size(col, row), CV_64FC1, *xt[0], 2688);
	
	xt[0].convertTo(xttemp, CV_8UC1, 1, 0);

	//for (i = 0; i < row; i++)
	//{
	//	for (j = 0; j < col; j++)
	//	{
	//		xttemp[0][i][j] = xt[0][i][j];
	//	}
	//}
	//{xttemp[0][i][j]=cvGetReal2D(groundtruth,i,j);xttemp[0][i][j] = unsigned(255.0-((256.0/MAX_CLASSES)*(xttemp[0][i][j]-1.0)));}

}	// end ofcreateblur