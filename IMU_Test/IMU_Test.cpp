#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <stdio.h> 
//#include <windows.h>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>


#include "ftd2xx.h"


using namespace std;

typedef struct
{

	float x;
	float y;
	float z;
	float xyz;
	float sqr,srt;

}

IMU_info;

struct ftdiDeviceDetails //structure storage for FTDI device details
{
	int devNumber;
	unsigned long type;
	unsigned long BaudRate;
	string Description;
	string serialNumber;
};

void configIMU(FT_HANDLE IMU_Handle);
void getIMUInfo(FT_HANDLE IMU_Handle, IMU_info *IMU_data);
FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript);
void getcurrenttime(char currenttime[]);

int main(int argc, char ** argv)
{
	unsigned int idx;
	
	ftdiDeviceDetails IMUDeviceDetails;
	FT_HANDLE IMU_Handle = NULL;
	//unsigned long BytesRead, BytesSent;

	IMU_info IMU_data;
	string IMUSaveFile;
	char currenttime[80];
	ofstream IMUDataLog;


	IMU_data.x = 0;
	IMU_data.y = 0;
	IMU_data.z = 0.0;
	IMU_data.sqr = 0.0;
	IMU_data.srt = 0.0;




	if (IMU_Handle == NULL)
	{
		IMUDeviceDetails.devNumber = 0;
		IMUDeviceDetails.type = 0;
		IMUDeviceDetails.Description = "";
		IMUDeviceDetails.serialNumber = "";

		IMU_Handle = OpenComPort(&IMUDeviceDetails, "RAZOR-9DOF");
		//FTDI manual says that the buffer goes up to 64 KB
		//SetupComm(lensDriver, 16 * 4 * 1024, 4 * 1024);
		if (IMU_Handle == NULL)
		{
			return 1;
		}
		else
		{
			//print device info		
			//cout << "Device[" << driverDeviceDetails.devNumber << "] " << driverDeviceDetails.Description << ", SN: " << driverDeviceDetails.serialNumber << endl;
		}

	}

	getcurrenttime(currenttime);
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
	IMUSaveFile = "IMU_Log_" + (string)currenttime + ".txt";
#else
	IMUSaveFile = "/home/odroid/Videos/IMU_Log_" + (string)currenttime + ".txt";
#endif
	


	cout << "Log File Location: " << endl;
	cout << IMUSaveFile << endl;


	// clear FTDI buffer
	FT_Purge(IMU_Handle, FT_PURGE_RX | FT_PURGE_TX);

	//string temp = "$GPGGA,044500.000,3914.4270,N,08636.7004,W,1,8,1.02,228.2,M,-33.6,M,,*67";

	//configIMU(IMU_Handle);

	while(1)
	{
		IMUDataLog.open(IMUSaveFile.c_str(), ios::out | ios::app);

		for (int idx = 0; idx < 60; idx++)
		{
			//ReadFile(IMU_Handle, rx_data, 79, &dwRead, &osReader);
			getIMUInfo(IMU_Handle, &IMU_data);
			//cout << rx_data;

			cout << "xyz: " << setfill('0') << setw(4) << IMU_data.x << ":" << setfill('0') << setw(4) << IMU_data.y << ":" << fixed << setfill('0') << setw(4) << IMU_data.z << "\t";
			//cout << "sqr: " << std::fixed << std::setprecision(6) << IMU_data.sqr << "\t";
			cout << "\tsrt: " << std::fixed << std::setprecision(6) << IMU_data.srt << endl;

			IMUDataLog << "xyz: " << setfill('0') << setw(2) << IMU_data.x << ":" << setfill('0') << setw(2) << IMU_data.y << ":" << fixed << setfill('0') << setw(4) << setprecision(1) << IMU_data.z << ",";
			//IMUDataLog << "sqr: " << std::fixed << std::setprecision(6) << IMU_data.sqr << ",";
			IMUDataLog << "srt: " << std::fixed << std::setprecision(6) << IMU_data.srt << endl;

		}

		IMUDataLog.close();

	}






	FT_Close(IMU_Handle);
	IMU_Handle = NULL;
	IMUDataLog.close();
	cout << "Press any key to continue..." << endl;
	cin.ignore();

	return 0;
}

void configIMU(FT_HANDLE IMU_Handle)
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
	char cc = 0;
	for (int idx = 1; idx < 42; idx++)
	{
		cc ^= PMTK_Config[idx];
	}
	//PMTK_Config[45] = (cc>4)&0x0F;
	//PMTK_Config[46] = (cc & 0x0F);

	ft_write_Status = FT_Write(IMU_Handle, PMTK_Config, 48, &dwBytesWritten);
	//ft_write_Status = FT_Write(IMU_Handle, RMC, 25, &dwBytesWritten);
	//ft_write_Status = FT_Write(IMU_Handle, GSA, 25, &dwBytesWritten);
	//WriteFile(IMU_Handle, GGA, 26, &dwBytesWritten, NULL);

}

void getIMUInfo(FT_HANDLE IMU_Handle, IMU_info *IMU_data)
{
	unsigned int BytesRead;
	char rx_data[96] = { 0 };

	//OVERLAPPED osReader = { 0 };
	//BOOL comm_result;
	ULONG ft_read_Status;

	FT_Purge(IMU_Handle, FT_PURGE_RX | FT_PURGE_TX);

	ft_read_Status = FT_Read(IMU_Handle, rx_data, 13, &BytesRead);
	if (ft_read_Status == FT_OK)
	{
		string IMU_str = (string)rx_data;
		vector<string> vect;
		string temp;

		stringstream ss(IMU_str);
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


		if (vect.size() == 3)
		{

			IMU_data->x = atof(vect[0].c_str());
			IMU_data->y = atof(vect[1].c_str());
			IMU_data->z = atof(vect[2].c_str());
			IMU_data->sqr=IMU_data->y*IMU_data->y+IMU_data->z*IMU_data->z;
			IMU_data->srt=sqrt(IMU_data->sqr);
			//cout << IMU_data->srt << endl;
			//IMU_data->Latitude = atof(vect[2].c_str());
			//if (vect[3] == "S")
			//{
			//	IMU_data->Latitude = -IMU_data->sqr;
			//}
			
			//IMU_data->Longitude = atoi(vect[4].c_str());
			//if (vect[5] == "W")
			//{
			//	IMU_data->Longitude = -IMU_data->srt;
			//}

			//IMU_data->x = atoi(vect[1].substr(0, 2).c_str());	//atoi(vect[1][0]) * 10 + vect[1][1];
			//IMU_data->y = atoi(vect[1].substr(2, 2).c_str());	//vect[1][2] * 10 + vect[1][3];
			//IMU_data->z = atof(vect[1].substr(4, 6).c_str());	//vect[1][4] * 10 + vect[1][5];
		}
		//else
		{
			//cout << rx_data;
		}


	}

}	// end of main


FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript)
{
	FT_HANDLE ftHandle = NULL;
	FT_HANDLE ftHandleTemp;
	FT_DEVICE_LIST_INFO_NODE devInfo[32];
	DWORD numDevices = 0;
	int dev_number, found;
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

