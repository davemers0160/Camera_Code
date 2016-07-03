// C++ Includes
//#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
//#include <vector>
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
	int imageCapture(Camera *cam, HANDLE lensDriver, string file_base, unsigned int numCaptures, float fps)
#else
	int imageCapture(Camera *cam, FT_HANDLE lensDriver, string file_base, unsigned int numCaptures, float fps)
#endif
{
	// timing variables
	double duration=0;
	double tickFreq = 1000.0 / getTickFrequency();

	// image variables
	unsigned int count = 0;
	unsigned int image_rows = 0;
	unsigned int image_cols = 0;
	unsigned int image_stride = 0;
	unsigned int image_data_size = 0;
	char count_string[10];
	BOOL status;

	// Lens Driver Variables
	LensFocus LensDfD((unsigned char)137, (unsigned char)142);
	LensTxPacket Focus(FAST_SET_VOLT, 1, &LensDfD.Focus[0]);
	LensTxPacket DeFocus(FAST_SET_VOLT, 1, &LensDfD.Focus[1]);

	// Camera variables
	Error error;
	Image rawImage;
	PixelFormat pixFormat;

	// file operation variables
	string save_file_name;

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
	Mat imageFrame;
	//VideoWriter focusVideo, defocusVideo;
	Image convertedImageCV;	
	//char* Window1 = "Video Display";
	//namedWindow(Window1, WINDOW_NORMAL);   

#else
	clock_t tick1, tick2;
	
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

		//image_cols = rawImage.GetCols();
		//image_rows = rawImage.GetRows();
		//image_stride = rawImage.GetStride();
		//image_data_size = rawImage.GetDataSize();
		BayerTileFormat btFormat;

		rawImage.GetDimensions(&image_rows, &image_cols, &image_stride, &pixFormat, &btFormat);

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
		
		image_size = Size((int)image_cols, (int)image_rows);
		
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
		
		sprintf( count_string, "%03d", count );

//////////////////////////// START FOCUS CAPTURE //////////////////////////////
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
		//save_file_name.clear();
		//save_file_name.str("");
		//save_file_name << file_base << "_focus_" << setfill('0') << setw(5) << count << ".png";


		save_file_name = file_base + "_focus_" + (string)count_string	+ ".png";

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

		imageFrame = Mat(image_size, CV_8UC3, image_data, image_stride);
		//double t7 = (double)getTickCount();

		imwrite(save_file_name, imageFrame);
		//focusVideo.write(video_frame);
		//double t8 = (double)getTickCount();

		//cout << "t6-t5: " << ((t6 - t5) * 1000) * tickFreq << endl;
		//cout << "t7-t6: " << ((t7 - t6) * 1000) * tickFreq << endl;
		//cout << "t8-t7: " << ((t8 - t7) * 1000) * tickFreq << endl;

#else
		// FlyCapture2 Append image to AVI file
		error = rawImage.Save(save_file_name.str().c_str());
		if (error != PGRERROR_OK)
		{
			PrintError( error );
			continue;
		}

#endif

/////////////////////////// START DEFOCUS CAPTURE /////////////////////////////

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

		// send the focus command to the lens driver ahead of the next image read 
		// to allow for settling and to make sure that the lens is at the right voltage 
		sendLensPacket(Focus, lensDriver);

		//save_file_name.clear();
		//save_file_name.str("");

		//save_file_name << file_base << "_defocus_" << setfill('0') << setw(5) << count << ".png";
		save_file_name = file_base + "_focus_" + (string)count_string + ".png";

#ifdef USE_OPENCV
		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		//error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		//if (error != PGRERROR_OK)
		//{
		//	PrintError(error);
		//	return -1;
		//}

		// Convert data to opencv format
		imageFrame = Mat(image_size, CV_8UC3, image_data, image_stride);
		imwrite(save_file_name, imageFrame);

#else
		// FlyCapture2 Append image to AVI file
		error = rawImage.Save(save_file_name.str().c_str());
		if (error != PGRERROR_OK)
		{
			PrintError( error );
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
	
	//focusVideo.release(); 
	//defocusVideo.release();
	destroyAllWindows();

#else
	// FlyCapture2 functions to complete actions
	cout << "Average Frame Rate (fps): " << dec << (unsigned short)(1.0/(duration/(numCaptures*2.0))) << endl;

#endif
	sendLensPacket(Focus, lensDriver);

	cout << "Finished Writing Images!" << endl;

	return 0;

}	// end of imageCapture



