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
//#include "stdafx.h"
#include "FlyCapture2.h"
#include "Config_Chameleon.h"

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
	Property shutter, gain, sharpness, framerate;
	int temp_int_property;
	float temp_f_property;
	Image rawImage, convertedImageCV;
	unsigned int rowBytes;

	//Lens_Driver test_lens;
	//unsigned char data[4] = { 1, 2, 3, 4 };
	LensTxPacket LensTx(CON, 0);
	LensRxPacket LensRx;
	LensDriverInfo LensInfo;
	unsigned char status;
	unsigned char stepStart = 125;
	unsigned char stepStop = 160;
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

	//Mat black = Mat(img_size, CV_8UC1, Scalar(0));
	//Mat white = Mat(img_size, CV_8UC1, Scalar(255));


	PrintBuildInfo();

	configLensDriver(lensPort, lensDriver);

	sendLensPacket(LensTx, lensDriver);
	status = readLensPacket(&LensRx, lensDriver, 9);

	if (status == false)
	{
		cout << "Error communicating with lens driver." << endl;
		//cin.ignore();
		return 1;
	}
	getLensDriverInfo(&LensInfo, LensRx);
	PrintDriverInfo(&LensInfo);




	// read in image from file
	//camImage = imread("4_bar_test.png", CV_LOAD_IMAGE_GRAYSCALE);


	sendLensPacket(Focus, lensDriver);


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
	offsetX = 0;		
	width = 1280;		

	offsetY = 0;		
	height = 1024;		

	pixelFormat = PIXEL_FORMAT_422YUV8;
	configImagerFormat(&cam, offsetX, offsetY, width, height, pixelFormat);


	configProperty(&cam, framerate, FRAME_RATE, false, true, true);
	error = setProperty(&cam, framerate, 30.0);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 1;
	}


	configProperty(&cam, shutter, SHUTTER, true, true, true);
	temp_f_property = getABSProperty(&cam, shutter);
	configProperty(&cam, shutter, SHUTTER, false, true, true);
	error = setProperty(&cam, shutter, temp_f_property);


	configProperty(&cam, gain, GAIN, true, true, true);
	temp_f_property = getABSProperty(&cam, gain);
	configProperty(&cam, gain, GAIN, false, false, true);
	error = setProperty(&cam, gain, temp_f_property);

	// auto tune the sharpness for the current capture
	configProperty(&cam, sharpness, SHARPNESS, true, true, false);
	temp_int_property = getProperty(&cam, sharpness);
	configProperty(&cam, sharpness, SHARPNESS, false, false, false);
	error = setProperty(&cam, sharpness, temp_int_property);


	error = cam.StartCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}


	std::cout << "Press esc to exit! " << std::endl;


	while (exit == false)
	{
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


		//if (GetAsyncKeyState(VK_ESCAPE))
		//{
		//	exit = true;
		//}
		//

		namedWindow(orig_img_window, WINDOW_NORMAL);
		imshow(orig_img_window, camImage);
		setMouseCallback(orig_img_window, mouseROI_Handler);

		namedWindow(roi_img_window, WINDOW_NORMAL);
		imshow(roi_img_window, ROI_img);

		std::cout << "Press 'q' to accept the selected ROI. " << std::endl;
		do
		{
			if (update == true)
			{
				ROI_img = Mat(camImageGray, ROI_Box);

				destroyWindow(roi_img_window);
				imshow(roi_img_window, ROI_img);
				update = false;
			}

			key = (char)waitKey(1);
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

			for (idx = 0; idx < 30; idx++)
			{

				Focus.Data[0] = idx + stepStart;
				sendLensPacket(Focus, lensDriver);
				Sleep(100);


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

				//image_cols = convertedImageCV.GetCols();
				//image_rows = convertedImageCV.GetRows();
				//image_size = Size(image_cols, image_rows);

				//double temp = convertedImageCV.GetDataSize();
				//rowBytes = (unsigned int)(temp / (double)image_rows);

				//image_data = convertedImageCV.GetData();
				camImage = Mat(image_size, CV_8UC3, convertedImageCV.GetData(), rowBytes);
				cvtColor(camImage, camImageGray, CV_BGR2GRAY);




				//waitKey(0);


				Mat InputImage = Mat(camImageGray, ROI_Box);
				Mat blurDFT;
				Mat paddedImage;

				namedWindow(blur_window, WINDOW_NORMAL);
				imshow(blur_window, InputImage);

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

				namedWindow(dft_window, WINDOW_NORMAL);
				imshow(dft_window, magI);

				Mat temp = magI.row((unsigned int)(magI.rows / 2));

				// ignore the first two terms and only add the positive freq components
				sum = cv::sum(temp.colRange(2, (unsigned int)(temp.cols / 2) - 1))[0];
				cout << "Voltage Step: " << (int)Focus.Data[0] << "\tDFT sum of image: " << sum << endl;
				
				if (sum > maxDFTValue)
				{
					maxDFTValue = sum;
					bestStep = Focus.Data[0];
				}
				
				waitKey(400);

			}	// end of for loop

			cout << endl << "Best Voltage Step Value: " << (int)bestStep << endl << endl;
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

	cout << "Done! Press Enter to exit..." << endl;
	cin.ignore();
	
	destroyAllWindows();
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