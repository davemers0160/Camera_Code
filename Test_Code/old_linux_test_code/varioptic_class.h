// Class to create packet structure for sending data to the drivboard
// 

#ifndef VARIOPTIC_CLASS_H
#define VARIOPTIC_CLASS_H

#define MAX_PACKET_LENGTH	20
#define FOCUS_VOLTAGE_MIN	24.0
#define FOCUS_VOLTAGE_MAX	70.0




class varioptic_class
{

public:
	unsigned char Packet[MAX_PACKET_LENGTH];		// data packet

	unsigned int packet_length = 0;		// length of the packet without th CRC value

	varioptic_class();

	void startup_1();
	void startup_2();
	void voltage(double volts);

	void generateCRC();

};


#endif