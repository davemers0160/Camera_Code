/*
Chameleon 3 Camera configuration file

This file contains the configures the routines for the Chameleon 3 camera. 

*/
#include <iostream>
#include <ctime>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	#include <Windows.h>
	#include "FlyCapture2.h"
#else
	#include <unistd.h>
	#include <time.h>
	//#include <linux/types.h>
	#include <sys/stat.h>
	#include "../Chameleon_Test_Linux/include/FlyCapture2.h"
#endif


#include "Chameleon_Utilities.h"

using namespace FlyCapture2;
using namespace std;

void getcurrenttime(char currenttime[])
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(currenttime, 80, "%Y%m%d_%H%M%S", timeinfo);
	string str(currenttime);

}	// end of getcurrenttime

void PrintError(FlyCapture2::Error error)
{
	error.PrintErrorTrace();
}

void PrintCameraInfo(CameraInfo *pCamInfo)
{
	cout << endl;
	cout << "*** CAMERA INFORMATION ***" << endl;
	cout << "Serial number - " << pCamInfo->serialNumber << endl;
	cout << "Camera model - " << pCamInfo->modelName << endl;
	cout << "Camera vendor - " << pCamInfo->vendorName << endl;
	cout << "Sensor - " << pCamInfo->sensorInfo << endl;
	cout << "Resolution - " << pCamInfo->sensorResolution << endl;
	cout << "Bayer Tile Format - " << pCamInfo->bayerTileFormat << endl;
	cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
	cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;

}	// end of PrintCameraInfo

void PrintBuildInfo(void)
{
	FC2Version fc2Version;
	Utilities::GetLibraryVersion(&fc2Version);

	ostringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	cout << version.str() << endl;

	ostringstream timeStamp;
	timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
	cout << timeStamp.str() << endl << endl;
}	// end of PrintBuildInfo

void cameraConnect(PGRGuid guid, Camera *cam)
{
	FlyCapture2::Error error;
	//CameraInfo camInfo;

	error = cam->Connect(&guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	//error = cam->GetCameraInfo(&camInfo);
	//if (error != PGRERROR_OK)
	//{
	//	PrintError(error);
	//}

	//PrintCameraInfo(&camInfo);

	//return cam;

}	// end of cameraConnect


FlyCapture2::Error configImagerFormat(Camera *cam, unsigned int offsetX, unsigned int offsetY, unsigned int width, unsigned int height, PixelFormat pixelFormat)
{
	Format7ImageSettings CameraSettings;
	Format7PacketInfo PacketInfo;
	FlyCapture2::Error error;
	bool validSettings;
	
	CameraSettings.mode = (Mode)0;
	CameraSettings.offsetX = offsetX;
	CameraSettings.offsetY = offsetY;
	CameraSettings.width = width;
	CameraSettings.height = height;
	CameraSettings.pixelFormat = pixelFormat;
		
    // Validate the settings to make sure that they are valid
	error = cam->ValidateFormat7Settings(&CameraSettings, &validSettings, &PacketInfo);
    if (error != PGRERROR_OK)
    {
        //PrintError( error );
        return error;
    }

	if (!validSettings)
    {
        // Settings are not valid
		cout << "Format7 settings are not valid" << endl; 
		return error;
    }

    // Set the settings to the camera
	error = cam->SetFormat7Configuration(&CameraSettings, PacketInfo.recommendedBytesPerPacket);
    if (error != PGRERROR_OK)
    {
        //PrintError( error );
		return error;
    }	
	
	return error;

}	// end of configCam

int getProperty(Camera *cam, Property &prop)
{
	int value = 0;

	cam->GetProperty(&prop);
	value = prop.valueA;

	return value;
}	// end of getProperty


float getABSProperty(Camera *cam, Property &prop)
{
	float value = 0;

	cam->GetProperty(&prop);
	value = prop.absValue;

	return value;
}	// end of getABSProperty


void configProperty(Camera *cam, Property &prop, PropertyType type, bool AutoMode, bool OnOff, bool absControl)
{
	//Define the property to adjust.
	prop.type = type;

	// Configure Property: True=>On, False=>Off
	prop.onOff = OnOff;
	
	// Configure auto adjust mode: True=>On, False=>Off 
	prop.autoManualMode = AutoMode;

	//Ensure the property is set up to use absolute value control.
	prop.absControl = absControl;

	FlyCapture2::Error error = setProperty(cam, prop);

}	// end of configProperty

FlyCapture2::Error setProperty(Camera *cam, Property &prop, float value)
{
	FlyCapture2::Error error;

	prop.absValue = value;
	error = cam->SetProperty(&prop);

	return error;
}	// end of setProperty

FlyCapture2::Error setProperty(Camera *cam, Property &prop)
{
	FlyCapture2::Error error = cam->SetProperty(&prop);

	return error;
}	// end of setProperty


FlyCapture2::Error configCameraPropeties(Camera *cam, int *sharpness, float *shutter, float *gain, float *brightness, float *auto_exp, float fps)
{
	FlyCapture2::Error error;

	Property Shutter, Gain, Sharpness, Framerate, Brightness, Auto_Exposure;
	
	*sharpness = 1200;
	*shutter = 16.0;
	*gain = 10.0;
	*auto_exp = 1.0;
	*brightness = 2.0;

	// set the frame rate for the camera
	configProperty(cam, Framerate, FRAME_RATE, false, true, true);
	error = setProperty(cam, Framerate, fps);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	sleep_ms(500);

	// config Shutter to initial value and set to auto
	configProperty(cam, Shutter, SHUTTER, true, true, true);
	error = setProperty(cam, Shutter, *shutter);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	// config Gain to initial value and set to auto
	configProperty(cam, Gain, GAIN, true, true, true);
	error = setProperty(cam, Gain, *gain);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	// config Sharpness to initial value and set to auto
	configProperty(cam, Sharpness, SHARPNESS, true, true, false);
	error = setProperty(cam, Sharpness, (float)*sharpness);
	if (error != PGRERROR_OK)
	{
		return error;
	}	

	// configure the auto-exposure property
	configProperty(cam, Auto_Exposure, AUTO_EXPOSURE, true, true, true);
	error = setProperty(cam, Auto_Exposure, *auto_exp);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	// configure the brightness property
	configProperty(cam, Brightness, BRIGHTNESS, true, true, true);
	error = setProperty(cam, Brightness, *brightness);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	sleep_ms(500);

	// get the auto values
	*shutter = getABSProperty(cam, Shutter);
	*gain = getABSProperty(cam, Gain);
	*sharpness = getProperty(cam, Sharpness);
	*auto_exp = getABSProperty(cam, Auto_Exposure);
	*brightness = getABSProperty(cam, Brightness);

	//sleep_ms(500);

	// get the auto values
	//*shutter = getABSProperty(cam, Shutter);
	//*gain = getABSProperty(cam, Gain);
	//*sharpness = getProperty(cam, Sharpness);
	//*brightness = getABSProperty(cam, Brightness);
	//*auto_exp = getABSProperty(cam, Auto_Exposure);
	//int temp = getProperty(cam, Brightness);


	// set the auto values to fixed
	// configProperty(cam, Shutter, SHUTTER, false, false, true);
	// error = setProperty(cam, Shutter, *shutter);
	// configProperty(cam, Gain, GAIN, false, false, true);
	// error = setProperty(cam, Gain, *gain);
	// configProperty(cam, Sharpness, SHARPNESS, false, false, false);
	// error = setProperty(cam, Sharpness, (float)*sharpness);
	// configProperty(cam, Auto_Exposure, AUTO_EXPOSURE, false, false, true);
	// error = setProperty(cam, Auto_Exposure, *auto_exp);

	return error;

}	// end ofconfigCameraPropeties


FlyCapture2::Error Camera_PowerOff(Camera *cam)
{
	FlyCapture2::Error error; 
	//const unsigned int powerReg = 0x610;
	//unsigned int powerRegVal = 0;

	//powerRegVal = (on == true) ? 0x80000000 : 0x0;

	error = cam->WriteRegister(0x610, 0x00);

	return error;
}	// end of Camera_PowerOff


FlyCapture2::Error Camera_PowerOn(Camera *cam)
{
	FlyCapture2::Error error;


	unsigned int regVal = 0;
	unsigned int retries = 10;

	error = cam->WriteRegister(0x610, 0x80000000);
	if (error != PGRERROR_OK)
	{
		//PrintError(error);
		return error;
	}

	// Wait for camera to complete power-up
	do
	{
		sleep_ms(200);

		error = cam->ReadRegister(0x610, &regVal);
		if (error == PGRERROR_TIMEOUT)
		{
			// ignore timeout errors, camera may not be responding to
			// register reads during power-up
		}
		else if (error != PGRERROR_OK)
		{
			//PrintError(error);
			return error;
		}

		retries--;
	} while ((regVal & 0x80000000) == 0 && retries > 0);

	return error;
}	// end of Camera_PowerOn

FlyCapture2::Error setSoftwareTrigger(Camera *cam, bool onOff)
{
	FlyCapture2::Error error;
	TriggerMode triggerMode;

	// Get current trigger settings
	error = cam->GetTriggerMode(&triggerMode);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	// Set camera to trigger mode 0
	triggerMode.onOff = onOff;
	triggerMode.mode = 0;
	triggerMode.parameter = 0;
	triggerMode.source = 7;


	error = cam->SetTriggerMode(&triggerMode);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	return error;
}	// SetSoftwareTrigger


bool PollForTriggerReady(Camera *cam)
{
	const unsigned int k_softwareTrigger = 0x62C;
	FlyCapture2::Error error;
	unsigned int regVal = 0;

	do
	{
		error = cam->ReadRegister(k_softwareTrigger, &regVal);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return false;
		}

	} while ((regVal >> 31) != 0);

	return true;
}	// end of PollForTriggerReady

bool FireSoftwareTrigger(Camera *cam)
{
	const unsigned int k_softwareTrigger = 0x62C;
	const unsigned int k_fireVal = 0x80000000;
	FlyCapture2::Error error;

	error = cam->WriteRegister(k_softwareTrigger, k_fireVal);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return false;
	}

	return true;
}	// end of FireSoftwareTrigger

void *saveVideo_t(void *args )
{
	int idx;
	videoSaveStruct *videoSaveParam = (videoSaveStruct *)(args);


	//for(idx=0; idx<videoSaveParam->FrameCount ; idx++)
	//{
		videoSaveParam->VideoFile.write(videoSaveParam->VideoFrame);
	//}

	pthread_exit(NULL);

}	// end of saveVideo

void *saveBinVideo_t(void *args )
{
	videoSaveStruct *videoSaveParam = (videoSaveStruct *)(args);

	int idx=0;
	int frameSize;
	ofstream saveFile;

	saveFile.open(videoSaveParam->FileName.c_str(), ios::out | ios::binary);

	frameSize = 3 * videoSaveParam->VideoFrame.rows * videoSaveParam->VideoFrame.cols;

	//videoSaveParam->VideoFrame[idx].data;

	for(idx=0; idx<videoSaveParam->FrameCount ; idx++)
	{
		//const char *data = (char *)videoSaveParam->VideoFrame[idx].data;
		saveFile.write((char *)videoSaveParam->VideoFrame.data,frameSize);
	}


	saveFile.close();

	pthread_exit(NULL);


}	// end of saveBinVideo_t

// create a sleep function that can be used in both Windows and Linux
void sleep_ms(int value)
{

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	Sleep(value);
#else
	const timespec delay[]= {0, value*1000000L} ;
	//delay->tv_sec = 0;
	//delay->tv_nsec = value*1000000L;
	nanosleep(delay, NULL);
	//nanosleep((const struct timespec[]){ {0, value*1000000L} }, NULL);
#endif

}

void mkDir(string directory_path, string new_folder)
{

	string temp_path = directory_path + "\\" + new_folder;

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)

	wstring full_path = wstring(temp_path.begin(), temp_path.end());
	boolean status = CreateDirectoryW(full_path.c_str(), NULL);

#else

	mode_t mode = 0766;
	int status = mkdir(temp_path.c_str(), mode);

#endif

	//return (int)status;

}	// end of makeDir

