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
// added
#include <opencv2/core/core.hpp>           
#include <opencv2/highgui/highgui.hpp>     
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv/cv.h>

// GC
#include "GCoptimization.h"

#include <time.h>
#include <fstream>
using namespace std;

#define PI  3.1415926535897932386
#define MAX_CLASSES 256
#define MAXPRIME  2147483647       /*  MAXPRIME = (2^31)-1     */
#define PI        3.14159265358979323846

//unsigned char	**get_img(int,int,unsigned char);
//unsigned char	**gamma[1000], **atlas[1000];
int				i,j,l,k,c,r,edgevalue,texturevalue,texturevalueCr,texturevalueCb,tempmin,counter = 0;
double			random2(),mm,sum,AveCost,diff[MAX_CLASSES+1], prior[MAX_CLASSES+1],DiSum,PiSum, assist;
double			**xrv, x, flag, ratio,current,invannealtemp, compare[MAX_CLASSES];

ofstream outfile("logpost.txt",ios::out);

double callogpost(double **diff_sq[],double **atlas[],unsigned char **xttemp[],int cols, int rows, int ATLAS, double gama, int classes, double beta, double *d, double *con, double *logpost, int edgevalue, int texturevalue, IplImage *highpass, IplImage* texture);

void GridGraph_DArraySArray(int width,int height,int num_labels,double **logpost1[],int *result);
//void GeneralGraph_DArraySArray(int width,int height,int num_labels,double **logpost1[], int *result);
///////////////////////////////////////////////////////////////////////////////////
//																				 //
//    Part 1:																	 //
//			Generate Random numbers                                              //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////


/* PORTABILITY 1:  The functions in this file assume that a long is 32 bits
      and a short is 16 bits.  These conventions are machine dependent and
      must therefore be verified before using.                     */

static unsigned long   sd[2],tmp;   /*  tmp: 31 bit seed in GF( (2^31)-1 )   */
                                    /*  sd[0]: high order 15 bits of tmp     */
                                    /*  sd[1]: low order 16 bits of tmp      */
                 /* NOTE: high order 16 bits of sd[0] and sd[1] are 0        */

unsigned long srandom2(unsigned long num)
/* Set a new seed for random # generator  */
{
 tmp=num;
 *sd=tmp>>16;
 *(sd+1)=tmp & 0xffff;
 return *sd;
}
/*
void readseed()
/*  Reads random # generator seed from file: /tmp/randomseedmlc
{
 FILE	*fp1;
 char	*calloc();
 void	writeseed();

   if((fp1 = fopen("randomseedmlc","r"))==NULL ) {
     fprintf(stderr,"readseed: creating file /tmp/randomseedmlc\n");
     tmp=143542612;
     writeseed();
     srandom2(tmp);
   } else {
     fscanf(fp1,"%d",&tmp);
     srandom2(tmp);
     fclose(fp1);
   }
}

void writeseed()
  Writes random # generator seed from file: /tmp/randomseedmlc
{
 FILE  *fp1;
 char	*calloc();

   if((fp1 = fopen("randomseedmlc","w"))==NULL ) {
     fprintf(stderr,"writeseed: can't open file /tmp/randomseedmlc\n");
     exit(1);
   } else {
     fprintf(fp1,"%d",tmp);
     fclose(fp1);
   }
}
*/

double random2()
/* Uniform random number generator on (0,1] */
/*  Algorithm:  newseed = (16807 * oldseed) MOD [(2^31) - 1]  ;
                returned value = newseed / ( (2^31)-1 )  ;
      newseed is stored in tmp and sd[0] and sd[1] are updated;
      Since 16807 is a primitive element of GF[(2^31)-1], repeated calls
      to random2() should generate all positive integers <= MAXPRIME
      before repeating any one value.
    Tested: Feb. 16, 1988;  verified the length of cycle of integers 
                             generated by repeated calls to random2()  */
{
 *(sd+1) *= 16807;
 *sd *= 16807;
 tmp=((*sd)>>15)+(((*sd)&0x7fff)<<16);
 tmp += (*(sd+1));
 if ( tmp & 0x80000000 ) {
   tmp++;
   tmp &= 0x7fffffff;
 }
 *sd=tmp>>16;
 *(sd+1)=tmp & 0xffff;
 return(((double)tmp)/MAXPRIME);   
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//																				 //
//    Part 2:																	 //
//			Subfunction of calculating logpost                                   //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////


double callogpost(double **diff_sq[], double **atlas[],unsigned char **xttemp[],int cols, int rows, int ATLAS, double gama, int classes, double beta, double *d, double *con, double *logpost, int edgevalue, int texturevalue, IplImage *highpass, IplImage* texture)
{		
	int edgevalue1,texture1;
	double weight;
	double alpha = 1.0;
	double tempsq;
//////////// More Texture region //////////////////////////////////////////////////////////////////////////////////////

	if (texturevalue == 0 ) // more texture region
		{					
			for (k=1; k<=MAX_CLASSES; k++)
				{
					prior[k] = 0;
					diff[k] = 0;
					diff[k]+=diff_sq[k][i][j];

					//**************************************************************************//
					if (i-1 >= 0) 
						{                       							
						  prior[k]+=(double)abs(xttemp[0][i-1][j]-k); 
						  diff[k]+=diff_sq[k][i-1][j]; //neighbors difference       left	
									
						  if (j-1 >=0) {
										diff[k]+=diff_sq[k][i-1][j-1]; //neighbors difference    left top  
										prior[k]+=(double)abs(xttemp[0][i-1][j-1]-k); 
									   }                           
						  if (j+1 <= cols-1) 
									   {
										diff[k]+=diff_sq[k][i-1][j+1]; //neighbors difference   left bottom
										prior[k]+=(double)abs(xttemp[0][i-1][j+1]-k); 
									   } 
						}

					//**************************************************************************//
					if (i+1 <= rows-1) 
						{						
						  prior[k]+=(double)abs(xttemp[0][i+1][j]-k);
						  diff[k]+=diff_sq[k][i+1][j]; //neighbors difference

						  if (j-1 >=0) {
										diff[k]+=diff_sq[k][i+1][j-1];
										prior[k]+=(double)abs(xttemp[0][i+1][j-1]-k); 
									   } 									
						  if (j+1 <= cols-1) 
									   {      
										diff[k]+=diff_sq[k][i+1][j+1];
										prior[k]+=(double)abs(xttemp[0][i+1][j+1]-k); 
									   }
						}

					//**************************************************************************//
					if (j-1 >=0) 
					    {									
						  prior[k]+=(double)abs(xttemp[0][i][j-1]-k);									
						  diff[k]+=diff_sq[k][i][j-1]; //neighbors difference
						}

					//**************************************************************************//
					if (j+1 <= cols-1) 
					    {									
						  prior[k]+=(double)abs(xttemp[0][i][j+1]-k);
						  diff[k]+=diff_sq[k][i][j+1]; //neighbors difference
						}
				}
		}

//////////// Less Texture region //////////////////////////////////////////////////////////////////////////////

		if (texturevalue == 255  ) // less texture 
			{	
			    for (k=1; k<=MAX_CLASSES; k++)
				{
				  prior[k] = 0;
				  diff[k] = 0;
				  diff[k]+=diff_sq[k][i][j];

				  //**************************************************************************//
				  if (i-1 >= 0) 
				     {                       
						if (xttemp[0][i-1][j] != k) 
							{	
								texture1=cvGetReal2D(texture, i-1, j);	
								if (texture1==0 ) 
									{
									  alpha=0;
									  weight=10;
									  if ( abs(xttemp[0][i-1][j]-xttemp[0][i][j])<1 ) cvSetReal2D(texture, i, j,0);
									}
								else 
									{
									  edgevalue1=cvGetReal2D(highpass, i-1, j); 
									  if (edgevalue1 == 0) weight = 0.08;
									  if (edgevalue1 != 0) weight = 1.0;
									}										
								prior[k]+=weight*(double)abs(xttemp[0][i-1][j]-k);
							}

						 diff[k]+=diff_sq[k][i][j]; //neighbors difference       left	

						 if (j-1 >=0) (diff[k]+=diff_sq[k][i-1][j-1]); //neighbors difference    left top     

						 if (j+1 <= cols-1) (diff[k]+=diff_sq[k][i-1][j+1]); //neighbors difference   left bottom
						}

				    //**************************************************************************//
					if (i+1 <= rows-1) 
					    {					
						  if (xttemp[0][i+1][j] != k) 
							{
								texture1=cvGetReal2D(texture, i+1, j);	
								if (texture1==0 )
									{
									  alpha=0;
									  weight=10;
									  if (abs(xttemp[0][i+1][j]-xttemp[0][i][j])<1) cvSetReal2D(texture, i, j,0);
									}
								else 
									{
									  edgevalue1=cvGetReal2D(highpass, i+1, j); 
									  if (edgevalue1 == 0) weight = 0.08;
									  if (edgevalue1 != 0) weight = 1.0;
									}
								prior[k]+=weight*(double)abs(xttemp[0][i+1][j]-k);
							}
									
						  if (j-1 >=0) (diff[k]+=diff_sq[k][i+1][j-1]); //neighbors difference

						  diff[k]+=diff_sq[k][i+1][j]; //neighbors difference
						  if (j+1 <= cols-1) (diff[k]+=diff_sq[k][i+1][j+1]); //neighbors difference//////////////////////////////////////error
						}

					//**************************************************************************//
					if (j-1 >=0) 
					  {
						if (xttemp[0][i][j-1] != k) 
						 {
							texture1=cvGetReal2D(texture, i, j-1);	
							if (texture1==0 ) 
							  {
								alpha=0;
								weight=10;
								if (abs(xttemp[0][i][j-1]-xttemp[0][i][j])<1) cvSetReal2D(texture, i, j,0);
							  }
							else 
							  {
								edgevalue1=cvGetReal2D(highpass, i, j-1); 
								if (edgevalue1 == 0) weight = 0.08;
								if (edgevalue1 != 0) weight = 1.0;
							  }
								prior[k]+=weight*(double)abs(xttemp[0][i][j-1]-k);
						  }
									
						diff[k]+=diff_sq[k][i][j-1]; //neighbors difference
					  }

					//**************************************************************************//
					if (j+1 <= cols-1) 
					  {
						if (xttemp[0][i][j+1] != k) 
						 {
							texture1=cvGetReal2D(texture, i, j+1);	
							if (texture1==0 ) 
							  {
								alpha=0;
								weight=10;
								if( abs(xttemp[0][i][j+1]-xttemp[0][i][j])<1) cvSetReal2D(texture, i, j,0);
							  }
							else 
							  {
								edgevalue1=cvGetReal2D(highpass, i, j+1); 
								if (edgevalue1 == 0) weight = 0.08;
								if (edgevalue1 != 0) weight = 1.0;
							  }
							prior[k]+=weight*(double)abs(xttemp[0][i][j+1]-k);
						 }
									
						 diff[k]+=diff_sq[k][i][j+1]; //neighbors difference
					   }		
				 }
			}


//////////// combine by Bayes rule log p(X|Y) = log(p(Y|X)  + p(X)  + Gamma , using attenuation Function call //////////////////////////////////////////////////////////////////////////////
      for (k=1; k<=classes; k++)
          {
			if (texturevalue == 0 & edgevalue == 0 ) beta = 0.1;   // more texture region and pixel not on edge
			if (texturevalue == 0 & edgevalue != 0 ) beta = 0.01;	// more texture region and pixel  on edge
			if (texturevalue == 255 & edgevalue == 0 ) beta = 1.5; // less texture region and pixel not on edge
			if (texturevalue == 255 & edgevalue != 0 ) beta = 0.01; // less texture region and pixel  on edge

            /* calculating gamma term */
	//	    if (atlas[0][i][j] == k)   gamma[0][i][j] = 0;
    //        if (atlas[0][i][j] != k)   gamma[0][i][j] = 1;

//			if (prior[k] < 0) (prior[k]=0); assist =  gama * gamma[0][i][j];
			/* Switch for atlas method */
			if (ATLAS==1)
                 logpost[k] = diff[k] + beta*(double)(prior[k]) + assist;
            if (ATLAS==0)
                logpost[k] = 3*(diff[k]) ;
				//logpost[k] = (con[k] + alpha*(diff[k]/d[k])) ;
				//;+ beta*(double)(prior[k]);
	      }

 return *logpost;
}


///////////////////////////////////////////////////////////////////////////////////
//																				 //
//    Part 3:																	 //
//			Revised MAP Estimation Function                                      //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////



void map3(double **y[], double **xt[], double **diff_y[], double **diff_Cr[], double **diff_Cb[], double **atlas[], double beta,double gama, int ATLAS, int ICM, int cols, int rows, int classes, int map_iter, double *v, double *yaccum, double *ysquaredaccum,double *Num,unsigned char **xttemp[],unsigned char **xttempCr[],unsigned char **xttempCb[],IplImage*texture,IplImage*textureCb,IplImage*textureCr,IplImage*highpass,IplImage*highpassCr,IplImage*highpassCb)////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
//	gamma[0] = (unsigned char **)get_img(cols,rows,sizeof(unsigned char));
	double logpostCr[MAX_CLASSES+1], logpostCb[MAX_CLASSES+1],logposty[MAX_CLASSES+1];
	double **logpost1[MAX_CLASSES+1];
	for ( int dd = 1; dd <= MAX_CLASSES; dd++)
	{
		logpost1[dd] = (double **)get_img(cols,rows,sizeof(double));
		
	}
	//int **logpost1[MAX_CLASSES] = new int[MAX_CLASSES][600][600];

	xrv= (double **)get_img(cols,rows,sizeof(double));
	diff[0]=0;
	//logpost[0]=0;
	prior[0]=0;


	//readseed();

////////////////////////////////////////////////////////////////////////////////

	double sqrt2pi,con[MAX_CLASSES+1],d[MAX_CLASSES+1];
	double mean[MAX_CLASSES+1], var[MAX_CLASSES+1];

	/*  constant  */
	sqrt2pi = sqrt(2.0*PI);

	/*  constants for each class due to variance  */
	for (k=1; k<=classes; k++)
		{
			if (v[k]<0.025) v[k]=0.025;
			else con[k] = log(sqrt2pi*sqrt(v[k]));

			if (con[k] < 0) con[k] = 0;
			else d[k] = 2.0*v[k];

	/*		for (i=0; i<rows; i++)
				for (j=0; j<cols; j++)
					{	
						diff_y[k][i][j] = diff_y[k][i][j]/d[k];
						diff_Cr[k][i][j] = diff_Cr[k][i][j]/d[k];
						diff_Cb[k][i][j] = diff_Cb[k][i][j]/d[k];
					 }
*/

		}
	/* initialize accumulation registers */
	for (k=1; k<=classes; k++) 
		{
			mean[k]=0;
			var[k]=0;
			yaccum[k]=0;
			ysquaredaccum[k]=0;
			Num[k]=0;
		}

///////////////////////////////////////////////////////////////////////////////////////

    /* Map loop */
   for (l=0; l<map_iter; l++)
    {
	   AveCost = 0; 

	 	/* Initialize random variable array */
	   for (i=0; i<rows; i++)
		  for (j=0; j<cols; j++)
		     {	
				xrv[i][j] = random2();
		     }

		//**************************************************************************//
		// Begin calculation pixel by pixel
	    for (i=0; i<rows; i++)
           for (j=0; j<cols; j++)
            {

		//**************************************************************************//
		// Calculating diff temm 
			    mm=(double)y[1][i][j];

/////////////////////////////////   read in texture and edge information for each pixel ////////////////////////////
				texturevalue=cvGetReal2D(texture, i, j);
				texturevalueCr=cvGetReal2D(textureCr, i, j);
				texturevalueCb=cvGetReal2D(textureCb, i, j);

				if(texturevalue==255) beta=0.1; //    texture
				if(texturevalue==0) beta=0.01;
           
				edgevalue=cvGetReal2D(highpass, i, j);
				*logposty=callogpost(diff_y,atlas,xttemp,cols, rows,  ATLAS,  gama, classes,  beta,  d, con,logposty,edgevalue,texturevalue,highpass,texture);
				
				edgevalue=cvGetReal2D(highpassCr, i, j);
				*logpostCr=callogpost(diff_Cr,atlas,xttempCr,cols, rows,  ATLAS,  gama, classes,  beta,  d, con, logpostCr,edgevalue,texturevalueCr,highpassCr,textureCr);
							
				edgevalue=cvGetReal2D(highpassCb, i, j);
				*logpostCb=callogpost(diff_Cb,atlas,xttempCb,cols, rows,  ATLAS,  gama, classes,  beta,  d, con,logpostCb,edgevalue,texturevalueCb,highpassCb,textureCb);

				for (k=1; k<=MAX_CLASSES; k++)
				  { 
					logpost1[k][i][j]=(logposty[k]+logpostCr[k]+logpostCb[k]);	
				  }
               
                /*  find min of logpost[k] */
/*
                if (ICM == 1)
                {
                    tempmin = 1;
                    for (k=1; k<=classes; k++)
                    {
                        if ((logpost[tempmin])>(logpost[k])) tempmin=k;
                    }
                    xttemp[0][i][j]=tempmin;
					xttempCr[0][i][j]=tempmin;
					xttempCb[0][i][j]=tempmin;                  
                }
                else 
				{	
					if (ICM == 0) invannealtemp = log((float)(l+2))/3.0;
					if (ICM == 2) invannealtemp = 1.0;
					sum=0;
					for (k=1; k<=classes; k++) 
					{
						compare[k] = exp(-invannealtemp*logpost[k]*10);
						sum += compare[k];
					}

					while (sum==0)
					{
					  for (k=1; k<=classes; k++) 
						{	
						  compare[k] = exp(-invannealtemp*logpost[k]/100);
						  sum += compare[k];
						}		
					}

					current = 0;

					for (k=1; k<=classes; k++)
					{
						ratio=(compare[k])/sum;
					
						if (((xrv[i][j])>=current)&&((xrv[i][j])<=(current + ratio)))
						{
							xttemp[0][i][j] = k;
							xttempCr[0][i][j] = k;
							xttempCb[0][i][j] = k;
						}
			
                        current += ratio;
					}
				}
*/
		//	# pragma omp critical 
		//				{
		//					yaccum[(xttemp[0][i][j])] += mm;					/* for mean EM calculation*/ 
		//					ysquaredaccum[(xttemp[0][i][j])] += (mm)*(mm);		/* for variance EM caculation */
		//					Num[(xttemp[0][i][j])] += 1;						/* for mean & variance EM calculation*/ 										
		//				}				

	//		AveCost += logpost[xttemp[0][i][j]];
		} 
		   int num_pixels = cols * rows;
			int *result = new int[num_pixels]; 
		    double start= (double)cvGetTickCount();
        
     /*
        CvMat* testmat=(CvMat*)cvLoad("/Users/ChaoLiu/Pictures/Depth_from_Defocus_Database/Temp_data_for_program/shapeCost.xml");
        
        for ( int i = 0; i < rows; i++ )
            for (int j = 0; j < cols; j++ )
                for (int k = 1; k < MAX_CLASSES+1; k++)
                {
                    logpost1[k][i][j]=cvGetReal2D(testmat,i*cols+j,k);
                    
                }
      */  
        
                
    

        
        
		   GridGraph_DArraySArray(cols,rows,MAX_CLASSES,logpost1,result);
			//GeneralGraph_DArraySArray(cols,rows,MAX_CLASSES,logpost1,result);
		   double end= (double)cvGetTickCount();//���½�����ʱ�Ӽ���  
			double  t1= (end-start)/((double)cvGetTickFrequency()*1000.);//��������ʱ�䣬�Ժ���Ϊ��λ  
		printf( "Run time without OpenMP = %g ms\r\n", t1 );
	//**************************************************************************//
		//	std::cout<<"Total cost for iteration #"<<l<<" is : "<<AveCost<<"\n";
for ( int dd = 1; dd <= MAX_CLASSES; dd++)
	free_img((void **)logpost1[dd]);

		int tttt=0;
    for (i=0; i<rows; i++)
        for (j=0; j<cols; j++)
        {
            //xt[0][i][j] = unsigned(255.0-((256.0/MAX_CLASSES)*(xttemp[0][i][j]-1.0)));
			xt[0][i][j] = unsigned(255.0-((256.0/MAX_CLASSES)*(result[tttt]-1.0)));
			xttemp[0][i][j]=result[tttt];
			tttt++;
        } 

for (i=0; i<rows; i++)
        for (j=0; j<cols; j++)
        {
		yaccum[(xttemp[0][i][j])] += (double)y[1][i][j];					/* for mean EM calculation*/ 
		ysquaredaccum[(xttemp[0][i][j])] += ((double)y[1][i][j])*((double)y[1][i][j]);		/* for variance EM caculation */
		Num[(xttemp[0][i][j])] += 1;						/* for mean & variance EM calculation*/ 
		}


		delete [] result;
    }

}



////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
void GridGraph_DArraySArray(int width, int height, int num_labels, double **logpost1[], int *result)
{
	int num_pixels = width * height;
	//int *result = new int[num_pixels];   // stores result of optimization

	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	/*
	for ( int i = 0; i < num_pixels; i++ )
	for (int l = 0; l < num_labels; l++ )
	data[i] = 0;

	if (i < 25 ){
	if(  l == 1 ) data[i*num_labels+l] = 0;
	else data[i*num_labels+l] = 10;
	}
	else {
	if(  l == 4 ) data[i*num_labels+l] = 0;
	else data[i*num_labels+l] = 10;
	}
	*/
	int t = 0;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			for (int k = 1; k < num_labels + 1; k++)
			{
				data[t] = logpost1[k][i][j];
				t++;
			}


	// next set up the array for smooth costs
	int *smooth = new int[num_labels*num_labels];
	for (int l1 = 0; l1 < num_labels; l1++)
		for (int l2 = 0; l2 < num_labels; l2++)
			//smooth[l1+l2*num_labels] = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;
			smooth[l1 + l2*num_labels] = abs(l1 - l2);

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width, height, num_labels);


		gc->setDataCost(data);


		gc->setSmoothCost(smooth);



		printf("\nBefore optimization energy is %d\n", gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d\n", gc->compute_energy());

		for (int i = 0; i < num_pixels; i++)
		{
			result[i] = gc->whatLabel(i);
			//cout<<result[i]<<endl;
		}

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	//delete [] result;
	delete[] smooth;
	delete[] data;

}