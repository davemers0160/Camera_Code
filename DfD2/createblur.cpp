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


///////// Space Varying 2D filter ///////////////////////////////////////////////////////
void SpaceVaryingfilter2D (IplImage * image, int kernel_size,double min_sigma, double max_sigma, IplImage * filter);


void createblur(int col, int row, IplImage* ImageInFocus, int classes,double **y[],double **xt[], double **atlas[], int ATLAS, IplImage*ImageOutOfFocus, IplImage * SyntheticDefocus, IplImage* GauBlur[],unsigned char **xttemp[], IplImage *groundtruth)
{
	int i, j, dd, kk, step,channels;
	double x = rand()%1001;
	double flag, BlurStep, para;
	CvScalar r, s, t, a;
    int kernel_size;

  ///// Initialize image data structure ////////////////////////////////////////////////////////////////////////
	col = ImageInFocus->width , row = ImageInFocus->height;
	for ( dd = 0; dd <= classes+1; dd++)
	{
		xt[dd] = (double **)get_img(col,row,sizeof(double));
	}
	y[1] = (double **)get_img(col,row,sizeof(double));
    atlas[0] = (double **)get_img(col,row,sizeof(double));
	for( i=0;i<row;i++)
    	for( j=0;j<col;j++)
	    {
	    	atlas[0][i][j] = 0;
	    }


 ///// Loading the atlas for Gamma ////////////////////////////////////////////////////////////////////////////
	if(ATLAS==1)
	{
	    IplImage* inputdepth = cvLoadImage("TrueBlurrMap000_remake.png",1);
	    IplImage* graydepth = cvCreateImage(cvGetSize(inputdepth), 8, 1);
	    for(i=0;i<row;i++)
		    for(j=0;j<col;j++)
		{
			a = cvGet2D(graydepth,i,j);
			atlas[0][i][j] = a.val[0];
		}
	}


  /////  Initialize Markov Random Field ///////////////////////////////////////////////////////////////////////
	for (i=0; i<row; i++)
		for (j=0; j<col; j++)
		{
			x = rand()%1001;
			flag = x/500;
			xt[0][i][j] = classes/2;
			for (kk=1; kk <= classes; kk++) 
				if (((double)(kk)/(double)(classes) < flag) && (flag <= (double)(kk+1)/(double)(classes)))
				{
					xt[0][i][j] = kk;
					break;
				}
		}

    for(i=0;i<row;i++)
    	for(j=0;j<col;j++)
	    {
	    	r = cvGet2D(ImageInFocus,i,j);
	    	xt[1][i][j] = r.val[0];
	    }

 /////  Initialize input blur array ////////////////////////////////////////////////////////////////////////////
	for(i=0;i<row;i++)
		for(j=0;j<col;j++)
		{
			s = cvGet2D(ImageOutOfFocus,i,j);
			y[1][i][j] = s.val[0];
		}
			
 /////// Create Multi-level blur image //////////////////////////////////////////////////////////////////////////
	for( int l = 1 ;l < classes+1 ; l = ++l )
	{
	  BlurStep =MaxSigma/(double(MAX_CLASSES));
	  para = BlurStep * l;
	  cvSmooth( ImageInFocus , SyntheticDefocus , CV_GAUSSIAN, 0, 0, para, para);
      //kernel_size = cvRound(para*8 + 1)|1;
      //SpaceVaryingfilter2D (ImageInFocus, kernel_size, para, 0.5*BlurStep, SyntheticDefocus);
      GauBlur[l] = SyntheticDefocus;

	  for(i=0;i<row;i++)
		  for(j=0;j<col;j++)
		  {
		   	  t = cvGet2D(GauBlur[l],i,j);
			  xt[l+1][i][j] = t.val[0];
		  }  
	}
    

///// Use initial depth map as starting point, here "groundtruth" is initial depth map //////////////////////////
	xttemp[0] = (unsigned char **)get_img(col,row,sizeof(double));
	for (i=0; i<row; i++)
        for (j=0; j<col; j++)
			{xttemp[0][i][j] = xt[0][i][j];}
		  //{xttemp[0][i][j]=cvGetReal2D(groundtruth,i,j);xttemp[0][i][j] = unsigned(255.0-((256.0/MAX_CLASSES)*(xttemp[0][i][j]-1.0)));}


}