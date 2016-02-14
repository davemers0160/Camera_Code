//=============================================================================
// 
// 
// 
//=============================================================================

// LINUX Includes
#include <stdio.h>   /* Standard input/output definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

// C++ Includes
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>

// windows Includes
//#include <windows.h>

// OPENCV Includes
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  

// Point Grey Includes
#include "stdafx.h"
#include "include/FlyCapture2.h"
#include "Config_Chameleon.h"

// Varioptic Lens Includes
#include "varioptic_class.h"

using namespace std;
using namespace cv;
using namespace FlyCapture2;


void getcurrenttime(char currenttime[]);
//bool configLensDriver(string port, int &serialHandle);
//void cameraConnect(PGRGuid guid, Camera &cam);
int videoCapture(Camera *cam, int serialHandle, string save_file);


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





int main(int /*argc*/, char** /*argv*/)
{    
	
	// Camera specific variables
	Error error;
	BusManager busMgr;
    PGRGuid guid;
	Camera cam;
	FC2Config cameraConfig;
	unsigned int numCameras;
	unsigned int offsetX, offsetY, width, height;
	PixelFormat pixelFormat;
	Property shutter, gain;

	// Serial Port specific variables
	//LPCWSTR commPort = L"\\\\.\\COM5";
	//HANDLE lensDriver = NULL;
	string commPort = "/dev/ttyUSB0";
	int lensDriver = 0;

	string save_file;
	char currenttime[80];

	VideoWriter outputVideo();

	getcurrenttime(currenttime);

	save_file = "test_recording_" + (string)currenttime + ".avi";
	//save_file = "/media/odroid/TOSHIBA EXT/Temp/test_recording_" + (string)currenttime + ".avi";
    PrintBuildInfo();
    
	//configLensDriver(commPort, lensDriver);

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        //PrintError( error );
		return EXIT_FAILURE;
    }

    cout << "Number of cameras detected: " << numCameras << endl; 

    //for (unsigned int i=0; i < numCameras; i++)
    //{
    
    error = busMgr.GetCameraFromIndex(0, &guid);
    if (error != PGRERROR_OK)
    {
        //PrintError( error );
        return EXIT_FAILURE;
    }

	// connect to the camera
	cameraConnect(guid, &cam);
	if (error != PGRERROR_OK)
	{
		//PrintError(error);
		return EXIT_FAILURE;
	}

	// Get the camera configuration
	error = cam.GetConfiguration(&cameraConfig);
	if (error != PGRERROR_OK)
	{
		//PrintError(error);
		return EXIT_FAILURE;
	}


	// configure the image size and the pixel format for the video
	offsetX = 40;
	offsetY = 224;
	width = 1200;
	height = 768;
	pixelFormat = PIXEL_FORMAT_411YUV8;
	configImagerFormat(&cam, offsetX, offsetY, width, height, pixelFormat);

	configProperty(shutter, SHUTTER, true, true);
	configProperty(gain, GAIN, false, false);

	//error = setProperty(cam, gain, 4.0);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return EXIT_FAILURE;
	}


	// begin the video capture
	videoCapture(&cam, lensDriver, save_file);

	// Disconnect the camera
	error = cam.Disconnect();
	if (error != PGRERROR_OK)
	{
		//PrintError(error);
		return -1;
	}

	// disconnect from lens driver
	//close(lensDriver);

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

	strftime(currenttime, 80, "%Y%m%d_%H%M%S", timeinfo);
	string str(currenttime);
	//cout << currenttime << endl;
	//cout << str << endl;

	//return 0;
}
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
	serialParams.BaudRate = 57600;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
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
	timeout.ReadTotalTimeoutConstant = 50;
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
 *
 */

int videoCapture(Camera *cam, int lensDriver, string save_file)
{

	unsigned long dwBytesWritten;
	double tick;

	Error error;
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
	unsigned int idx = 0;
	double voltage[2] = { 40.0, 43.0 };
	varioptic_class Casp_Lens;
	//BOOL comm_result;

	unsigned long dwRead;
	char rx_data[4] = { 0, 0, 0, 0 };
	//OVERLAPPED osReader = { 0 };

	namedWindow(Window1, WINDOW_NORMAL);       

	// Start capturing images
	error = cam->StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	Image rawImage;
	Image convertedImageCV;
	unsigned char *image_data = NULL;

	error = cam->RetrieveBuffer(&rawImage);
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
		rowBytes = (unsigned int)((double)convertedImageCV.GetDataSize() / (double)convertedImageCV.GetRows());
		image_size = Size((int)image_cols, (int)image_rows);
		outputVideo.open(save_file, codec, fps, image_size, true);
		cout << "video size: " << image_cols << " x " << image_rows << endl;

	}

	idx = 0;

	while (key != 'q')
	{
		tick = (double)getTickCount();		// start timing process 

		// send infocus voltage to lens driver
		Casp_Lens.voltage(voltage[0]);
//		WriteFile(lensDriver, Casp_Lens.Packet, Casp_Lens.packet_length + 1, &dwBytesWritten, NULL);
//		ReadFile(lensDriver, rx_data, sizeof(rx_data), &dwRead, &osReader);

		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

		// Convert the raw image	PIXEL_FORMAT_BGR for opencv
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		// convert data to opencv format
		image_data = convertedImageCV.GetData();
		video_frame = Mat(image_size, CV_8UC3, image_data, rowBytes);

		//ReadFile(lensDriver, rx_data, sizeof(rx_data), &dwRead, &osReader);	
		
		// display images
		//imshow(Window1, video_frame);

		// write video frame to file
		outputVideo.write(video_frame);
		//key = waitKey(delay);

		//tick = (double)getTickCount() - tick;
		//cout << "Execution Time: " << fixed << setw(5) << setprecision(0) << (tick*1000. / getTickFrequency()) << "ms" << endl;

		// send blurred voltage to lens driver
		Casp_Lens.voltage(voltage[1]);
//		WriteFile(lensDriver, Casp_Lens.Packet, Casp_Lens.packet_length + 1, &dwBytesWritten, NULL);

		// Retrieve an image
		error = cam->RetrieveBuffer(&rawImage);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			continue;
		}

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

		//ReadFile(lensDriver, rx_data, sizeof(rx_data), &dwRead, &osReader);	

		//imshow(Window1, video_frame);

		// write video frame to file
		outputVideo.write(video_frame);
		key = waitKey(delay);

		tick = (double)getTickCount() - tick;
		cout << "Execution Time: " << fixed << setw(5) << setprecision(0) << (tick*1000. / getTickFrequency()) << "ms" << endl;

	}

	outputVideo.release();

	destroyAllWindows();
	cout << "Finished Writing Video!" << endl;

	//CloseHandle(serialHandle);

	// Stop capturing images
	error = cam->StopCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	return 0;

}	// end of videoCapture


