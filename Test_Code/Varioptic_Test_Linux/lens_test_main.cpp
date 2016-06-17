/* Small program to test the ability to open up the virtual comm port
* created by the SiLabs CP2102 attached to the Varioptic lens driver.
*/

#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h> 
#include <fcntl.h>
//#include <errno.h>
#include <termios.h> 

#include "varioptic_class.h"

int initSerialPort(struct termios port_settings, int commPort, int baudrate);

using namespace std;

void print_usage()
{
	cout << "Usage: varioptic_test [-v <voltage level ranging from 24.0 to 70.0>]" << endl;
	cout << "\t\t      [-c <comm port in the form: ttyXX, where X=port number>]" << endl;
}


int main(int argc, char ** argv)
{
	int Casp_retVal=0;
	unsigned int idx;
	int commPort;
	int rd_result, wr_result;
	struct termios port_settings;
	string port = "/dev/ttyS0";
	double voltage = 0.0;
	char rx_data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	varioptic_class Casp_Lens, Casp_startup1, Casp_startup2;
	
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
			else if (arg == "-c")
			{
				port = "/dev/" + (string)argv[idx + 1];
			}

		}	
		
		
		// open the port
		commPort = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
		
		if (commPort == -1)
		{
			cout << "Unable to open " << port << endl;
			return -1;
		}
		else
		{
			fcntl(commPort, F_SETFL, 0);
			initSerialPort(port_settings, commPort, B57600);
			
		}
	
		//Casp_Lens.startup_1();
		//result = write(commPort, Casp_Lens.Packet, Casp_Lens.packet_length + 1);
		//Casp_Lens.startup_2();
		//result = write(commPort, Casp_Lens.Packet, Casp_Lens.packet_length + 1);
		
		Casp_Lens.voltage(voltage);
		cout << "Voltage: " << voltage << endl;
		cout << "Packet: ";

		for (idx = 0; idx <= Casp_Lens.packet_length; idx++)
		{
			cout << "0x" << hex << setw(2) << setfill('0') << uppercase << (unsigned int)Casp_Lens.Packet[idx] << "  ";
		}
		cout << endl;
		
		wr_result = write(commPort, Casp_Lens.Packet, Casp_Lens.packet_length + 1);
		rd_result = read(commPort, rx_data, 4);
		
		// for dedugging purposes
		cout << "Write Result: << wr_result << endl;
		cout << "Read Result: << rd_result << endl;
		
		close(commPort);
	}
	
	return 0;
	
	
} // end of main



/****************************Serial Port Functions****************************/
// set the port to 8N1
int initSerialPort(struct termios port_settings, int commPort, int baudrate)
{
	// Get the current options for the port
	tcgetattr(commPort, &port_settings);

	// Set the baud rates
	cfsetispeed(&port_settings, baudrate);
	cfsetospeed(&port_settings, baudrate);
	
	port_settings.c_cflag |= (CLOCAL | CREAD);
	
	port_settings.c_cflag &= ~PARENB
	port_settings.c_cflag &= ~CSTOPB
	port_settings.c_cflag &= ~CSIZE;
	port_settings.c_cflag |= CS8;
	
	// Set the new options for the port
	tcsetattr(commPort, TCSANOW, &port_settings);
	
	return 0;

}	// end of initSerialPort


