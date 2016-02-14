#include <iostream>
#include <iomanip>
#include <stdio.h> 
#include <windows.h> 
#include <ctime>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

typedef struct
{
	tm Time;
	float Lat;
	float Long;

} GPS_info;

void configGPS(HANDLE GPS_Handle);
void getGPSInfo(HANDLE GPS_Handle, GPS_info &GPS_data);


int main(int argc, char ** argv)
{

	int Casp_retVal = 0;
	unsigned int idx;
	string port;// = "\\\\.\\COM1";
	
	BOOL comm_result;
	LPCWSTR lpFileName = L"\\\\.\\COM3";

	DWORD dwRead;
	char rx_data[96] = { 0 };
	OVERLAPPED osReader = { 0 };
	unsigned long dwBytesWritten;
	HANDLE GPS_Handle;

	GPS_info GPS_data;

	GPS_data.Time = { 0 };
	GPS_data.Lat = 0.0;
	GPS_data.Long = 0.0;

	GPS_Handle = CreateFileW(lpFileName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);



	// Do some basic settings
	DCB serialParams = { 0 };
	serialParams.DCBlength = sizeof(serialParams);

	if (!GetCommState(GPS_Handle, &serialParams))
		cout << "Error getting information from the specified serial port" << endl;

	//GetCommState(serialHandle, &serialParams);
	serialParams.BaudRate = 4800;
	serialParams.ByteSize = 8;
	serialParams.StopBits = ONESTOPBIT;
	serialParams.Parity = NOPARITY;
	//serialParams.fInX = 1;

	comm_result = SetCommState(GPS_Handle, &serialParams);
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

	comm_result = SetCommTimeouts(GPS_Handle, &timeout);
	if (!comm_result)
	{
		cout << "Error setting serial port timeout parameters" << endl;
		return EXIT_FAILURE;
	}


	//char temp[79] = "$GPGGA,014715.000,3914.4363,N,08636.6954,W,1,07,1.3,239.6,M,-33.6,M,,0000*60";


	//configGPS(GPS_Handle);


	//getGPSInfo(GPS_Handle, GPS_data);

	for (int idx = 0; idx < 20; idx++)
	{
		//ReadFile(GPS_Handle, rx_data, 79, &dwRead, &osReader);
		getGPSInfo(GPS_Handle, GPS_data);
		//cout << rx_data;

		cout << "Time: " << GPS_data.Time.tm_hour << ":" << GPS_data.Time.tm_min << ":" << GPS_data.Time.tm_sec << endl;
		cout << "Latitude: " << std::fixed << std::setprecision(5) << GPS_data.Lat << endl;
		cout << "Longitude: " << std::fixed << std::setprecision(5) << GPS_data.Long << endl;
	}

	CloseHandle(GPS_Handle);

	return EXIT_SUCCESS;
}

void configGPS(HANDLE GPS_Handle)
{
	unsigned long dwBytesWritten;

	// message checksum is bitwise XOR between $ and *; PSRF103 = 0x25;
	// disable GSV
	char GSV[] = "$PSRF103,03,00,00,01*27\r\n";

	// disable RMC
	char RMC[] = "$PSRF103,04,00,00,01*20\r\n";

	// disable GSA
	char GSA[] = "$PSRF103,02,00,00,01*26\r\n";

	// disable GGA
	char GGA[] = "$PSRF103,00,00,00,01*24\r\n";


	WriteFile(GPS_Handle, GSV, 26, &dwBytesWritten, NULL);
	WriteFile(GPS_Handle, RMC, 26, &dwBytesWritten, NULL);
	WriteFile(GPS_Handle, GSA, 26, &dwBytesWritten, NULL);
	//WriteFile(GPS_Handle, GGA, 26, &dwBytesWritten, NULL);

}

void getGPSInfo(HANDLE GPS_Handle, GPS_info &GPS_data)
{
	DWORD dwRead;
	char rx_data[96] = { 0 };
	OVERLAPPED osReader = { 0 };
	BOOL comm_result;

	PurgeComm(GPS_Handle, PURGE_RXABORT);

	comm_result = ReadFile(GPS_Handle, rx_data, 79, &dwRead, &osReader);
	if (comm_result == true)
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

		
		if ((vect.size() > 10) && (vect[0][0] == '$'))
		{
			
			GPS_data.Lat = stof(vect[2])/100;
			if (vect[3] == "S")
			{
				GPS_data.Lat = -GPS_data.Lat;
			}
			
			GPS_data.Long = stof(vect[4])/100;
			if (vect[5] == "W")
			{
				GPS_data.Long = -GPS_data.Long;
			}

			GPS_data.Time.tm_hour = stoi((string)(vect[1]).substr(0,2));	//atoi(vect[1][0]) * 10 + vect[1][1];
			GPS_data.Time.tm_min = stoi((string)(vect[1]).substr(2, 2));	//vect[1][2] * 10 + vect[1][3];
			GPS_data.Time.tm_sec = stof((string)(vect[1]).substr(4, 6));	//vect[1][4] * 10 + vect[1][5];

		}
		else
		{
			cout << rx_data;
		}


	}


}