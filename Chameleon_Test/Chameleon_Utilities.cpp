/*
Chameleon 3 Camera configuration file

This file contains the configures the routines for the Chameleon 3 camera. 

*/
#include <iostream>
#include <ctime>
//#include <sstream>
//#include <string>
//#include <iomanip>

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
	#include <Windows.h>
#else
	#include <unistd.h>
#endif


#include "FlyCapture2.h"
#include "Chameleon_Utilities.h"

using namespace FlyCapture2;
using namespace std;


void PrintError(FlyCapture2::Error error)
{
	error.PrintErrorTrace();
}

void PrintCameraInfo(CameraInfo* pCamInfo)
{
	cout << endl;
	cout << "*** CAMERA INFORMATION ***" << endl;
	cout << "Serial number -" << pCamInfo->serialNumber << endl;
	cout << "Camera model - " << pCamInfo->modelName << endl;
	cout << "Camera vendor - " << pCamInfo->vendorName << endl;
	cout << "Sensor - " << pCamInfo->sensorInfo << endl;
	cout << "Resolution - " << pCamInfo->sensorResolution << endl;
	cout << "Bayer Tile Format - " << pCamInfo->bayerTileFormat << endl;
	cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
	cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl << endl;

}

void cameraConnect(PGRGuid guid, Camera *cam)
{
	FlyCapture2::Error error;
	//	Camera cam;
	CameraInfo camInfo;

	error = cam->Connect(&guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	error = cam->GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	PrintCameraInfo(&camInfo);

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
	*shutter = 33.0;
	*gain = 10.0;
	*auto_exp = 1.0;

	// set the frame rate for the camera
	configProperty(cam, Framerate, FRAME_RATE, false, true, true);
	error = setProperty(cam, Framerate, fps);
	if (error != PGRERROR_OK)
	{
		return error;
	}

	sleep_ms(200);

//#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
//	Sleep(200);
//#else
//	nanosleep((const struct timespec[]){ {0, 200000000L} }, NULL);
//#endif

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
	error = setProperty(cam, Sharpness, *sharpness);
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
	//configProperty(cam, Brightness, BRIGHTNESS, true, true, true);
	//error = setProperty(cam, Brightness, *brightness);
	//if (error != PGRERROR_OK)
	//{
	//	return error;
	//}

	// get the auto values
	*shutter = getABSProperty(cam, Shutter);
	*gain = getABSProperty(cam, Gain);
	*sharpness = getProperty(cam, Sharpness);
	*auto_exp = getABSProperty(cam, Auto_Exposure);


	sleep_ms(200);

//#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
//	Sleep(200);
//#else
//	nanosleep((const struct timespec[]){{0, 200000000L}}, NULL);
//#endif

	// get the auto values
	*shutter = getABSProperty(cam, Shutter);
	*gain = getABSProperty(cam, Gain);
	*sharpness = getProperty(cam, Sharpness);
	*brightness = getABSProperty(cam, Brightness);
	*auto_exp = getABSProperty(cam, Auto_Exposure);
	int temp = getProperty(cam, Brightness);


	// set the auto values to fixed
	configProperty(cam, Shutter, SHUTTER, false, false, true);
	error = setProperty(cam, Shutter, *shutter);
	configProperty(cam, Gain, GAIN, false, false, true);
	error = setProperty(cam, Gain, *gain);
	configProperty(cam, Sharpness, SHARPNESS, false, false, false);
	error = setProperty(cam, Sharpness, *sharpness);
	configProperty(cam, Auto_Exposure, AUTO_EXPOSURE, false, false, true);
	error = setProperty(cam, Auto_Exposure, *auto_exp);

	return error;

}	// end ofconfigCameraPropeties


FlyCapture2::Error SetCameraPower(Camera *cam, bool on)
{
	FlyCapture2::Error error; 
	const unsigned int powerReg = 0x610;
	unsigned int powerRegVal = 0;

	powerRegVal = (on == true) ? 0x80000000 : 0x0;

	error = cam->WriteRegister(powerReg, powerRegVal);

	return error;
}

// create a sleep function that can be used in both Windows and Linux
void sleep_ms(int value)
{

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
	Sleep(value);
#else
	nanosleep((const struct timespec[]){ {0, value*1000000L} }, NULL);
#endif

}