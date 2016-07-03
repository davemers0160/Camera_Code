#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <stdio.h> 
//#include <windows.h>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>


//#include "ftd2xx.h"

// GPIO Control Includes
#include "GPIO_Ctrl.h"

// DEFINES
#define STNDBY_PIN	31	/* Pin to monitor the standby/recording  status */

void sleep_ms(int value);

int main(int argc, char ** argv)
{
	unsigned int idx;
	int current_state, prev_state;
	int pin_state;
	

	current_state = 0;
	prev_state = 0;

	int export_status = exportPin(STNDBY_PIN);
	int dir_status = setPinDirection(STNDBY_PIN, 1);


	while(1)
	{
		

		pin_state = readPin(STNDBY_PIN);

		if(pin_state == 0)
		{
		
			current_state = 0;
			if(current_state != prev_state)
			{
				cout << "Pin State: " << pin_state << endl;
				cout << "Entering standby mode..." << endl;
				prev_state = current_state;
			}

		}
 		else if (pin_state == 1)
		{

 			current_state = 1;
			if(current_state != prev_state)
			{
				cout << "Pin State: " << pin_state << endl;
				cout << "Entering record mode..." << endl;
				prev_state = current_state;
			}

		}
		else
		{
			cout << "Error reading pin state.  Value returned: " << pin_state << endl;
		}


		readPin(STNDBY_PIN);
		sleep_ms(100);



	}	// end of while(1)

}	// end of main

void sleep_ms(int value)
{

#if defined(_WIN32) | defined(__WIN32__) | defined(__WIN32) | defined(_WIN64) | defined(__WIN64)
	Sleep(value);
#else
	const timespec delay[]= {0, value*1000000L} ;
	//delay->tv_sec = 0;
	//delay->tv_nsec = value*1000000L;
	nanosleep(delay, NULL);
	//nanosleep((const struct timespec[]){ {0, value*1000000L} }, NULL);
#endif

}
