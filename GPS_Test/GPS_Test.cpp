#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <stdio.h> 
#include <windows.h> 
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>

#include "ftd2xx.h"


using namespace std;

typedef struct
{
	int hour, minute;
	float second;
	float Lat;
	float Long;
	float Speed;

} GPS_info;

struct ftdiDeviceDetails //structure storage for FTDI device details
{
	int devNumber;
	unsigned long type;
	unsigned long BaudRate;
	string Description;
	string serialNumber;
};

void configGPS(FT_HANDLE GPS_Handle);
void getGPSInfo(FT_HANDLE GPS_Handle, GPS_info &GPS_data);
FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript);
void getcurrenttime(char currenttime[]);

int main(int argc, char ** argv)
{
	unsigned int idx;
	
	ftdiDeviceDetails gpsDeviceDetails;
	FT_HANDLE GPS_Handle = NULL;
	unsigned long BytesRead, BytesSent;

	GPS_info GPS_data;
	string gpsSaveFile;
	char currenttime[80];
	ofstream gpsDataLog;


	GPS_data.hour = 0;
	GPS_data.minute = 0;
	GPS_data.second = 0.0;
	GPS_data.Lat = 0.0;
	GPS_data.Long = 0.0;




	if (GPS_Handle == NULL)
	{
		gpsDeviceDetails.devNumber = 0;
		gpsDeviceDetails.type = 0;
		gpsDeviceDetails.BaudRate = 57600;
		gpsDeviceDetails.Description = "";
		gpsDeviceDetails.serialNumber = "";

		GPS_Handle = OpenComPort(&gpsDeviceDetails, "LOCOSYS GPS MC-1513");
		//FTDI manual says that the buffer goes up to 64 KB
		//SetupComm(lensDriver, 16 * 4 * 1024, 4 * 1024);
		if (GPS_Handle == NULL)
		{
			exit(0);
		}
		else
		{
			//print device info		
			//cout << "Device[" << driverDeviceDetails.devNumber << "] " << driverDeviceDetails.Description << ", SN: " << driverDeviceDetails.serialNumber << endl;
		}

	}

	getcurrenttime(currenttime);
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
	gpsSaveFile = "GPS_Log_" + (string)currenttime + ".txt";
#else
	gpsSaveFile = "/media/odroid/Videos/GPS_Log_" + (string)currenttime + ".txt";
#endif
	
	gpsDataLog.open(gpsSaveFile, ios::out);



	// clear FTDI buffer
	FT_Purge(GPS_Handle, FT_PURGE_RX | FT_PURGE_TX);

	//string temp = "$GPGGA,044500.000,3914.4270,N,08636.7004,W,1,8,1.02,228.2,M,-33.6,M,,*67";

	//configGPS(GPS_Handle);

	for (int idx = 0; idx < 20; idx++)
	{
		//ReadFile(GPS_Handle, rx_data, 79, &dwRead, &osReader);
		getGPSInfo(GPS_Handle, GPS_data);
		//cout << rx_data;

		cout << "Time: " << setfill('0') << setw(2) << GPS_data.hour << ":" << setfill('0') << setw(2) << GPS_data.minute << ":" << fixed << setfill('0') << setw(4) << setprecision(1) << GPS_data.second << "\t";
		cout << "Latitude: " << std::fixed << std::setprecision(6) << GPS_data.Lat << "\t";
		cout << "Longitude: " << std::fixed << std::setprecision(6) << GPS_data.Long << endl;

		gpsDataLog << "Time: " << setfill('0') << setw(2) << GPS_data.hour << ":" << setfill('0') << setw(2) << GPS_data.minute << ":" << fixed << setfill('0') << setw(4) << setprecision(1) << GPS_data.second << ",";
		gpsDataLog << "Latitude: " << std::fixed << std::setprecision(6) << GPS_data.Lat << ",";
		gpsDataLog << "Longitude: " << std::fixed << std::setprecision(6) << GPS_data.Long << endl;

	}

	FT_Close(GPS_Handle);
	GPS_Handle = NULL;
	gpsDataLog.close();
	cout << "Press any key to continue..." << endl;
	cin.ignore();

	return 0;
}

void configGPS(FT_HANDLE GPS_Handle)
{
	unsigned long dwBytesWritten;
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
	char cc = 0;
	for (int idx = 1; idx < 42; idx++)
	{
		cc ^= PMTK_Config[idx];
	}
	//PMTK_Config[45] = (cc>4)&0x0F;
	//PMTK_Config[46] = (cc & 0x0F);

	ft_write_Status = FT_Write(GPS_Handle, PMTK_Config, 48, &dwBytesWritten);
	//ft_write_Status = FT_Write(GPS_Handle, RMC, 25, &dwBytesWritten);
	//ft_write_Status = FT_Write(GPS_Handle, GSA, 25, &dwBytesWritten);
	//WriteFile(GPS_Handle, GGA, 26, &dwBytesWritten, NULL);

}

void getGPSInfo(FT_HANDLE GPS_Handle, GPS_info &GPS_data)
{
	unsigned long BytesRead;
	char rx_data[96] = { 0 };
	//OVERLAPPED osReader = { 0 };
	//BOOL comm_result;
	ULONG ft_read_Status;

	FT_Purge(GPS_Handle, FT_PURGE_RX | FT_PURGE_TX);

	ft_read_Status = FT_Read(GPS_Handle, rx_data, 72, &BytesRead);
	if (ft_read_Status == FT_OK)
	{
		string gps_str = (string)rx_data;
		vector<string> vect;
		string temp;

		stringstream ss(gps_str);
		//while (ss >> temp)
		//{
		//	vect.push_back(temp);

		//	if (ss.peek() == ',')
		//		ss.ignore();
		//}
		while (ss.good())
		{
			string substr;
			getline(ss, substr, ',');
			vect.push_back(substr);

		}

		
		//if ((vect.size() > 10) && (vect[0][0] == '$'))
		if ((vect.size() > 10) && (vect[0] == "$GPGGA"))
		{
			
			GPS_data.Lat = stof(vect[2]);
			if (vect[3] == "S")
			{
				GPS_data.Lat = -GPS_data.Lat;
			}
			
			GPS_data.Long = stof(vect[4]);
			if (vect[5] == "W")
			{
				GPS_data.Long = -GPS_data.Long;
			}

			GPS_data.hour = atoi(vect[1].substr(0, 2).c_str());	//atoi(vect[1][0]) * 10 + vect[1][1];
			GPS_data.minute = atoi(vect[1].substr(2, 2).c_str());	//vect[1][2] * 10 + vect[1][3];
			GPS_data.second = atof(vect[1].substr(4, 6).c_str());	//vect[1][4] * 10 + vect[1][5];
		}
		else
		{
			cout << rx_data;
		}


	}

}	// end of main


FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript)
{
	FT_HANDLE ftHandle = NULL;
	FT_HANDLE ftHandleTemp;
	FT_DEVICE_LIST_INFO_NODE devInfo[32];
	DWORD numDevices = 0;
	int dev_number, found, i;
	DWORD Flags;
	DWORD ID;
	DWORD Type;
	DWORD LocId;
	char SerialNumber[16];
	char Description[64];
	LONG lComPortNumber;

	ftHandle = NULL;
	dev_number = 0;
	found = 0;

	// search for devices connected to the USB port
	FT_CreateDeviceInfoList(&numDevices);

	if (numDevices > 0)
	{
		if (FT_GetDeviceInfoList(devInfo, &numDevices) == FT_OK)
		{
			while ((dev_number < (int)numDevices) && !found)
			{
				if (FT_GetDeviceInfoDetail(dev_number, &Flags, &Type, &ID, &LocId, SerialNumber, Description, &ftHandleTemp) == FT_OK)
				{

					if ((string)Description == descript)
					{
						device->devNumber = dev_number;
						device->type = Type;
						device->Description = (string)Description;
						device->serialNumber = (string)SerialNumber;
						found = 1;
					}
				}
				if (!found)
					dev_number++;
			}
		}
	}
	else
	{
		cout << "No devices found." << endl;
		return ftHandle;
	}

	if (found)
	{
		if (FT_OpenEx((void *)device->Description.c_str(), FT_OPEN_BY_DESCRIPTION, &ftHandle) == FT_OK)
		{

			if (FT_SetBaudRate(ftHandle, device->BaudRate) != FT_OK)
				//			if (FT_SetBaudRate(ftHandle, 230400l) != FT_OK)
				//printf("ERROR: Baud rate not supported\n");

				FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
			FT_SetTimeouts(ftHandle, 5000, 100);
			if (FT_GetComPortNumber(ftHandle, &lComPortNumber) == FT_OK)
			{
				if (lComPortNumber == -1) // No COM port assigned } 
					printf("No Comm Port assigned device found!\n");
				else
					printf("FTDI device found on COM:%d\n", lComPortNumber);
			}
		}
	}
	else
	{
		cout << "No valid FTDI device found!" << endl;
	}

	return ftHandle;
}	// end of OpenComPort

void getcurrenttime(char currenttime[])
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(currenttime, 80, "%m%d%Y_%H%M%S", timeinfo);
	string str(currenttime);
	//cout << currenttime << endl;
	//cout << str << endl;

	//return 0;
}

