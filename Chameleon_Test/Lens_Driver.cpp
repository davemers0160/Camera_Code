#include <iostream>
#include <sstream>
#include <math.h>
#include <windows.h> 

#include "Lens_Driver.h"


//void Lens_Driver::Lens_Driver()
//{
//	Packet.Command = 0;
//	Packet.ByteCount = 0;
//	Packet.Checksum = 0;
//	serial_number = 0;
//	driver_type = 0;
//
//}
//
//Lens_Driver::Lens_Driver(unsigned char Command, unsigned char ByteCount)
//{
//	Packet.Command = Command;
//	Packet.ByteCount = ByteCount;
//	Packet.Checksum = 0;
//	serial_number = 0;
//	driver_type = 0;
//}


//Lens_Driver::~Lens_Driver()
//{
//}

/* Function: genChecksum(unsigned char Packet[], unsigned char length)
*
* Input Arguments:
* 1. Packet[]: This input is an array which contains the data to perform
*              the checksum calculations.
*
* Return Value:
* 1. sum: The output is an unsigned char representing the containing the 
*         calculated checksum.
*
* Description: Function designed to calculate the checksum 
*/
unsigned short Lens_Driver::genChecksum(LensTxPacket Packet)
{
	//unsigned int sum = 0;
	//unsigned char idx;

	//sum = Packet.Command;
	//sum += Packet.ByteCount;

	//for (idx = 0; idx < Packet.ByteCount; idx++)
	//{
	//	sum += Packet.Data[idx];
	//}
	//idx = sum % 256;

	//return idx;
	unsigned short sum1 = 0;
	unsigned short sum2 = 0;
	unsigned short Checksum;

	// calculate the sums - Command
	sum1 = (sum1 + Packet.Command) % 255;
	sum2 = (sum2 + sum1) % 255;

	// calculate the sums - ByteCount
	sum1 = (sum1 + Packet.ByteCount) % 255;
	sum2 = (sum2 + sum1) % 255;

	// calculate for data
	for (int index = 0; index < Packet.ByteCount-2; index++)
	{
		sum1 = (sum1 + Packet.Data[index]) % 255;
		sum2 = (sum2 + sum1) % 255;
	}

	// generate final checksum
	Checksum = (sum2 << 8) | (sum1 & 0x00FF);

	return Checksum;

}   // end of check

bool Lens_Driver::checkChecksum(LensRxPacket Packet)
{
	bool status = false;

	unsigned short sum1 = 0;
	unsigned short sum2 = 0;
	unsigned short Checksum;

	// calculate the sums - Command
	sum1 = (sum1 + Packet.Command) % 255;
	sum2 = (sum2 + sum1) % 255;

	// calculate the sums - ByteCount
	sum1 = (sum1 + Packet.ByteCount) % 255;
	sum2 = (sum2 + sum1) % 255;

	// calculate for data
	for (int index = 0; index < Packet.ByteCount - 2; index++)
	{
		sum1 = (sum1 + Packet.Data[index]) % 255;
		sum2 = (sum2 + sum1) % 255;
	}

	// generate final checksum
	Checksum = (sum2 << 8) | (sum1 & 0x00FF);

	if (Packet.Checksum == Checksum)
	{
		status = true;
	}
	return status;

}   // end of check

//VOUT(RMS) = 9.8VRMS + AMP • 205mVRMS
unsigned char Lens_Driver::LensFocus::setVoltage(double volts)
{
	//unsigned char amp = 0;

	if (volts > FOCUS_VOLTAGE_MAX)
	{
		return 255;
	}
	else if (volts < FOCUS_VOLTAGE_MIN)
	{
		return 0;
	}
	else
	{
		return (unsigned char)floor(((volts - 9.8) / 0.205) + 0.5);
	}
	
	//return amp;
}

//VOUT(RMS) = 9.8VRMS + AMP • 205mVRMS
double Lens_Driver::LensFocus::getVoltage(unsigned char amp)
{
	double volts = 0.0;

	volts = 9.8 + amp*0.205;

	return volts;
}

unsigned char Lens_Driver::sendLensPacket(LensTxPacket Packet, HANDLE lensDriver)
{
	unsigned long dwBytesWritten;
	unsigned char data[MAX_PACKET_LENGTH];

	data[0] = Packet.Start;
	data[1] = Packet.Command;
	data[2] = Packet.ByteCount;
	std::copy(Packet.Data, Packet.Data + Packet.ByteCount, &data[3]);

	WriteFile(lensDriver, data, Packet.ByteCount + 3, &dwBytesWritten, NULL);

	return (unsigned char)dwBytesWritten;
}	// end of sendLensPacket

bool Lens_Driver::readLensPacket(LensRxPacket *Packet, HANDLE lensDriver, unsigned char count)
{
	bool status = false;
	unsigned long dwRead=0;
	unsigned char rx_data[MAX_PACKET_LENGTH] = { 0 };
	unsigned char idx;
	OVERLAPPED osReader = { 0 };

	ReadFile(lensDriver, rx_data, count, (LPDWORD)&dwRead, &osReader);

	if (dwRead > 0)
	{
		Packet->Command = rx_data[0];
		Packet->ByteCount = rx_data[1];
		for (idx = 0; idx < Packet->ByteCount - 2; idx++)
		{
			Packet->Data[idx] = rx_data[idx + 2];
		}
		Packet->Checksum = (rx_data[idx + 2] << 8) | rx_data[idx + 3];

		status = checkChecksum(*Packet);

	}
	else
	{
		std::cout << "No data received from Lens Driver." << std::endl;
		status = false;
	}
	return status;

}

void Lens_Driver::setLensDriverInfo(LensDriverInfo *LensInfo, LensRxPacket Packet)
{
	LensInfo->SerialNumber = Packet.Data[0];
	LensInfo->FirmwareVersion[0] = Packet.Data[1];
	LensInfo->FirmwareVersion[1] = Packet.Data[2];
	LensInfo->DriverType = Packet.Data[3];
	LensInfo->Amplitude = Packet.Data[4];

}