#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h> 
#include <windows.h> 

#include "varioptic_class.h"

using namespace std;

void print_usage()
{
	cout << "Usage: varioptic_test [-v <voltage level ranging from 24.0 to 70.0>]" << endl;
	cout << "\t\t      [-p <comm port in the form: COMX, where X=port number>]" << endl;
}


int main(int argc, char* argv[])
{
	int Casp_retVal=0;
	unsigned int idx;
	string port;// = "\\\\.\\COM1";
	double voltage = 0.0;
	varioptic_class Casp_Lens, Casp_startup1, Casp_startup2;
	BOOL comm_result;
	LPCWSTR lpFileName = L"\\\\.\\COM5";

	DWORD dwRead;
	char rx_data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	OVERLAPPED osReader = { 0 };

	if (argc == 1)
	{
		print_usage();
		exit(0);
	}

	else if (argc >= 3)
	{
		for (idx = 1; idx < argc; idx+=2)
		{
			string arg = argv[idx];
			if (arg == "-v")
			{
				voltage = atof(argv[idx + 1]);
			}
			else if (arg == "-p")
			{
				/*
				port = ("\\\\.\\" + (string)argv[idx + 1]);
				char* temp_port = &port[0];
				lpFileName = (LPCWSTR)temp_port;//("\\\\.\\" + (string)argv[idx + 1]);
				printf("lpFileName: %s\n", lpFileName);
				*/
			}

		}

		unsigned long dwBytesWritten;
		HANDLE serialHandle;
		//TEXT("\\\\.\\COM5")
		serialHandle = CreateFileW(lpFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);



		// Do some basic settings
		DCB serialParams = { 0 };
		serialParams.DCBlength = sizeof(serialParams);

		if (!GetCommState(serialHandle, &serialParams))
			cout << "Error getting information from the specified serial port" << endl;

		//GetCommState(serialHandle, &serialParams);
		serialParams.BaudRate = 57600;
		serialParams.ByteSize = 8;
		serialParams.StopBits = ONESTOPBIT;
		serialParams.Parity = NOPARITY;
		//serialParams.fInX = 1;

		comm_result = SetCommState(serialHandle, &serialParams);
		if (!comm_result)
		{
			cout << "Error setting serial port configuration" << endl;
			return EXIT_FAILURE;
		}

		// Set timeouts
		COMMTIMEOUTS timeout = { 0 };
		timeout.ReadIntervalTimeout = 50;
		timeout.ReadTotalTimeoutConstant = 50;
		timeout.ReadTotalTimeoutMultiplier = 50;
		timeout.WriteTotalTimeoutConstant = 50;
		timeout.WriteTotalTimeoutMultiplier = 10;

		comm_result = SetCommTimeouts(serialHandle, &timeout);
		if (!comm_result)
		{
			cout << "Error setting serial port timeout parameters" << endl;
			return EXIT_FAILURE;
		}

		/*
		Casp_Lens.startup_1();
		WriteFile(serialHandle, Casp_Lens.Packet, Casp_Lens.packet_length + 1, &dwBytesWritten, NULL);
		ReadFile(serialHandle, rx_data, sizeof(rx_data), &dwRead, &osReader);

		Casp_Lens.startup_2();
		WriteFile(serialHandle, Casp_Lens.Packet, Casp_Lens.packet_length + 1, &dwBytesWritten, NULL);
		ReadFile(serialHandle, rx_data, sizeof(rx_data), &dwRead, &osReader);
		*/
		Casp_Lens.voltage(voltage);

		cout << "Voltage: " << voltage << endl;
		cout << "Packet: ";

		for (idx = 0; idx <= Casp_Lens.packet_length; idx++)
		{
			cout << "0x" << hex << setw(2) << setfill('0') << uppercase << (unsigned int)Casp_Lens.Packet[idx] << "  ";
		}
		cout << endl;
		

		WriteFile(serialHandle, Casp_Lens.Packet, Casp_Lens.packet_length+1, &dwBytesWritten, NULL);
		// clear tx and rx serial buffers
		//PurgeComm(serialHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT);
		ReadFile(serialHandle, rx_data, sizeof(rx_data), &dwRead, &osReader);

		CloseHandle(serialHandle);

	}

	return EXIT_SUCCESS;

}	// end of main