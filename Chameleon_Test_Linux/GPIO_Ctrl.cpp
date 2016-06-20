#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <vector>


using namespace std;

string run_cmd(const char* cmd)
{
    char buffer[128];
    std::string result = "";

    FILE* pipe = popen(cmd, "r");

    if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}

    try
    {
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
			{
            	result += buffer;
			}
        }
    }
    catch (...)
    {
        pclose(pipe);
        throw;
    }

    pclose(pipe);
    return result;

}	// end of run_cmd


int exportPin(unsigned char pin_number)
{

	char pin_num_string[3];
	string export_cmd;


	sprintf( pin_num_string, "%u", pin_number );

	export_cmd = "echo " + (string)pin_num_string +" > /sys/class/gpio/export";

	string result = run_cmd(export_cmd.c_str());

	if(result == "")
	{
		return 0;
	}
	else
	{
		return -1;
	}

}	// end of export_pin


int setPinDirection(unsigned char pin_number, unsigned char direction)
{
	char pin_num_string[3];

	string direction_cmd;
	string pin_dir_string = "";

	sprintf( pin_num_string, "%u", pin_number );

	if(direction == 0)
	{
		pin_dir_string = "out";
	}
	else
	{
		pin_dir_string = "in";
	}

	direction_cmd = "echo " + pin_dir_string + " > /sys/class/gpio/gpio" + (string)pin_num_string + "/direction";

	string result = run_cmd(direction_cmd.c_str());

	if(result == "")
	{
		return 0;
	}
	else
	{
		return -1;
	}

}	// end of setPinDirection


int readPin(unsigned char pin_number)
{
	char pin_num_string[3];

	string read_cmd;
	string result = "";
	int value = -1;

	sprintf( pin_num_string, "%u", pin_number );


	read_cmd = "cat /sys/class/gpio/gpio" + (string)pin_num_string + "/value";

	result = run_cmd(read_cmd.c_str());

	vector<string> vect;
	//string temp;

	stringstream ss(result);

	while (ss.good())
	{
		string substr;
		getline(ss, substr, '\n');
		vect.push_back(substr);

	}


	if (vect.size() > 0)
	{
		value = atoi(vect[0].c_str());
	}
	else
	{
		value = -1;
	}

	return value;

}	// end of readPin

int setPinValue(unsigned char pin_number, unsigned char pin_value)
{
	char pin_num_string[3];

	string set_value_cmd;

	sprintf( pin_num_string, "%u", pin_number );

	if(pin_value == 0)
	{
		set_value_cmd = "echo 0 > /sys/class/gpio/gpio" +(string)pin_num_string + "/value";
	}
	else if (pin_value == 1)
	{
		set_value_cmd = "echo 1 > /sys/class/gpio/gpio" +(string)pin_num_string + "/value";
	}
	else
	{
		cout << "value should be either 0 or 1." << endl;
		return -1;

	}

	string result = run_cmd(set_value_cmd.c_str());

	if(result == "")
	{
		return 0;
	}
	else
	{
		return -1;
	}

}	// end of setPinValue
