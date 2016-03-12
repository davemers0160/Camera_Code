//=============================================================================
// 
// 
// 
//=============================================================================

// C++ Includes
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>

// windows Includes
#include <windows.h> 

// OPENCV Includes
#define USE_OPENCV

#ifdef USE_OPENCV
	#include <opencv2/core/core.hpp>           
	#include <opencv2/highgui/highgui.hpp>     
	#include <opencv2/imgproc/imgproc.hpp>  
	using namespace cv;
#endif

// Point Grey Includes
#include "stdafx.h"
#include "FlyCapture2.h"
#include "Config_Chameleon.h"

// Lens Driver Includes
#include "Lens_Driver.h"

using namespace std;
using namespace FlyCapture2;
using namespace Lens_Driver;

struct ftdiDeviceDetails //structure storage for FTDI device details
{
	int devNumber;
	unsigned long type;
	unsigned long BaudRate;
	string Description;
	string serialNumber;
};

void getcurrenttime(char currenttime[]);
bool configLensDriver(LPCWSTR port, HANDLE &serialHandle);
//void cameraConnect(PGRGuid guid, Camera &cam);
int videoCapture(Camera *cam, HANDLE serialHandle, string save_file);


void PrintBuildInfo()
{
    FC2Version fc2Version;
    Utilities::GetLibraryVersion( &fc2Version );
    
	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	cout << version.str() << endl;  
    
	ostringstream timeStamp;
    timeStamp <<"Application build date: " << __DATE__ << " " << __TIME__;
	cout << timeStamp.str() << endl << endl;  
}


void PrintDriverInfo(LensDriverInfo *LensInfo)
{
	cout << "*** LENS DRIVER INFO ***" << endl;
	cout << "Serial Number: " << (unsigned int)LensInfo->SerialNumber << endl;
	cout << "Firmware Version: " << (unsigned int)LensInfo->FirmwareVersion[0] << "." << setfill('0') << setw(2) << (unsigned int)LensInfo->FirmwareVersion[1] << endl;
	if (LensInfo->DriverType == 0)
	{
		cout << "Driver Type: Microchip HV892" << endl;
	}
	cout << endl;
	

}	// end of PrintDriverInfo


int main(int /*argc*/, char** /*argv*/)
{    
	
	// Camera specific variables
	FlyCapture2::Error error;
	BusManager busMgr;
    PGRGuid guid;
	Camera cam = Camera();
	FC2Config cameraConfig;
	unsigned int numCameras;
	unsigned int offsetX, offsetY, width, height;
	PixelFormat pixelFormat;
	Property shutter, gain;

	
	//Lens_Driver test_lens;
	//unsigned char data[4] = { 1, 2, 3, 4 };
	LensTxPacket LensTx(CON,0);
	LensRxPacket LensRx;
	LensDriverInfo LensInfo;
	unsigned char status;

	// Serial Port specific variables
	wstring port = L"\\\\.\\COM9";
	LPCWSTR lensPort = port.c_str();	//L"\\\\.\\COM9"; //(LPCWSTR)port.c_str();	//"\\\\.\\COM7";
	HANDLE lensDriver = NULL;

	string save_file;
	char currenttime[80];

	//test_lens.driver_type = 0;
	//test_lens.setVoltage(10.0);
	//test_lens.Packet.ByteCount = 1;

	getcurrenttime(currenttime);

	save_file = "test_recording_" + (string)currenttime + ".mp4";
    
	PrintBuildInfo();

	configLensDriver(lensPort, lensDriver);

	sendLensPacket(LensTx, lensDriver);
	status = readLensPacket(&LensRx, lensDriver, 9);

	if (status == false)
	{
		cout << "Error communicating with lens driver." << endl;
		cin.ignore();
		return 1;
	}
	getLensDriverInfo(&LensInfo, LensRx);
	PrintDriverInfo(&LensInfo);
	

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
		cin.ignore();
		return 1;
    }

    cout << "Number of cameras detected: " << numCameras << endl; 
    
    error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
		cin.ignore();
        return 1;
    }

	// connect to the camera
	cameraConnect(guid, &cam);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		cin.ignore();
		return 1;
	}

	// Get the camera configuration
	error = cam.GetConfiguration(&cameraConfig);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		cin.ignore();
		return 1;
	}

	// configure the image size and the pixel format for the video
	// 1.216 MB/s
	offsetX = 80;		// 40
	width = 1120;		// 1200;
	
	offsetY = 228;		// 224;
	height = 724;		// 768;

	pixelFormat = PIXEL_FORMAT_422YUV8;
	configImagerFormat(&cam, offsetX, offsetY, width, height, pixelFormat);

	configProperty(shutter, SHUTTER, true, true);
	configProperty(gain, GAIN, false, false);

	error = setProperty(&cam, gain, 12.0);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}

	// begin the video capture
	videoCapture(&cam, lensDriver, save_file);

	// Disconnect the camera
	error = cam.Disconnect();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}

	// disconnect from lens driver
	CloseHandle(lensDriver);

    cout << "Done! Press Enter to exit..." << endl; 
    cin.ignore();

    return 0;
}





// support functions
void getcurrenttime(char currenttime[])
{
	time_t rawtime;
	struct tm * timeinfo;
	
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(currenttime, 80, "%m%d%Y_%H%M%S", timeinfo);
	string str(currenttime);
	//cout << currenttime << endl;
	//cout << str << endl;

	//return 0;
}

bool configLensDriver(LPCWSTR commPort, HANDLE &serialHandle)
{
	BOOL status = false;
	//HANDLE serialHandle;
	DCB serialParams = { 0 };

	serialParams.DCBlength = sizeof(serialParams);

	// open up serial port and set paramters
	serialHandle = CreateFileW(commPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (!GetCommState(serialHandle, &serialParams))
		cout << "Error getting information from the specified serial port" << endl;

	//GetCommState(serialHandle, &serialParams);
	serialParams.BaudRate = 250000;
	serialParams.ByteSize = 8;
	serialParams.StopBits = TWOSTOPBITS;
	serialParams.Parity = NOPARITY;
	
	status = SetCommState(serialHandle, &serialParams);
	if (!status)
	{
		cout << "Error setting serial port configuration" << endl;
		return false;
	}

	// Set timeouts
	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = 50;
	timeout.ReadTotalTimeoutConstant = 100;
	timeout.ReadTotalTimeoutMultiplier = 50;
	timeout.WriteTotalTimeoutConstant = 50;
	timeout.WriteTotalTimeoutMultiplier = 10;

	status = SetCommTimeouts(serialHandle, &timeout);
	if (!status)
	{
		cout << "Error setting serial port timeout parameters" << endl;
		return false;
	}

	return true;

}	// end of configLensDriver




int videoCapture(Camera *cam, HANDLE lensDriver, string save_file)
{
	// timing variables
	auto tick1 = chrono::high_resolution_clock::now();
	auto tick2 = chrono::high_resolution_clock::now();
	double duration=0;

	unsigned int key = 0;
	float fps = 55.0;
	unsigned int idx = 0;
	unsigned int image_rows, image_cols;
	unsigned int numCaptures = 20;

	// Lend Driver Variables
	//LensFocus LensDfD(38.295, 40.345);
	LensFocus LensDfD((unsigned char)144, (unsigned char)140);
	LensTxPacket Focus(FAST_SET_VOLT, 1, &LensDfD.Focus[0]);
	LensTxPacket DeFocus(FAST_SET_VOLT, 1, &LensDfD.Focus[1]);

	// Camera variables
	FlyCapture2::Error error;
	Image rawImage;
	

#ifdef USE_OPENCV
	// OpenCV variables	
	//int codec = -1;
	//int codec = CV_FOURCC('D', 'V', 'I', 'X');		// 3.1.0 mp4 won't save, mpg won't save, avi good
	//int codec = CV_FOURCC('M', 'J', 'P', 'G');	// 3.1.0 avi won't play, mp4 won't save, mpg won't play
	int codec = CV_FOURCC('H', '2', '6', '4');		// 3.1.0 mp4 very blurry, mpg won't play, avi good
	//int codec = CV_FOURCC('I', 'Y', 'U', 'V');	// completely uncompressed
	//int codec = CV_FOURCC('I', '4', '2', '0');	// completely uncompressed
	//int codec = CV_FOURCC('M', 'P', '4', '2');		// 3.1.0 very blocky under avi, mp4 won't save, mpg won't play

	unsigned int rowBytes;
	Size image_size;
	Mat video_frame;
	Mat video_frame2;
	VideoWriter outputVideo;
	char* Window1 = "Video Display";
	int delay = 1;
	Image convertedImageCV;
	//namedWindow(Window1, WINDOW_NORMAL);   
	unsigned char *image_data = NULL;
	unsigned char *raw_data = NULL;
#else
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
	error = cam->StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
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
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		

		double raw_temp = rawImage.GetDataSize();
		double raw_rows = rawImage.GetRows();
		image_cols = convertedImageCV.GetCols();
		image_rows = convertedImageCV.GetRows();
		double temp = convertedImageCV.GetDataSize();
		rowBytes = (unsigned int)(temp / (double)image_rows);
		//rowBytes = (unsigned int)((double)convertedImageCV.GetDataSize() / (double)convertedImageCV.GetRows());
		image_size = Size((int)image_cols, (int)image_rows);
		

		// temp to see what the data looks like
		/*
		raw_data = rawImage.GetData();
		//BayerTileFormat bayerTemp = rawImage.GetBayerTileFormat();
		image_data = convertedImageCV.GetData();
		Mat video_frame2 = Mat(image_size, CV_8UC3, raw_data, rowBytes);
		Mat video_frame16bit = Mat(image_size, CV_16UC3, raw_data);
		Mat video_frame8bit = video_frame16bit.clone();

		video_frame8bit.convertTo(video_frame8bit, CV_8UC2, 1/256);

		video_frame = Mat(image_size, CV_8UC3, image_data, rowBytes);
		//Mat rgb8BitMat(image_size, CV_8UC3);

		cv::cvtColor(video_frame16bit, video_frame2, CV_BayerGB2BGR);
		*/
		// end temp

		bool status = outputVideo.open(save_file, codec, fps, image_size, true);
#else
		image_cols = rawImage.GetCols();
		image_rows = rawImage.GetRows();

#endif

		cout << "video size: " << image_cols << " x " << image_rows << endl;

	}

	//sendLensPacket(Focus, lensDriver);


	while (key < numCaptures)
	{
		tick1 = chrono::high_resolution_clock::now();

		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

		// send blurred voltage to lens driver
		//sendLensPacket(DeFocus, lensDriver);

#ifdef USE_OPENCV
		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// Convert data to opencv format
		//image_data = rawImage.GetData();	// just to see what rawImage looks like
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

		//sendLensPacket(Focus, lensDriver);


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
		//key = waitKey(delay);

#else
		// FlyCapture2 Append image to AVI file
		error = videoFile.AVIAppend(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

#endif

		//tick = (double)getTickCount() - tick;
		tick2 = chrono::high_resolution_clock::now();
		duration += chrono::duration_cast<std::chrono::milliseconds>(tick2 - tick1).count();

		key++;

	}

	//cout << "Average Frame Rate: " << fixed << setw(5) << setprecision(2) << (unsigned char)(1000/(duration/(numCaptures*2))) << endl;
	cout << "Average Frame Rate: " << dec << (unsigned short)(1000 / (duration / (numCaptures * 2))) << endl;

	// Finish writing video and close out file
#ifdef USE_OPENCV
	// OpenCV functions to complete Actions
	outputVideo.release();
	destroyAllWindows();

#else
	// FlyCapture2 functions to complete actions
	error = videoFile.AVIClose();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
#endif

	cout << "Finished Writing Video!" << endl;

	// Stop capturing images
	error = cam->StopCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	return 0;

}	// end of videoCapture