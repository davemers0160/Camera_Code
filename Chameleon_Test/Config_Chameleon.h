

#ifndef CHAMELEON_CONFIG_H
#define CHAMELEON_CONFIG_H

using namespace FlyCapture2;

void cameraConnect(PGRGuid guid, Camera &cam);
void configImagerFormat(Camera &cam, unsigned int offsetX, unsigned int offsetY, unsigned int width, unsigned int height, PixelFormat pixelFormat);
void PrintError(Error error);
void PrintCameraInfo(CameraInfo* pCamInfo);

#endif
