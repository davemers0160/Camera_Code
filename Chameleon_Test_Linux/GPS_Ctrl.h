
#ifndef GPS_CTRL_H
#define GPS_CTRL_H

#include "ftd2xx.h"


using namespace std;



typedef struct
{
	int hour, minute;
	float second;
	float Latitude;
	float Longitude;
	float Speed;

} GPS_info;

typedef struct
{
	FT_HANDLE GPS_Handle;
	ofstream gpsDataLog;
	string filename;
} GPS_Thread_Vars;

extern volatile bool quit_GPS_Logging;

//void logGPSData(GPS_Thread_Vars *args);
void *logGPSData(void *args);

void configGPS(FT_HANDLE GPS_Handle);
void getGPSInfo(FT_HANDLE GPS_Handle, GPS_info *GPS_data);


#endif	// GPS_CTRL_H
