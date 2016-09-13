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
//void createblur(int col, int row, Mat ImageInFocus, int classes, vector<Mat> &xt, double **atlas[], int ATLAS, Mat ImageOutOfFocus, Mat SyntheticDefocus, Mat &xttemp)
void createblur(int col, int row, Mat ImageInFocus, int classes, vector<Mat> &xt, Mat SyntheticDefocus, Mat &xttemp)
{
	int i, j, dd, kk, step,channels;
	int idx, jdx, kdx;
	double x = rand()%1001;
	double flag, BlurStep, para;
    int kernel_size;

  /////  Initialize Markov Random Field ///////////////////////////////////////////////////////////////////////
	for (idx = 0; idx < row; idx++)
	{
		for (jdx = 0; jdx < col; jdx++)
		{
			x = rand() % 1001;
			flag = x / 500;

			xt[0].at<double>(idx, jdx) = classes / 2.0;
			for (kdx = 0; kdx <= classes; kdx++)
			{
				if (((double)(kdx) / (double)(classes) < flag) && (flag <= (double)(kdx + 1) / (double)(classes)))
				{
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
			xt[1].at<double>(idx, jdx) = ImageInFocus.at<double>(idx, jdx);
		}
	}

 /////// Create Multi-level blur image //////////////////////////////////////////////////////////////////////////

	for( int l = 0 ;l < classes ; l = ++l )
	{
		BlurStep = MaxSigma/(double(MAX_CLASSES));
		para = BlurStep * (l+1);

		GaussianBlur(ImageInFocus, SyntheticDefocus, Size(0, 0), para, para, BORDER_REPLICATE);
		
		//kernel_size = cvRound(para*8 + 1)|1;
		//SpaceVaryingfilter2D (ImageInFocus, kernel_size, para, 0.5*BlurStep, SyntheticDefocus);

		for (idx = 0; idx < row; idx++)
		{
			for (jdx = 0; jdx < col; jdx++)
			{
				xt[l + 1].at<double>(idx, jdx) = SyntheticDefocus.at<double>(idx, jdx);
			}
		}
	}

///// Use initial depth map as starting point, here "groundtruth" is initial depth map //////////////////////////

	xt[0].convertTo(xttemp, CV_8UC1, 1, 0);


}	// end ofcreateblur