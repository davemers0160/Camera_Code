

#ifndef CHAMELEON_UTILITIES_H
#define CHAMELEON_UTILITIES_H

#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace FlyCapture2;
using namespace std;

typedef struct
{
	int FrameCount;
	string FileName;
	VideoWriter VideoFile;
	Mat VideoFrame;

} videoSaveStruct;

void getcurrenttime(char currenttime[]);
void PrintError(FlyCapture2::Error error);
void PrintBuildInfo(void);
void PrintCameraInfo(CameraInfo* pCamInfo);
void cameraConnect(PGRGuid guid, Camera *cam);
FlyCapture2::Error configImagerFormat(Camera *cam, unsigned int offsetX, unsigned int offsetY, unsigned int width, unsigned int height, PixelFormat pixelFormat);
FlyCapture2::Error configCameraPropeties(Camera *cam, int *sharpness, float *shutter, float *gain, float *brightness, float *auto_exp, float fps);
void configProperty(Camera *cam, Property &prop, PropertyType type, bool AutoMode, bool OnOff, bool absControl);
FlyCapture2::Error setProperty(Camera *cam, Property &prop, float value);
FlyCapture2::Error setProperty(Camera *cam, Property &prop);
int getProperty(Camera *cam, Property &prop);
float getABSProperty(Camera *cam, Property &prop);
FlyCapture2::Error configCameraPropeties(Camera *cam, int *sharpness, float *shutter, float *gain, float fps);
FlyCapture2::Error Camera_PowerOff(Camera *cam);
FlyCapture2::Error Camera_PowerOn(Camera *cam);
FlyCapture2::Error setSoftwareTrigger(Camera *cam, bool onOff);
bool PollForTriggerReady(Camera *cam);
bool FireSoftwareTrigger(Camera *cam);
void *saveVideo_t(void *args );
void *saveBinVideo_t(void *args );

void sleep_ms(int value);
void mkDir(string directory_path, string new_folder);


#endif
