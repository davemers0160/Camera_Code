
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <stdio.h>
#include <cstdlib>
#include <pthread.h>

#include "ftd2xx.h"
#include "GPS_Ctrl.h"

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	#include <windows.h>
#else
	#include <unistd.h>
	//#include <time.h>
	#include <sys/stat.h>
#endif

using namespace std;



//void logGPSData(GPS_Thread_Vars *GPS_Ctrl_Info)
void *logGPSData(void *args)
{
	ofstream gpsDataLog;
	GPS_info GPS_data;

	GPS_Thread_Vars *GPS_Ctrl_Info = (GPS_Thread_Vars *)(args);



	while(quit_GPS_Logging == false)
	{

		//for (int idx = 0; idx < 60; idx++)
		//{

			getGPSInfo(GPS_Ctrl_Info->GPS_Handle, &GPS_data);

			//std::cout << "Time: " << setfill('0') << setw(2) << GPS_data.hour << ":" << setfill('0') << setw(2) << GPS_data.minute << ":" << fixed << setfill('0') << setw(4) << setprecision(1) << GPS_data.second << "\t";
			//std::cout << "Latitude: " << std::fixed << std::setprecision(6) << GPS_data.Latitude << "\t";
			//std::cout << "Longitude: " << std::fixed << std::setprecision(6) << GPS_data.Longitude << endl;

			GPS_Ctrl_Info->gpsDataLog << "Time: " << setfill('0') << setw(2) << GPS_data.hour << ":" << setfill('0') << setw(2) << GPS_data.minute << ":" << std::fixed << setfill('0') << setw(4) << setprecision(1) << GPS_data.second << ",";
			GPS_Ctrl_Info->gpsDataLog << " Latitude: " << std::fixed << std::setprecision(6) << GPS_data.Latitude << ",";
			GPS_Ctrl_Info->gpsDataLog << " Longitude: " << std::fixed << std::setprecision(6) << GPS_data.Longitude << endl;

		//}

	}

	pthread_exit(NULL);

}	// end of logGPSData



void configGPS(FT_HANDLE GPS_Handle)
{
	unsigned int dwBytesWritten;
	ULONG ft_write_Status;

	// message checksum is bitwise XOR between $ and *; PSRF103 = 0x25;
	// disable GSV
	//char GSV[] = "$PSRF103,03,00,00,01*27\r\n";

	// disable RMC
	//char RMC[] = "$PSRF103,04,00,00,01*20\r\n";

	// disable GSA
	//char GSA[] = "$PSRF103,02,00,00,01*26\r\n";

	// disable GGA
	//char GGA[] = "$PSRF103,00,00,00,01*24\r\n";

	char PMTK_Config[] = "$PMTK314,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0*2D\r\n";	//47
	
	// this section computes the checksum value for a standad NMEA GPS message
	// since the message is precomputed the code is currently not needed but left for
	// referrence
	//char cc = 0;
	//for (int idx = 1; idx < 42; idx++)
	//{
	//	cc ^= PMTK_Config[idx];
	//}
	//PMTK_Config[45] = (cc>4)&0x0F;
	//PMTK_Config[46] = (cc & 0x0F);

	ft_write_Status = FT_Write(GPS_Handle, PMTK_Config, 47, &dwBytesWritten);


}	// end of configGPS


void getGPSInfo(FT_HANDLE GPS_Handle, GPS_info *GPS_data)
{
	unsigned int BytesRead;
	char rx_data[96] = { 0 };
	//OVERLAPPED osReader = { 0 };
	//BOOL comm_result;
	ULONG ft_read_Status;
	double tempLatDD, tempLatMM, tempLongDD, tempLongMM;
	unsigned int tempLatInt, tempLongInt;

	FT_Purge(GPS_Handle, FT_PURGE_RX | FT_PURGE_TX);

	ft_read_Status = FT_Read(GPS_Handle, rx_data, 72, &BytesRead);
	if (ft_read_Status == FT_OK)
	{
		string gps_str = (string)rx_data;
		vector<string> vect;
		string temp;

		stringstream ss(gps_str);

		while (ss.good())
		{
			string substr;
			getline(ss, substr, ',');
			vect.push_back(substr);

		}

		
		//if ((vect.size() > 10) && (vect[0][0] == '$'))
		if ((vect.size() > 10) && (vect[0] == "$GPGGA"))
		{
			
			//GPS_data->Latitude = atof(vect[2].c_str());
			tempLatDD = (atof(vect[2].c_str())/100.0);
			tempLatInt = (int)(tempLatDD);
			tempLatMM = (tempLatDD - tempLatInt)*100;

			GPS_data->Latitude = (double)tempLatInt + (tempLatMM / 60.0);
			if (vect[3] == "S")
			{
				GPS_data->Latitude = -GPS_data->Latitude;
			}
			
			//GPS_data->Longitude = atof(vect[4].c_str());
			tempLongDD = (atof(vect[4].c_str()) / 100.0);
			tempLongInt = (int)(tempLongDD);
			tempLongMM = (tempLongDD - tempLongInt) * 100;

			GPS_data->Longitude = (double)tempLongInt + (tempLongMM / 60.0);
			if (vect[5] == "W")
			{
				GPS_data->Longitude = -GPS_data->Longitude;
			}

			GPS_data->hour = atoi(vect[1].substr(0, 2).c_str());	//atoi(vect[1][0]) * 10 + vect[1][1];
			GPS_data->minute = atoi(vect[1].substr(2, 2).c_str());	//vect[1][2] * 10 + vect[1][3];
			GPS_data->second = atof(vect[1].substr(4, 6).c_str());	//vect[1][4] * 10 + vect[1][5];
		}
		else
		{
			cout << rx_data;
		}


	}

}	// end of getGPSInfo

