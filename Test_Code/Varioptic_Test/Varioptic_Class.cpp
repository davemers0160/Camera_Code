
/* packet class for the caspian M... lens and the USB Drivboard
packet comes from reverse engineering the comms packet from the FocusLabC
Program.

Packet Structure is as follows:
| STX  | MODE | UNK  | UNK  | DATA2 | DATA1 | CRC |
| 0x02 | 0x37 | 0x00 | 0x02 | XXXX  | XXXX  | CCC |   

Class contains the functions to generate the CRC and convert the voltage
to the correct format for sending to the USB Drivboard

*/


#include "varioptic_class.h"


varioptic_class::varioptic_class()
{
	Packet[0] = 0x02;
	Packet[1] = 0x37;
	Packet[2] = 0x00;
	Packet[3] = 0x02;

	packet_length = 4;
	generateCRC();
}	// end of varioptic_class


void varioptic_class::startup_1()
{
	Packet[0] = 0x02;
	Packet[1] = 0x38;
	Packet[2] = 0x00;
	Packet[3] = 0x01;

	packet_length = 4;
	generateCRC();
}

void varioptic_class::startup_2()
{
	Packet[0] = 0x02;
	Packet[1] = 0x38;
	Packet[2] = 0x00;
	Packet[3] = 0x02;

	packet_length = 4;
	generateCRC();
}

void varioptic_class::voltage(double volts)
{
	Packet[0] = 0x02;
	Packet[1] = 0x37;
	Packet[2] = 0x00;
	Packet[3] = 0x02;

	if (volts <= FOCUS_VOLTAGE_MIN)
		volts = FOCUS_VOLTAGE_MIN;
	else if (volts >= FOCUS_VOLTAGE_MAX)
		volts = FOCUS_VOLTAGE_MAX;

	Packet[4] = (unsigned char)((volts - FOCUS_VOLTAGE_MIN) * 1000) % 256;
	Packet[5] = (unsigned char)(((volts - FOCUS_VOLTAGE_MIN) * 1000) / 256);

	packet_length = 6;
	generateCRC();

}	// end of voltage


void varioptic_class::generateCRC()
{
	unsigned int idx;
	unsigned char sum=0;

	for (idx = 0; idx < packet_length; idx++)
	{
		sum += Packet[idx];
	}
	Packet[packet_length] = sum%256;

}	// end of generateCRC

