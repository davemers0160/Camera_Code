//#include <tchar.h>
//#include <windows.h>
#include <string.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <algorithm>
#include <cstdlib> 
#include <cstdio>
//#include <unistd.h>

#include "ftd2xx.h"

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)

#else
#include <pthread.h>
#endif

/*
#define NUM_BYTES_IN_MSG 32128
#define UINT8 unsigned char
#define UINT16 unsigned short
#define INT16 short
#define INT32 int
#define UINT32 unsigned int
*/

//#define DESCRIPTIONSIZE 9
//#define SERIALNUMSIZE 8
//change this whenever ReadBuffer size changes
//#define READBUFFERSIZE 32

using namespace std;

struct ftdiDeviceDetails //structure storage for FTDI device details
{
	int devNumber;
	unsigned long type;
	unsigned long BaudRate;
	string Description;
	string serialNumber;
};


FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript);

void printHelp()
{
	system("cls");
	cout << "Lens Driver Command Line Interface (CLI)" << endl;
	cout << "David Emerson" << endl << endl;
	cout << "Set the Lens Driver to a single step." << endl;
	cout << "Usage: LensDriver-cli -s <Step Number>" << endl << endl;

	cout << "Set the Lens Driver to toggle between two values with a given delay." << endl;
	cout << "Usage: LensDriver-cli -s <Step Number> -t <Step Number> -d <Delay in units of seconds>" << endl << endl;

	//cout << "Run program and track objects in recorded video file." << endl;
	//cout << "Usage: Final_Project -f <Input File Name>" << endl;
}

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
DWORD WINAPI myThread(__in LPVOID lpParameter)
{
	char& key = *((char*)lpParameter);
	//{
	cin >> key;
	//}
	return 0;
}
#else
void *myThread(void *Parameter)
{
	char key = *((char*)Parameter);
	//{
	cin >> *((char*)Parameter);
	//}
	//return 0;
	cout << "Closing Thread" << endl;
	//pthread_exit(NULL);
}
#endif


int main(int argc, char *argv[])
{

	FT_HANDLE lensDriver = NULL;
	unsigned int BytesRead, BytesSent;

	ftdiDeviceDetails driverDeviceDetails, gpsDeviceDetails;

	unsigned char idx;
	unsigned char set[2] = { 0,0 };
	double delay = 0.0;

	unsigned char txPacket[16] = { '$', 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char rxPacket[16] = { 0 };

	bool single = true;
	char key = ' ';

	// thread stuff
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
	DWORD myThreadID;
	HANDLE myHandle;
#else
	pthread_t myThreadID;
#endif

	if (argc < 3)
	{
		printHelp();
		return 1;
	}
	else if (argc == 3)
	{
		if (strcmp(argv[1], "-s") == 0)
		{
			set[0] = (unsigned char)atoi(argv[2]);
			single = true;
		}
	}
	else if(argc == 7)
	{
		for (idx = 1; idx < argc; idx+=2)
		{
			if ((strcmp(argv[idx], "-s") == 0) && (idx < argc - 1))
			{
				set[0] = (unsigned char)atoi(argv[idx + 1]);
				single = false;
			}
			if ((strcmp(argv[idx], "-t") == 0) && (idx < argc - 1))
			{
				set[1] = (unsigned char)atoi(argv[idx + 1]);
				single = false;
			}

			if ((strcmp(argv[idx], "-d") == 0) && (idx < argc - 1))
			{
				delay = atof(argv[idx + 1]);
				if(delay >= 1.0)
				{
					delay = 0.999;
					cout << "Delay value too large setting delay to 999ms." << endl;
				}
				single = false;
			}
		}

	}
	else
	{
		cout << "Incorrect number of input arguments." << endl;
		printHelp();
		return 1;
	}

	//readSize = 48170;



	// check for lens driver
	if (lensDriver == NULL)
	{
		driverDeviceDetails.devNumber = 0;
		driverDeviceDetails.type = 0;
		driverDeviceDetails.BaudRate = 250000;
		driverDeviceDetails.Description = "";
		driverDeviceDetails.serialNumber = "";

		lensDriver = OpenComPort(&driverDeviceDetails, "Microfluidic Lens Driver");
		//FTDI manual says that the buffer goes up to 64 KB
		//SetupComm(lensDriver, 16 * 4 * 1024, 4 * 1024);
		if (lensDriver == NULL)
		{
			return 1;
		}
		else
		{
			//print device info		
			//cout << "Device[" << driverDeviceDetails.devNumber << "] " << driverDeviceDetails.Description << ", SN: " << driverDeviceDetails.serialNumber << endl;
		}

	}

	// clear FTDI buffer
	FT_Purge(lensDriver, FT_PURGE_RX | FT_PURGE_TX);

	ULONG ft_write_Status = FT_Write(lensDriver, txPacket, txPacket[2] + 3, &BytesSent);
	ULONG ft_read_Status = FT_Read(lensDriver, rxPacket, 8, &BytesRead);

	if (ft_read_Status == FT_OK)
	{
		//get data from stream
		if (BytesRead > 5)
		{
			cout << "SN: " <<  (int)rxPacket[2] << endl;
			cout << "Firmware: " << dec << (int)rxPacket[3] << "." << dec << (int)rxPacket[4] << endl;
			cout << "Driver Type: " << dec << (int)rxPacket[5] << endl;
		}

	}
	else
	{
		std::cout << "error, read status is:" << ft_read_Status << std::endl;
	}


	txPacket[1] = 0x51;
	txPacket[2] = 1;

	//cout << "start: ";
	//cin >> key;

	if (single == true)
	{

		txPacket[3] = set[0];
		ft_write_Status = FT_Write(lensDriver, txPacket, txPacket[2] + 3, &BytesSent);


	}
	else
	{
#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
		myHandle = CreateThread(0, 0, myThread, &key, 0, &myThreadID);
#else
		pthread_create (&myThreadID, NULL, myThread, (void *)&key);
#endif

		cout << "Press q to quit..." << endl;
		//for (idx = 0; idx < 100; idx++)
		//while(key != 'q')
		do
		{
			txPacket[3] = set[0];
			ft_write_Status = FT_Write(lensDriver, txPacket, txPacket[2] + 3, &BytesSent);

			usleep(delay * 1000000);

			txPacket[3] = set[1];
			ft_write_Status = FT_Write(lensDriver, txPacket, txPacket[2] + 3, &BytesSent);

			usleep(delay * 1000000);
			//cout << ".";

		} while (key != 'q');


#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32)
		CloseHandle(myHandle);
#else
		pthread_exit (NULL);
#endif
		cout << "Closing Main" << endl;
	}

	FT_Close(lensDriver);
	lensDriver = NULL;

	//cout << "Press any key to continue..." << endl;
	//cin.ignore();
	return 0;
}





FT_HANDLE OpenComPort(ftdiDeviceDetails *device, string descript)
{
	FT_HANDLE ftHandle = NULL;
	FT_HANDLE ftHandleTemp;
	FT_DEVICE_LIST_INFO_NODE devInfo[32];
	DWORD numDevices=0;
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
					//else if ((string)Description == "LOCOSYS GPS MC-1513")
					//{
					//	device->devNumber = dev_number;
					//	device->type = Type;
					//	device->Description = (string)Description;
					//	device->serialNumber = (string)SerialNumber;
					//	found = 1;
					//}


					/*
					if (Type == FT_DEVICE_232R)
					{
						found = 1;
						device->devNumber = dev_number;
						device->type = Type;
						for (i = 0; i < DESCRIPTIONSIZE; i++)
						{
							device->Description += Description[i];
						}
						for (i = 0; i < SERIALNUMSIZE; i++)
						{
							device->serialNumber += SerialNumber[i];
						}
					}
					*/

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

			FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
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
