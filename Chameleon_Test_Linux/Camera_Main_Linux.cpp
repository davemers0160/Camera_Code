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
#include <cstdlib>

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
#include "stdafx.h"
#include "include/FlyCapture2.h"
#include "Chameleon_Utilities.h"

// Lens Driver Includes
#include "Lens_Driver.h"

// GPS Includes
#include "GPS_Ctrl.h"

// FTDI Driver Includes
#include "ftd2xx.h"

// GPIO Control Includes
#include "GPIO_Ctrl.h"

// Threading includes
#include <pthread.h>

// DEFINES
#define STNDBY_PIN	31	/* Pin to monitor the standby/recording  status */

using namespace std;
using namespace FlyCapture2;
using namespace Lens_Driver;

// GLOBAL Variables
struct ftdiDeviceDetails //structure storage for FTDI device details
{
	int devNumber;
	unsigned long type;
	unsigned long BaudRate;
	string Description;
	string serialNumber;
};

volatile bool quit_GPS_Logging = true;

//double tickFreq = 1000.0 / getTickFrequency();

//void getcurrenttime(char currenttime[]);
//void logGPSData(GPS_Thread_Vars *GPS_Ctrl_Info);

int videoCaptureInt(Camera *cam, FT_HANDLE lensDriver, string focus_save_file, string defocus_save_file, unsigned int numCaptures, float fps);

int videoCapture(Camera *cam, FT_HANDLE lensDriver, string focus_save_file, string defocus_save_file, unsigned int numCaptures, float fps);
int imageCapture(Camera *cam, FT_HANDLE lensDriver, string file_base, unsigned int numCaptures, float fps);


FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript);

int main(int /*argc*/, char** /*argv*/)
{    
	// GPIO Specific Variables
	int export_status;
	int dir_status;
	int pin_value;
	
	// Camera Specific Variables
	FlyCapture2::Error error;
	BusManager busMgr;
    PGRGuid guid;
	Camera cam;
	FC2Config cameraConfig;
	CameraInfo camInfo;
	char camSerialNumber[16];
	unsigned int numCameras;
	unsigned int offsetX, offsetY, width, height;
	PixelFormat pixelFormat;
	float shutter, gain, brightness, auto_exp;
	int sharpness;
	float cam_framerate = 30.0;
	bool camera_on = true;

	
	//Lens_Driver Specific Variables
	FT_HANDLE lensDriver = NULL;
	ftdiDeviceDetails driverDeviceDetails;
	LensTxPacket LensTx(CON,0);
	LensRxPacket LensRx;
	LensDriverInfo LensInfo;
	unsigned char status;
	unsigned char data[] = {136};
	//data[0] = 135;
	LensTxPacket Focus(FAST_SET_VOLT, 1, &data[0]);
	unsigned char CheckCount = 0;	// counter to limit the number of check to see it the LensDriver is attached

	// GPS Specific Variables
	GPS_Thread_Vars GPS_Ctrl_Info;
	GPS_Ctrl_Info.GPS_Handle = NULL;
	pthread_t GPS_Thread;


	//FT_HANDLE gpsHandle = NULL;
	ftdiDeviceDetails gpsDeviceDetails;
	bool gps_connected = false;



	// File Operations Specific Variables
	string save_path;
	string file_base;
	string recording_name = "test_recording";
	string image_save_file;
	string video_save_file;
	string focus_save_file;
	string defocus_save_file;
	string config_save_file;	
	string GPS_save_file;
	string file_extension = ".avi";
	ofstream configFile;
	char currenttime[80];

	// Timing Specific Variables
	double start, stop;
	double delta;


	PrintBuildInfo();

	// configure pin for reading standby status
	export_status = exportPin(STNDBY_PIN);
	dir_status = setPinDirection(STNDBY_PIN, 1);


	cout << "Connecting to Lens Driver..." << endl;
	// check for lens driver
	while ((lensDriver == NULL) && (CheckCount<10))
	{
		driverDeviceDetails.devNumber = 0;
		driverDeviceDetails.type = 0;
		driverDeviceDetails.BaudRate = 250000;
		driverDeviceDetails.Description = "";
		driverDeviceDetails.serialNumber = "";

		lensDriver = OpenComPort(&driverDeviceDetails, "Microfluidic Lens Driver");

		CheckCount++;
	}
	CheckCount = 0;
	if(lensDriver == NULL)
	{
		cout << "No Lens Driver found... exiting!" << endl;
		return -1;
	}


	cout << "Connecting to GPS Module..." << endl;
	while ((GPS_Ctrl_Info.GPS_Handle == NULL) && (CheckCount<10))
	{
		gpsDeviceDetails.devNumber = 0;
		gpsDeviceDetails.type = 0;
		gpsDeviceDetails.BaudRate = 57600;
		gpsDeviceDetails.Description = "";
		gpsDeviceDetails.serialNumber = "";

		GPS_Ctrl_Info.GPS_Handle = OpenComPort(&gpsDeviceDetails, "LOCOSYS GPS MC-1513");

		CheckCount++;

	}

	if(GPS_Ctrl_Info.GPS_Handle == NULL)
	{
		gps_connected = false;
		cout << "No GPS was found!" << endl << endl;
	}
	else
	{
		gps_connected = true;
		// clear FTDI buffer
		FT_Purge(GPS_Ctrl_Info.GPS_Handle, FT_PURGE_RX | FT_PURGE_TX);
		
		// configure the GPS to GGA messages only
		configGPS(GPS_Ctrl_Info.GPS_Handle);
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

	// get the number of cameras attached
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
		return 1;
    }

    cout << "Number of cameras detected: " << numCameras << endl; 
    
    // assume that we want to connect to the first camera and get the GUID for that camera
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

	// get the infor from the camera
	error = cam.GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	PrintCameraInfo(&camInfo);
	sprintf(camSerialNumber,"%u",camInfo.serialNumber);


	// Get the camera configuration
	error = cam.GetConfiguration(&cameraConfig);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}

	cameraConfig.grabTimeout = 500;// (unsigned int)(1000 / framerate);
	cameraConfig.highPerformanceRetrieveBuffer = true;
	cameraConfig.asyncBusSpeed = BUSSPEED_ANY;

	// Set the camera configuration
	error = cam.SetConfiguration(&cameraConfig);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	// set lens to initial focus
	sendLensPacket(Focus, lensDriver);

	// configure the image size and the pixel format for the video
	// 1.216 MB/s
	offsetX = 40;		// 40
	width = 1200;		// 1200;
	
	offsetY = 228;		// 228;
	height = 720;		// 724;

	cout << "Configuring Camera!" << endl;

	//pixelFormat = PIXEL_FORMAT_422YUV8;
	//pixelFormat = PIXEL_FORMAT_444YUV8;
	pixelFormat = PIXEL_FORMAT_RGB8;
	error = configImagerFormat(&cam, offsetX, offsetY, width, height, pixelFormat);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	//main loop to do the recording
	while(1)
	{

		pin_value = readPin(STNDBY_PIN);
		cout << "Pin Value: " << pin_value << endl;

		// standby
		if(pin_value == 0)
		{
			cout << "Standby Mode..." << endl;
			if(camera_on == true)
			{
				error = Camera_PowerOff(&cam);
				if (error != PGRERROR_OK)
				{
					PrintError(error);
				}
				camera_on = false;
				cout << "Turning Camera Off.." << endl;
			}
			delta = getTickFrequency();

			start = (double)getTickCount();
			do
			{
				stop = (double)getTickCount();
			} while ((stop - start) < delta);
		}
		else
		{
			if(camera_on == false)
			{
				cout << "Turning Camera On.." << endl;
				error = Camera_PowerOn(&cam);
				if (error != PGRERROR_OK)
				{
					PrintError(error);
				}
				else
				{
					camera_on = true;
					cout << "Camera On!" << endl;
				}

				// wait for a little bit to allow the camera to turn on
				delta = 2*getTickFrequency();

				start = (double)getTickCount();
				do
				{
					stop = (double)getTickCount();
				} while ((stop - start) < delta);
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
			//save_path = "/home/odroid/Videos/";
			save_path = "/media/odroid/DATA_DRIVE/Data/";
		#endif

			file_base = (string)currenttime + "_" + recording_name + "_" + (string)camSerialNumber;
			//video_save_file = (string)currenttime + "_" + recording_name + "_raw" + file_extension;
			//focus_save_file = (string)currenttime + "_" + recording_name + "_focus" + file_extension;
			//defocus_save_file = (string)currenttime + "_" + recording_name + "_defocus" + file_extension;
			focus_save_file = file_base + "_focus" + file_extension;
			defocus_save_file = file_base + "_defocus" + file_extension;

			//config_save_file = (string)currenttime + "_" + recording_name + "_config.txt";
			//GPS_save_file = (string)currenttime + "_" + recording_name + "_GPS_Data.txt";
			config_save_file = file_base + "_config.txt";
			GPS_save_file = file_base + "_GPS_Data.txt";
			configFile.open((save_path+config_save_file).c_str(), ios::out | ios::app);
			///////////////////////////////////////////////////////////////////////////

			if(gps_connected == true)
			{
				quit_GPS_Logging = false;
				cout << GPS_save_file << endl;
				GPS_Ctrl_Info.gpsDataLog.open((save_path+GPS_save_file).c_str(), ios::out | ios::app);
				
				// open up a new thread and begin capturing the GPS data
				// the new thread allows the code to continue and capture video images while 
				// still recording the GPS data
				// ???????
				//int create1 = pthread_create(&GPS_Thread, NULL, logGPSData, reinterpret_cast<void*>(&GPS_Ctrl_Info) );
				int create1 = pthread_create(&GPS_Thread, NULL, logGPSData, (void*)(&GPS_Ctrl_Info) );
				//logGPSData(&GPS_Ctrl_Info);
				if(create1 != 0)
				{
					cout << "Error Creating GPS Thread." << endl;
				}

			}

			// configure the auto properties for the sharpness, shutter, gain, brightness, auto_exp
			error = configCameraPropeties(&cam, &sharpness, &shutter, &gain, &brightness, &auto_exp, 2*cam_framerate);
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

			videoCaptureInt(&cam, lensDriver, save_path+focus_save_file, save_path+defocus_save_file, (unsigned int)(cam_framerate*15), cam_framerate);

			// test of imu interface
			/*
			delta = 20*getTickFrequency();

			start = (double)getTickCount();
			do
			{
				stop = (double)getTickCount();
			} while ((stop - start) < delta);
			*/
		#endif

			quit_GPS_Logging = true;
			// stop GPS Thread
			if(gps_connected == true)
			{
				pthread_join(GPS_Thread, NULL);

				GPS_Ctrl_Info.gpsDataLog.close();
			}
			// stop the capture process
			error = cam.StopCapture();
			if (error != PGRERROR_OK)
			{
				PrintError(error);
				return -1;
			}

			// clear the software trigger
			setSoftwareTrigger(&cam, false);




		}	// end of if statement <- stanby/record


	}	// end of while(1)

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

}	// end of main





// support functions

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
	//DWORD iOldVID, iOldPID;
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





