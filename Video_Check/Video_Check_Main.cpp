// C++ Includes
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>

// windows Includes
#include <windows.h> 

// OPENCV Includes
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
using namespace cv;

// Point Grey Includes
//#include "stdafx.h"
//#include "FlyCapture2.h"
//#include "Chameleon_Utilities.h"

// Lens Driver Includes
//#include "Lens_Driver.h"

using namespace std;
//using namespace FlyCapture2;
//using namespace Lens_Driver;

struct VideoCaptureInfo
{
	VideoCapture Video;
	Mat Frame;
	Mat trackingFrame;
	int frameWidth = 0;
	int frameHeight = 0;
	int fps = 0;
	int frameCount = 0;
	int currentFrame = 0;
	int realTime = 0;
	bool open = false;
	bool running = false;
};

void getVideoInfo(string filename, VideoCaptureInfo &VidCapInfo);

void printHelp()
{
	system("cls");
	cout << "Lens Driver Command Line Interface (CLI)" << endl;
	cout << "David Emerson" << endl << endl;
	cout << "Set the Lens Driver to a single step." << endl;
	cout << "Usage: LensDriver-cli -s <Step Number>" << endl << endl;

	cout << "Set the Lens Driver to toggle between two values with a given delay." << endl;
	cout << "Usage: LensDriver-cli -s <Step Number> -t <Step Number> -d <Delay in units of seconds>" << endl << endl;

	//cout << "Run program and track objects in recorded video file." << endl;
	//cout << "Usage: Final_Project -f <Input File Name>" << endl;
}


int main(int argc, char** argv)
{
	unsigned char idx;
	unsigned char set[2] = { 0, 0 };

	string focusfilename, defocusfilename;
	string filepath = "D:\\IUPUI\\Test_Data\\Data1\\";
	//string filepath = "G:\\Data\\";



	if (argc < 3)
	{
		printHelp();
		return 1;
	}
	else if (argc == 5)
	{
		if (strcmp(argv[1], "-f") == 0)
		{
			focusfilename = filepath + argv[2];
		}

		if (strcmp(argv[3], "-d") == 0)
		{
			defocusfilename = filepath + argv[4];
		}

	}
	else
	{
		cout << "Incorrect number of input arguments." << endl;
		printHelp();
		return 1;
	}

	// get the desktop size
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);

	cout << "Desktop size: " << desktop.right << " x " << desktop.bottom << endl;


	VideoCaptureInfo focusVideoCap, defocusVideoCap;
	getVideoInfo(focusfilename, focusVideoCap);
	getVideoInfo(defocusfilename, defocusVideoCap);
	char* Window1 = "Focus";
	char* Window2 = "Defocus";
	Mat focus, defocus;
	unsigned char key = 0;
	int frameCount = 0;

	bool frameRead;
	int windowBoarder = 19;
	int windowHeight = 600;
	int windowWidth = 800;
	cv::namedWindow(Window1, WINDOW_NORMAL | WINDOW_KEEPRATIO);
	cv::namedWindow(Window2, WINDOW_NORMAL | WINDOW_KEEPRATIO);
	cv::resizeWindow(Window1, windowWidth, windowHeight);
	cv::resizeWindow(Window2, windowWidth, windowHeight);
	cv::moveWindow(Window1, 100, 100);
	cv::moveWindow(Window2, 100 + windowWidth + windowBoarder, 100);

	while (key != 'q')
	{
		frameRead = focusVideoCap.Video.read(focus);
		frameRead = defocusVideoCap.Video.read(defocus);

		imshow(Window1, focus);
		imshow(Window2, defocus);

		frameCount++;

		if (frameCount < focusVideoCap.frameCount)
		{
			key = waitKey(500);
		}
		else
		{
			key = 'q';
		}

	}


	cout << "Press any key to continue..." << endl;
	waitKey(0);
	destroyAllWindows();
	return 0;
}


void getVideoInfo(string filename, VideoCaptureInfo &VidCapInfo)
{
	// open up the video file for reading
	VidCapInfo.Video.open(filename);

	// check the see if the file has been opened correctly
	if (VidCapInfo.Video.isOpened() == false)
	{
		VidCapInfo.frameWidth = 0;
		VidCapInfo.frameHeight = 0;
		VidCapInfo.fps = 0;
		VidCapInfo.frameCount = 0;
		VidCapInfo.currentFrame = 0;
		VidCapInfo.realTime = 2;
		VidCapInfo.open = false;
		VidCapInfo.running = false;
		cout << "Cannot open the file: " << filename << endl;
	}
	else
	{
		VidCapInfo.frameWidth = (int)VidCapInfo.Video.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
		VidCapInfo.frameHeight = (int)VidCapInfo.Video.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
		VidCapInfo.fps = (int)VidCapInfo.Video.get(CV_CAP_PROP_FPS);
		VidCapInfo.frameCount = (int)VidCapInfo.Video.get(CV_CAP_PROP_FRAME_COUNT);
		VidCapInfo.currentFrame = 0;
		VidCapInfo.realTime = 2;
		VidCapInfo.open = true;
		VidCapInfo.running = true;
		if (VidCapInfo.fps <= 0)
		{
			VidCapInfo.fps = 25;
		}

		int ex = static_cast<int>(VidCapInfo.Video.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form

		// Transform from int to char via Bitwise operators
		char EXT[] = { (char)(ex & 0XFF), (char)((ex & 0XFF00) >> 8), (char)((ex & 0XFF0000) >> 16), (char)((ex & 0XFF000000) >> 24), 0 };

		cout << "Video file was opened successfully!" << endl;
		cout << "Frame size : " << VidCapInfo.frameWidth << " x " << VidCapInfo.frameHeight << endl;
		cout << "Frame Count : " << VidCapInfo.frameCount << endl;
		cout << "FPS : " << VidCapInfo.fps << endl;
		cout << "CODEC: " << EXT << endl;
	}
}	// end of getVideoInfo