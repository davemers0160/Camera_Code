// C++ Includes
//#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
//#include <iomanip>
#include <ctime>
//#include <chrono>
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

//volatile extern double tickFreq;


#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	int videoCapture(Camera *cam, HANDLE lensDriver, string focus_save_file, string defocus_save_file, unsigned int numCaptures, float fps)
#else
	int videoCapture(Camera *cam, FT_HANDLE lensDriver, string focus_save_file, string defocus_save_file, unsigned int numCaptures, float fps)
#endif
{
	// timing variables
	//auto tick1 = chrono::high_resolution_clock::now();
	//auto tick2 = chrono::high_resolution_clock::now();

	double tickFreq = 1000.0 / getTickFrequency();

	unsigned int count = 0;
	unsigned int image_rows = 0;
	unsigned int image_cols = 0;
	unsigned int image_stride = 0;
	unsigned int image_data_size = 0;
	BOOL status;

	// Lens Driver Variables
	LensFocus LensDfD((unsigned char)137, (unsigned char)142);
	LensTxPacket Focus(FAST_SET_VOLT, 1, &LensDfD.Focus[0]);
	LensTxPacket DeFocus(FAST_SET_VOLT, 1, &LensDfD.Focus[1]);

	// Camera variables
	Error error;
	Image rawImage;
	unsigned char *image_data = NULL;

#ifdef USE_OPENCV
	// OpenCV variables
	double tick1, tick2;
	double duration = 0.0;
	double delta = (0.020*getTickFrequency());
	double start, stop;
	
	int codec = CV_FOURCC('M', 'J', 'P', 'G');
	//int codec = CV_FOURCC('D', 'I', 'V', 'X');
	//int codec = CV_FOURCC('H', '2', '6', '4');
	//int codec = CV_FOURCC('X', '2', '6', '4');
	//int codec = CV_FOURCC('L', 'A', 'G', 'S');
	//int codec = CV_FOURCC('M', 'J', '2', 'C');
	//int codec = -1;

	Size image_size;
	Mat focus_frame, defocus_frame;
	VideoWriter focusVideo, defocusVideo;
	Image convertedImageCV;	
	//char* Window1 = "Video Display";
	//namedWindow(Window1, WINDOW_NORMAL);   

#else
	clock_t tick1, tick2;
	AVIRecorder videoFile;
	MJPGOption option;
	option.frameRate = fps;
	option.quality = 100;
	
	// Point Grey FlyCapture2 AVI saving 
	error = videoFile.AVIOpen(save_file.c_str(), &option);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
#endif
	


	sendLensPacket(Focus, lensDriver);

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

#ifdef USE_OPENCV
		// OpenCV functions to save video
		//error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		//if (error != PGRERROR_OK)
		//{
		//	PrintError(error);
		//	return -1;
		//}
		
		//image_cols = convertedImageCV.GetCols();
		//image_rows = convertedImageCV.GetRows();
		//image_stride = convertedImageCV.GetStride();
		//image_data_size = convertedImageCV.GetDataSize();
		
		image_cols = rawImage.GetCols();
		image_rows = rawImage.GetRows();
		image_stride = rawImage.GetStride();
		image_data_size = rawImage.GetDataSize();

		image_size = Size((int)image_cols, (int)image_rows);
		
		focusVideo.open(focus_save_file, codec, fps, image_size, true);
		defocusVideo.open(defocus_save_file, codec, fps, image_size, true);

#else
		image_cols = rawImage.GetCols();
		image_rows = rawImage.GetRows();

#endif

		cout << endl << "Video size: " << image_cols << " x " << image_rows << "\tImage stride: " << image_stride << endl;

	}




	// start of the main capture loop
	while (count < numCaptures)
	{
#ifdef USE_OPENCV
		tick1 = (double)getTickCount();
#else
		tick1 = clock();
#endif
		
		start = (double)getTickCount();
		// poll the camera to see if it is ready for a software trigger
		PollForTriggerReady(cam);
		
		//double t2 = (double)getTickCount();
		status = FireSoftwareTrigger(cam);
		if (status == false)
		{
			cout << "Error firing software trigger" << endl;
			continue;
		}

		//double t3 = (double)getTickCount();

		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}


		//cout << "t3-t2: " << ((t3 - t2) * 1000) * tickFreq << endl;
		//cout << "t4-t3: " << ((t4 - t3) * 1000) * tickFreq << endl;


		// send blurred voltage to lens driver
		sendLensPacket(DeFocus, lensDriver);

		//stop = (double)getTickCount();

		//cout << "1. stop-start: " << ((stop-start) * 1000) * tickFreq << endl;

#ifdef USE_OPENCV
		//double t5 = (double)getTickCount();
		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		//error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		//if (error != PGRERROR_OK)
		//{
		//	PrintError(error);
		//	return -1;
		//}
		//double t6 = (double)getTickCount();

		//unsigned int temp_rows = rawImage.GetRows();
		//unsigned int temp_cols = rawImage.GetCols();
		//unsigned int temp_stride = rawImage.GetStride();
		//unsigned int temp_data_size = rawImage.GetDataSize();
		//unsigned int image_data_size = convertedImageCV.GetDataSize();

		//unsigned char *temp_image_data = NULL;
		//temp_image_data = rawImage.GetData();
		//Mat temp_video = Mat(Size(temp_cols, temp_rows), CV_8UC3, temp_image_data, temp_stride);

		// Convert data to opencv format
		//image_data = convertedImageCV.GetData();
		image_data = rawImage.GetData();

		focus_frame = Mat(image_size, CV_8UC3, image_data, image_stride);

		do
		{
			stop = (double)getTickCount();
		} while((stop-start) < delta);

		//focusVideo.write(focus_frame);
		//double t8 = (double)getTickCount();
		cout << "1. stop-start: " << ((stop-start)) * tickFreq << endl;

		//cout << "t6-t5: " << ((t6 - t5)) * tickFreq << endl;
		//cout << "t7-t6: " << ((t7 - t6)) * tickFreq << endl;
		//cout << "t8-t7: " << ((t8 - t7)) * tickFreq << endl;

#else
		// FlyCapture2 Append image to AVI file
		error = videoFile.AVIAppend(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

#endif
		start = (double)getTickCount();

		// poll the camera to see if it is ready for a software trigger
		PollForTriggerReady(cam);

		//t2 = (double)getTickCount();
		status = FireSoftwareTrigger(cam);
		if (status == false)
		{
			cout << "Error firing software trigger" << endl;
			continue;
		}

		//t3 = (double)getTickCount();
		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

		//cout << "t2-t1: " << ((t2 - t1) ) * tickFreq << endl;
		//cout << "t3-t2: " << ((t3 - t2) ) * tickFreq << endl;
		//cout << "t4-t1: " << ((t4 - t1) ) * tickFreq << endl;

		sendLensPacket(Focus, lensDriver);

		//stop = (double)getTickCount();

		//cout << "3. stop-start: " << ((stop-start) ) * tickFreq << endl;

#ifdef USE_OPENCV
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

		defocus_frame = Mat(image_size, CV_8UC3, image_data, image_stride);

		do
		{
			stop = (double)getTickCount();
		} while((stop-start) < delta);

		cout << "2. stop-start: " << ((stop-start) ) * tickFreq << endl;

		//t7 = (double)getTickCount();
		focusVideo.write(focus_frame);

		defocusVideo.write(defocus_frame);

		stop = (double)getTickCount();

		cout << "3. stop-start: " << ((stop-start) ) * tickFreq << endl;

#else
		// FlyCapture2 Append image to AVI file
		error = videoFile.AVIAppend(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

#endif


#ifdef USE_OPENCV
		tick2 = (double)getTickCount();
		duration += (tick2 - tick1);// * tickFreq;
#else
		tick2 = clock();
		//duration += (double)((tick2 - tick1)/ CLOCKS_PER_SEC);
#endif

		//cout << (double)(t2 - t1)/ CLOCKS_PER_SEC  << "ms / CPS: " << CLOCKS_PER_SEC << endl;
		//cout << (tick2 - tick1) * tickFreq << "ms" << endl;
		count++;

	}	// end of while loop


//	cout << "Average Execution Time: " << fixed << setw(5) << setprecision(2) << 1/((duration * tickFreq)/ (numCaptures*2)) << "ms" << endl;
//	duration /= CLOCKS_PER_SEC;

	// Finish writing video and close out file
#ifdef USE_OPENCV
	// OpenCV functions to complete Actions
	cout << "Average Frame Rate (fps): " << dec << (unsigned short)(1000/((duration * tickFreq)/ (numCaptures*2.0))) << endl;
	
	focusVideo.release(); 
	defocusVideo.release();
	destroyAllWindows();

#else
	// FlyCapture2 functions to complete actions
	cout << "Average Frame Rate (fps): " << dec << (unsigned short)(1.0/(duration/(numCaptures*2.0))) << endl;
	error = videoFile.AVIClose();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
#endif
	sendLensPacket(Focus, lensDriver);

	cout << "Finished Writing Video!" << endl;

	// Stop capturing images
	//error = cam->StopCapture();
	//if (error != PGRERROR_OK)
	//{
	//	PrintError(error);
	//	return -1;
	//}

	return 0;

}	// end of videoCapture



