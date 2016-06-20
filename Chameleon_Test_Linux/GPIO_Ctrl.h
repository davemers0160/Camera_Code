

#ifndef GPIO_CTRL_H
#define GPIO_CTRL_H

using namespace std;

string run_cmd(const char* cmd);
int exportPin(unsigned char pin_number);
int setPinDirection(unsigned char pin_number, unsigned char direction);
int readPin(unsigned char pin_number);
int setPinValue(unsigned char pin_number, unsigned char pin_value);


#endif	// GPIO_CTRL_H
