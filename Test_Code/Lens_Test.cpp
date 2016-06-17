#include <tchar.h>
#include <windows.h>
#include <string>
#include <time.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <algorithm>

#include "ftd2xx.h"

#define NUM_BYTES_IN_MSG 32128
#define UINT8 unsigned char
#define UINT16 unsigned short
#define INT16 short
#define INT32 int
#define UINT32 unsigned int
#define MAX_WORDS (1023/1)
//modify these two macros to display more or less information from the ftdi embedded chip description and serial number
#define DESCRIPTIONSIZE 9
#define SERIALNUMSIZE 8
//change this whenever ReadBuffer size changes
#define READBUFFERSIZE 32

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


void main(void)
{
	int numErrors;
	FT_HANDLE lensDriver = NULL;
	FT_HANDLE gpsHandle = NULL;
	unsigned long BytesRead, BytesSent;	


	ftdiDeviceDetails driverDeviceDetails, gpsDeviceDetails;

	unsigned long queueRx;
	unsigned long queueTx;
	unsigned long dwEvent;
	unsigned int readSize;
	//char ReadBuffer[48170];

	unsigned char txPacket[16] = { '$', 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	unsigned char rxPacket[16] = { 0 };



	//readSize = 48170;



	numErrors = 0;
	
	BytesRead = 0;

	// check for lens driver
	while (lensDriver == NULL)
	{
		driverDeviceDetails.devNumber = 0;
		driverDeviceDetails.type = 0;
		driverDeviceDetails.BaudRate = 250000;
		driverDeviceDetails.Description = "";
		driverDeviceDetails.serialNumber = "";

		lensDriver = OpenComPort(&driverDeviceDetails, "Microfluidic Lens Driver");
		//FTDI manual says that the buffer goes up to 64 KB
		SetupComm(lensDriver, 16 * 4 * 1024, 4 * 1024);
	}

	if (gpsHandle == NULL)
	{
		gpsDeviceDetails.devNumber = 0;
		gpsDeviceDetails.type = 0;
		gpsDeviceDetails.BaudRate = 57600;
		gpsDeviceDetails.Description = "";
		gpsDeviceDetails.serialNumber = "";

		gpsHandle = OpenComPort(&gpsDeviceDetails, "LOCOSYS GPS MC-1513");
		//FTDI manual says that the buffer goes up to 64 KB
		SetupComm(gpsHandle, 16 * 4 * 1024, 4 * 1024);
	}

		//print device info
		cout << "Device[" << driverDeviceDetails.devNumber << "] " << driverDeviceDetails.Description << ", SN: " << driverDeviceDetails.serialNumber << endl;
		cout << "Device[" << gpsDeviceDetails.devNumber << "] " << gpsDeviceDetails.Description << ", SN: " << gpsDeviceDetails.serialNumber << endl << endl;

		/************************************************
		GET DATA
		************************************************/

		// clear FTDI buffer
		FT_Purge(lensDriver, FT_PURGE_RX | FT_PURGE_TX);
		FT_Purge(gpsHandle, FT_PURGE_RX | FT_PURGE_TX);

		// clear read buffer array
		//memset(ReadBuffer, 0, sizeof(ReadBuffer));			

		//FT_GetStatus(lensDriver, &queueRx, &queueTx, &dwEvent);
		//while(queueRx < readSize)
		//	FT_GetStatus(lensDriver, &queueRx, &queueTx, &dwEvent);

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

	FT_Close(lensDriver);
	lensDriver = NULL;
	cout << "Press any key to continue..." << endl;
	cin.ignore();
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
