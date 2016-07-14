// C++ Includes
//#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
//#include <iomanip>
#include <ctime>
#include <pthread.h>
#include <stdio.h>

// windows Includes
//#include <windows.h>

// OPENCV Includes
#define USE_OPENCV

#ifdef USE_OPENCV
	#include <opencv2/core/core.hpp>
	#include <opencv2/highgui/highgui.hpp>     
	#include <opencv2/imgproc/imgproc.hpp>  
	using namespace cv;
#endif

// Point Grey Includes
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	#include "FlyCapture2.h"
#else
	#include "../Chameleon_Test_Linux/include/FlyCapture2.h"
#endif

#include "Chameleon_Utilities.h"

// Lens Driver Includes
#include "Lens_Driver.h"

// FTDI Driver Includes
#include "ftd2xx.h"

using namespace std;
using namespace FlyCapture2;
using namespace Lens_Driver;

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	int videoCaptureInt(Camera *cam, HANDLE lensDriver, string focus_save_file, string defocus_save_file, unsigned int numCaptures, float fps)
#else
	int videoCaptureInt(Camera *cam, FT_HANDLE lensDriver, string focus_save_file, string defocus_save_file, unsigned int numCaptures, float fps)
#endif
{

	unsigned int idx;
	unsigned int count = 0;
	unsigned int image_rows = 0;
	unsigned int image_cols = 0;
	unsigned int image_stride = 0;
	unsigned int image_data_size = 0;
	BOOL status;

	// Lens Driver Variables
	LensFocus LensDfD((unsigned char)136, (unsigned char)140);
	LensTxPacket Focus(FAST_SET_VOLT, 1, &LensDfD.Focus[0]);
	LensTxPacket DeFocus(FAST_SET_VOLT, 1, &LensDfD.Focus[1]);

	// Camera variables
	Error error;
	Image rawImage;

	// OpenCV variables
	double tick1, tick2;
	double duration = 0.0;
	double tickFreq = 1000.0 / getTickFrequency();
	double delta = (getTickFrequency()/fps);
	double start, stop;


	//int codec = CV_FOURCC('M', 'J', 'P', 'G');
	//int codec = CV_FOURCC('D', 'I', 'V', 'X');
	//int codec = CV_FOURCC('X', 'V', 'I', 'D');
	//int codec = CV_FOURCC('H', '2', '6', '4'); 		// not on Odroid
	//int codec = CV_FOURCC('X', '2', '6', '4');		// not on Odroid
	//int codec = CV_FOURCC('M', 'J', '2', 'C');
	int codec = CV_FOURCC('M', 'P', '4', 'V');
	//int codec = -1;


	pthread_t Focus_Thread, Defocus_Thread;
	videoSaveStruct videoSaveFocus;
	videoSaveStruct videoSaveDefocus;

	//videoSaveFocus.VideoFrame = vector<Mat> (numCaptures);
	//videoSaveDefocus.VideoFrame = vector<Mat> (numCaptures);
	videoSaveFocus.FrameCount = 1;
	videoSaveDefocus.FrameCount = 1;

	Size image_size;
	//Mat video_frame;
	//vector<Mat> focusFrame(numCaptures);
	//vector<Mat> defocusFrame(numCaptures);
	//VideoWriter focusVideo, defocusVideo;
	Image convertedImageCV;	
	
	unsigned char *image_data = NULL;

	// poll the camera to see if it is ready for a software trigger
	PollForTriggerReady(cam);

	status = FireSoftwareTrigger(cam);
	if (status == false)
	{
		cout << "Error firing software trigger" << endl;
	}

	error = cam->RetrieveBuffer(&rawImage);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
	else
	{

		// OpenCV functions to save video
		//error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		//if (error != PGRERROR_OK)
		//{
		//	PrintError(error);
		//	return -1;
		//}
		//
		//image_cols = convertedImageCV.GetCols();
		//image_rows = convertedImageCV.GetRows();
		//image_stride = convertedImageCV.GetStride();
		//image_data_size = convertedImageCV.GetDataSize();
		
		image_cols = rawImage.GetCols();
		image_rows = rawImage.GetRows();
		image_stride = rawImage.GetStride();
		image_data_size = rawImage.GetDataSize();

		image_size = Size((int)image_cols, (int)image_rows);

		start = (double)getTickCount();
		//for (idx = 0; idx < numCaptures; idx++)
		//{
			//videoSaveFocus.VideoFrame[idx] = Mat(image_size, CV_8UC3);
			//videoSaveDefocus.VideoFrame[idx] = Mat(image_size, CV_8UC3);
			//focusFrame[idx] = Mat(image_size, CV_8UC3);
			//defocusFrame[idx] = Mat(image_size, CV_8UC3);

		//}

		videoSaveFocus.VideoFrame = Mat(image_size, CV_8UC3);
		videoSaveDefocus.VideoFrame = Mat(image_size, CV_8UC3);

		stop = (double)getTickCount();
		//cout << "1. stop-start: " << ((stop - start)) * tickFreq << endl;

		videoSaveFocus.FileName = focus_save_file;
		videoSaveDefocus.FileName = defocus_save_file;
		videoSaveFocus.VideoFile.open(focus_save_file, codec, fps, image_size, true);
		videoSaveDefocus.VideoFile.open(defocus_save_file, codec, fps, image_size, true);
		//focusVideo.open(focus_save_file, codec, fps, image_size, true);
		//defocusVideo.open(defocus_save_file, codec, fps, image_size, true);

		cout << endl << "Video size: " << image_cols << " x " << image_rows << "\tImage stride: " << image_stride << endl;

	}

	sendLensPacket(Focus, lensDriver);

	cout << "Starting capture loop (" << numCaptures << ")..." << endl;
	double loopstart = (double)getTickCount();
	// start of the main capture loop
	while (count < numCaptures)
	{

		tick1 = (double)getTickCount();
///////////////////////////////////////////////////////////////////////////////
//				 		 START THE FOCUS CAPTURE		  					 //
///////////////////////////////////////////////////////////////////////////////

		start = (double)getTickCount();

		double t1 = (double)getTickCount();
		// poll the camera to see if it is ready for a software trigger
		PollForTriggerReady(cam);
		
		double t2 = (double)getTickCount();
		status = FireSoftwareTrigger(cam);
		if (status == false)
		{
			cout << "Error firing software trigger" << endl;
			continue;
		}

		double t3 = (double)getTickCount();

		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			cout << "Count: " << count << endl;

		}

		double t4 = (double)getTickCount();

		//cout << "t2-t1: " << ((t2 - t1) ) * tickFreq << endl;
		//cout << "t3-t2: " << ((t3 - t2) ) * tickFreq << endl;
		//cout << "t4-t1: " << ((t4 - t1) ) * tickFreq << endl;

		// send blurred voltage to lens driver
		sendLensPacket(DeFocus, lensDriver);


		double t5 = (double)getTickCount();
		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		//error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		//if (error != PGRERROR_OK)
		//{
		//	PrintError(error);
		//	return -1;
		//}

		// Convert data to opencv format
		//image_data = convertedImageCV.GetData();
		image_data = rawImage.GetData();

		double t6 = (double)getTickCount();

		//video_frame = Mat(image_size, CV_8UC3, image_data, image_stride);
		//focusFrame[count] = Mat(image_size, CV_8UC3, image_data, image_stride);
		videoSaveFocus.VideoFrame = Mat(image_size, CV_8UC3, image_data, image_stride);

		pthread_create(&Focus_Thread, NULL, saveVideo_t, (void*)(&videoSaveFocus) );


		double t7 = (double)getTickCount();

		do
		{
			stop = (double)getTickCount();
		} while ((stop - start) < delta);
		//cout << "1. stop-start: " << ((stop - start)) * tickFreq << endl;

		if(count != 0)
		{
			pthread_join(Defocus_Thread, NULL);
		}

		//focusVideo.write(video_frame);
		double t8 = (double)getTickCount();

		//cout << "t6-t5: " << ((t6 - t5) ) * tickFreq << endl;
		//cout << "t7-t6: " << ((t7 - t6) ) * tickFreq << endl;
		//cout << "t8-t7: " << ((t8 - t7) ) * tickFreq << endl;

///////////////////////////////////////////////////////////////////////////////
//						START THE DEFOCUS CAPTURE							 //
///////////////////////////////////////////////////////////////////////////////

		start = (double)getTickCount();
		double t9 = (double)getTickCount();
		// poll the camera to see if it is ready for a software trigger
		PollForTriggerReady(cam);

		double t10 = (double)getTickCount();
		status = FireSoftwareTrigger(cam);
		if (status == false)
		{
			cout << "Error firing software trigger" << endl;
			continue;
		}

		double t11 = (double)getTickCount();
		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			cout << "Count: " << count << endl;

		}
		double t12 = (double)getTickCount();

		//cout << "t2-t1: " << ((t2 - t1) ) * tickFreq << endl;
		//cout << "t3-t2: " << ((t3 - t2) ) * tickFreq << endl;
		//cout << "t4-t1: " << ((t4 - t1) ) * tickFreq << endl;

		sendLensPacket(Focus, lensDriver);

		double t13 = (double)getTickCount();
		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		//error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		//if (error != PGRERROR_OK)
		//{
		//	PrintError(error);
		//	return -1;
		//}

		// Convert data to opencv format
		//image_data = convertedImageCV.GetData();
		image_data = rawImage.GetData();
		double t14 = (double)getTickCount();

		//video_frame = Mat(image_size, CV_8UC3, image_data, image_stride);
		//defocusFrame[count] = Mat(image_size, CV_8UC3, image_data, image_stride);
		videoSaveDefocus.VideoFrame = Mat(image_size, CV_8UC3, image_data, image_stride);
		pthread_create(&Defocus_Thread, NULL, saveVideo_t, (void*)(&videoSaveDefocus) );


		double t15 = (double)getTickCount();

		do
		{
			stop = (double)getTickCount();
		} while((stop-start) < delta);
		//cout << "2. stop-start: " << ((stop - start)) * tickFreq << endl;
		pthread_join(Focus_Thread, NULL);

		double t16 = (double)getTickCount();

		tick2 = (double)getTickCount();
		duration += (tick2 - tick1);// * tickFreq;

		//cout << (double)(t2 - t1)/ CLOCKS_PER_SEC  << "ms / CPS: " << CLOCKS_PER_SEC << endl;
		//cout << (tick2 - tick1) * tickFreq << "ms" << endl;
		count++;

		//cout << "Poll:        " << ((t2 - t1) ) * tickFreq << endl;
		//cout << "Trigger:     " << ((t3 - t2) ) * tickFreq << endl;
		//cout << "Get Data:    " << ((t4 - t3) ) * tickFreq << endl;
		//cout << "Send Focus:  " << ((t5 - t4) ) * tickFreq << endl;
		//cout << "Data 2 char: " << ((t6 - t5) ) * tickFreq << endl;
		//cout << "Save File:   " << ((t7 - t6) ) * tickFreq << endl;
		//cout << "Wait Delay:  " << ((t8 - t7) ) * tickFreq << endl;
		cout << "Focus Time:  " << ((t8 - t1) ) * tickFreq << endl;
		//cout << "Poll:        " << ((t10 - t9) ) * tickFreq << endl;
		//cout << "Trigger:     " << ((t11 - t10) ) * tickFreq << endl;
		//cout << "Get Data:    " << ((t12 - t11) ) * tickFreq << endl;
		//cout << "Send Defocus:" << ((t13 - t12) ) * tickFreq << endl;
		//cout << "Data 2 char: " << ((t14 - t13) ) * tickFreq << endl;
		//cout << "Save File:   " << ((t15 - t14) ) * tickFreq << endl;
		//cout << "Wait Delay:  " << ((t16 - t15) ) * tickFreq << endl;
		cout << "Defcous Time:" << ((t16 - t9) ) * tickFreq << endl;
		cout << endl;

		//pthread_join(Focus_Thread, NULL);
		//pthread_join(Defocus_Thread, NULL);

	}	// end of while loop
	double loopstop = (double)getTickCount();

	cout << "stop-start: " << ((loopstop - loopstart) ) * tickFreq << endl;
	cout << "Average Frame Rate (fps): " << (int)((2000.0*numCaptures)/((loopstop - loopstart) * tickFreq)) << endl;
	cout << "Average Frame Rate (fps): " << dec << (unsigned short)(1000/((duration * tickFreq)/ (numCaptures*2.0))) << endl;

///////////////////////////////////////////////////////////////////////////////
//						START THE IMAGE WRITING 							 //
///////////////////////////////////////////////////////////////////////////////

	// write the vector mat images to a video file
	//cout << "Writing video files..." << endl;
	//double t1 = (double)getTickCount();


	//for (idx = 0; idx < numCaptures; idx++)
	//{
	//	focusVideo.write(focusFrame[idx]);
	//	defocusVideo.write(defocusFrame[idx]);
	//}
	//pthread_create(&Focus_Thread, NULL, saveVideo_t, (void*)(&videoSaveFocus) );
	//pthread_create(&Defocus_Thread, NULL, saveVideo_t, (void*)(&videoSaveDefocus) );


	sendLensPacket(Focus, lensDriver);

	//pthread_join(Focus_Thread, NULL);
	pthread_join(Defocus_Thread, NULL);

	//double t2 = (double)getTickCount();
	//cout << "File Write:      " << ((t2 - t1) ) * tickFreq << endl;

	//focusVideo.release();
	//defocusVideo.release();

	videoSaveFocus.VideoFile.release();
	videoSaveDefocus.VideoFile.release();

	//cv::destroyAllWindows();

	cout << "Finished Writing Video!" << endl;

	return 0;

}	// end of videoCapture



