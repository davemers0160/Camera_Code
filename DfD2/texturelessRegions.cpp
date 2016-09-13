#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>


#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>

#include <time.h>
#include <fstream>

using namespace std;
using namespace cv;

void texturelessRegions(Mat &inputImage, Mat &textureImage, int windowSize, int thresh)
{
	int idx, jdx, kdx, ldx;
	int nChannels = inputImage.channels();
	int nr = inputImage.rows;
	int nc = inputImage.cols;
	double sum, diff, avg;
	int scale = 1;
	int delta = 0;

	Mat threshGrad;
	Mat erodeThresh;
	Mat lapGrad;

	int bp_stop = 0;

	if (windowSize % 2 == 0)
	{
		windowSize++;
		cout << "Window size not odd, increasing by 1." << endl;
		cout << "New window size: " << windowSize << endl;
	}
	
	//int win = (windowSize - 1) / 2;

	//Mat texturelessImg = Mat::zeros(nr, nc, CV_8UC1);

	//Mat sqGradImg = Mat::zeros(nr, nc, CV_8UC1);

	// = Mat::zeros(nr, nc, CV_8UC1);

	//Mat inputImage;// = Mat(Size(nr, nc), CV_32F);
	//img.convertTo(inputImage, CV_32F,1/255.0);

	// BORDER_REPLICATE / BORDER_DEFAULT
	//MORPH_RECT / MORPH_ELLIPSE / MORPH_CROSS
	Mat element = getStructuringElement(MORPH_RECT,	Size(7,7),	Point(-1, -1));

	//Mat grad_x, grad_y;
	//Mat scharrGrad;
	//Scharr(inputImage, grad_x, CV_8UC1, 1, 0, scale, delta, BORDER_REPLICATE);
	//Scharr(inputImage, grad_y, CV_8UC1, 0, 1, scale, delta, BORDER_REPLICATE);
	//addWeighted(grad_x, 0.5, grad_y, 0.5, 0, scharrGrad);


	//Mat sob_x, sob_y;
	//Mat sobelGrad;
	//Sobel(inputImage, sob_x, CV_8UC1, 1, 0, 5, scale, delta, BORDER_REPLICATE);
	//Sobel(inputImage, sob_y, CV_8UC1, 0, 1, 5, scale, delta, BORDER_REPLICATE);
	//addWeighted(sob_x, 0.5, sob_y, 0.5, 0, sobelGrad);

	Laplacian(inputImage, lapGrad, CV_8UC1, windowSize, scale, delta, BORDER_REPLICATE);

	threshold(lapGrad, threshGrad, thresh, 255, THRESH_BINARY_INV);

	//erode(threshGrad, erodeThresh, element);
	erode(threshGrad, textureImage, element);


		//% Produce Squared Horizontal Gradient image sqGradImg
	//for (idx = 0; idx < nr; idx++)
	//{
	//	for (jdx = 0; jdx < (nc - 1); jdx++)
	//	{
	//		sum = 0.0;

	//		if (inputImage.dims >= 3)
	//		{
	//			for (kdx = 0; kdx < nChannels; kdx++)
	//			{
	//				diff = inputImage.at<uchar>(idx, jdx, kdx) - inputImage.at<uchar>(idx, jdx + 1, kdx);
	//				sum = sum + (diff*diff);
	//			}
	//		}
	//		else
	//		{
	//			diff = inputImage.at<uchar>(idx, jdx) - inputImage.at<uchar>(idx, jdx + 1);
	//			sum = sum + (diff*diff);
	//		}
	//		sum = sum / nChannels;
	//		sqGradImg.at<uchar>(idx, jdx + 1) = sum;

	//		if (jdx == 0)
	//		{
	//			sqGradImg.at<uchar>(idx, jdx) = sum;
	//		}
	//		
	//		if (sum > sqGradImg.at<uchar>(idx, jdx))
	//		{	
	//			sqGradImg.at<uchar>(idx, jdx) = sum;
	//		}
	//		
	//		
	//	}
	//}




	//% Compute average within predefined box window of size windowSize x windowSize
	//for (idx = win; idx < nr - win; idx++)
	//{
	//	for (jdx = win; jdx < nc - win; jdx++)	//% go over the square window
	//	{
	//		
	//		sum = 0.0;
	//		avg = 0.0;
	//		for (kdx = -win; kdx < win; kdx++)	//for (a=-win:1:win)
	//		{
	//			for (ldx = -win;  ldx < win; ldx++)		//for (b=-win:1:win) 
	//			{
	//				sum = sum + sqGradImg.at<uchar>(idx + kdx, jdx + ldx);
	//			}
	//		}
	//		//% Compute the average
	//		avg = sum / (windowSize*windowSize);
	//		
	//		//% Apply threshold
	//		if (avg < (thresh*thresh))
	//		{
	//			texturelessImg.at<uchar>(idx, jdx) = 255; //% mark detected textureless pixel as white
	//		}
	//	}
	//}


	bp_stop = 0;

}	// end of texturelessRegions