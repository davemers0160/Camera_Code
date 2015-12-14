//=============================================================================
// Copyright © 2008 Point Grey Research, Inc. All Rights Reserved.
//
// This software is the confidential and proprietary information of Point
// Grey Research, Inc. ("Confidential Information").  You shall not
// disclose such Confidential Information and shall use it only in
// accordance with the terms of the license agreement you entered into
// with PGR.
//
// PGR MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
// SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE, OR NON-INFRINGEMENT. PGR SHALL NOT BE LIABLE FOR ANY DAMAGES
// SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
// THIS SOFTWARE OR ITS DERIVATIVES.
//=============================================================================
//=============================================================================
// $Id: FlyCapture2Test.cpp,v 1.19 2010-03-11 22:58:37 soowei Exp $
//=============================================================================

#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <windows.h> 

// OPENCV includes
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  

// Point Grey Includes
#include "stdafx.h"
#include "include/FlyCapture2.h"

// Varioptic Lens Control
#include "varioptic_class.h"

using namespace std;
using namespace cv;
using namespace FlyCapture2;


void getcurrenttime(char currenttime[]);
bool configLensDriver(LPCWSTR port, HANDLE serialHandle);

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

void PrintCameraInfo( CameraInfo* pCamInfo )
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

void PrintError( Error error )
{
    error.PrintErrorTrace();
}

int RunSingleCamera(PGRGuid guid, HANDLE serialHandle)
{
    //const int k_numImages = 10;

    Error error;
    Camera cam;
	char* Window1 = "Video Display";
	char key = 0;
	double fps = 30.0;
	int codec = CV_FOURCC('M', 'J', 'P', 'G');
	unsigned int image_rows, image_cols, rowBytes;
	Size image_size; 
	Mat video_frame;
	VideoWriter outputVideo;
	int delay = 1;
	int Casp_retVal = 0;
	unsigned int idx=0;
	//string port;// = "\\\\.\\COM1";
	double voltage[2] = { 40.0, 41.0 };
	varioptic_class Casp_Lens;
	BOOL comm_result;
	
	unsigned long dwBytesWritten;// , dwRead;
	//HANDLE serialHandle;
	//DCB serialParams = { 0 };
	//char rx_data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	//OVERLAPPED osReader = { 0 };
	double tick;
	char currenttime[80];

	getcurrenttime(currenttime);

	string save_file = "test_recording_" + (string)currenttime + ".avi";

	//serialParams.DCBlength = sizeof(serialParams);

	namedWindow(Window1, WINDOW_NORMAL);

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    PrintCameraInfo(&camInfo);        

    // Start capturing images
    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    Image rawImage;         
	Image convertedImageCV;   
	unsigned char *image_data = NULL;

	error = cam.RetrieveBuffer(&rawImage);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		key = 'q';
	}
	else
	{
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		image_cols = convertedImageCV.GetCols();
		image_rows = convertedImageCV.GetRows();
		rowBytes = (double)convertedImageCV.GetDataSize() / (double)convertedImageCV.GetRows();
		image_size = Size((int)image_cols, (int)image_rows);
		outputVideo.open(save_file, codec, fps, image_size, true);
		cout << "video size: " << image_cols << " x " << image_rows << endl;
		
	//	// open up serial port and set paramters
	//	serialHandle = CreateFileW(lpFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	//	if (!GetCommState(serialHandle, &serialParams))
	//		cout << "Error getting information from the specified serial port" << endl;

	//	//GetCommState(serialHandle, &serialParams);
	//	serialParams.BaudRate = 57600;
	//	serialParams.ByteSize = 8;
	//	serialParams.StopBits = ONESTOPBIT;
	//	serialParams.Parity = NOPARITY;

	//	comm_result = SetCommState(serialHandle, &serialParams);
	//	if (!comm_result)
	//	{
	//		cout << "Error setting serial port configuration" << endl;
	//		return EXIT_FAILURE;
	//	}

	//	// Set timeouts
	//	COMMTIMEOUTS timeout = { 0 };
	//	timeout.ReadIntervalTimeout = 50;
	//	timeout.ReadTotalTimeoutConstant = 50;
	//	timeout.ReadTotalTimeoutMultiplier = 50;
	//	timeout.WriteTotalTimeoutConstant = 50;
	//	timeout.WriteTotalTimeoutMultiplier = 10;

	//	comm_result = SetCommTimeouts(serialHandle, &timeout);
	//	if (!comm_result)
	//	{
	//		cout << "Error setting serial port timeout parameters" << endl;
	//		return EXIT_FAILURE;
	//	}

	}
	
	idx = 0;
	//for ( int imageCnt=0; imageCnt < k_numImages; imageCnt++ )
	while (key != 'q')
    {                
		tick = (double)getTickCount();		// start timing process 
		Casp_Lens.voltage(voltage[(idx & 0x01)]);
		WriteFile(serialHandle, Casp_Lens.Packet, Casp_Lens.packet_length + 1, &dwBytesWritten, NULL);
		// clear tx and rx serial buffers
		//PurgeComm(serialHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT);

		// Retrieve an image
        error = cam.RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            continue;
        }

        //cout << "Grabbed image " << imageCnt << endl; 

        // Create a converted image


        // Convert the raw image	PIXEL_FORMAT_BGR for opencv
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}


		// display images

		image_data = convertedImageCV.GetData();		
		video_frame = Mat(image_size, CV_8UC3, image_data, rowBytes);
		
		//ReadFile(serialHandle, rx_data, sizeof(rx_data), &dwRead, &osReader);	

		//imshow(Window1, video_frame);
		outputVideo.write(video_frame);
		key = waitKey(delay);

		idx++;
		tick = (double)getTickCount() - tick;
		cout << "Execution Time: " << fixed << setw(5) << setprecision(0) << (tick*1000. / getTickFrequency()) << "ms" << endl;


		/*
        // Create a unique filename
		ostringstream filename;
		filename << "FlyCapture2Test-" << camInfo.serialNumber << "-" << imageCnt << ".tif";

        // Save the image. If a file format is not passed in, then the file
        // extension is parsed to attempt to determine the file format.
        error = convertedImage.Save( filename.str().c_str() );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }  
		*/
    }            
	outputVideo.release();
	destroyAllWindows();
	cout << "Finished Writing Video!" << endl;

	CloseHandle(serialHandle);

    // Stop capturing images
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }      

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    return 0;
}

int main(int /*argc*/, char** /*argv*/)
{    
	
	// Camera specific variables
	Error error;
	BusManager busMgr;
    PGRGuid guid;
	
	// Serial Port specific variables
	LPCWSTR commPort = L"\\\\.\\COM5";

	unsigned int numCameras;


    PrintBuildInfo();
    

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
	/*
	FILE* tempFile = fopen("test.txt", "w+");
	if (tempFile == NULL)
	{
		cout << "Failed to create file in current folder.  Please check permissions." << endl; 
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");
	*/


    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
		return EXIT_FAILURE;
    }

    cout << "Number of cameras detected: " << numCameras << endl; 

    //for (unsigned int i=0; i < numCameras; i++)
    //{
    
    error = busMgr.GetCameraFromIndex(1, &guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return EXIT_FAILURE;
    }

    RunSingleCamera( guid );
    //}

    cout << "Done! Press Enter to exit..." << endl; 
    cin.ignore();

    return EXIT_SUCCESS;
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

bool configLensDriver(LPCWSTR commPort, HANDLE serialHandle)
{
	bool status = false;
	//HANDLE serialHandle;
	DCB serialParams = { 0 };

	serialParams.DCBlength = sizeof(serialParams);

	// open up serial port and set paramters
	serialHandle = CreateFileW(commPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (!GetCommState(serialHandle, &serialParams))
		cout << "Error getting information from the specified serial port" << endl;

	//GetCommState(serialHandle, &serialParams);
	serialParams.BaudRate = 57600;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
	serialParams.Parity = NOPARITY;

	status = SetCommState(serialHandle, &serialParams);
	if (!status)
	{
		cout << "Error setting serial port configuration" << endl;
		return status;
	}

	// Set timeouts
	COMMTIMEOUTS timeout = { 0 };
	timeout.ReadIntervalTimeout = 50;
	timeout.ReadTotalTimeoutConstant = 50;
	timeout.ReadTotalTimeoutMultiplier = 50;
	timeout.WriteTotalTimeoutConstant = 50;
	timeout.WriteTotalTimeoutMultiplier = 10;

	status = SetCommTimeouts(serialHandle, &timeout);
	if (!status)
	{
		cout << "Error setting serial port timeout parameters" << endl;
		return status;
	}

	return status;

}	// end of configLensDriver

Error cameraConnect(PGRGuid guid, Camera cam)
{


}	// end of cameraConnect


Error videoCapture(Camera cam, HANDLE serialHandle)
{


}	// end of videoCapture