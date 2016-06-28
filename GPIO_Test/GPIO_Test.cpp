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

int main(int argc, char ** argv)
{
	unsigned int idx;
	int current_state, prev_state;
	int pin_state;
	

	current_state = 0;
	prev_state = 0;

	int export_status = exportPin(STNDBY_PIN);
	int dir_status = setPinDirection(STNDBY_PIN, 0);


	while(1)
	{
		

		pin_state = readPin(STNDBY_PIN);

		if(pin_state == 0)
		{
		

			if(current_state != prev_state)
			{
				cout << "Pin State: " << pin_state << endl;
				cout << "Entering standby mode..." << endl;
			}

		}
 		else if (pin_state == 1)
		{

			if(current_state != prev_state)
			{
				cout << "Pin State: " << pin_state << endl;
				cout << "Entering record mode..." << endl;
			}

		}
		else
		{
			cout << "Error reading pin state.  Value returned: " << pin_state << endl;
		}


	}	// end of while(1)

}	// end of main

