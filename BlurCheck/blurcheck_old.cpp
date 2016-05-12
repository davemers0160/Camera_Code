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
//#include "FlyCapture2.h"
//#include "Config_Chameleon.h"

// Lens Driver Includes
//#include "Lens_Driver.h"

using namespace std;
//using namespace FlyCapture2;
//using namespace Lens_Driver;

void mouseROI_Handler(int event, int x, int y, int flags, void* param);

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


//Mat img, roiImg; /* roiImg - the part of the image in the bounding box */
//int select_flag = 0;
//int drag = 0;




int main(int argc, char** argv)
{
	int height = 200;
	int width = 100;
	Size img_size;
	
	char *orig_img_window = "Original Image";
	char *roi_img_window = "Selected ROI";
	char *lap_img1_window = "Laplacian of Original Image";
	char *lap_img2_window = "Laplacian of Blurred Image";

	update = false;
	leftBtnDown = false;
	leftBtnUp = false;

	Mat original_image;
	Mat ROI_img;
	img_size = Size(width, height);
	Mat black = Mat(img_size, CV_8UC1, Scalar(0));
	Mat white = Mat(img_size, CV_8UC1, Scalar(255));

	


	// create the black and white half image
	original_image = Mat(Size(width * 2, height), CV_8UC1);
	Mat left(original_image, Rect(0, 0, width, height));
	black.copyTo(left);
	Mat right(original_image, Rect(width, 0, width, height));
	white.copyTo(right);

	// read in image from file
	//combined = imread("test1.png", CV_LOAD_IMAGE_GRAYSCALE);

	// set initial ROI to the full size image loaded in
	corner1 = Point(original_image.rows, 0);
	corner2 = Point(0, original_image.cols);
	
	ROI_Box.width = abs(corner1.x - corner2.x);
	ROI_Box.height = abs(corner1.y - corner2.y);
	ROI_Box.x = min(corner1.x, corner2.x);
	ROI_Box.y = min(corner1.y, corner2.y);

	ROI_img = Mat(original_image, ROI_Box);

	

	namedWindow(orig_img_window, WINDOW_NORMAL);
	imshow(orig_img_window, original_image);

	setMouseCallback(orig_img_window, mouseROI_Handler);
	//waitKey(0);
	namedWindow(roi_img_window, WINDOW_AUTOSIZE);
	while (char(waitKey(1) != 'q'))
	{
		


		if (update == true)
		{
			ROI_img = Mat(original_image, ROI_Box);

			destroyWindow(roi_img_window);
			imshow(roi_img_window, ROI_img);
			update = false;
		}

	}

	Mat blurred = Mat(Size(width * 2, height), CV_8UC1);

	GaussianBlur(original_image, blurred, Size(0, 0),0.8, 0.8, BORDER_REPLICATE);

	imshow("blur", blurred);
	waitKey(0);


	Mat img_check = Mat(blurred, ROI_Box);
	Mat good;
	Mat test;
	Mat abs_dst;

	Laplacian(ROI_img, good, CV_8U, 7, 1, 0, BORDER_DEFAULT);
	convertScaleAbs(good, abs_dst);

	/// Show what you got
	imshow("Lap_good", good);
	waitKey(0);

	Laplacian(img_check, test, CV_8U, 7, 1, 0, BORDER_DEFAULT);
	convertScaleAbs(test, abs_dst);

	imshow("Lap_blur", test);
	waitKey(0);

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

	if((leftBtnDown==true) &&(leftBtnUp==true))
	{
		ROI_Box.width = abs(corner1.x - corner2.x);
		ROI_Box.height = abs(corner1.y - corner2.y);
		ROI_Box.x = min(corner1.x, corner2.x);
		ROI_Box.y = min(corner1.y, corner2.y);
		update = true;
		leftBtnDown = false;
		leftBtnUp = false;
		//drag = 0;
		//roiImg = img(rect);
	}

}