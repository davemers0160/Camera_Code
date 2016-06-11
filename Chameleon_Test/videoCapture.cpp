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

int videoCapture(Camera *cam, HANDLE lensDriver, string save_file, unsigned int numCaptures, float fps)
{
	// timing variables

	//auto tick1 = chrono::high_resolution_clock::now();
	//auto tick2 = chrono::high_resolution_clock::now();
	double duration=0;



	unsigned int key = 0;
	//float fps = 55.0;
	//unsigned int idx = 0;
	unsigned int image_rows, image_cols, image_stride;
	//unsigned int numCaptures = 200;

	// Lend Driver Variables
	LensFocus LensDfD((unsigned char)134, (unsigned char)138);
	//LensFocus LensDfD(38.295, 40.345);
	LensTxPacket Focus(FAST_SET_VOLT, 1, &LensDfD.Focus[0]);
	LensTxPacket DeFocus(FAST_SET_VOLT, 1, &LensDfD.Focus[1]);

	// Camera variables
	Error error;
	Image rawImage;

#ifdef USE_OPENCV
	// OpenCV variables
	double tick1, tick2;
	
	//int codec = CV_FOURCC('M', 'J', 'P', 'G');
	int codec = CV_FOURCC('D', 'I', 'V', 'X');
	unsigned int rowBytes;
	Size image_size;
	Mat video_frame;
	VideoWriter outputVideo;
	//char* Window1 = "Video Display";
	//int delay = 1;
	Image convertedImageCV;
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

	// Start capturing images
	//error = cam->StartCapture();
	//if (error != PGRERROR_OK)
	//{
	//	PrintError(error);
	//	return -1;
	//}
	
	unsigned char *image_data = NULL;

	error = cam->RetrieveBuffer(&rawImage);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		key = 'q';
	}
	else
	{

#ifdef USE_OPENCV
		// OpenCV functions to save video
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		
		image_cols = convertedImageCV.GetCols();
		image_rows = convertedImageCV.GetRows();
		image_stride = convertedImageCV.GetStride();
		rowBytes = (unsigned int)((double)convertedImageCV.GetDataSize() / (double)convertedImageCV.GetRows());
		image_size = Size((int)image_cols, (int)image_rows);
		
		outputVideo.open(save_file, codec, fps, image_size, true);
#else
		image_cols = rawImage.GetCols();
		image_rows = rawImage.GetRows();

#endif

		cout << "video size: " << image_cols << " x " << image_rows << endl;

	}

	sendLensPacket(Focus, lensDriver);

	while (key < numCaptures)
	{
#ifdef USE_OPENCV
		tick1 = (double)getTickCount();
#else
		tick1 = clock();
#endif

		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

		// send blurred voltage to lens driver
		sendLensPacket(DeFocus, lensDriver);

#ifdef USE_OPENCV
		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		unsigned int temp_rows = rawImage.GetRows();
		unsigned int temp_cols = rawImage.GetCols();
		unsigned int temp_stride = rawImage.GetStride();
		unsigned int temp_data_size = rawImage.GetDataSize();
		unsigned int image_data_size = convertedImageCV.GetDataSize();

		unsigned char *temp_image_data = NULL;
		temp_image_data = rawImage.GetData();
		Mat temp_video = Mat(Size(temp_cols, temp_rows), CV_8UC3, temp_image_data, temp_stride);

		// Convert data to opencv format
		image_data = convertedImageCV.GetData();
		video_frame = Mat(image_size, CV_8UC3, image_data, rowBytes);

		// display images
		//imshow(Window1, video_frame);
		outputVideo.write(video_frame);

#else
		// FlyCapture2 Append image to AVI file
		error = videoFile.AVIAppend(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

#endif

		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

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
		video_frame = Mat(image_size, CV_8UC3, image_data, rowBytes);

		// display images
		//imshow(Window1, video_frame);
		outputVideo.write(video_frame);

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
		key++;

	}

//	cout << "Average Execution Time: " << fixed << setw(5) << setprecision(2) << 1/((duration * tickFreq)/ (numCaptures*2)) << "ms" << endl;
//	duration /= CLOCKS_PER_SEC;

	// Finish writing video and close out file
#ifdef USE_OPENCV
	// OpenCV functions to complete Actions
	cout << "Average Frame Rate (fps): " << dec << (unsigned short)(1/((duration * tickFreq)/ (numCaptures*2.0))) << endl;
	outputVideo.release();
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



