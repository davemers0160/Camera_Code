

#ifndef CHAMELEON_UTILITIES_H
#define CHAMELEON_UTILITIES_H

using namespace FlyCapture2;

void cameraConnect(PGRGuid guid, Camera *cam);
void configImagerFormat(Camera *cam, unsigned int offsetX, unsigned int offsetY, unsigned int width, unsigned int height, PixelFormat pixelFormat);
void PrintError(FlyCapture2::Error error);
void PrintCameraInfo(CameraInfo* pCamInfo);
void configProperty(Camera *cam, Property &prop, PropertyType type, bool AutoMode, bool OnOff, bool absControl);
FlyCapture2::Error setProperty(Camera *cam, Property &prop, float value);
FlyCapture2::Error setProperty(Camera *cam, Property &prop);
int getProperty(Camera *cam, Property &prop);
float getABSProperty(Camera *cam, Property &prop);
FlyCapture2::Error configCameraPropeties(Camera *cam, int *sharpness, float *shutter, float *gain, float fps);

#endif
