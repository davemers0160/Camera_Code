// C++ Includes
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

// OPENCV Includes
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  

using namespace cv;
using namespace std;


void saveLabel(ofstream &file, string imgfilename, cv::Mat image, int label)
{
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);

	file << imgfilename << "  " << label << endl;

	cv::imwrite(imgfilename, image, compression_params);

}	// end of saveLabel