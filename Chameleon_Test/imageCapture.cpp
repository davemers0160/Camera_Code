// C++ Includes
//#include <map>
#include <iostream>
#include <sstream>
#include <string>
//#include <vector>
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
//#include "stdafx.h"
#include "FlyCapture2.h"
#include "Chameleon_Utilities.h"

// Lens Driver Includes
#include "Lens_Driver.h"

// FTDI Driver Includes
#include "ftd2xx.h"

using namespace std;
using namespace FlyCapture2;
using namespace Lens_Driver;

extern double tickFreq;

int imageCapture(Camera *cam, HANDLE lensDriver, string file_base, unsigned int numCaptures, float fps)
{
	// timing variables
	double duration=0;

	// image variables
	unsigned int count = 0;
	unsigned int image_rows = 0;
	unsigned int image_cols = 0;
	unsigned int image_stride = 0;
	unsigned int image_data_size = 0;
	boolean status;

	// Lens Driver Variables
	LensFocus LensDfD((unsigned char)137, (unsigned char)142);
	LensTxPacket Focus(FAST_SET_VOLT, 1, &LensDfD.Focus[0]);
	LensTxPacket DeFocus(FAST_SET_VOLT, 1, &LensDfD.Focus[1]);

	// Camera variables
	Error error;
	Image rawImage;

	// file operation variables
	ostringstream save_file_name;

#ifdef USE_OPENCV
	// OpenCV variables
	double tick1, tick2;
	
	//int codec = CV_FOURCC('M', 'J', 'P', 'G');
	//int codec = CV_FOURCC('D', 'I', 'V', 'X');
	//int codec = CV_FOURCC('H', '2', '6', '4');
	//int codec = CV_FOURCC('X', '2', '6', '4');
	//int codec = CV_FOURCC('L', 'A', 'G', 'S');
	//int codec = CV_FOURCC('M', 'J', '2', 'C');
	//int codec = -1;

	Size image_size;
	Mat frame;
	//VideoWriter focusVideo, defocusVideo;
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

#ifdef USE_OPENCV
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
		
		//focusVideo.open(focus_save_file, codec, fps, image_size, true);
		//defocusVideo.open(defocus_save_file, codec, fps, image_size, true);

#else
		image_cols = rawImage.GetCols();
		image_rows = rawImage.GetRows();

#endif

		cout << endl << "Video size: " << image_cols << " x " << image_rows << "\tImage stride: " << image_stride << endl;

	}

	sendLensPacket(Focus, lensDriver);


	// start of the main capture loop
	while (count < numCaptures)
	{
#ifdef USE_OPENCV
		tick1 = (double)getTickCount();
#else
		tick1 = clock();
#endif
		
		//double t1 = (double)getTickCount();
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

		//double t4 = (double)getTickCount();

		//cout << "t2-t1: " << ((t2 - t1) * 1000) * tickFreq << endl;
		//cout << "t3-t2: " << ((t3 - t2) * 1000) * tickFreq << endl;
		//cout << "t4-t1: " << ((t4 - t1) * 1000) * tickFreq << endl;


		// send blurred voltage to lens driver
		sendLensPacket(DeFocus, lensDriver);

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


		//unsigned char *temp_image_data = NULL;
		//temp_image_data = rawImage.GetData();
		//Mat temp_video = Mat(Size(temp_cols, temp_rows), CV_8UC3, temp_image_data, temp_stride);

		// Convert data to opencv format
		//image_data = convertedImageCV.GetData();
		image_data = rawImage.GetData();

		video_frame = Mat(image_size, CV_8UC3, image_data, image_stride);
		//double t7 = (double)getTickCount();



		focusVideo.write(video_frame);
		//double t8 = (double)getTickCount();

		//cout << "t6-t5: " << ((t6 - t5) * 1000) * tickFreq << endl;
		//cout << "t7-t6: " << ((t7 - t6) * 1000) * tickFreq << endl;
		//cout << "t8-t7: " << ((t8 - t7) * 1000) * tickFreq << endl;

#else
		// FlyCapture2 Append image to AVI file
		error = videoFile.AVIAppend(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

#endif
		//t1 = (double)getTickCount();
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
		//t4 = (double)getTickCount();

		//cout << "t2-t1: " << ((t2 - t1) * 1000) * tickFreq << endl;
		//cout << "t3-t2: " << ((t3 - t2) * 1000) * tickFreq << endl;
		//cout << "t4-t1: " << ((t4 - t1) * 1000) * tickFreq << endl;

		sendLensPacket(Focus, lensDriver);


#ifdef USE_OPENCV
		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Convert data to opencv format
		image_data = convertedImageCV.GetData();
		//image_data = rawImage.GetData();

		video_frame = Mat(image_size, CV_8UC3, image_data, image_stride);

		// display images
		//imshow(Window1, video_frame);
		defocusVideo.write(video_frame);

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
		duration += (double)((tick2 - tick1)/ CLOCKS_PER_SEC);
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
	cout << "Average Frame Rate (fps): " << dec << (unsigned short)(1/((duration * tickFreq)/ (numCaptures*2.0))) << endl;
	
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



