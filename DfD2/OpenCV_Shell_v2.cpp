// OpenCV Shell for MAP Algorithm.

//#include "stdafx.h"
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include "allocate.h"
//#include "random.h"

// added
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>

#include "time.h"
#include <iostream>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>

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

//#define GEN_DEFOCUS_IMAGE

////////// read in files  ////////////////////////////////////////////////////////////////
void readFile(Mat &img1, Mat &img2, string f1, string f2, int type, int num);

////////// MAP estimation ////////////////////////////////////////////////////////////////
// 0. void map3(double **yY[], double **xtY[],double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta,double gamma, int ATLAS, int ICM, int cols, int rows, int classes,int map_iter,double *, double *,double *, double *,unsigned char **xttemp[],unsigned char **xttempCr[],unsigned char **xttempCb[],IplImage*texture,IplImage*textureCb,IplImage*textureCr,IplImage*highpass,IplImage*highpassCr,IplImage*highpassCb);
// 1. void map3(double **y[], Mat &Depth_Map, double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta, double gama, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, unsigned char **xttemp[], unsigned char **xttempCr[], unsigned char **xttempCb[], IplImage*texture, IplImage*textureCb, IplImage*textureCr, IplImage*highpass, IplImage*highpassCr, IplImage*highpassCb);
// 2. void map3(double **y[], Mat &Depth_Map, double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta, double gama, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, IplImage*texture, IplImage*textureCb, IplImage*textureCr, IplImage*highpass, IplImage*highpassCr, IplImage*highpassCb);
// 3. void map3(double **y_pp[], Mat &Depth_Map, double **diff_y_pp[], double **diff_Cr_pp[], double **diff_Cb_pp[], double **atlas[], double beta, double gamma, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb);
// 4. void map3(Mat yY, Mat &Depth_Map, double **diff_y_pp[], double **diff_Cr_pp[], double **diff_Cb_pp[], double **atlas[], double beta, double gamma, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb);
// 5. void map3(Mat yY, Mat &Depth_Map, vector<Mat> &diff_Y, double **diff_Cr_pp[], double **diff_Cb_pp[], double **atlas[], double beta, double gamma, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb);
void map3(string &DataLog, Mat yY, Mat &Depth_Map, vector<Mat> &diff_Y, vector<Mat> &diff_Cr, vector<Mat> &diff_Cb, double beta, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum, double *Num, Mat xttempY, Mat xttempCr, Mat xttempCb, Mat texture, Mat textureCb, Mat textureCr, Mat highpass, Mat highpassCr, Mat highpassCb);

////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////// Automatically add blur to original image based on disparity map///////////////
// 1. void createblur(int col, int row, IplImage* ImageInFocus, int classes,double **y[],double **xt[], double **atlas[], int ATLAS, IplImage*ImageOutOfFocus, IplImage * ImageInFocustemp, IplImage* GauBlur[], Mat &xttemp, IplImage *groundtruth);
// 2. void createblur(int col, int row, IplImage* ImageInFocus, int classes, double **y[], double **xt[], double **atlas[], int ATLAS, IplImage*ImageOutOfFocus, IplImage * ImageInFocustemp, IplImage* GauBlur[], Mat &xttemp);
// 3. void createblur(int col, int row, IplImage* ImageInFocus, int classes, double **xt[], double **atlas[], int ATLAS, IplImage*ImageOutOfFocus, IplImage * ImageInFocustemp, Mat &xttemp);
// 4. void createblur(int col, int row, Mat ImageInFocus, int classes, double **xt[], double **atlas[], int ATLAS, Mat ImageOutOfFocus, Mat ImageInFocustemp, Mat &xttemp);
// 5. void createblur(int col, int row, Mat ImageInFocus, int classes, vector<Mat> &xt, double **atlas[], int ATLAS, Mat ImageOutOfFocus, Mat ImageInFocustemp, Mat &xttemp);
void createblur(int col, int row, Mat ImageInFocus, int classes, vector<Mat> &xt, Mat SyntheticDefocus, Mat &xttemp);

///////// Space Varying 2D filter ///////////////////////////////////////////////////////
void SpaceVaryingfilter2D (IplImage * image, int kernel_size,double min_sigma, double max_sigma, IplImage * filter);

/////////////////////////// Textureless Regions /////////////////////////////////
void texturelessRegions(Mat &inputImage, Mat &textureImage, int windowSize, int thresh);

int main( int argc, char** argv ) 
{

	int idx, jdx, kdx;
	int col, row;

	double section_start, section_stop;
	double section_time;

	ofstream DataLogStream;
	string logfileName = "DfD_v2_logfile.txt";

	// base image locations
	string image_locations = "Image_Files\\";
	//string image_locations = "D:\\IUPUI\\Test_Data\\Data1\\";
	//string image_locations = "D:\\IUPUI\\Test_Data\\Test_Pics1\\";
	//string image_locations = "D:\\IUPUI\\Test_Data\\Test_Pics2\\";
	//string file_path;

	DataLogStream.open((image_locations+logfileName), ios::out);


	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);

	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//      Step 1:																	 //
	//				Use all in focus and ground truth to generate defocus image      //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

	cout << "Starting Step 1..." << endl;
	section_start = (double)cvGetTickCount();

#ifdef GEN_DEFOCUS_IMAGE

 /////// read in focus color image ///////////////////////////////////////////////////////
	file_path = image_locations + "view5_color.tif";
	IplImage*   ImageInFocus3 = cvLoadImage(file_path.c_str(), CV_LOAD_IMAGE_COLOR);

	Mat infocus3 = Mat(ImageInFocus3);

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
	
	file_path = image_locations + "ground_truth.png";

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
	col = ImageInFocus3->width;
	row = ImageInFocus3->height;
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
	
	file_path = image_locations + "blurmap_color.tif";
	cvSaveImage(file_path.c_str(), Blurmap);
#else
	cout << "Skipping defocus image generation" << endl;
	DataLogStream << "Skipping defocus image generation" << endl;
#endif
	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start) / ((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 1 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 1 in " << section_time << "ms." << endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 2:																	 //
	//			Read all in focus image and true defocus image                       //
    //          Convert the color images to YCrCb channel                            //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

	cout << "Starting Step 2" << endl;
	section_start = (double)cvGetTickCount();

//////// Read all in focus image and true defocus image /////////////////////////////////////////
	Mat infocusImage, defocusImage;
	string focusfilename, defocusfilename;

	focusfilename = "view5_color.tif";
	defocusfilename = "blurmap_color.tif";

	//focusfilename = "20160805_140903_test_recording_15356234_focus.avi"; 
	//defocusfilename = "20160805_140903_test_recording_15356234_defocus.avi";

	//focusfilename = "Test1_137_2016-09-11-132609-0000.tif";
	//defocusfilename = "Test1_139_2016-09-11-132747-0000.tif";
//	defocusfilename = "Test1_141_2016-09-11-132636-0000.tif";
//	defocusfilename = "Test1_143_2016-09-11-132732-0000.tif";
//	defocusfilename = "Test1_145_2016-09-11-132720-0000.tif";

	// read in still images
	infocusImage = imread((image_locations + focusfilename), CV_LOAD_IMAGE_COLOR);
	defocusImage = imread((image_locations + defocusfilename), CV_LOAD_IMAGE_COLOR);	

	// read in video and advance to a given frame
	//VideoCapture focusVideo, defocusVideo;

	//focusVideo.open(image_locations + focusfilename);
	//defocusVideo.open(image_locations + defocusfilename);
	//double frame_count = focusVideo.get(CV_CAP_PROP_FRAME_COUNT);

	//for (idx = 0; idx < 240; idx++)
	//{
	//	focusVideo.read(infocusImage);
	//	defocusVideo.read(defocusImage);
	//}

	//double current_frame = focusVideo.get(CV_CAP_PROP_POS_FRAMES);

	// read in multiple files to time average
	//focusfilename = "Test1_128_2016-09-11-140945-";
	//defocusfilename = "Test1_130_2016-09-11-140955-";
	//readFile(infocusImage, defocusImage, (image_locations + focusfilename), (image_locations + defocusfilename), 2, 10);


	Size imageSize = infocusImage.size();
	row = infocusImage.rows;
	col = infocusImage.cols;

////// Convert the two images from RGB to YCrCb (Here "in" means in focus and "out" means defocus) ///
	Mat YCbCrin = Mat(imageSize, CV_8UC3);
	Mat YCbCrout = Mat(imageSize, CV_8UC3);
	cvtColor(infocusImage, YCbCrin, CV_BGR2YCrCb, 3);
	cvtColor(defocusImage, YCbCrout, CV_BGR2YCrCb, 3);

	vector<Mat> YCRCB_IN(3);
	vector<Mat> YCRCB_OUT(3);
	for (idx = 0; idx < 3; idx++)
	{
		YCRCB_IN[idx] = Mat(imageSize, CV_8UC1);
		YCRCB_OUT[idx] = Mat(imageSize, CV_8UC1);
	}
  
//////// Split  images into 3 channels   /////////////////////////////////////////////////////////
	split(YCbCrin, YCRCB_IN);
	split(YCbCrout, YCRCB_OUT);

	imwrite((image_locations + "\\yin.png"), YCRCB_IN[0]);
	imwrite((image_locations + "\\Crin.png"), YCRCB_IN[1]);  
	imwrite((image_locations + "\\Cbin.png"), YCRCB_IN[2]);  
	imwrite((image_locations + "\\yout.png"), YCRCB_OUT[0]);
	imwrite((image_locations + "\\Crout.png"), YCRCB_OUT[1]);
	imwrite((image_locations + "\\Cbout.png"), YCRCB_OUT[2]);
	
	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start)/((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 2 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 2 in " << section_time << "ms." << endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 3:																	 //
	//			Convert all pixels in Y, Cr, Cb  from integer to floating point      //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

	cout << "Starting Step 3..." << endl;
	section_start = (double)cvGetTickCount();

	Mat ImageInFocusY = Mat(imageSize, CV_64FC1);
	Mat ImageInFocusCr = Mat(imageSize, CV_64FC1);
	Mat ImageInFocusCb = Mat(imageSize, CV_64FC1);
	YCRCB_IN[0].convertTo(ImageInFocusY, CV_64FC1, 1, 0);
	YCRCB_IN[1].convertTo(ImageInFocusCr, CV_64FC1, 1, 0);
	YCRCB_IN[2].convertTo(ImageInFocusCb, CV_64FC1, 1, 0);

	Mat ImageOutOfFocusY = Mat(imageSize, CV_64FC1);
	Mat ImageOutOfFocusCr = Mat(imageSize, CV_64FC1);
	Mat ImageOutOfFocusCb = Mat(imageSize, CV_64FC1);
	YCRCB_OUT[0].convertTo(ImageOutOfFocusY, CV_64FC1, 1, 0);
	YCRCB_OUT[1].convertTo(ImageOutOfFocusCr, CV_64FC1, 1, 0);
	YCRCB_OUT[2].convertTo(ImageOutOfFocusCb, CV_64FC1, 1, 0);

	Mat SyntheticDefocusY = Mat(imageSize, CV_64FC1);
	Mat SyntheticDefocusCr = Mat(imageSize, CV_64FC1);
	Mat SyntheticDefocusCb = Mat(imageSize, CV_64FC1);

	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start) / ((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 3 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 3 in " << section_time << "ms." << endl;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 4:																	 //
	//			use highpass filter to generate edge map                             //
    //          for each channel                                                     //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////
 
	cout << "Starting Step 4..." << endl;
	section_start = (double)cvGetTickCount();

 //////// use highpass filter to generate edge map /////////////////////////////////////
	Mat scr;
	Mat highpassY;
	Mat highpassCr;
	Mat highpassCb;

	Mat kernel(3,3,CV_32F,cv::Scalar(-0.8));    
	kernel.at<float>(1,1) = 6.4;
	
	// convert the infocus image to a grayscale image
	Mat infocusImage_Gray;
	cvtColor(infocusImage, infocusImage_Gray, CV_BGR2GRAY, 1);

	//scr = cv::imread(image_locations + "\\view5.tif", 0);   // read in focus image(grayscale)
	//filter2D(scr, rst, scr.depth(), kernel);
	filter2D(infocusImage_Gray, highpassY, infocusImage_Gray.depth(), kernel);
	imwrite(image_locations + "\\highpass.png", highpassY, compression_params);

	scr = cv::imread(image_locations+"\\Crin.png", 0);
	filter2D(scr, highpassCr, scr.depth(), kernel);
	imwrite(image_locations + "\\highpassCr.png", highpassCr, compression_params);

	scr = cv::imread(image_locations+"\\Cbin.png", 0);
	filter2D(scr, highpassCb, scr.depth(), kernel);
	imwrite(image_locations + "\\highpassCb.png", highpassCb, compression_params);

	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start) / ((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 4 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 4 in " << section_time << "ms." << endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 5:																	 //
	//			use texture identifer to generate texture map                        //
    //          for each channel                                                     //
	//																			     //
	///////////////////////////////////////////////////////////////////////////////////

	cout << "Starting Step 5..." << endl;
	section_start = (double)cvGetTickCount();

	Mat textureY;
	Mat textureCr;
	Mat textureCb;

	texturelessRegions(highpassY, textureY, 3, 100);
	texturelessRegions(highpassCr, textureCr, 3, 64);
	texturelessRegions(highpassCb, textureCb, 3, 64);

	imwrite((image_locations + "\\texturelessregionY.png"), textureY, compression_params);
	imwrite((image_locations + "\\texturelessregionCr.png"), textureCr, compression_params);
	imwrite((image_locations + "\\texturelessregionCb.png"), textureCb, compression_params);

	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start) / ((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 5 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 5 in " << section_time << "ms." << endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 6:																	 //
	//			Initialization of MAP estimation                                     //
    //                                                                               //
	///////////////////////////////////////////////////////////////////////////////////
	cout << "Starting Step 6..." << endl;
	section_start = (double)cvGetTickCount();

 ///// Initialize parameters for MAP subrutine  /////////////////////////////////////////////
	int    ICM         = Optimmization_method;
	int	   mapiter     = map_iteraton;
	int    EMiteration = EM_iteration;
	double beta        = weightingfactor_beta;		
	double gamma       = 1.5;
	int    classes     = MAX_CLASSES;
	int    ATLAS       = 0;	

 ///// Read in initial depth map resutl ////////////////////////////////////////////////////
	//IplImage* preresult = cvCreateImage(cvGetSize(inf),IPL_DEPTH_64F,0);  
	//preresult = cvLoadImage( "\\preresult.png" , 0 );

	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start) / ((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 6 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 6 in " << section_time << "ms." << endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 7:																	 //
	//			use all in focus image to generate 256 synthetic defocus images      //
    //          And use pre depth result to initial xttemp                           //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////
 
	cout << "Starting Step 7..." << endl;
	section_start = (double)cvGetTickCount();

	//IplImage* GauBlur[260];
 ////// Store blur index for each pixel each channel /////////////////////////////////////////	
	//unsigned char **xttempy[260];
	//unsigned char **xttempCr[260];
	//unsigned char **xttempCb[260];

	Mat xttempY = Mat(imageSize, CV_8UC1);
	Mat xttempCr = Mat(imageSize, CV_8UC1);
	Mat xttempCb = Mat(imageSize, CV_8UC1);

 ////// y : true defocus image ///////////////////////////////////////////////////////////////
 ////// xt : 256 synthetic defocus images ////////////////////////////////////////////////////
	double **yY[1], **yCr[1], **yCb[1];
	double **xtY[260], **xtCr[260], **xtCb[260];
	//double **atlas[1];

	vector<Mat> xt_Y(257);
	vector<Mat> xt_Cr(257);
	vector<Mat> xt_Cb(257);
	for (idx = 0; idx <= MAX_CLASSES; idx++)
	{
		xt_Y[idx] = Mat(imageSize, CV_64FC1);
		xt_Cr[idx] = Mat(imageSize, CV_64FC1);
		xt_Cb[idx] = Mat(imageSize, CV_64FC1);
	}

	std::thread t_Y(createblur, col, row, ImageInFocusY, classes, xt_Y, SyntheticDefocusY, xttempY);
	std::thread t_Cr(createblur, col, row, ImageInFocusCr, classes, xt_Cr, SyntheticDefocusCr, xttempCr);
	std::thread t_Cb(createblur, col, row, ImageInFocusCb, classes, xt_Cb, SyntheticDefocusCb, xttempCb);
	
	t_Y.join();
	t_Cr.join();
	t_Cb.join();

	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start) / ((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 7 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 7 in " << section_time << "ms." << endl;

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////
	//																				 //
	//    Step 8:																	 //
	//			EM calculation                                                       //
	//																				 //	
	///////////////////////////////////////////////////////////////////////////////////

	cout << "Starting Step 8..." << endl;
	section_start = (double)cvGetTickCount();

	//double **diff_y[260];	// , **diff_Cr[260], **diff_Cb[260];
	vector<Mat> diff_Y(257);
	vector<Mat> diff_Cr(257);
	vector<Mat> diff_Cb(257);
	vector<Mat> diff_Y1(257);
	vector<Mat> diff_Cr1(257);
	vector<Mat> diff_Cb1(257);
//// calculate Data term (y-b)^2 ////////////////////////////////////////////////////////
////// In order to save memory, here replace  diff_Cr, diff_cb with xtCr, xtCb/////////
	int64 temp_add = 0;
	Mat temp_image;
	for (kdx = 0; kdx <= MAX_CLASSES; kdx++)
	{
		//diff_Y[kdx] = Mat(imageSize, CV_64FC1);
		//diff_Cr[kdx] = Mat(imageSize, CV_64FC1);
		//diff_Cb[kdx] = Mat(imageSize, CV_64FC1);

		diff_Y[kdx] = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
		diff_Cr[kdx] = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
		diff_Cb[kdx] = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
		Mat temp_sub = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
		//Mat temp_sub2 = Mat(imageSize, CV_64FC1);

		subtract(ImageOutOfFocusY, xt_Y[kdx], temp_sub);
		//multiply(temp_sub, temp_sub, diff_Y1[kdx]);
		accumulateSquare(temp_sub, diff_Y[kdx]);

		subtract(ImageOutOfFocusCr, xt_Cr[kdx], temp_sub);
		//multiply(temp_sub, temp_sub, diff_Cr1[kdx]);
		accumulateSquare(temp_sub, diff_Cr[kdx]);

		subtract(ImageOutOfFocusCb, xt_Cb[kdx], temp_sub);
		//multiply(temp_sub, temp_sub, diff_Cb1[kdx]);
		accumulateSquare(temp_sub, diff_Cb[kdx]);

		//for (idx = 0; idx < row; idx++)
		//{
		//	for (jdx = 0; jdx < col; jdx++)
		//	{
		//		double y_Y = ImageOutOfFocusY.at<double>(idx, jdx);		//cvGetReal2D(ImageOutOfFocusY, idx, jdx);
		//		double y_Cr = ImageOutOfFocusCr.at<double>(idx, jdx);	//cvGetReal2D(ImageOutOfFocusCr, idx, jdx);
		//		double y_Cb = ImageOutOfFocusCb.at<double>(idx, jdx);	//cvGetReal2D(ImageOutOfFocusCb, idx, jdx);
		//		//diff_y[kdx][idx][jdx] = (double)(yY[0][idx][jdx] - xtY[kdx][idx][jdx])*(double)(yY[0][idx][jdx] - xtY[kdx][idx][jdx]);
		//		//xtCr[kdx][idx][jdx] = (double)(yCr[0][idx][jdx] - xtCr[kdx][idx][jdx])*(double)(yCr[0][idx][jdx] - xtCr[kdx][idx][jdx]);
		//		//xtCb[kdx][idx][jdx] = (double)(yCb[0][idx][jdx] - xtCb[kdx][idx][jdx])*(double)(yCb[0][idx][jdx] - xtCb[kdx][idx][jdx]);
		//		
		//		//diff_y[kdx][idx][jdx] = (double)(y_Y - xtY[kdx][idx][jdx])*(double)(y_Y - xtY[kdx][idx][jdx]);
		//		//xtCr[kdx][idx][jdx] = (double)(y_Cr - xtCr[kdx][idx][jdx])*(double)(y_Cr - xtCr[kdx][idx][jdx]);
		//		//xtCb[kdx][idx][jdx] = (double)(y_Cb - xtCb[kdx][idx][jdx])*(double)(y_Cb - xtCb[kdx][idx][jdx]);

		//		diff_Y[kdx].at<double>(idx, jdx) = (double)(y_Y - xt_Y[kdx].at<double>(idx, jdx))*(double)(y_Y - xt_Y[kdx].at<double>(idx, jdx));
		//		xt_Cr[kdx].at<double>(idx, jdx) = (double)(y_Cr - xt_Cr[kdx].at<double>(idx, jdx))*(double)(y_Cr - xt_Cr[kdx].at<double>(idx, jdx));
		//		xt_Cb[kdx].at<double>(idx, jdx) = (double)(y_Cb - xt_Cb[kdx].at<double>(idx, jdx))*(double)(y_Cb - xt_Cb[kdx].at<double>(idx, jdx));
		//	

		//	}	// end of jdx loop

		//}	// end of idx loop

		//temp_add = (int64)&xt_Y[kdx].data;
		temp_image = xt_Y[kdx];
		temp_add = sizeof(Mat);	
	}	// end of kdx loop
 

 //// Parameters for EM calculation 256 classes ////////////////////////////////////////
	double yaccum[257], ysquaredaccum[257], Num[257]; 
	double m_Y[257], v_Y[257], N_Y[257];	
  
 ////// Choose initial conditions for mean and variance ////////////////////////////////
	m_Y[0] = 0;
	for (int l=1; l<=classes; l++)
	{
		//if (l==0) 
		//{
		//	m_Y[l] =0;
		//}
		//else 
		//{
			//m_Y[l]  = m_Y[l-1]  + 255/(classes+1); 
			m_Y[l] = m_Y[l - 1] + 255.0 / (classes + 1);
		//}
		//N_Y[l]  = 0;
		//v_Y[l]  = 0.5;
	}
	memset(N_Y, 0, classes+1);
	std::fill_n(&v_Y[0], classes + 1, 0.5);

 //// EM begin /////////////////////////////////////////////////////////////////////////
	Mat DepthMap = Mat(Size(col, row), CV_8UC1);
	double start= (double)cvGetTickCount();

	//DataLogStream.close();

	for (int i=0; i<EMiteration; i++)
	{
		cout<<"EM iteration "<<i<<endl;

    /* Call MAP subtutine */	
		//map3(yY, xtY, yCr, xtCr, yCb, xtCb, atlas, beta, gama, ATLAS, ICM, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num,xttempy, xttempCr, xttempCb,texture,textureCb,textureCr,highpass,highpassCr,highpassCb);	  
		cout << "Running MAP..." << endl;
		string DataLog = ".";
//		map3(yY, xtY, diff_y, xtCr, xtCb, atlas, beta, gama, ATLAS, ICM, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num,xttempy, xttempCr, xttempCb,texture,textureCb,textureCr,highpass,highpassCr,highpassCb);	  
//		std::thread t(map3, std::ref(DataLog), ImageOutOfFocusY, DepthMap, diff_Y, diff_Cr, diff_Cb, atlas, beta, gamma, ATLAS, ICM, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num, xttempY, xttempCr, xttempCb, textureY, textureCb, textureCr, highpassY, highpassCr, highpassCb);
		std::thread t(map3, std::ref(DataLog), ImageOutOfFocusY, DepthMap, diff_Y, diff_Cr, diff_Cb, beta, col, row, classes, mapiter, v_Y, yaccum, ysquaredaccum, Num, xttempY, xttempCr, xttempCb, textureY, textureCb, textureCr, highpassY, highpassCr, highpassCb);
		t.join();

		cout << "MAP Complete." << endl;
		DataLogStream << DataLog;

		for (int l=0; l<=classes; l++)  
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

	cout << "Creating Depth Map..." << endl;

 ///// Generate depth map and save it ////////////////////////////////////////////////
    //IplImage* DepthMap = cvCreateImage(cvSize(col,row),8,1);

	

	//Mat DepthMap = Mat(Size(col, row), CV_8UC1 , (unsigned char *)xtY[0], col);

	//for (int i = 0; i < row; i++)
	//{
	//	for (int j = 0; j < col; j++)
	//	{
	//		DepthMap.at<uchar>(i, j) = xtY[0][i][j];
	//		//cvSetReal2D(DepthMap, i, j, (double)xtY[0][i][j]);
	//	}
	//}

	imwrite(image_locations + "\\Depth_Map.png", DepthMap, compression_params);



	imshow("Depth Map", DepthMap);

	//imshow("Ground Truth", groundtruth);

	waitKey(-1);

	destroyAllWindows();
	//cvSaveImage((image_locations + "\\blurmaplc64.png").c_str() , DepthMap);

//	cvSaveImage("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/blurmaplc64.png",BlurMap);

	section_stop = (double)cvGetTickCount();
	section_time = (section_stop - section_start) / ((double)cvGetTickFrequency()*1000.0);

	cout << "Completed Step 8 in " << section_time << "ms." << endl;
	DataLogStream << "Completed Step 8 in " << section_time << "ms." << endl;

	DataLogStream.close();


	cout << "End of Program." << endl;

	cin.ignore();
    return 0; 

}


