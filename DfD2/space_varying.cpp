
#include <iostream>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <math.h>

using namespace cv;
using namespace std;

void Custom_Gassian_Kernel(int kernel_size, double center, double theta, double sigmax,double sigmay,Mat &kernel_mat)
{
    int i,j,tx=1,ty=1;
    Mat X= cv::Mat::ones(kernel_size,kernel_size,CV_32FC1);
    Mat Y= cv::Mat::ones(kernel_size,kernel_size,CV_32FC1);
    
    for(i=0;i<kernel_size;i++)
    {
        tx=1;
        for(j=0;j<kernel_size;j++)
        {
            X.at<float>(i,j)=tx;
            tx++;
        }
    }
    for(j=0;j<kernel_size;j++)
    {
        ty=1;
        for(i=0;i<kernel_size;i++)
        {
            Y.at<float>(i,j)=ty;
            ty++;
        }
    }
    Mat Xc = X - center;
    Mat Yc = Y - center;
    //double theta = (CV_PI/180) *theta_degree;
    Mat xm = Xc*cos(theta) - Yc*sin(theta);
    Mat ym = Xc*sin(theta) + Yc*cos(theta);
    Mat u = (xm/sigmax).mul(xm/sigmax)+(ym/sigmay).mul(ym/sigmay);
    exp(-1.0*u/2,kernel_mat);
    
}




// Convolve image with kernel
// image is single channel
void SpaceVaryingfilter2D (IplImage * image, int kernel_size,double min_sigma, double max_sigma, IplImage * filter)
{
    int center;
    center= (kernel_size-1)/2;
    //IplImage *dst2 = cvCreateImage(cvSize(image->width,image->height) , image->depth, 1);
    
    //cvCopyMakeBorder( image,  dst2, cvPoint(1,1), IPL_BORDER_CONSTANT );
    // cvNamedWindow("gray_image with boundary",CV_WINDOW_AUTOSIZE);
    // cvShowImage("gray_image with boundary",dst2);
    
    CvMat *srcMat;
    srcMat = cvCreateMat(image->height, image->width, CV_32FC1);
    cvConvert(image, srcMat);
    
    double tmp=0 ;
    
    
    CvMat *dstMat;
    CvMat *abs_dstMat;
    dstMat = cvCreateMat(image->height, image->width, CV_32FC1);
    abs_dstMat = cvCreateMat(image->height, image->width, CV_32FC1);
///////////////////////////////////////////////////////////////////////////////////////
    double width = image->width;
    double height = image->height;
    double maxdistance = sqrt(pow((width/2-center-1),2)+pow((height/2-center-1),2));
    double SQ,sigmax,sigmay;
    double theta=0,theta1,theta2;

///////////////////////////////////////////////////////////////////////////////////////
    int i,j,m,l,tmpx,tmpy;
    // for(i=center;i<=srcMat->height-1-center;i++)
    //     for(j=center;j<=srcMat->width-1-center;j++)
    for(i=0;i<=srcMat->height-1;i++)
        for(j=0;j<=srcMat->width-1;j++)
        {
///////////////////////////////////////////////////////////////////////////////////////
            SQ = sqrt(pow((width/2-j),2)+pow((height/2-i),2));
            sigmax = min_sigma+max_sigma*SQ/maxdistance;
            sigmay = min_sigma;
            theta1 =  180 * asin((i-round(height/2))/SQ)/CV_PI;
            theta2 =  180* asin((j-round(width/2))/SQ)/CV_PI;
            
            if (theta1 <0 && theta2<0)
                theta = (CV_PI/180) *(180+theta1);
            
            if (theta1 <0 && theta2>0)
                theta = (CV_PI/180) *(-1*theta1);
            
            if (theta1 >0 && theta2<0)
                theta = (CV_PI/180) *theta1;
            
            if (theta1 >0 && theta2>0)
                theta = (CV_PI/180) *(-1*theta1);
            
            Mat kernel_mat;
           
            
            Custom_Gassian_Kernel(kernel_size, (kernel_size+1)/2, theta, sigmax, sigmay, kernel_mat);
            
            
            Scalar S =sum(kernel_mat);
            double sum_coe = S(0);
            
            kernel_mat = kernel_mat / sum_coe;
///////////////////////////////////////////////////////////////////////////////////////
            
            

            for(m=0;m<kernel_size;m++)
                for(l=0;l<kernel_size;l++)
                {
                   // cout<<cvmGet(srcMat,i+(m-(kernel_size-1)/2),j+(l-(kernel_size-1)/2))<<endl;
                    if (i+(m-(kernel_size-1)/2) <0)
                        tmpx=i-(m-(kernel_size-1)/2);
                    else
                        if (i+(m-(kernel_size-1)/2)>srcMat->height-1)
                            tmpx=i-(m-(kernel_size-1)/2);
                        else
                            tmpx = i+(m-(kernel_size-1)/2);
                    
                    if (j+(l-(kernel_size-1)/2) <0)
                        tmpy=j-(l-(kernel_size-1)/2);
                    else
                        if (j+(l-(kernel_size-1)/2)>srcMat->width-1)
                            tmpy=j-(l-(kernel_size-1)/2);
                        else
                            tmpy=j+(l-(kernel_size-1)/2);
                    
                    tmp=tmp+cvmGet(srcMat,tmpx,tmpy)* kernel_mat.at<float>(m,l);
                    //cvmGet(&kernel,center+m,center+l);
                }
            cvmSet( dstMat, i, j, tmp );
            tmp=0;
        }
    cvConvertScale(dstMat,abs_dstMat,1,0 );
    cvConvert(dstMat, filter);
    
}


