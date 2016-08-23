// C++ Includes
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>
#include <fstream>

// windows Includes
#include <windows.h> 
#include <WinUser.h>

// OPENCV Includes
#define USE_OPENCV

#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
using namespace cv;
#endif

// Point Grey Includes
//#include "stdafx.h"
#include "FlyCapture2.h"
#include "Chameleon_Utilities.h"

// Lens Driver Includes
#include "Lens_Driver.h"

using namespace std;
using namespace FlyCapture2;
using namespace Lens_Driver;


//GLOBAL VARIABLES & STRUCTURES
struct ftdiDeviceDetails //structure storage for FTDI device details
{
	int devNumber;
	unsigned long type;
	unsigned long BaudRate;
	string Description;
	string serialNumber;
};

Point corner1, corner2;			// corners for the ROI selection box

Rect ROI_Box;					// The actual ROI box
bool update;
bool leftBtnDown, leftBtnUp;


// PROTOTYPE DEFINITIONS
bool configLensDriver(LPCWSTR port, HANDLE &serialHandle);
int videoCapture(Camera *cam, HANDLE serialHandle, string save_file);
void PrintBuildInfo();
void PrintDriverInfo(LensDriverInfo *LensInfo);
void mouseROI_Handler(int event, int x, int y, int flags, void* param);
void getcurrenttime(char currenttime[]);



int main(int argc, char** argv)
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
	//Property Shutter, Gain, Sharpness, Framerate;
	int temp_sharp, temp_sharp1;
	float temp_shutter, temp_shutter1;
	float temp_gain, temp_gain1;
	Image rawImage, convertedImageCV;
	unsigned int rowBytes;
	float shutter, gain;
	int sharpness;
	float framerate = 30.0;

	//Lens_Driver test_lens;
	//unsigned char data[4] = { 1, 2, 3, 4 };
	LensTxPacket LensTx(CON, 0);
	LensRxPacket LensRx;
	LensDriverInfo LensInfo;
	unsigned char status;
	unsigned char stepStart = 125;
	unsigned char stepRange = 31;
	unsigned char data[1] {stepStart};
	LensTxPacket Focus(FAST_SET_VOLT, 1, &data[0]);

	// Serial Port specific variables
	string port_num = "COM9";
	wstring port = L"\\\\.\\"+ wstring(port_num.begin(),port_num.end());
	LPCWSTR lensPort = port.c_str();	//L"\\\\.\\COM9"; //(LPCWSTR)port.c_str();	//"\\\\.\\COM7";
	HANDLE lensDriver = NULL;


	// OPENCV variables
	char *orig_img_window = "Original Image";
	char *roi_img_window = "Selected ROI";
	char *dft_window = "Magnitude of DFT of Image";
	char *blur_window = "Blurred Image";

	int image_rows = 200;
	int image_cols = 100;
	int c;
	double sum;
	double sigma;
	update = false;
	leftBtnDown = false;
	leftBtnUp = false;
	//int codec = CV_FOURCC('D', 'I', 'V', 'X');
	//int codec = CV_FOURCC('M', 'J', 'P', 'G');
	//int codec = CV_FOURCC('H', '2', '6', '4');
	//int codec = CV_FOURCC('I', 'Y', 'U', 'V');
	//int codec = CV_FOURCC('I', '4', '2', '0');
	//int codec = CV_FOURCC('W', 'M', 'V', '2');
	//int codec = CV_FOURCC('P', 'I', 'M', '1');
	//int codec = CV_FOURCC('M', 'P', '4', '2');
	int codec = CV_FOURCC('F', 'M', 'P', '4');
	//int codec = -1;
	//string file_extension = ".mp4";
	string file_extension = ".avi";

	//Mat originalImage;
	Mat camImage;
	Mat camImageGray;
	Mat ROI_img;
	Mat magI;
	Size image_size;// = Size(width, height);

	// general variables
	bool exit = false;
	char key = 0;
	int idx, jdx;
	unsigned char bestStep = stepStart;
	double maxDFTValue;
	double DFTSum;
	string log_save_file;
	string video_save_file;
	
	char currenttime[80];
	ofstream DataLogStream;

	// get the desktop size
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);

	cout << "Desktop size: " << desktop.right << " x " << desktop.bottom << endl;



	PrintBuildInfo();

	//configLensDriver(lensPort, lensDriver);

	//sendLensPacket(LensTx, lensDriver);
	//status = readLensPacket(&LensRx, lensDriver, 9);

	//if (status == false)
	//{
	//	cout << "Error communicating with lens driver." << endl;
	//	cin.ignore();
	//	return 1;
	//}
	//getLensDriverInfo(&LensInfo, LensRx);
	//PrintDriverInfo(&LensInfo);




	// read in image from file
	//camImage = imread("4_bar_test.png", CV_LOAD_IMAGE_GRAYSCALE);


	//sendLensPacket(Focus, lensDriver);


	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		cin.ignore();
		return 1;
	}

	cout << "Number of cameras detected: " << numCameras << endl;

	error = busMgr.GetCameraFromIndex(0, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		cin.ignore();
		return 1;
	}

	// connect to the camera
	cameraConnect(guid, &cam);

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
	offsetX = 0;		
	width = 1280;		

	offsetY = 0;		
	height = 1024;		

	pixelFormat = PIXEL_FORMAT_444YUV8;
	configImagerFormat(&cam, offsetX, offsetY, width, height, pixelFormat);

	error = cam.StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	//configProperty(&cam, framerate, FRAME_RATE, false, true, true);
	//error = setProperty(&cam, framerate, 30.0);
	//if (error != PGRERROR_OK)
	//{
	//	PrintError(error);
	//	return 1;
	//}

	//Sleep(100);
	/*
	configProperty(&cam, shutter, SHUTTER, true, true, true);
	temp_shutter = 40.0;
	//temp_shutter = getABSProperty(&cam, shutter);
	//configProperty(&cam, shutter, SHUTTER, false, true, true);
	error = setProperty(&cam, shutter, temp_shutter);
	Sleep(1000);
	temp_shutter = getABSProperty(&cam, shutter);
	configProperty(&cam, shutter, SHUTTER, false, true, true);
	error = setProperty(&cam, shutter, temp_shutter);
	
	configProperty(&cam, gain, GAIN, true, true, true);
	temp_gain = 30.0;
	//configProperty(&cam, gain, GAIN, false, false, true);
	error = setProperty(&cam, gain, temp_gain);
	Sleep(1000);
	temp_gain = getABSProperty(&cam, gain);
	configProperty(&cam, gain, GAIN, false, false, true);
	error = setProperty(&cam, gain, temp_gain);
	Sleep(50);

	// auto tune the sharpness for the current capture
	configProperty(&cam, sharpness, SHARPNESS, true, true, false);
	temp_sharp = 1023;
	//configProperty(&cam, sharpness, SHARPNESS, false, false, false);
	error = setProperty(&cam, sharpness, temp_sharp);
	Sleep(1000);
	temp_sharp = getProperty(&cam, sharpness);
	configProperty(&cam, sharpness, SHARPNESS, false, false, false);
	error = setProperty(&cam, sharpness, temp_sharp);
	*/

	//// set the shutter speed to auto
	//configProperty(&cam, shutter, SHUTTER, true, true, true);
	//error = setProperty(&cam, shutter, 33.0);

	//// set the gain to auto
	//configProperty(&cam, gain, GAIN, true, true, true);
	//error = setProperty(&cam, gain, 10.0);

	//// set the sharpness to auto
	//configProperty(&cam, sharpness, SHARPNESS, true, true, false);
	//error = setProperty(&cam, sharpness, 1200);

	//temp_shutter1 = getABSProperty(&cam, shutter);
	//temp_gain1 = getABSProperty(&cam, gain);
	//temp_sharp1 = getProperty(&cam, sharpness);

	//Sleep(100);

	//// get the auto values
	//temp_shutter = getABSProperty(&cam, shutter);
	//temp_gain = getABSProperty(&cam, gain);
	//temp_sharp = getProperty(&cam, sharpness);

	//// set the values to fixed
	//configProperty(&cam, shutter, SHUTTER, false, true, true);
	//error = setProperty(&cam, shutter, temp_shutter);
	//configProperty(&cam, gain, GAIN, false, false, true);
	//error = setProperty(&cam, gain, temp_gain);
	//configProperty(&cam, sharpness, SHARPNESS, false, false, false);
	//error = setProperty(&cam, sharpness, temp_sharp);


	error = configCameraPropeties(&cam, &sharpness, &shutter, &gain, framerate);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}


	cout << "Shutter Speed (ms): " << shutter << endl;
	cout << "Gain (dB): " << gain << endl;
	cout << "Sharpness: " << sharpness << endl;

	//error = cam.StartCapture();
	//if (error != PGRERROR_OK)
	//{
	//	PrintError(error);
	//	return -1;
	//}

	error = cam.RetrieveBuffer(&rawImage);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
	
	error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	image_cols = convertedImageCV.GetCols();
	image_rows = convertedImageCV.GetRows();
	image_size = Size(image_cols, image_rows);

	double temp = convertedImageCV.GetDataSize();
	rowBytes = (unsigned int)(temp / (double)image_rows);

	namedWindow(orig_img_window, WINDOW_NORMAL);
	resizeWindow(orig_img_window, (int)(image_cols / 2), (int)image_rows / 2);
	moveWindow(orig_img_window, (int)(desktop.right / 3)-50, 0);

	namedWindow(roi_img_window, WINDOW_NORMAL);
	resizeWindow(roi_img_window, (int)(image_cols / 2), (int)image_rows / 2);
	moveWindow(roi_img_window, (int)((desktop.right / 3) + (image_cols / 2)-20), 0);

	std::cout << "Press esc to exit! " << std::endl;

	// start the loop to continually take data
	while (exit == false)
	{
		error = cam.RetrieveBuffer(&rawImage);
		error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}	

		//image_data = convertedImageCV.GetData();
		camImage = Mat(image_size, CV_8UC3, convertedImageCV.GetData(), rowBytes);
		cvtColor(camImage, camImageGray, CV_BGR2GRAY);		

		// set initial ROI to the full size image loaded in
		corner1 = Point(camImageGray.cols, 0);
		corner2 = Point(0, camImageGray.rows);

		ROI_Box.width = abs(corner1.x - corner2.x)-2;
		ROI_Box.height = abs(corner1.y - corner2.y)-2;
		ROI_Box.x = min(corner1.x, corner2.x)+1;
		ROI_Box.y = min(corner1.y, corner2.y)+1;

		ROI_img = Mat(camImageGray, ROI_Box);


		//sendLensPacket(Focus, lensDriver);

		imshow(orig_img_window, camImage);
		setMouseCallback(orig_img_window, mouseROI_Handler);

		imshow(roi_img_window, ROI_img);

		std::cout << "Press 'q' to accept the selected ROI. " << std::endl;
		do
		{
			error = cam.RetrieveBuffer(&rawImage);
			error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
			if (error != PGRERROR_OK)
			{
				PrintError(error);
				return -1;
			}

			//image_data = convertedImageCV.GetData();
			camImage = Mat(image_size, CV_8UC3, convertedImageCV.GetData(), rowBytes);
			cvtColor(camImage, camImageGray, CV_BGR2GRAY);
			imshow(orig_img_window, camImage);

			if (update == true)
			{
				ROI_img = Mat(camImageGray, ROI_Box);

				//destroyWindow(roi_img_window);
				//namedWindow(roi_img_window, WINDOW_NORMAL);
				imshow(roi_img_window, ROI_img);
				update = false;
			}

			key = (char)waitKey(50);
			if (key == 0x1B)
			{
				exit = true;
				key = 'q';
			}
		}while (key != 'q');

		if (exit == false)
		{
			Mat blurred = Mat(Size(image_cols * 2, image_rows), CV_8UC1);
			Mat planes[2];// = { Mat_<float>(paddedImage), Mat::zeros(paddedImage.size(), CV_32F) };
			maxDFTValue = 0.0;
			VideoWriter outputVideo;
			Mat InputImage = Mat(camImageGray, ROI_Box);
			c = getOptimalDFTSize(InputImage.cols);
			Mat video_frame = Mat(Size(ROI_img.cols + c, ROI_img.rows), CV_8UC1);
			Mat video_frame_color = Mat(Size(ROI_img.cols + c, ROI_img.rows), CV_8UC3);

			// get the current time for the video file name and saving the output data
			//getcurrenttime(currenttime);
			//log_save_file = "blurcheck_log_" + (string)currenttime + ".txt";
			//video_save_file = "blurcheck_" + (string)currenttime + file_extension;

			//outputVideo.open(video_save_file, codec, 10.0, Size(ROI_img.cols + c, ROI_img.rows),true);

			//DataLogStream.open(log_save_file.c_str(), ios::out);
			
			//DataLogStream << "Shutter Speed (ms):, " << temp_shutter << endl;
			//DataLogStream << "Gain (dB):, " << temp_gain << endl;
			//DataLogStream << "Sharpness:, " << temp_sharp << endl;

			// for loop to loop through variaous voltage levels for the lens
			//for (idx = 0; idx < stepRange; idx++)
			do
			{

				//Focus.Data[0] = idx + stepStart;
				//sendLensPacket(Focus, lensDriver);
				//Sleep(100);


				// simulated blurs
				//sigma = 5.0*idx / 30.0+0.001;
				//GaussianBlur(originalImage, blurred, Size(0, 0), sigma, sigma, BORDER_REPLICATE);

				// get the images from the camera
				error = cam.RetrieveBuffer(&rawImage);
				error = rawImage.Convert(PIXEL_FORMAT_BGR, &convertedImageCV);
				if (error != PGRERROR_OK)
				{
					PrintError(error);
					return -1;
				}

				//image_data = convertedImageCV.GetData();
				camImage = Mat(image_size, CV_8UC3, convertedImageCV.GetData(), rowBytes);
				cvtColor(camImage, camImageGray, CV_BGR2GRAY);

				InputImage = Mat(camImageGray, ROI_Box);
				Mat blurDFT;
				Mat paddedImage;

				//namedWindow(blur_window, WINDOW_NORMAL);
				//imshow(blur_window, InputImage);

				// start the DFT process on the infocus test image
				//int c = getOptimalDFTSize(ROI_img.cols);		// get the optimal DFT size for rows
				//copyMakeBorder(ROI_img, paddedImage, 0, 0, 0, c - ROI_img.cols, BORDER_CONSTANT, Scalar::all(0));	// zero pad the image

				//Mat planes[] = { Mat_<float>(paddedImage), Mat::zeros(paddedImage.size(), CV_32F) };
				//merge(planes, 2, goodDFT);


				//dft(goodDFT, goodDFT, DFT_ROWS | DFT_SCALE);

				//split(goodDFT, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
				//magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
				//Mat magI = planes[0];

				///// Show what you got
				//imshow("InFocus DFT", magI);
				//double sum = cv::sum(magI.row(2))[0];
				//cout << "DFT sum of infocus image: " << sum << endl;
				//waitKey(0);


				// start the DFT process on the blurred test image
				c = getOptimalDFTSize(InputImage.cols);
				copyMakeBorder(InputImage, paddedImage, 0, 0, 0, c - ROI_img.cols, BORDER_CONSTANT, Scalar::all(0));	// zero pad the image

				planes[0] = Mat_<float>(paddedImage);
				planes[1] = Mat::zeros(paddedImage.size(), CV_32F);

				merge(planes, 2, blurDFT);
				dft(blurDFT, blurDFT, DFT_ROWS | DFT_SCALE);

				split(blurDFT, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
				magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
				magI = planes[0];

				InputImage(Rect(0, 0, InputImage.cols, InputImage.rows)).copyTo(video_frame(Rect(0, 0, InputImage.cols, InputImage.rows)));
				Mat tempMagI = magI * 255;
				tempMagI(Rect(0, 0, magI.cols, magI.rows)).copyTo(video_frame(Rect(InputImage.cols, 0, magI.cols, magI.rows)));

				cvtColor(video_frame, video_frame_color, CV_GRAY2BGR);

				namedWindow(dft_window, WINDOW_NORMAL);
				imshow(dft_window, video_frame_color);

				DFTSum = cv::sum(magI)[0];
				std::cout << "DFT sum of image: " << DFTSum << endl;
				//DataLogStream << (int)Focus.Data[0] << "," << DFTSum << endl;

				//if (DFTSum > maxDFTValue)
				//{
				//	maxDFTValue = DFTSum;
				//	bestStep = Focus.Data[0];
				//}

				//for (int jdx = 0; jdx < 5; jdx++)
				//{

					//outputVideo.write(video_frame_color);
				//}

				key = (char)waitKey(200);
				if (key == 0x1B)
				{
					exit = true;
					key = 'q';
				}
			} while (key != 'q');	// end of loop

			//outputVideo.release();
			std::cout << endl << "Best Voltage Step Value: " << (int)bestStep << endl << endl;
			DataLogStream << "Best Voltage Step Value:, " << (int)bestStep << endl;
			DataLogStream.close();

			Focus.Data[0] = bestStep;
			sendLensPacket(Focus, lensDriver);
			destroyWindow(dft_window);
			destroyWindow(blur_window);

		}	// end of if check for exit criteria

	}	// end of while loop looking for ESC to be pressed

	//waitKey(0);

	

	// Disconnect the camera
	error = cam.Disconnect();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}

	// disconnect from lens driver
	CloseHandle(lensDriver);
	destroyAllWindows();

	//std::cout << "Done! Press Enter to exit..." << endl;
	//std::cin.ignore();
	
	
	return 0;


}	// end of main



void mouseROI_Handler(int event, int x, int y, int flags, void* param)
{
	if (event == CV_EVENT_LBUTTONDOWN)// && !drag)
	{
		/* left button clicked. ROI selection begins */
		leftBtnDown = true;
		corner1 = Point(x, y);
		//drag = 1;
	}

	//if (event == CV_EVENT_MOUSEMOVE)// && drag)
	//{
	//	/* mouse dragged. ROI being selected */
	//	
	//	//Mat img1 = img.clone();
	//	corner2 = Point(x, y);
	//	//rectangle(img1, corner1, corner2, CV_RGB(255, 0, 0), 3, 8, 0);
	//	//imshow("image", img1);
	//}

	if (event == CV_EVENT_LBUTTONUP)// && drag)
	{
		leftBtnUp = true;
		corner2 = Point(x, y);

	}

	if ((leftBtnDown == true) && (leftBtnUp == true))
	{
		ROI_Box.width = abs(corner1.x - corner2.x);
		ROI_Box.height = abs(corner1.y - corner2.y);
		ROI_Box.x = min(corner1.x, corner2.x);
		ROI_Box.y = min(corner1.y, corner2.y);
		if (ROI_Box.width == 0 || ROI_Box.height == 0)
		{
			update = false;
			leftBtnDown = false;
			leftBtnUp = false;
		}
		else
		{
			update = true;
			leftBtnDown = false;
			leftBtnUp = false;
		}

		//drag = 0;
		//roiImg = img(rect);
	}

}	// end of mouseROI_Handler


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
		std::cout << "Error setting serial port configuration" << endl;
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



void PrintBuildInfo()
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

void getcurrenttime(char currenttime[])
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(currenttime, 80, "%m%d%Y_%H%M%S", timeinfo);
	string str(currenttime);

}	// end of getcurrenttime

