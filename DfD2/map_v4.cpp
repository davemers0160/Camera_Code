//#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
//#include <omp.h>
#include "allocate.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>

// added
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>

// GC
#include "GCoptimization.h"

#include <time.h>
#include <fstream>

using namespace std;
using namespace cv;

#define MAX_CLASSES 256
#define MAXPRIME  2147483647       /*  MAXPRIME = (2^31)-1     */
#define PI        3.14159265358979323846

//unsigned char	**get_img(int,int,unsigned char);
//unsigned char	**gamma[1000], **atlas[1000];
int				i, j, l, k, c, r, edgevalue, edgevalueCr, edgevalueCb, texturevalue, texturevalueCr, texturevalueCb, tempmin, counter = 0;
double			random2(),mm,sum,AveCost,diff[MAX_CLASSES+1], prior[MAX_CLASSES+1],DiSum,PiSum, assist;
double			**xrv, x, flag, ratio,current,invannealtemp, compare[MAX_CLASSES];

ofstream outfile("logpost.txt",ios::out);

// 0. double callogpost(cv::Point pixel, double **diff_sq[], double **atlas[], unsigned char **xttemp[], int cols, int rows, int ATLAS, double gama, int classes, double beta, double *d, double *con, double *logpost, int edgevalue, int texturevalue, IplImage *highpass, IplImage* texture);
// 1. double callogpost(cv::Point pixel, double **diff_sq[], double **atlas[], unsigned char **xttemp[], int cols, int rows, int ATLAS, double gama, int classes, double beta, double *d, double *con, double *logpost, int edgevalue, int texturevalue, Mat highpass, Mat texture);
// 2. double callogpost(cv::Point pixel, double **diff_sq[], double **atlas[], Mat xttemp, int cols, int rows, int ATLAS, double gamma, int classes, double beta, double *d, double *con, double *logpost, int edgevalue, int texturevalue, Mat highpass, Mat texture);
// 3. double callogpost(cv::Point pixel, vector<Mat> &diff_sq, Mat xttemp, int cols, int rows, double gamma, int classes, double beta, double *d, double *con, double *logpost, int edgevalue, int texturevalue, Mat highpass, Mat texture);
void callogpost(cv::Point pixel, vector<Mat> &diff_sq, Mat xttemp, int cols, int rows, int classes, double beta, double *logpost, int edgevalue, int texturevalue, Mat highpass, Mat texture);

//void GridGraph_DArraySArray(int width,int height,int num_labels,double **logpost1[],int *result);
//void GridGraph_DArraySArray(int width, int height, int num_labels, vector<cv::Mat> &logpost1, int *result);
void GridGraph_DArraySArray(int width, int height, int num_labels, vector<cv::Mat> &logpost1, cv::Mat &gridResult, string &DataLog);

//void GeneralGraph_DArraySArray(int width,int height,int num_labels,double **logpost1[], int *result);

///////////////////////////////////////////////////////////////////////////////////
//																				 //
//    Part 1:																	 //
//			Generate Random numbers                                              //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////


/* PORTABILITY 1:  The functions in this file assume that a long is 32 bits
      and a short is 16 bits.  These conventions are machine dependent and
      must therefore be verified before using.                     */

static unsigned long   sd[2],tmp;   /*  tmp: 31 bit seed in GF( (2^31)-1 )   */
                                    /*  sd[0]: high order 15 bits of tmp     */
                                    /*  sd[1]: low order 16 bits of tmp      */
                 /* NOTE: high order 16 bits of sd[0] and sd[1] are 0        */

unsigned long srandom2(unsigned long num)
/* Set a new seed for random # generator  */
{
	tmp=num;
	*sd=tmp>>16;
	*(sd+1)=tmp & 0xffff;
	return *sd;
}
/*
void readseed()
/*  Reads random # generator seed from file: /tmp/randomseedmlc
{
 FILE	*fp1;
 char	*calloc();
 void	writeseed();

   if((fp1 = fopen("randomseedmlc","r"))==NULL ) {
     fprintf(stderr,"readseed: creating file /tmp/randomseedmlc\n");
     tmp=143542612;
     writeseed();
     srandom2(tmp);
   } else {
     fscanf(fp1,"%d",&tmp);
     srandom2(tmp);
     fclose(fp1);
   }
}

void writeseed()
  Writes random # generator seed from file: /tmp/randomseedmlc
{
 FILE  *fp1;
 char	*calloc();

   if((fp1 = fopen("randomseedmlc","w"))==NULL ) {
     fprintf(stderr,"writeseed: can't open file /tmp/randomseedmlc\n");
     exit(1);
   } else {
     fprintf(fp1,"%d",tmp);
     fclose(fp1);
   }
}
*/

// Uniform random number generator on (0,1]
//Algorithm:  newseed = (16807 * oldseed) MOD [(2^31) - 1]  ;
//returned value = newseed / ( (2^31)-1 )  ;
//newseed is stored in tmp and sd[0] and sd[1] are updated;
//Since 16807 is a primitive element of GF[(2^31)-1], repeated calls
//to random2() should generate all positive integers <= MAXPRIME
//before repeating any one value.
//Tested: Feb. 16, 1988;  verified the length of cycle of integers 
//generated by repeated calls to random2() 
double random2()

{
	*(sd+1) *= 16807;
	*sd *= 16807;

	tmp=((*sd)>>15)+(((*sd)&0x7fff)<<16);
	tmp += (*(sd+1));

	if ( tmp & 0x80000000 ) 
	{
		tmp++;
		tmp &= 0x7fffffff;
	}

	*sd=tmp>>16;
	*(sd+1)=tmp & 0xffff;

	return(((double)tmp)/MAXPRIME);   
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//																				 //
//    Part 2:																	 //
//			Subfunction of calculating logpost                                   //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////


//double callogpost(cv::Point pixel, double **diff_sq[], double **atlas[], unsigned char **xttemp[], int cols, int rows, int ATLAS, double gama, int classes, double beta, double *d, double *con, double logpost[], int edgevalue, int texturevalue, Mat highpass, Mat texture)
//double callogpost(cv::Point pixel, double **diff_sq[], double **atlas[], Mat xttemp, int cols, int rows, int ATLAS, double gamma, int classes, double beta, double *d, double *con, double logpost[], int edgevalue, int texturevalue, Mat highpass, Mat texture)
void callogpost(cv::Point pixel, vector<Mat> &diff_sq, Mat xttemp, int cols, int rows, int classes, double beta, double *logpost, int edgevalue, int texturevalue, Mat highpass, Mat texture)
{
	int idx, jdx, kdx;
	int edgevalue1,texture1;
	double weight;
	double alpha = 1.0;
	double tempsq;

//////////// More Texture region //////////////////////////////////////////////////////////////////////////////////////
	int i, j;

	i = pixel.x;
	j = pixel.y;

	if (texturevalue == 0 ) // more texture region
	{					
		for (k=0; k<=MAX_CLASSES; k++)
		{
			prior[k] = 0;
			diff[k] = 0;
			diff[k] += diff_sq[k].at<double>(i, j);

			//**************************************************************************//
			if (i-1 >= 0) 
			{                       							
				//prior[k]+=(double)abs(xttemp[0][i-1][j]-k); 
				prior[k] += (double)abs(xttemp.at<unsigned char>(i - 1,j) - k);
				//diff[k]+=diff_sq[k][i-1][j]; //neighbors difference       left	
				diff[k] += diff_sq[k].at<double>(i-1, j);
									
				if (j-1 >=0) 
				{
					//diff[k]+=diff_sq[k][i-1][j-1]; //neighbors difference    left top  
					diff[k] += diff_sq[k].at<double>(i-1, j-1);
					//prior[k]+=(double)abs(xttemp[0][i-1][j-1]-k); 
					prior[k] += (double)abs(xttemp.at<unsigned char>(i - 1, j - 1) - k);
				} 

				if (j+1 <= cols-1) 
				{
					//diff[k]+=diff_sq[k][i-1][j+1]; //neighbors difference   left bottom
					diff[k] += diff_sq[k].at<double>(i-1, j+1);
					//prior[k]+=(double)abs(xttemp[0][i-1][j+1]-k); 
					prior[k] += (double)abs(xttemp.at<unsigned char>(i - 1, j + 1) - k);
				} 
			}

			//**************************************************************************//
			if (i+1 <= rows-1) 
			{						
				//prior[k]+=(double)abs(xttemp[0][i+1][j]-k);
				prior[k] += (double)abs(xttemp.at<unsigned char>(i + 1, j) - k);
				//diff[k]+=diff_sq[k][i+1][j]; //neighbors difference
				diff[k] += diff_sq[k].at<double>(i+1, j);

				if (j-1 >=0) 
				{
					//diff[k]+=diff_sq[k][i+1][j-1];
					diff[k] += diff_sq[k].at<double>(i+1, j-1);
					//prior[k]+=(double)abs(xttemp[0][i+1][j-1]-k); 
					prior[k] += (double)abs(xttemp.at<unsigned char>(i + 1, j - 1) - k);
				} 									
				if (j+1 <= cols-1) 
				{      
					//diff[k]+=diff_sq[k][i+1][j+1];
					diff[k] += diff_sq[k].at<double>(i+1, j+1);
					//prior[k]+=(double)abs(xttemp[0][i+1][j+1]-k); 
					prior[k] += (double)abs(xttemp.at<unsigned char>(i + 1, j + 1) - k);
				}
			}

			//**************************************************************************//
			if (j-1 >=0) 
			{									
				//prior[k]+=(double)abs(xttemp[0][i][j-1]-k);	
				prior[k] += (double)abs(xttemp.at<unsigned char>(i, j - 1) - k);
				//diff[k]+=diff_sq[k][i][j-1]; //neighbors difference
				diff[k] += diff_sq[k].at<double>(i, j-1);
			}

			//**************************************************************************//
			if (j+1 <= cols-1) 
			{									
				//prior[k]+=(double)abs(xttemp[0][i][j+1]-k);
				prior[k] += (double)abs(xttemp.at<unsigned char>(i, j + 1) - k);
				//diff[k]+=diff_sq[k][i][j+1]; //neighbors difference
				diff[k] += diff_sq[k].at<double>(i, j+1);
			}
		}
	}	// end of if (texturevalue == 0 )

//////////// Less Texture region //////////////////////////////////////////////////////////////////////////////

	if (texturevalue == 255  ) // less texture 
	{	
		for (k=0; k<=MAX_CLASSES; k++)
		{
			prior[k] = 0;
			diff[k] = 0;
			//diff[k] += diff_sq[k][i][j];
			diff[k] += diff_sq[k].at<double>(i, j);

			//**************************************************************************//
			if (i-1 >= 0) 
			{                       
				//if (xttemp[0][i-1][j] != k)
				if(xttemp.at<unsigned char>(i - 1, j) != k)
				{	
					//texture1=cvGetReal2D(texture, i-1, j);
					texture1 = texture.at<unsigned char>(i - 1, j);
					if (texture1==0 ) 
					{
						alpha=0;
						weight=10;
						//if (abs(xttemp[0][i - 1][j] - xttemp[0][i][j]) < 1)
						if (xttemp.at<unsigned char>(i - 1, j) - xttemp.at<unsigned char>(i, j) == 0)
						{
							texture.at<unsigned char>(i, j) = 0;
						}
					}
					else 
					{
						edgevalue1 = highpass.at<unsigned char>(i - 1, j);
						if (edgevalue1 == 0)
						{
							weight = 0.08;
						}
						else         // if (edgevalue1 != 0)
						{
							weight = 1.0;
						}
					}										
					//prior[k]+=weight*(double)abs(xttemp[0][i-1][j]-k);
					prior[k] += weight*(double)abs(xttemp.at<unsigned char>(i - 1, j) - k);
				}

				//diff[k]+=diff_sq[k][i][j]; //neighbors difference       left
				diff[k] += diff_sq[k].at<double>(i, j);

				if (j - 1 >= 0)
				{
					//(diff[k] += diff_sq[k][i - 1][j - 1]); //neighbors difference    left top 
					diff[k] += diff_sq[k].at<double>(i-1, j-1);
				}

				if (j + 1 <= cols - 1)
				{
					//(diff[k] += diff_sq[k][i - 1][j + 1]); //neighbors difference   left bottom
					diff[k] += diff_sq[k].at<double>(i-1, j+1);
				}
			}

			//**************************************************************************//
			if (i+1 <= rows-1) 
			{					
				//if (xttemp[0][i+1][j] != k) 
				if (xttemp.at<unsigned char>(i + 1, j) != k)
				{
					//texture1=cvGetReal2D(texture, i+1, j);	
					texture1 = texture.at<unsigned char>(i + 1, j);
					if (texture1==0 )
					{
						alpha=0;
						weight=10;
						//if (abs(xttemp[0][i + 1][j] - xttemp[0][i][j]) < 1)
						if (xttemp.at<unsigned char>(i + 1, j) - xttemp.at<unsigned char>(i, j) == 0)
						{
							//cvSetReal2D(texture, i, j, 0);
							texture.at<unsigned char>(i, j) = 0;
						}
					}
					else 
					{
						//edgevalue1=cvGetReal2D(highpass, i+1, j); 
						edgevalue1 = highpass.at<unsigned char>(i + 1, j);
						if (edgevalue1 == 0)
						{
							weight = 0.08;
						}
						else      //if (edgevalue1 != 0) 
						{
							weight = 1.0;
						}
					}
					//prior[k]+=weight*(double)abs(xttemp[0][i+1][j]-k);
					prior[k] += weight*(double)abs(xttemp.at<unsigned char>(i + 1, j) - k);
				}
									
				if (j - 1 >= 0)
				{
					//(diff[k] += diff_sq[k][i + 1][j - 1]); //neighbors difference
					diff[k] += diff_sq[k].at<double>(i+1, j-1);
				}

				//diff[k]+=diff_sq[k][i+1][j]; //neighbors difference
				diff[k] += diff_sq[k].at<double>(i+1, j);

				if (j + 1 <= cols - 1)
				{
					//(diff[k] += diff_sq[k][i + 1][j + 1]); //neighbors difference//////////////////////////////////////error
					diff[k] += diff_sq[k].at<double>(i+1, j+1);
				}
			}

			//**************************************************************************//
			if (j-1 >=0) 
			{
				//if (xttemp[0][i][j-1] != k) 
				if (xttemp.at<unsigned char>(i, j - 1) != k)
				{
					//texture1=cvGetReal2D(texture, i, j-1);	
					texture1 = texture.at<unsigned char>(i, j-1);
					if (texture1==0 ) 
					{
						alpha=0;
						weight=10;
						//if (abs(xttemp[0][i][j - 1] - xttemp[0][i][j]) < 1)
						if (xttemp.at<unsigned char>(i, j - 1) - xttemp.at<unsigned char>(i, j) == 0)
						{
							//cvSetReal2D(texture, i, j, 0);
							texture.at<unsigned char>(i, j) = 0;
						}
					}
					else 
					{
						//edgevalue1=cvGetReal2D(highpass, i, j-1); 
						edgevalue1 = highpass.at<unsigned char>(i, j-1);
						if (edgevalue1 == 0)
						{
							weight = 0.08;
						}
						else
						{
							weight = 1.0;
						}
						//if (edgevalue1 != 0) weight = 1.0;
					}
					//prior[k]+=weight*(double)abs(xttemp[0][i][j-1]-k);
					prior[k] += weight*(double)abs(xttemp.at<unsigned char>(i, j - 1) - k);
				}
									
				//diff[k]+=diff_sq[k][i][j-1]; //neighbors difference
				diff[k] += diff_sq[k].at<double>(i, j-1);
			}

			//**************************************************************************//
			if (j+1 <= cols-1) 
			{
				//if (xttemp[0][i][j+1] != k) 
				if (xttemp.at<unsigned char>(i, j + 1) != k)
				{
					//texture1=cvGetReal2D(texture, i, j+1);	
					texture1 = texture.at<unsigned char>(i, j+1);
					if (texture1==0 ) 
					{
						alpha=0;
						weight=10;
						//if (abs(xttemp[0][i][j + 1] - xttemp[0][i][j]) < 1)
						if (xttemp.at<unsigned char>(i, j + 1) - xttemp.at<unsigned char>(i, j) == 0)
						{
							//cvSetReal2D(texture, i, j, 0);
							texture.at<unsigned char>(i, j) = 0;
						}
					}
					else 
					{
						//edgevalue1=cvGetReal2D(highpass, i, j+1); 
						edgevalue1 = highpass.at<unsigned char>(i, j + 1);
						if (edgevalue1 == 0)
						{
							weight = 0.08;
						}
						//if (edgevalue1 != 0) weight = 1.0;
						else
						{
							weight = 1.0;
						}
					}
					//prior[k]+=weight*(double)abs(xttemp[0][i][j+1]-k);
					prior[k] += weight*(double)abs(xttemp.at<unsigned char>(i, j + 1) - k);
				}
									
				//diff[k]+=diff_sq[k][i][j+1]; //neighbors difference
				diff[k] += diff_sq[k].at<double>(i, j+1);
			}		
		}
	}	// end of if (texturevalue == 255  ) 


//////////// combine by Bayes rule log p(X|Y) = log(p(Y|X)  + p(X)  + Gamma , using attenuation Function call //////////////////////////////////////////////////////////////////////////////
	for (kdx = 0; kdx <= classes; kdx++)
	{
		//if (texturevalue == 0 & edgevalue == 0 ) 
		//	beta = 0.1;   // more texture region and pixel not on edge

		//if (texturevalue == 0 & edgevalue != 0 ) 
		//	beta = 0.01;	// more texture region and pixel  on edge

		//if (texturevalue == 255 & edgevalue == 0 ) 
		//	beta = 1.5; // less texture region and pixel not on edge

		//if (texturevalue == 255 & edgevalue != 0 ) 
		//	beta = 0.01; // less texture region and pixel  on edge

		logpost[kdx] = 3 * (diff[kdx]);
	}

	//return *logpost;
}


///////////////////////////////////////////////////////////////////////////////////
//																				 //
//    Part 3:																	 //
//			Revised MAP Estimation Function                                      //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////


//original function def
//void map3(double **y[], double **xt[], double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta,double gama, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum,double *Num,unsigned char **xttemp[],unsigned char **xttempCr[],unsigned char **xttempCb[],IplImage*texture,IplImage*textureCb,IplImage*textureCr,IplImage*highpass,IplImage*highpassCr,IplImage*highpassCb)////////////////////////////////////////////////////////////////////////////////////////////////////////////

// new function def
//void map3(double **y[], Mat &Depth_Map, double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta, double gama, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, unsigned char **xttemp[], unsigned char **xttempCr[], unsigned char **xttempCb[], IplImage* texture_I, IplImage* textureCb_I, IplImage* textureCr_I, IplImage* highpass_I, IplImage* highpassCr_I, IplImage* highpassCb_I)
//void map3(double **y_pp[], Mat &Depth_Map, double **diff_y_pp[], double **diff_Cr_pp[], double **diff_Cb_pp[], double **atlas[], double beta, double gama, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, IplImage* texture_I, IplImage* textureCb_I, IplImage* textureCr_I, IplImage* highpass_I, IplImage* highpassCr_I, IplImage* highpassCb_I)
//void map3(Mat yY, Mat &Depth_Map, double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta, double gamma, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb)
//void map3(Mat yY, Mat &Depth_Map, vector<Mat> &diff_Y, double **diff_Cr_pp[], double **diff_Cb_pp[], double **atlas[], double beta, double gamma, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb)
//void map3(string &DataLog, Mat yY, Mat &Depth_Map, vector<Mat> &diff_Y, vector<Mat> &diff_Cr, vector<Mat> &diff_Cb, double **atlas[], double beta, double gamma, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb)
void map3(string &DataLog, Mat yY, Mat &Depth_Map, vector<Mat> &diff_Y, vector<Mat> &diff_Cr, vector<Mat> &diff_Cb, double beta, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb)
{
	int idx, jdx, kdx;
	double tick, tock, delta_T;
	double tick_Freq = ((double)cvGetTickFrequency()*1000.0);

	double logpostCr[MAX_CLASSES + 1];
	double logpostCb[MAX_CLASSES + 1];
	double logposty[MAX_CLASSES + 1];

	cv::Point pixel;
	cv::Size logSize = cv::Size(cols, rows);
	vector<Mat> logpost1(MAX_CLASSES);

	tick = (double)cvGetTickCount();

	for (idx = 0; idx < MAX_CLASSES; idx++)
	{
		logpost1[idx] = cv::Mat(logSize, CV_64F, cv::Scalar(0.0));
	}

	//xrv= (double **)get_img(cols,rows,sizeof(double));
	diff[0]=0;
	prior[0]=0;

////////////////////////////////////////////////////////////////////////////////

	//double sqrt2pi,con[MAX_CLASSES+1],d[MAX_CLASSES+1];
	//double mean[MAX_CLASSES+1], var[MAX_CLASSES+1];

	/*  constant  */
	//sqrt2pi = sqrt(2.0*PI);

	/*  constants for each class due to variance  */
	//for (idx = 0; idx < classes; idx++)
	//{
	//	if (v[idx]<0.025) v[idx] = 0.025;
	//	else con[idx] = log(sqrt2pi*sqrt(v[idx]));

	//	if (con[idx] < 0) con[idx] = 0;
	//	else d[idx] = 2.0*v[idx];

	//}

	/* initialize accumulation registers */
	//std::fill_n(&mean[0], classes + 1, 0.0);
	//std::fill_n(&var[0], classes + 1, 0.0);
	std::fill_n(&yaccum[0], classes + 1, 0.0);
	std::fill_n(&ysquaredaccum[0], classes + 1, 0.0);
	std::fill_n(&Num[0], classes + 1, 0.0);
	std::fill_n(&logpostCr[0], classes + 1, 0.0);
	std::fill_n(&logpostCb[0], classes + 1, 0.0);
	std::fill_n(&logposty[0], classes + 1, 0.0);

///////////////////////////////////////////////////////////////////////////////////////

    /* Map loop */
	for (l=0; l<map_iter; l++)
	{
		//AveCost = 0; 

		/* Initialize random variable array */
		//for (idx = 0; idx < rows; idx++)
		//{
		//	for (jdx = 0; jdx < cols; jdx++)
		//	{
		//		xrv[idx][jdx] = random2();
		//	}
		//}

		//**************************************************************************//
		// Begin calculation pixel by pixel
		for (idx = 0; idx < rows; idx++)
		{
			for (jdx = 0; jdx < cols; jdx++)
			{

				// get the pixel location
				pixel.x = idx;
				pixel.y = jdx;

/////////////////////////////////   read in texture and edge information for each pixel ////////////////////////////

				texturevalue = texture.at<unsigned char>(idx, jdx);
				texturevalueCr = textureCr.at<unsigned char>(idx, jdx);
				texturevalueCb = textureCb.at<unsigned char>(idx, jdx);

				if (texturevalue == 255)
				{
					beta = 0.1; //    texture
				}
				else if (texturevalue == 0)
				{
					beta = 0.01;
				}

				edgevalue = highpass.at<unsigned char>(idx, jdx);
				//*logposty = callogpost(pixel, diff_y, atlas, xttemp, cols, rows, ATLAS, gama, classes, beta, d, con, logposty, edgevalue, texturevalue, highpass_I, texture_I);
				callogpost(pixel, diff_Y, xttempY, cols, rows, classes, beta, logposty, edgevalue, texturevalue, highpass, texture);
				//std::thread lp_Y(callogpost, pixel, diff_Y, atlas, xttempY, cols, rows, ATLAS, gamma, classes, beta, d, con, logposty, edgevalue, texturevalue, highpass, texture);

				edgevalueCr = highpassCr.at<unsigned char>(idx, jdx);
				//*logpostCr = callogpost(pixel, diff_Cr, atlas, xttempCr, cols, rows, ATLAS, gama, classes, beta, d, con, logpostCr, edgevalueCr, texturevalueCr, highpassCr_I, textureCr_I);
				callogpost(pixel, diff_Cr, xttempCr, cols, rows, classes, beta, logpostCr, edgevalueCr, texturevalueCr, highpassCr, textureCr);
				//std::thread lp_Cr(callogpost, pixel, diff_Cr, atlas, xttempCr, cols, rows, ATLAS, gamma, classes, beta, d, con, logpostCr, edgevalueCr, texturevalueCr, highpassCr, textureCr);

				edgevalueCb = highpassCb.at<unsigned char>(idx, jdx);
				//*logpostCb = callogpost(pixel, diff_Cb, atlas, xttempCb, cols, rows, ATLAS, gama, classes, beta, d, con, logpostCb, edgevalueCb, texturevalueCb, highpassCb_I, textureCb_I);
				callogpost(pixel, diff_Cb, xttempCb, cols, rows, classes, beta, logpostCb, edgevalueCb, texturevalueCb, highpassCb, textureCb);
				//std::thread lp_Cb(callogpost, pixel, diff_Cb, atlas, xttempCb, cols, rows, ATLAS, gamma, classes, beta, d, con, logpostCb, edgevalueCb, texturevalueCb, highpassCb, textureCb);

				//lp_Y.join();
				//lp_Cr.join();
				//lp_Cb.join();

				for (kdx = 0; kdx < MAX_CLASSES; kdx++)
				{
					logpost1[kdx].at<double>(idx,jdx) = (logposty[kdx] + logpostCr[kdx] + logpostCb[kdx]);
				}

			}	// end of jdx loop

		}	// end of idx loop

		//int num_pixels = cols * rows;
              
		cv::Mat gridResult = cv::Mat(Size(cols, rows), CV_8U, Scalar::all(0));
		cv::Mat gridResult_N = cv::Mat(cv::Size(cols, rows), CV_8U);

		cout << "Starting GridGraph routines..." << endl;
		double start= (double)cvGetTickCount();

		GridGraph_DArraySArray(cols, rows, MAX_CLASSES, logpost1, gridResult, DataLog);
		//GeneralGraph_DArraySArray(cols,rows,MAX_CLASSES,logpost1,result);
		
		double end = (double)cvGetTickCount();  
		double  t1 = (end - start) / tick_Freq;
		cout << "Run time without OpenMP = " << t1 << "ms." << endl;
		
		//**************************************************************************//
		//	std::cout<<"Total cost for iteration #"<<l<<" is : "<<AveCost<<"\n";
		for (int dd = 0; dd < MAX_CLASSES; dd++)
		{
			logpost1[dd].~Mat();
		}

		Mat xt_Mat = Mat(Size(cols, rows), CV_8U, Scalar::all(0));

		bitwise_not(gridResult, Depth_Map);

		for (idx = 0; idx < rows; idx++)
		{
			for (jdx = 0; jdx < cols; jdx++)
			{
				int index = gridResult.at<unsigned char>(idx, jdx);
				double temp_y = yY.at<double>(idx, jdx);
				yaccum[index] += temp_y;								// for mean EM calculation
				ysquaredaccum[index] += temp_y*temp_y;					// for variance EM caculation 
				Num[index] += 1;										// for mean & variance EM calculation
			}
		}

		tock = (double)cvGetTickCount();  
		delta_T = (tock - tick) / tick_Freq; 
		cout << "Elapsed time for MAP Estimation: " << delta_T << endl;
	}

}	// end of map3



////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
//void GridGraph_DArraySArray(int width, int height, int num_labels, double **logpost1[], int *result)
void GridGraph_DArraySArray(int width, int height, int num_labels, vector<cv::Mat> &logpost1, cv::Mat &gridResult, string &DataLog)
{
	int idx, jdx, kdx;
	int num_pixels = width * height;
	int *result = new int[num_pixels];   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];

	/*
	for ( int i = 0; i < num_pixels; i++ )
	for (int l = 0; l < num_labels; l++ )
	data[i] = 0;

	if (i < 25 ){
	if(  l == 1 ) data[i*num_labels+l] = 0;
	else data[i*num_labels+l] = 10;
	}
	else {
	if(  l == 4 ) data[i*num_labels+l] = 0;
	else data[i*num_labels+l] = 10;
	}
	*/
	int t = 0;
	for (int idx = 0; idx < height; idx++)
	{
		for (int jdx = 0; jdx < width; jdx++)
		{
			for (int kdx = 0; kdx < num_labels; kdx++)
			{
				//data[t] = logpost1[kdx][idx][jdx]; 
				data[t] = logpost1[kdx].at<double>(idx,jdx);
				t++;
			}
		}
	}

	// next set up the array for smooth costs
	int *smooth = new int[num_labels*num_labels];
	for (int l1 = 0; l1 < num_labels; l1++)
	{
		for (int l2 = 0; l2 < num_labels; l2++)
		{	//smooth[l1+l2*num_labels] = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;
			smooth[l1 + l2*num_labels] = abs(l1 - l2);
		}
	}

	try
	{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width, height, num_labels);

		gc->setDataCost(data);

		gc->setSmoothCost(smooth);
		
		string energy = to_string(gc->compute_energy());
		cout << endl << "Before optimization energy is " << energy << endl;
		DataLog = "\nBefore optimization energy is " + energy + "\n";

		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		
		energy = to_string(gc->compute_energy());
		cout << "After optimization energy is " << energy << endl;
		DataLog += "After optimization energy is " + energy + "\n\n";

		for (idx = 0; idx < num_pixels; idx++)
		{
			result[idx] = gc->whatLabel(idx);
		}

		gridResult = cv::Mat(cv::Size(width, height), CV_32S, result, (width * 4));
		gridResult.convertTo(gridResult, CV_8U, 1.0, 0.0);

		delete gc;
	}
	catch (GCException e)
	{
		e.Report();
	}

	delete[] smooth;
	delete[] data;
	delete[] result;

}	// end of GridGraph_DArraySArray