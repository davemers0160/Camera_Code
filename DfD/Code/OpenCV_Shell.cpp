// OpenCV Shell for MAP Algorithm.

//#include "stdafx.h"
#include <cmath>
#include <cstdlib>
#include <omp.h>
#include "allocate.h"
#include <Windows.h>
//#include "random.h"

#include "time.h"
#include <iostream>


// OPENCV Includes
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  

#define MAX_CLASSES 256.0
#define MaxSigma 5.0
#define EM_iteration 2
#define map_iteraton 20
#define weightingfactor_beta 0.01 
#define Optimmization_method 2 // :0-SA, 1-ICM, 2-MPM

using namespace cv;
using namespace std;

/* Declearation of the MAP estimator 
   y[]     - Defoucused scene.
   xt[]    - Focused scene & blur levels created artificially.
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



int main(int argc, char** argv)
{
	int idx, jdx, kdx;
	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//      Step 1:																	 //
	//				Use all in focus and ground truth to generate defocus image      //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

	/////// read in focus color image ///////////////////////////////////////////////////////
	//IplImage*   ImageInFocus3 = cvLoadImage("view5_color.tif", CV_LOAD_IMAGE_COLOR);
	Mat ImageInFocus3 = imread("view5_color.tif", CV_LOAD_IMAGE_COLOR);
	Size InFocusSize = ImageInFocus3.size();

	/////// split the in focus color image into RGB channel (integers) ///////////////////////
	//IplImage* ImageInFocusR=cvCreateImage(cvGetSize(ImageInFocus3), 8, 1);
	//IplImage* ImageInFocusG=cvCreateImage(cvGetSize(ImageInFocus3), 8, 1);
	//IplImage* ImageInFocusB=cvCreateImage(cvGetSize(ImageInFocus3), 8, 1);
	//cvSplit(ImageInFocus3, ImageInFocusR, ImageInFocusG, ImageInFocusB, 0);

	vector<Mat> ImageInFocus_RGB(3);
	split(ImageInFocus3, ImageInFocus_RGB);

	/////// convert RGB channels to floating point //////////////////////////////////////////
	//IplImage* ImageInFocusR1 = cvCreateImage(cvGetSize(ImageInFocus3),IPL_DEPTH_64F,0);  
	//cvConvertScale(ImageInFocusR,ImageInFocusR1,1.0,0.0);  
	//IplImage* ImageInFocusG1 = cvCreateImage(cvGetSize(ImageInFocus3),IPL_DEPTH_64F,0);  
	//cvConvertScale(ImageInFocusG,ImageInFocusG1,1.0,0.0);  
	//IplImage* ImageInFocusB1 = cvCreateImage(cvGetSize(ImageInFocus3),IPL_DEPTH_64F,0);  
	//cvConvertScale(ImageInFocusB,ImageInFocusB1,1.0,0.0);  

	vector<Mat> ImageInFocus_RGB_64F(3);

	for (idx = 0; idx < 3; idx++)
	{
		//ImageInFocus_RGB_64F[idx] = Mat(InFocusSize, CV_64F);
		//ImageInFocus_RGB[idx].copyTo(ImageInFocus_RGB_64F[idx]);
		//ImageInFocus_RGB_64F[idx].convertTo(ImageInFocus_RGB_64F[idx], CV_64FC1);
		ImageInFocus_RGB[idx].convertTo(ImageInFocus_RGB_64F[idx], CV_64FC1, 1.0);

	}

	///////  read in ground truth image ///////////////////////////////////////////////////// 
	// IplImage*   groundtruth = cvLoadImage("disp1_1.png", 0);
	Mat groundtruth = imread("disp1_1.png", 0);

	//////   Create defocus image for R,G,B channles ////////////////////////////////////////
	//IplImage *  Blurmap = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 3);
	//IplImage *  BlurmapR = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 1);
	//IplImage *  BlurmapG = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 1);
	//IplImage *  BlurmapB = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 1);

	Mat Blurmap = Mat(ImageInFocus3.size(), CV_64FC3);
	vector<Mat> BlurmapRGB(3);
	BlurmapRGB[0] = Mat(ImageInFocus3.size(), CV_64F);
	BlurmapRGB[1] = Mat(ImageInFocus3.size(), CV_64F);
	BlurmapRGB[2] = Mat(ImageInFocus3.size(), CV_64F);

	/////  Define Gaussian blur parameter /////////////////////////////////////////////////
	double sigma = 0.0;

	///// Create 256 Gaussian blur matrix for 3 channels/////////////////////////////////// 
	//IplImage*  GauBlurR[256];
	//IplImage*  GauBlurG[256];
	//IplImage*  GauBlurB[256];

	vector<Mat> GauBlurR(256);
	vector<Mat> GauBlurG(256);
	vector<Mat> GauBlurB(256);

	for (idx = 0; idx < 256; idx++)
	{
		//GauBlurR[l] = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 0);
		//GauBlurG[l] = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 0);
		//GauBlurB[l] = cvCreateImage(cvGetSize(ImageInFocus3), IPL_DEPTH_64F, 0);
		GauBlurR[idx] = Mat(ImageInFocus3.size(), CV_64F);
		GauBlurG[idx] = Mat(ImageInFocus3.size(), CV_64F);
		GauBlurB[idx] = Mat(ImageInFocus3.size(), CV_64F);
	}

	////// For Ground truth map, the min is 0 and max is 255 ///////////////////////////////
	////// The total number of blur is 256					  ///////////////////////////////
	double MinValue = 0;
	double MaxValue = 255;
	double blurnumber = 256;
	double index;
	int	 index1;

	////// Create 256 levels of Gaussian parameters sigma  /////////////////////////////////
	////// The range is from 0 to MaxSigma                 /////////////////////////////////
	////// Based on 256 different sigma, generate 256 blur image (whole image has the same sigma)//
	cout << "Generating Blur Levels";
	for (kdx = MinValue; kdx < MaxValue + 1; kdx++)
	{
		index = kdx;
		index1 = (int)(index - MinValue) / ((MaxValue - MinValue) / (blurnumber - 1));
		sigma = MaxSigma*(blurnumber - index1) / blurnumber;
		//  if(sigma<=0) sigma=0.000000001;   
		//cvSmooth( ImageInFocusR1 , GauBlurR[index1] , CV_GAUSSIAN, 0, 0,sigma,sigma);
		//cvSmooth( ImageInFocusG1 , GauBlurG[index1] , CV_GAUSSIAN, 0, 0,sigma,sigma);
		//cvSmooth( ImageInFocusB1 , GauBlurB[index1] , CV_GAUSSIAN, 0, 0,sigma,sigma);
		GaussianBlur(ImageInFocus_RGB_64F[0], GauBlurR[index1], Size(0, 0), sigma, sigma, BORDER_REPLICATE);
		GaussianBlur(ImageInFocus_RGB_64F[1], GauBlurG[index1], Size(0, 0), sigma, sigma, BORDER_REPLICATE);
		GaussianBlur(ImageInFocus_RGB_64F[2], GauBlurB[index1], Size(0, 0), sigma, sigma, BORDER_REPLICATE);
		cout << ".";
	}

	cout << endl << "Generation Complete!" << endl;

	////// Based on the value of each pixel in ground truth map, /////////////////////////////
	////// Find out which blur index each pixel belongs to        /////////////////////////////
	//int col = ImageInFocus3->width;
	//int row = ImageInFocus3->height;
	int col = ImageInFocus3.cols;
	int row = ImageInFocus3.rows;
	double tt = 0.0;

	for (idx = 0; idx < row; idx++)
	{	
		for (jdx = 0; jdx < col; jdx++)
		{
			//index = cvGetReal2D(groundtruth, idx, jdx);
			//index = (int)index;

			index = groundtruth.at<UINT8>(idx, jdx);

			index1 = (int)(index - MinValue) / ((MaxValue - MinValue) / (blurnumber - 1));

			//tt = cvGetReal2D(GauBlurR[index1], idx, jdx);
			tt = GauBlurR[index1].at<double>(idx, jdx);
		
			//cvSetReal2D(BlurmapR, idx, jdx, tt);
			BlurmapRGB[0].at<double>(idx, jdx) = tt;

			//tt = cvGetReal2D(GauBlurG[index1], idx, jdx);
			tt = GauBlurG[index1].at<double>(idx, jdx);

			//cvSetReal2D(BlurmapG, idx, jdx, tt);
			BlurmapRGB[1].at<double>(idx, jdx) = tt;

			//tt = cvGetReal2D(GauBlurB[index1], idx, jdx);
			tt = GauBlurB[index1].at<double>(idx, jdx);

			//cvSetReal2D(BlurmapB, idx, jdx, tt);
			BlurmapRGB[2].at<double>(idx, jdx) = tt;

		}
	}


/////// Combine the three channels and save the defocus image /////////////////////////////
	//cvMerge(BlurmapR, BlurmapG, BlurmapB, NULL, Blurmap );
	merge(BlurmapRGB, Blurmap);

	//cvSaveImage("blurmap_color.tif",Blurmap);
	imwrite("blurmap_color.tif", Blurmap);
  
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
	
	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 2:																	 //
	//			Read all in focus image and true defocus image                       //
    //          Convert the color images to YCrCb channel                            //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

/*
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
*/

//////// Read all in focus image and true defocus image /////////////////////////////////////////
  //inf=cvLoadImage("view5_color.tif", CV_LOAD_IMAGE_COLOR);
  //outf=cvLoadImage("blurmap_color.tif", CV_LOAD_IMAGE_COLOR);
	Mat Focus = imread("view5_color.tif", CV_LOAD_IMAGE_COLOR);
	Mat Defocus = imread("blurmap_color.tif", CV_LOAD_IMAGE_COLOR);
	Size FocusSize = Focus.size();
	Size DefocusSize = Defocus.size();

////// Convert the two images from RGB to YCrCb (Here "in" means in focus and "out" means defocus) ///
  //yCbCrin=cvCreateImage(cvGetSize(inf), 8, 3);
  //yCbCrout=cvCreateImage(cvGetSize(outf), 8, 3);
  //yin=cvCreateImage(cvGetSize(inf), 8, 1);
  //yout=cvCreateImage(cvGetSize(outf), 8, 1);
  //Cbin=cvCreateImage(cvGetSize(inf), 8, 1);
  //Cbout=cvCreateImage(cvGetSize(outf), 8, 1); 
  //Crin=cvCreateImage(cvGetSize(inf), 8, 1);
  //Crout=cvCreateImage(cvGetSize(outf), 8, 1);  
	//cvtColor(inf, yCbCrFocus, CV_RGB2YCrCb);
	//cvtColor(outf, yCbCrout, CV_RGB2YCrCb);
	Mat yCbCrFocus = Mat(FocusSize, CV_8U, 3);
	Mat yCbCrDefocus = Mat(DefocusSize, CV_8U, 3);

	cvtColor(Focus, yCbCrFocus, CV_RGB2YCrCb);
	cvtColor(Defocus, yCbCrDefocus, CV_RGB2YCrCb);

	vector<Mat> YCBCR_F(3);			// vector of YCRCB channels for the focused image
	vector<Mat> YCBCR_D(3);			// vector of YCRCB channels for the defocused image



//////// Split  images into 3 channels   /////////////////////////////////////////////////////////
  //cvSplit(yCbCrin, yin, Crin, Cbin, 0);
  //cvSplit(yCbCrout, yout, Crout, Cbout, 0);
  //cvSaveImage("yin.png",yin);
  //cvSaveImage("yout.png",yout);
  //cvSaveImage("Crin.png",Crin);
  //cvSaveImage("Crout.png",Crout);
  //cvSaveImage("Cbin.png",Cbin);
  //cvSaveImage("Cbout.png",Cbout);

	split(yCbCrFocus, YCBCR_F);
	split(yCbCrDefocus, YCBCR_D);



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 3:																	 //
	//			Convert all pixels in Y, Cr, Cb  from integer to floating point      //
    //                                                                               //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////
  
  //IplImage* ImageInFocusY = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  //IplImage* ImageOutOfFocusY = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  //IplImage* SyntheticDefocusY = cvCreateImage(cvGetSize(inf), IPL_DEPTH_64F, 0);
  //cvConvertScale(yin,ImageInFocusY,1.0,0.0); 
  //cvConvertScale(yout,ImageOutOfFocusY,1.0,0.0);


  //IplImage* ImageInFocusCr = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  //IplImage* ImageOutOfFocusCr = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  //IplImage* SyntheticDefocusCr = cvCreateImage(cvGetSize(inf), IPL_DEPTH_64F, 0);
  //cvConvertScale(Crin,ImageInFocusCr,1.0,0.0); 
  //cvConvertScale(Crout,ImageOutOfFocusCr,1.0,0.0);

  //IplImage* ImageInFocusCb = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  //IplImage* ImageOutOfFocusCb = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
  //IplImage* SyntheticDefocusCb = cvCreateImage(cvGetSize(inf), IPL_DEPTH_64F, 0);
  //cvConvertScale(Cbin,ImageInFocusCb,1.0,0.0); 
  //cvConvertScale(Cbout,ImageOutOfFocusCb,1.0,0.0);
  
	Mat ImageFocus_Y = Mat(FocusSize, CV_64F, 1);
	Mat ImageFocus_CB = Mat(FocusSize, CV_64F, 1);
	Mat ImageFocus_CR = Mat(FocusSize, CV_64F, 1);
	YCBCR_F[0].convertTo(ImageFocus_Y, CV_64F, 1.0);
	YCBCR_F[1].convertTo(ImageFocus_CB, CV_64F, 1.0);
	YCBCR_F[2].convertTo(ImageFocus_CR, CV_64F, 1.0);

	Mat ImageDefocus_Y = Mat(FocusSize, CV_64F, 1);
	Mat ImageDefocus_CB = Mat(FocusSize, CV_64F, 1);
	Mat ImageDefocus_CR = Mat(FocusSize, CV_64F, 1);
	YCBCR_D[0].convertTo(ImageDefocus_Y, CV_64F, 1.0);
	YCBCR_D[1].convertTo(ImageDefocus_CB, CV_64F, 1.0);
	YCBCR_D[2].convertTo(ImageDefocus_CR, CV_64F, 1.0);


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
  Mat kernel(3, 3, CV_32F, Scalar(-0.8));    
  kernel.at<float>(1,1) = 6.4;

  Mat scr = imread("view5.tif", CV_8U);   // read in focus image(grayscale) 
  Mat rst; 

  filter2D(scr, rst, scr.depth(), kernel);     
  imwrite("highpass.png",rst);

  scr = cv::imread("Crin.png",0);   
  filter2D(scr, rst, scr.depth(), kernel);  
  imwrite("highpassCr.png",rst);

  scr = cv::imread("Cbin.png",0);  
  filter2D(scr, rst, scr.depth(), kernel);  
  imwrite("highpassCb.png", rst);

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

  /*
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
  preresult = cvLoadImage( "preresult.png" , 0 );

 ///// Bring in edge information and texture information
  IplImage* texture     = cvLoadImage("texturelessregion.png",0);
  IplImage* textureCb   = cvLoadImage("texturelessregionCb.png",0);
  IplImage* textureCr   = cvLoadImage("texturelessregionCr.png",0);
  IplImage* texture_tmp = cvLoadImage("texturelessregion.png",0);	
  IplImage* highpass    = cvLoadImage("highpass.png",0);
  IplImage* highpassCr  = cvLoadImage("highpassCr.png",0);
  IplImage* highpassCb  = cvLoadImage("highpassCb.png",0);


  */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 7:																	 //
	//			use all in focus image to generate 256 synthetic defocus images      //
    //          And use pre depth result to initial xttemp                           //
	//																			     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////
 
  Mat SyntheticDefocus_Y = Mat(FocusSize, CV_64F, 1);
  Mat SyntheticDefocus_CR = Mat(FocusSize, CV_64F, 1);
  Mat SyntheticDefocus_CB = Mat(FocusSize, CV_64F, 1);
  /*

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

*/


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
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
		v_Y[l]  = 0.22;	
     }

 //// EM brgin /////////////////////////////////////////////////////////////////////////
  
 double start= (double)cvGetTickCount();

  for (int i=0; i<EMiteration; i++)
	{
		cout<<"EM iteration "<<i<<endl;

    ///// Call MAP subtutine //////	
		//map3(yY, xtY, yCr, xtCr, yCb, xtCb, atlas, beta, gama, ATLAS, ICM, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num,xttempy, xttempCr, xttempCb,texture,textureCb,textureCr,highpass,highpassCr,highpassCb);	  
		map3(yY, xtY, diff_y, xtCr, xtCb, atlas, beta, gama, ATLAS, ICM, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num,xttempy, xttempCr, xttempCb,texture,textureCb,textureCr,highpass,highpassCr,highpassCb);	  

		for (int l=1; l<=classes; l++)  
		{
			N_Y[l]=Num[l];
			if (N_Y[l]!=0) 
			{												// no variable mean
			m_Y[l]= (yaccum[l])/(N_Y[l]);
			v_Y[l]= (ysquaredaccum[l]-(2.0*(yaccum[l])*(m_Y[l]))+((m_Y[l])*(m_Y[l])*(N_Y[l])))/(N_Y[l]);
			v_Y[l]=v_Y[l]/500;
			}
		
		}
	}

*/

 ///// Generate depth map and save it ////////////////////////////////////////////////
    IplImage* BlurMap = cvCreateImage(cvSize(col,row),8,1);
	/*
	for(int i=0;i<row;i++)
		for(int j=0;j<col;j++)
		{
			cvSetReal2D(BlurMap,i,j, (double)xtY[0][i][j]);
		}
	
	cvSaveImage("blurmaplc64.png",BlurMap);

 */
    return 0; 

}


