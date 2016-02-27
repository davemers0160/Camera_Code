#pragma once

#ifndef LENS_DRIVER_CLASS_H
#define LENS_DRIVER_CLASS_H


#include <memory>
#include <string.h>

#include "ftd2xx.h"

namespace Lens_Driver
{
	#define MAX_DATA_LENGTH		16
	#define MAX_PACKET_LENGTH	(MAX_DATA_LENGTH+4)
	
	#define FOCUS_VOLTAGE_MIN	10.0
	#define FOCUS_VOLTAGE_MAX	62.075	
	
	#define LED_OPS     0x20        /* Used to control the LED on the board */

	#define CON         0x40        /* Used to determine connection to the PIC */
	#define GET_FW      0x41        /* Used to get firmware version of the PIC */
	#define GET_SN      0x42        /* Used to get serial number of the PIC */
	#define GET_DRV     0x43        /* Used to get the Lens Driver Chip */
	#define GET_I2C     0x44        /* Used to get the current I2C error status */
	#define GET_AMP     0x45        /* Used to get the current voltage setting stored by the PIC */

	#define SET_VOLT    0x50        /* Used to set the voltage level for the driver */
	#define FAST_SET_VOLT  0x51     /* Used to set the voltage level for the driver/ no response back from PIC */


	// variable declarations
	struct LensDriverInfo
	{

		unsigned char SerialNumber;
		unsigned char FirmwareVersion[2];
		unsigned char DriverType;
		unsigned char Amplitude;

		LensDriverInfo()
		{
			SerialNumber = 0;
			FirmwareVersion[0] = 0;
			FirmwareVersion[1] = 0;
			DriverType = 0;
			Amplitude = 0;
		}

/*
		LensDriverInfo(unsigned char SN, unsigned char FW[], unsigned char DT, unsigned char A)
		{
			SerialNumber = SN;
			FirmwareVersion[0] = FW[0];
			FirmwareVersion[1] = FW[1];
			DriverType = DT;
			Amplitude = A;
		}
		*/

	};


	struct LensRxPacket
	{
		unsigned char Command;
		unsigned char ByteCount;
		unsigned char Data[MAX_DATA_LENGTH];
		unsigned short Checksum;

		LensRxPacket()
		{
			Command = 0;
			ByteCount = 0;
			Checksum = 0;
			memset(Data, 0, MAX_DATA_LENGTH);
		}

		//LensRxPacket(unsigned char com, unsigned char bc)
		//{
		//	Command = com;
		//	ByteCount = bc;
		//	memset(Data, 0, MAX_DATA_LENGTH);
		//	Checksum = 0;
		//}

		//LensRxPacket(unsigned char com, unsigned char bc, unsigned char data[])
		//{
		//	Command = com;
		//	ByteCount = bc;
		//	std::copy(data, data + bc, Data);
		//	Checksum = 0;
		//}
	};


	struct LensTxPacket
	{
		unsigned char Start;
		unsigned char Command;
		unsigned char ByteCount;
		unsigned char Data[MAX_DATA_LENGTH];


		LensTxPacket()
		{
			Start = '$';
			Command = 0;
			ByteCount = 0;
			memset(Data, 0, MAX_DATA_LENGTH);
		}

		LensTxPacket(unsigned char com, unsigned char bc)
		{
			Start = '$';
			Command = com;
			ByteCount = bc;
			memset(Data, 0, MAX_DATA_LENGTH);
		}

		LensTxPacket(unsigned char com, unsigned char bc, unsigned char data[])
		{
			Start = '$';
			Command = com;
			ByteCount = bc;
			std::copy(data, data + bc, Data);
		}
	};

	struct LensFocus
	{
		unsigned char Focus[2];

		LensFocus()
		{
			Focus[0] = 0;
			Focus[1] = 0;
		}

		LensFocus(unsigned char F, unsigned char D)
		{
			Focus[0] = F;
			Focus[1] = D;
		}

		LensFocus(double F, double D)
		{
			Focus[0] = setVoltage(F);
			Focus[1] = setVoltage(D);
		}
		unsigned char setVoltage(double volts);
		double getVoltage(unsigned char data);

	};
	//unsigned char Amplitude;

	//Lens_Struct Packet;

	// function declarations
	//Lens_Driver();
	//Lens_Driver(unsigned char Command, unsigned char ByteCount);
	//~Lens_Driver();	

	//unsigned char setVoltage(double volts);
	//double getVoltage(unsigned char data);
	unsigned short genChecksum(LensTxPacket Packet);
	bool checkChecksum(LensRxPacket Packet);

	//unsigned char genChecksum(Packet_Struct Packet);
	unsigned char sendLensPacket(LensTxPacket Packet, FT_HANDLE lensDriver);
	bool readLensPacket(LensRxPacket *Packet, FT_HANDLE lensDriver, unsigned int count);
	void getLensDriverInfo(LensDriverInfo *LensInfo, LensRxPacket Packet);

};

#endif
