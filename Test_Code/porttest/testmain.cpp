/*
 * testmain.c
 *
 *  Created on: Jan 12, 2016
 *      Author: odroid
 */




#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */


int open_port(void);
int initSerialPort(struct termios port_settings, int commPort, int baudrate);

int main(void)
{

	int n, fd;
	struct termios port_settings;




	fd = open_port();

	initSerialPort(port_settings, fd, B57600);

	n = write(fd, "Test1\r\n", 6);
	if (n < 0)
	  fputs("write() of 4 bytes failed!\n", stderr);

	close(fd);

	return 0;

}


/****************************Serial Port Functions****************************/
/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

int
open_port(void)
{
  int fd; /* File descriptor for the port */


  fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
  {
   /*
    * Could not open the port.
    */

    perror("open_port: Unable to open /dev/ttyUSB0 - ");
  }
  else
    fcntl(fd, F_SETFL, 0);

  return (fd);
}


// set the port to 8N1
int initSerialPort(struct termios port_settings, int commPort, int baudrate)
{
	// Get the current options for the port
	tcgetattr(commPort, &port_settings);

	// Set the baud rates
	cfsetispeed(&port_settings, baudrate);
	cfsetospeed(&port_settings, baudrate);

	port_settings.c_cflag |= (CLOCAL | CREAD);

	port_settings.c_cflag &= ~PARENB;
	port_settings.c_cflag &= ~CSTOPB;
	port_settings.c_cflag &= ~CSIZE;
	port_settings.c_cflag |= CS8;

	// Set the new options for the port
	tcsetattr(commPort, TCSANOW, &port_settings);

	return 0;

}	// end of initSerialPort


