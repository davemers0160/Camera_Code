//#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <omp.h>
#include "allocate.h"
#include <iostream>
#include <fstream>
//#include "cv.h"
//#include "highgui.h"
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>
#include <time.h>
#include <fstream>
using namespace std;

#define MAX_CLASSES 256

unsigned char	**gamma[1000], **atlas[1000];
//int				i,j,k,c,r,edgevalue,texturevalue,texturevalueCr,texturevalueCb,tempmin,counter = 0;
//double			random2(),mm,sum,AveCost,diff[MAX_CLASSES+1], prior[MAX_CLASSES+1],DiSum,PiSum, assist;



