//=============================================================================
// 
// 
// 
//=============================================================================

// C++ Includes
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
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
#include "../Camera_Shared_Code/stdafx.h"
#include "include/FlyCapture2.h"
#include "../Camera_Shared_Code/Chameleon_Utilities.h"

// Lens Driver Includes
#include "../Camera_Shared_Code/Lens_Driver.h"

// FTDI Driver Includes
#include "../Camera_Shared_Code/ftd2xx.h"

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

double tickFreq = 1.0 / getTickFrequency();

//void getcurrenttime(char currenttime[]);
int videoCapture(Camera *cam, FT_HANDLE lensDriver, string focus_save_file, string defocus_save_file, unsigned int numCaptures, float fps);
int imageCapture(Camera *cam, FT_HANDLE lensDriver, string file_base, unsigned int numCaptures, float fps);



FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript);



//void PrintBuildInfo()
//{
//    FC2Version fc2Version;
//    Utilities::GetLibraryVersion( &fc2Version );
    
//	ostringstream version;
//	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
//	cout << version.str() << endl;  
    
//	ostringstream timeStamp;
//    timeStamp <<"Application build date: " << __DATE__ << " " << __TIME__;
//	cout << timeStamp.str() << endl << endl;  
//}


//void PrintDriverInfo(LensDriverInfo *LensInfo)
//{
//	cout << "*** LENS DRIVER INFO ***" << endl;
//	cout << "Serial Number: " << (unsigned int)LensInfo->SerialNumber << endl;
//	cout << "Firmware Version: " << (unsigned int)LensInfo->FirmwareVersion[0] << "." << setfill('0') << setw(2) << (unsigned int)LensInfo->FirmwareVersion[1] << endl;
//	if (LensInfo->DriverType == 0)
//	{
//		cout << "Driver Type: Microchip HV892" << endl;
//	}
//	cout << endl;
//	
//
//}	// end of PrintDriverInfo


int main(int /*argc*/, char** /*argv*/)
{    
	
	// Camera specific variables
	FlyCapture2::Error error;
	BusManager busMgr;
    PGRGuid guid;
	Camera cam;
	FC2Config cameraConfig;
	unsigned int numCameras;
	unsigned int offsetX, offsetY, width, height;
	PixelFormat pixelFormat;
	float shutter, gain, brightness, auto_exp;
	int sharpness;
	float framerate = 60.0;

	
	//Lens_Driver
	LensTxPacket LensTx(CON,0);
	LensRxPacket LensRx;
	LensDriverInfo LensInfo;
	unsigned char status;
	unsigned char data[1] {135};
	LensTxPacket Focus(FAST_SET_VOLT, 1, &data[0]);

	// Serial Port specific variables
	FT_HANDLE lensDriver = NULL;
	FT_HANDLE gpsHandle = NULL;
	ftdiDeviceDetails driverDeviceDetails, gpsDeviceDetails;


	// file operations
	string save_path;
	string file_base;
	string recording_name = "test_recording";
	string image_save_file;
	string video_save_file;
	string focus_save_file;
	string defocus_save_file;
	string config_save_file;	
	string file_extension = ".avi";
	std::ofstream configFile;
	char currenttime[80];


	// place videos in specific location based on the OS that the code is running on
	//videoSaveFile = "/home/odroid/Videos/test_recording_" + (string)currenttime + ".avi";
	//videoSaveFile = "/media/odroid/TOSHIBA EXT/Videos/test_recording_" + (string)currenttime + ".avi";


	PrintBuildInfo();

	// check for lens driver
	while (lensDriver == NULL)
	{
		driverDeviceDetails.devNumber = 0;
		driverDeviceDetails.type = 0;
		driverDeviceDetails.BaudRate = 250000;
		driverDeviceDetails.Description = "";
		driverDeviceDetails.serialNumber = "";

		lensDriver = OpenComPort(&driverDeviceDetails, "Microfluidic Lens Driver");
		//FTDI manual says that the buffer goes up to 64 KB
		//SetupComm(lensDriver, 16 * 4 * 1024, 4 * 1024);
	}

	sendLensPacket(LensTx, lensDriver);

	status = readLensPacket(&LensRx, lensDriver, 9);

	if (status == false)
	{
		cout << "Error communicating with lens driver." << endl;
		return 1;
	}
	getLensDriverInfo(&LensInfo, LensRx);
	PrintDriverInfo(&LensInfo);


    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
		return 1;
    }

    cout << "Number of cameras detected: " << numCameras << endl; 
    
    error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return 1;
    }

	// connect to the camera
	cameraConnect(guid, &cam);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}

	// Get the camera configuration
	error = cam.GetConfiguration(&cameraConfig);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}


	cameraConfig.grabTimeout = 40;// (unsigned int)(1000 / framerate);
	//cameraConfig.highPerformanceRetrieveBuffer = true;
	cameraConfig.asyncBusSpeed = BUSSPEED_ANY;

	// Set the camera configuration
	error = cam.SetConfiguration(&cameraConfig);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	// set lens to intial focus
	sendLensPacket(Focus, lensDriver);

	// configure the image size and the pixel format for the video
	// 1.216 MB/s
	offsetX = 80;		// 40
	width = 1120;		// 1200;
	
	offsetY = 228;		// 224;
	height = 724;		// 768;

	cout << "Configuring Camera!" << endl;

	//pixelFormat = PIXEL_FORMAT_422YUV8;
	pixelFormat = PIXEL_FORMAT_444YUV8;
	//pixelFormat = PIXEL_FORMAT_RGB8;
	error = configImagerFormat(&cam, offsetX, offsetY, width, height, pixelFormat);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	error = cam.StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}


	///////////////////////////////////////////////////////////////////////////
	getcurrenttime(currenttime);
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	save_path = "d:\\IUPUI\\Test_Data\\";
#else
	save_path = "/home/odroid/Videos/test_recording_" + (string)currenttime + ".avi";
	//save_path = "/media/odroid/TOSHIBA EXT/Videos/test_recording_" + (string)currenttime + ".avi";
#endif

	file_base = (string)currenttime + "_" + recording_name + "_raw";
	video_save_file = (string)currenttime + "_" + recording_name + "_raw" + file_extension;
	focus_save_file = (string)currenttime + "_" + recording_name + "_focus" + file_extension;
	defocus_save_file = (string)currenttime + "_" + recording_name + "_defocus" + file_extension;
	config_save_file = (string)currenttime + "_" + recording_name + "_config.txt";
	configFile.open((save_path+config_save_file).c_str(), ios::out | ios::app);
	///////////////////////////////////////////////////////////////////////////


	error = configCameraPropeties(&cam, &sharpness, &shutter, &gain, &brightness, &auto_exp, framerate);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}

	cout << "Shutter Speed (ms): " << shutter << endl;
	cout << "Gain (dB): " << gain << endl;
	cout << "Sharpness: " << sharpness << endl;
	cout << "Brightness: " << brightness << endl;
	cout << "Auto Exposure: " << auto_exp << endl;
	cout << endl;

	// write camera configuration values to a file
	configFile << "Shutter Speed (ms): " << shutter << endl;
	configFile << "Gain (dB): " << gain << endl;
	configFile << "Sharpness: " << sharpness << endl;
	configFile << "Brightness: " << brightness << endl;
	configFile << "Auto Exposure: " << auto_exp << endl;
	configFile << endl;
	configFile.close();


	// begin the video capture
	cout << "Beginning Video Capture." << endl;

	// set the camera to software trigger mode
	setSoftwareTrigger(&cam, true);

	// begin the capture process
#ifdef IMG_CAP
	string dir_name = (string)currenttime + "_" + recording_name + "_raw";
	mkDir(save_path, dir_name);
	imageCapture(&cam, lensDriver, (save_path + dir_name + "\\" + file_base), framerate, framerate);

#else
	// videoCapture(&cam, lensDriver, video_save_file, framerate, framerate);
	videoCapture(&cam, lensDriver, save_path+focus_save_file, save_path+defocus_save_file, (unsigned int)framerate, framerate);

#endif



	// stop the capture process
	error = cam.StopCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	// clear the software trigger
	setSoftwareTrigger(&cam, false);

	// Disconnect the camera
	error = cam.Disconnect();

	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	// disconnect from lens driver
	FT_Close(lensDriver);

    cout << "Done! Press Enter to exit..." << endl; 
    cin.ignore();

    return 0;
}





// support functions
// void getcurrenttime(char currenttime[])
// {
	// time_t rawtime;
	// struct tm * timeinfo;
	
	// time(&rawtime);
	// timeinfo = localtime(&rawtime);

	// strftime(currenttime, 80, "%m%d%Y_%H%M%S", timeinfo);
	// string str(currenttime);
// }

/*
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
*/


/*
int videoCapture(Camera *cam, FT_HANDLE lensDriver, string save_file, unsigned int numCaptures)
{
	// timing variables

	//auto tick1 = chrono::high_resolution_clock::now();
	//auto tick2 = chrono::high_resolution_clock::now();
	double duration=0;



	unsigned int key = 0;
	float fps = 55.0;
	//unsigned int idx = 0;
	unsigned int image_rows, image_cols;
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
	double tickFreq = 1.0/getTickFrequency();
	int codec = CV_FOURCC('M', 'J', 'P', 'G');
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
	error = cam->StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
	
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
	error = cam->StopCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	return 0;

}	// end of videoCapture

*/

FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript)
{
	FT_HANDLE ftHandle = NULL;
	FT_HANDLE ftHandleTemp;
	FT_DEVICE_LIST_INFO_NODE devInfo[32];
	DWORD numDevices=0;
	int dev_number, found;
	DWORD Flags;
	DWORD ID;
	DWORD Type;
	DWORD LocId;
	DWORD iOldVID, iOldPID;
	char SerialNumber[16];
	char Description[64];
	LONG lComPortNumber;

	ftHandle = NULL;
	dev_number = 0;
	found = 0;

	// search for devices connected to the USB port
	FT_CreateDeviceInfoList(&numDevices);

	if (numDevices > 0)
	{
		//FT_GetVIDPID(&iOldVID, &iOldPID);
		//FT_STATUS status = FT_SetVIDPID (0x0403, 0x0A23);
		if (FT_GetDeviceInfoList(devInfo, &numDevices) == FT_OK)
		{
			while ((dev_number < (int)numDevices) && !found)
			{
				if (FT_GetDeviceInfoDetail(dev_number, &Flags, &Type, &ID, &LocId, SerialNumber, Description, &ftHandleTemp) == FT_OK)
				{

					if ((string)Description == descript)
					{
						device->devNumber = dev_number;
						device->type = Type;
						device->Description = (string)Description;
						device->serialNumber = (string)SerialNumber;
						found = 1;
					}

				}
				if (!found)
					dev_number++;
			}
		}
	}
	else
	{
		cout << "No devices found." << endl;
		return ftHandle;
	}

	if (found)
	{
		if (FT_OpenEx((void *)device->Description.c_str(), FT_OPEN_BY_DESCRIPTION, &ftHandle) == FT_OK)
		{

			if (FT_SetBaudRate(ftHandle, device->BaudRate) != FT_OK)
//			if (FT_SetBaudRate(ftHandle, 230400l) != FT_OK)
			//printf("ERROR: Baud rate not supported\n");

			FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
			FT_SetTimeouts(ftHandle, 5000, 100);
			if (FT_GetComPortNumber(ftHandle, &lComPortNumber) == FT_OK)
			{
				if (lComPortNumber == -1) // No COM port assigned }
					printf("No Comm Port assigned device found!\n");
				else
					printf("FTDI device found on COM:%d\n", lComPortNumber);
			}
		}
	}
	else
	{
		cout << "No valid FTDI device found!" << endl;
	}

	return ftHandle;
}	// end of OpenComPort


