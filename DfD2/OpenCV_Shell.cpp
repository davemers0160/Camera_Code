// OpenCV Shell for MAP Algorithm.

//#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
//#include <omp.h>
#include "allocate.h"
//#include "random.h"
//#include "cv.h"	// removed
//#include "highgui.h" // removed

// added
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>

#include "time.h"
#include <iostream>

#define MAX_CLASSES 256.0
#define MaxSigma 2.5
#define EM_iteration 1
#define map_iteraton 1
#define weightingfactor_beta 0.01 
#define Optimmization_method 2 // :0-SA, 1-ICM, 2-MPM

using namespace std;
using namespace cv;


/* Declearation of the MAP estimator 
   y[]     - Defoucused scene.
   xt[]    - Focused scene & 63 blur levels created artificially.
   atlas[] - The assisting atlas.
   beta    - Weighting factor for beta term.
   gamma   - Weighting factor for atlas term.
   ATLAS   - Switch for atlas method: 0-off, 1-on.
   ICM     - Optimization methods:0-SA, 1-ICM, 2-MPM.
   rol, col- Image size.
   classes - Predefined as 64.
   map_iter- The loop will be forced to stop when reach this number.
*/



////////// MAP estimation ////////////////////////////////////////////////////////////////
void map3(double **yY[], double **xtY[],double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta,double gamma, int ATLAS, int ICM, int cols, int rows, int classes,int map_iter,double *, double *,double *, double *,unsigned char **xttemp[],unsigned char **xttempCr[],unsigned char **xttempCb[],IplImage*texture,IplImage*textureCb,IplImage*textureCr,IplImage*highpass,IplImage*highpassCr,IplImage*highpassCb);

////////// Automatically add blur to original image based on disparity map///////////////
void createblur(int col, int row, IplImage* ImageInFocus, int classes,double **y[],double **xt[], double **atlas[], int ATLAS, IplImage*ImageOutOfFocus, IplImage * ImageInFocustemp, IplImage* GauBlur[],unsigned char **xttemp[], IplImage *groundtruth);

///////// Space Varying 2D filter ///////////////////////////////////////////////////////
void SpaceVaryingfilter2D (IplImage * image, int kernel_size,double min_sigma, double max_sigma, IplImage * filter);


int main( int argc, char** argv ) 
{

	string image_locations = "Image_Files";
	string file_path;

	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//      Step 1:																	 //
	//				Use all in focus and ground truth to generate defocus image      //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

	cout << "Starting Step 1..." << endl;

 /////// read in focus color image ///////////////////////////////////////////////////////
	file_path = image_locations + "\\view5_color.tif";
	IplImage*   ImageInFocus3 = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_COLOR);

 /////// split the in focus color image into RGB channel (integers) ///////////////////////
	IplImage* ImageInFocusR=cvCreateImage(cvGetSize(ImageInFocus3), 8, 1);
	IplImage* ImageInFocusG=cvCreateImage(cvGetSize(ImageInFocus3), 8, 1);
	IplImage* ImageInFocusB=cvCreateImage(cvGetSize(ImageInFocus3), 8, 1);
	cvSplit(ImageInFocus3, ImageInFocusR, ImageInFocusG, ImageInFocusB, 0);


 /////// convert RGB channels to floating point //////////////////////////////////////////
	IplImage* ImageInFocusR1 = cvCreateImage(cvGetSize(ImageInFocus3),IPL_DEPTH_64F,0);  
	cvConvertScale(ImageInFocusR,ImageInFocusR1,1.0,0.0);  
	IplImage* ImageInFocusG1 = cvCreateImage(cvGetSize(ImageInFocus3),IPL_DEPTH_64F,0);  
	cvConvertScale(ImageInFocusG,ImageInFocusG1,1.0,0.0);  
	IplImage* ImageInFocusB1 = cvCreateImage(cvGetSize(ImageInFocus3),IPL_DEPTH_64F,0);  
	cvConvertScale(ImageInFocusB,ImageInFocusB1,1.0,0.0);  


 ///////  read in ground truth image ///////////////////////////////////////////////////// 
	
	file_path = image_locations + "\\disp1_1.png";

	IplImage*   groundtruth = cvLoadImage(file_path.c_str(), 0);
	//IplImage*   groundtruth = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/disp1_1.png", 0);

 //////   Create defocus image for R,G,B channles ////////////////////////////////////////
	IplImage *  Blurmap = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 3);
	IplImage *  BlurmapR = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 1);
	IplImage *  BlurmapG = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 1);
	IplImage *  BlurmapB = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 1);

  /////  Define Gaussian blur parameter /////////////////////////////////////////////////
	double sigma;

  ///// Create 256 Gaussian blur matrix for 3 channels/////////////////////////////////// 
	IplImage*  GauBlurR[256];
	IplImage*  GauBlurG[256];
	IplImage*  GauBlurB[256];
	for( int l = 0 ;l < 256 ; l = ++l )
	{
		GauBlurR[l] = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 0);
		GauBlurG[l] = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 0);
		GauBlurB[l] = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 0);
	}
 
 ////// For Ground truth map, the min is 0 and max is 255 ///////////////////////////////
 ////// The total number of blur is 256					  ///////////////////////////////
	double MinValue=0;
	double MaxValue=255;
	double blurnumber=256;
	double index;
	int	 index1;
	int kernel_size;

 ////// Create 256 levels of Gaussian parameters sigma  /////////////////////////////////
 ////// The range is from 0 to MaxSigma                 /////////////////////////////////
 ////// Based on 256 different sigma, generate 256 blur image (whole image has the same sigma)//
	for(int k=MinValue;k<MaxValue+1;k++)
	{
		index=k;
		index1=(int)(index-MinValue)/((MaxValue-MinValue)/(blurnumber-1)); 	
		sigma=MaxSigma*(blurnumber-index1)/blurnumber;
		if(sigma<=0) sigma=0.000000001;
		cvSmooth( ImageInFocusR1 , GauBlurR[index1] , CV_GAUSSIAN, 0, 0,sigma,sigma);
		cvSmooth( ImageInFocusG1 , GauBlurG[index1] , CV_GAUSSIAN, 0, 0,sigma,sigma);
		cvSmooth( ImageInFocusB1 , GauBlurB[index1] , CV_GAUSSIAN, 0, 0,sigma,sigma);
      
		//double start= (double)cvGetTickCount();
		//kernel_size = cvRound(sigma*8 + 1)|1;
		//SpaceVaryingfilter2D (ImageInFocusR1, kernel_size, sigma, 50*MaxSigma/blurnumber, GauBlurR[index1]);
		//SpaceVaryingfilter2D (ImageInFocusG1, kernel_size, sigma, 50*MaxSigma/blurnumber, GauBlurG[index1]);
		//SpaceVaryingfilter2D (ImageInFocusB1, kernel_size, sigma, 50*MaxSigma/blurnumber, GauBlurB[index1]);
      
		//double end= (double)cvGetTickCount();
		//double  t1= (end-start)/((double)cvGetTickFrequency()*1000.);
		//printf( "Run time without OpenMP = %g ms/n", t1 );
	}


 ////// Based on the value of each pixel in ground truth map, /////////////////////////////
 ////// Find out which blur index each pixel belongs to        /////////////////////////////
	int col = ImageInFocus3->width;
	int row = ImageInFocus3->height;
	double tt;

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			index = cvGetReal2D(groundtruth, i, j);
			index = (int)index;
			index1 = (int)(index - MinValue) / ((MaxValue - MinValue) / (blurnumber - 1));
			tt = cvGetReal2D(GauBlurR[index1], i, j);
			cvSetReal2D(BlurmapR, i, j, tt);
			tt = cvGetReal2D(GauBlurG[index1], i, j);
			cvSetReal2D(BlurmapG, i, j, tt);
			tt = cvGetReal2D(GauBlurB[index1], i, j);
			cvSetReal2D(BlurmapB, i, j, tt);
		}
	}
 /////// Combine the three channels and save the defocus image /////////////////////////////
	cvMerge(BlurmapR, BlurmapG, BlurmapB, NULL, Blurmap );

	//cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/blurmap_color.tif",Blurmap);
	
	file_path = image_locations + "\\blurmap_color.tif";
	cvSaveImage(file_path.c_str(), Blurmap);

	cout << "Completed Step 1." << endl;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
	
	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 2:																	 //
	//			Read all in focus image and true defocus image                       //
    //          Convert the color images to YCrCb channel                            //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

	cout << "Starting Step 2" << endl;

///////  Create image matrix for color image  ////////////////////////////////////////////////
  IplImage * inf=0;
  IplImage * outf=0;
  IplImage * yCbCrin=0;
  IplImage * yCbCrout=0;
  IplImage * yin=0;
  IplImage * yout=0;
  IplImage * Cbin=0;
  IplImage * Cbout=0;
  IplImage * Crin=0;
  IplImage * Crout=0;

//////// Read all in focus image and true defocus image /////////////////////////////////////////
  file_path = image_locations + "\\view5_color.tif";
  inf = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_COLOR);
  file_path = image_locations + "\\blurmap_color.tif";
  outf = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_COLOR);

  //inf=cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/view5_color.tif", CV_LOAD_IMAGE_COLOR);
  //outf=cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/blurmap_color.tif", CV_LOAD_IMAGE_COLOR);

////// Convert the two images from RGB to YCrCb (Here "in" means in focus and "out" means defocus) ///
  yCbCrin=cvCreateImage(cvGetSize(inf), 8, 3);
  yCbCrout=cvCreateImage(cvGetSize(outf), 8, 3);
  cvCvtColor(inf, yCbCrin, CV_RGB2YCrCb);
  cvCvtColor(outf, yCbCrout, CV_RGB2YCrCb);
  yin=cvCreateImage(cvGetSize(inf), 8, 1);
  yout=cvCreateImage(cvGetSize(outf), 8, 1);
  Cbin=cvCreateImage(cvGetSize(inf), 8, 1);
  Cbout=cvCreateImage(cvGetSize(outf), 8, 1);
  Crin=cvCreateImage(cvGetSize(inf), 8, 1);
  Crout=cvCreateImage(cvGetSize(outf), 8, 1);

//////// Split  images into 3 channels   /////////////////////////////////////////////////////////
  cvSplit(yCbCrin, yin, Crin, Cbin, 0);
  cvSplit(yCbCrout, yout, Crout, Cbout, 0);
  cvSaveImage((image_locations + "\\yin.png").c_str(),yin);
  cvSaveImage((image_locations + "\\yout.png").c_str(), yout);
  cvSaveImage((image_locations + "\\Crin.png").c_str(), Crin);
  cvSaveImage((image_locations + "\\Crout.png").c_str(), Crout);
  cvSaveImage((image_locations + "\\Cbin.png").c_str(), Cbin);
  cvSaveImage((image_locations + "\\Cbout.png").c_str(), Cbout);

  //cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/yin.png", yin);
  //cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/yout.png", yout);
  //cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/Crin.png", Crin);
  //cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/Crout.png", Crout);
  //cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/Cbin.png", Cbin);
  //cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/Cbout.png", Cbout);

  cout << "Completed Step 2." << endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 3:																	 //
	//			Convert all pixels in Y, Cr, Cb  from integer to floating point      //
    //                                                                               //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

  cout << "Starting Step 3..." << endl;

  IplImage* ImageInFocusY = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  IplImage* ImageOutOfFocusY = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  IplImage* SyntheticDefocusY = cvCreateImage(cvGetSize(inf), IPL_DEPTH_64F, 0);
  cvConvertScale(yin,ImageInFocusY,1.0,0.0); 
  cvConvertScale(yout,ImageOutOfFocusY,1.0,0.0);

  IplImage* ImageInFocusCr = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  IplImage* ImageOutOfFocusCr = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  IplImage* SyntheticDefocusCr = cvCreateImage(cvGetSize(inf), IPL_DEPTH_64F, 0);
  cvConvertScale(Crin,ImageInFocusCr,1.0,0.0); 
  cvConvertScale(Crout,ImageOutOfFocusCr,1.0,0.0);

  IplImage* ImageInFocusCb = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  IplImage* ImageOutOfFocusCb = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  IplImage* SyntheticDefocusCb = cvCreateImage(cvGetSize(inf), IPL_DEPTH_64F, 0);
  cvConvertScale(Cbin,ImageInFocusCb,1.0,0.0); 
  cvConvertScale(Cbout,ImageOutOfFocusCb,1.0,0.0);

  cout << "Completed Step 3." << endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 4:																	 //
	//			use highpass filter to generate edge map                             //
    //          for each channel                                                     //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////
 
 //////// use highpass filter to generate edge map /////////////////////////////////////
  cv:: Mat kernel(3,3,CV_32F,cv::Scalar(-0.8));    
  kernel.at<float>(1,1) = 6.4;

  cv::Mat scr = cv::imread("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/view5.tif",0);   // read in focus image(grayscale)
  cv::Mat rst; 

  filter2D(scr,rst,scr.depth(),kernel);     
  cv::imwrite("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/highpass.png",rst);

  scr = cv::imread("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/Crin.png",0);
  filter2D(scr,rst,scr.depth(),kernel);  
  cv::imwrite("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/highpassCr.png",rst);

  scr = cv::imread("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/Cbin.png",0);
  filter2D(scr,rst,scr.depth(),kernel);  
  cv::imwrite("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/highpassCb.png",rst);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    


	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 5:																	 //
	//			use texture identifer to generate texture map                        //
    //          for each channel                                                     //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



  	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 6:																	 //
	//																				 //
	//			Initialization of MAP estimation                                     //
    //                                                                               //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

 ///// Initialize parameters for MAP subrutine  /////////////////////////////////////////////
  int    ICM         = Optimmization_method;
  int	 mapiter     = map_iteraton;
  int    EMiteration = EM_iteration;
  double beta        = weightingfactor_beta;		
  double gama        = 1.5;
  int    classes     = MAX_CLASSES;
  int    ATLAS       = 0;	

 ///// Read in initial depth map resutl ////////////////////////////////////////////////////
  IplImage* preresult = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  preresult = cvLoadImage( "/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/preresult.png" , 0 );

 ///// Bring in edge information and texture information
  IplImage* texture     = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/texturelessregion.png",0);
  IplImage* textureCb   = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/texturelessregionCb.png",0);
  IplImage* textureCr   = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/texturelessregionCr.png",0);
  IplImage* texture_tmp = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/texturelessregion.png",0);
  IplImage* highpass    = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/highpass.png",0);
  IplImage* highpassCr  = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/highpassCr.png",0);
  IplImage* highpassCb  = cvLoadImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/highpassCb.png",0);
	 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 7:																	 //
	//			use all in focus image to generate 256 synthetic defocus images      //
    //          And use pre depth result to initial xttemp                           //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////
  
  IplImage* GauBlur[260];
 ////// Store blur index for each pixel each channel /////////////////////////////////////////	
  unsigned char **xttempy[260];
  unsigned char **xttempCr[260];
  unsigned char **xttempCb[260];

 ////// y : true defocus image ///////////////////////////////////////////////////////////////
 ////// xt : 256 synthetic defocus images ////////////////////////////////////////////////////
  double **yY[260], **xtY[260], **atlas[260];
  double **yCr[260], **xtCr[260];
  double **yCb[260], **xtCb[260];
  double **diff_y[260], **diff_Cr[260], **diff_Cb[260];

  createblur(col, row, ImageInFocusY, classes, yY, xtY, atlas, ATLAS, ImageOutOfFocusY, SyntheticDefocusY, GauBlur, xttempy, preresult);
  createblur(col, row, ImageInFocusCr, classes, yCr, xtCr, atlas, ATLAS, ImageOutOfFocusCr, SyntheticDefocusCr, GauBlur, xttempCr,preresult);
  createblur(col, row, ImageInFocusCb, classes, yCb, xtCb, atlas, ATLAS, ImageOutOfFocusCb, SyntheticDefocusCb, GauBlur, xttempCb,preresult);

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 8:																	 //
	//			EM calculation                                                       //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

//// calculate Data term (y-b)^2 ////////////////////////////////////////////////////////

	for ( int dd = 1; dd <= MAX_CLASSES; dd++)
	{
		diff_y[dd] = (double **)get_img(col,row,sizeof(double));
		//diff_Cr[dd] = (double **)get_img(col,row,sizeof(double));
		//diff_Cb[dd] = (double **)get_img(col,row,sizeof(double));
	}

////// In order to save memory, here replace  diff_Cr, diff_cb with xtCr, xtCb/////////
	for (int k=1; k<=MAX_CLASSES; k++)
		for(int i=0;i<row;i++)
			for(int j=0;j<col;j++)
				{
					diff_y[k][i][j]=(double)(yY[1][i][j]-xtY[k][i][j])*(double)(yY[1][i][j]-xtY[k][i][j]);
					xtCr[k][i][j]=(double)(yCr[1][i][j]-xtCr[k][i][j])*(double)(yCr[1][i][j]-xtCr[k][i][j]);
					xtCb[k][i][j]=(double)(yCb[1][i][j]-xtCb[k][i][j])*(double)(yCb[1][i][j]-xtCb[k][i][j]);
				}

 

 //// Parameters for EM calculation 256 classes ////////////////////////////////////////
  double yaccum[257], ysquaredaccum[257], Num[257]; 
  double m_Y[257], v_Y[257], N_Y[257];	
  
 ////// Choose initial conditions for mean and variance ////////////////////////////////
  for (int l=1; l<=classes; l++)
	{
		if (l==1) 
			{
				m_Y[l] =0;
			}
		else 
			{
				m_Y[l]  = m_Y[l-1]  + 255/(classes+1); 
			}
		N_Y[l]  = 0;
		v_Y[l]  = 0.5;
     }

 //// EM brgin /////////////////////////////////////////////////////////////////////////
  
 double start= (double)cvGetTickCount();

  for (int i=0; i<EMiteration; i++)
	{
		cout<<"EM iteration "<<i<<endl;

    /* Call MAP subtutine */	
		//map3(yY, xtY, yCr, xtCr, yCb, xtCb, atlas, beta, gama, ATLAS, ICM, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num,xttempy, xttempCr, xttempCb,texture,textureCb,textureCr,highpass,highpassCr,highpassCb);	  
		map3(yY, xtY, diff_y, xtCr, xtCb, atlas, beta, gama, ATLAS, ICM, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num,xttempy, xttempCr, xttempCb,texture,textureCb,textureCr,highpass,highpassCr,highpassCb);	  

		for (int l=1; l<=classes; l++)  
		{
			N_Y[l]=Num[l];
			if (N_Y[l]!=0) 
			{												// no variable mean
			m_Y[l]= (yaccum[l])/(N_Y[l]);
			v_Y[l]= (ysquaredaccum[l]-(2.0*(yaccum[l])*(m_Y[l]))+((m_Y[l])*(m_Y[l])*(N_Y[l])))/(N_Y[l]);
			//cout<<v_Y[l]<<endl;
			v_Y[l]=v_Y[l]/3000;
			}
		
		}
	}

 double end= (double)cvGetTickCount();
 double  t1= (end-start)/((double)cvGetTickFrequency()*1000.);
 //printf( "Run time without OpenMP = %g ms/n", t1 );

 ///// Generate depth map and save it ////////////////////////////////////////////////
    IplImage* BlurMap = cvCreateImage(cvSize(col,row),8,1);

	for(int i=0;i<row;i++)
		for(int j=0;j<col;j++)
		{
			cvSetReal2D(BlurMap,i,j, (double)xtY[0][i][j]);
		}
	
	cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/blurmaplc64.png",BlurMap);

 
    return 0; 

}


