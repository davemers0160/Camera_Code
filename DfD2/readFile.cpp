#include <cstdlib>
#include <cstdio>

#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>

#include "time.h"
#include <iostream>

using namespace std;
using namespace cv;



void readFile(Mat &img1, Mat &img2, string f1, string f2, int type, int num)
{
	int idx, jdx;
	Mat av1, av2, av1f, av2f;
	vector<Mat> YCRCB_IN_I(3);
	vector<Mat> YCRCB_IN_O(3);
	vector<Mat> YCRCB_AVG_I(3);
	vector<Mat> YCRCB_AVG_O(3);
	Size imageSize; 	
	string file1, file2;

	switch (type)
	{
		case 0:		// two individual files


			break;

		case 1:		// avi 


			break;


		case 2:		// time average files

			// temp read of the file to get the image information
			file1 = f1 + "0000.tif";
			av1 = imread(file1, CV_LOAD_IMAGE_COLOR);
			imageSize = av1.size();

			img1 = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
			img2 = Mat(imageSize, CV_64FC1, Scalar::all(0.0));

			for (idx = 0; idx < 3; idx++)
			{
				YCRCB_IN_I[idx] = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
				YCRCB_IN_O[idx] = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
				YCRCB_AVG_I[idx] = Mat(imageSize, CV_64FC1, Scalar::all(0.0));
				YCRCB_AVG_O[idx] = Mat(imageSize, CV_64FC1, Scalar::all(0.0));

			}

			for (idx = 0; idx < num; idx++)
			{
				file1 = f1 + "000" + to_string(idx) + ".tif";
				file2 = f2 + "000" + to_string(idx) + ".tif";
				av1 = imread(file1, CV_LOAD_IMAGE_COLOR);
				av2 = imread(file2, CV_LOAD_IMAGE_COLOR);
				av1.convertTo(av1f, CV_64FC3, 1.0, 0.0);
				av2.convertTo(av2f, CV_64FC3, 1.0, 0.0);

				split(av1f, YCRCB_IN_I);
				split(av2f, YCRCB_IN_O);

				for (jdx = 0; jdx < 3; jdx++)
				{
					YCRCB_AVG_I[jdx] += YCRCB_IN_I[jdx];
					YCRCB_AVG_O[jdx] += YCRCB_IN_O[jdx];
				}

			}

			for (jdx = 0; jdx < 3; jdx++)
			{
				YCRCB_AVG_I[jdx] = YCRCB_AVG_I[jdx]*( 1.0 / num);
				YCRCB_AVG_O[jdx] = YCRCB_AVG_O[jdx] * (1.0 / num);
			}

			merge(YCRCB_AVG_I, img1);
			merge(YCRCB_AVG_O, img2);

			img1.convertTo(img1, CV_8UC3, 1.0, 0);
			img2.convertTo(img2, CV_8UC3, 1.0, 0);


			break;

		default:


			break;

	}	// end of switch

}	// end of readFile





