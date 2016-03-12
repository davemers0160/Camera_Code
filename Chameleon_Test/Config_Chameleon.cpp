/*
Chameleon 3 Camera configuration file

This file contains the configures the routines for the Chameleon 3 camera. 

*/
#include <iostream>
//#include <sstream>
//#include <string>
//#include <iomanip>

#include "FlyCapture2.h"
#include "Config_Chameleon.h"

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
		//return error;
	}

	error = cam->GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		//return error;
	}

	PrintCameraInfo(&camInfo);

	//return cam;

}	// end of cameraConnect


void configImagerFormat(Camera *cam, unsigned int offsetX, unsigned int offsetY, unsigned int width, unsigned int height, PixelFormat pixelFormat)
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
        PrintError( error );
        return;
    }

	if (!validSettings)
    {
        // Settings are not valid
		cout << "Format7 settings are not valid" << endl; 
        return;
    }

    // Set the settings to the camera
	error = cam->SetFormat7Configuration(&CameraSettings, PacketInfo.recommendedBytesPerPacket);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return;
    }	
	

}	// end of configCam

void configProperty(Property &prop, PropertyType type, bool mode, bool OnOff)
{

	//Define the property to adjust.
	prop.type = type;
	//Ensure the property is on.
	if (OnOff == true)
	{
		prop.onOff = true;
	}

	//Ensure auto-adjust mode is off.
	prop.autoManualMode = mode;
	//Ensure the property is set up to use absolute value control.
	prop.absControl = true;
	//Set the absolute value of shutter to 20 ms.

}	// end of configProperty

FlyCapture2::Error setProperty(Camera *cam, Property &prop, float value)
{
	FlyCapture2::Error error;

	prop.absValue = value;
	error = cam->SetProperty(&prop);

	return error;
}	// end of setProperty

