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

int RunSingleCamera( PGRGuid guid )
{
    const int k_numImages = 10;

    Error error;
    Camera cam;
	char* Window1 = "Video Display";
	char key = 0;
	double fps = 60.0;
	int codec = CV_FOURCC('M', 'J', 'P', 'G');
	unsigned int image_rows, image_cols, rowBytes;
	Size image_size; 
	Mat video_frame;
	VideoWriter outputVideo;
	string save_file = "test_recording.avi";
	int delay = (int)(1);
	int Casp_retVal = 0;
	unsigned int idx=0;
	string port;// = "\\\\.\\COM1";
	double voltage[2] = { 42.0, 44.0 };
	varioptic_class Casp_Lens;
	BOOL comm_result;
	LPCWSTR lpFileName = L"\\\\.\\COM5";
	unsigned long dwBytesWritten, dwRead;
	HANDLE serialHandle;
	DCB serialParams = { 0 };
	char rx_data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	OVERLAPPED osReader = { 0 };


	serialParams.DCBlength = sizeof(serialParams);

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
		
		// open up serial port and set paramters
		serialHandle = CreateFileW(lpFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		if (!GetCommState(serialHandle, &serialParams))
			cout << "Error getting information from the specified serial port" << endl;

		//GetCommState(serialHandle, &serialParams);
		serialParams.BaudRate = 57600;
		serialParams.ByteSize = 8;
		serialParams.StopBits = ONESTOPBIT;
		serialParams.Parity = NOPARITY;

		comm_result = SetCommState(serialHandle, &serialParams);
		if (!comm_result)
		{
			cout << "Error setting serial port configuration" << endl;
			return EXIT_FAILURE;
		}

		// Set timeouts
		COMMTIMEOUTS timeout = { 0 };
		timeout.ReadIntervalTimeout = 50;
		timeout.ReadTotalTimeoutConstant = 50;
		timeout.ReadTotalTimeoutMultiplier = 50;
		timeout.WriteTotalTimeoutConstant = 50;
		timeout.WriteTotalTimeoutMultiplier = 10;

		comm_result = SetCommTimeouts(serialHandle, &timeout);
		if (!comm_result)
		{
			cout << "Error setting serial port timeout parameters" << endl;
			return EXIT_FAILURE;
		}


	}
	
	idx = 0;
	//for ( int imageCnt=0; imageCnt < k_numImages; imageCnt++ )
	while (key != 'q')
    {                

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


        // Convert the raw image	PIXEL_FORMAT_RGB
        //error = rawImage.Convert( PIXEL_FORMAT_MONO8, &convertedImage );
		/*
		error = rawImage.Convert(PIXEL_FORMAT_RGB, &convertedImage);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        } 
		*/
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}


		// display images
		//char key = 0;



		image_data = convertedImageCV.GetData();		
		//Mat image = Mat();
		//Mat image = cv::Mat(convertedImageCV.GetRows(), convertedImageCV.GetCols(), CV_8UC3, image_data, rowBytes);
		video_frame = Mat(image_size, CV_8UC3, image_data, rowBytes);
		
		
		imshow(Window1, video_frame);
		outputVideo.write(video_frame);



		key = waitKey(delay);
		ReadFile(serialHandle, rx_data, sizeof(rx_data), &dwRead, &osReader);
		idx++;
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
    PrintBuildInfo();

    Error error;

    // Since this application saves images in the current folder
    // we must ensure that we have permission to write to this folder.
    // If we do not have permission, fail right away.
	FILE* tempFile = fopen("test.txt", "w+");
	if (tempFile == NULL)
	{
		cout << "Failed to create file in current folder.  Please check permissions." << endl; 
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");

    BusManager busMgr;
    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    cout << "Number of cameras detected: " << numCameras << endl; 

    for (unsigned int i=0; i < numCameras; i++)
    {
        PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        RunSingleCamera( guid );
    }

    cout << "Done! Press Enter to exit..." << endl; 
    cin.ignore();

    return 0;
}
