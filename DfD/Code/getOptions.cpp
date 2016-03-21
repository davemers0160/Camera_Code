#include <windows.h>
#include <string>
#include <cstdlib> 
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

typedef struct
{
	vector<string> Option_Arg;
	vector<string> Option;

} Option_Struct;

int getOptionCount(string options, vector<string> &option_vector);


Option_Struct getOptions(int argc, char *argv[], string options)
{
	int idx, jdx;
	int option_count = 0;
	unsigned int option_index = 0;
	vector<string> option_vector;
	Option_Struct Options;

	option_count = getOptionCount(options, option_vector);

	if (argc > 1)
	{
		for (idx = 0; idx < option_count; idx++)
		{
			string option = "-" + option_vector[idx];
			for (jdx = 1; jdx < argc; jdx++)
			{
				if (strcmp(argv[jdx], option.c_str()) == 0)
				{
					//set[0] = (unsigned char)atoi(argv[2]);
					bool single = true;

					//option_vector.at(0)
					//Options.Option[option_index] = option_vector[idx];
					//Options.Option_Arg[option_index] = argv[idx + 1];

					Options.Option.push_back(option_vector[idx]);
					Options.Option_Arg.push_back(argv[jdx + 1]);
					option_index++;

				}

			}

		}

	}


	return Options;


		//if (argc == 3)
		//{
		//	if (strcmp(argv[1], "-s") == 0)
		//	{
		//		//set[0] = (unsigned char)atoi(argv[2]);
		//		//single = true;
		//	}
		//}
		//else if (argc == 7)
		//{
		//	for (idx = 1; idx < argc; idx += 2)
		//	{
		//		if ((strcmp(argv[idx], "-s") == 0) && (idx < argc - 1))
		//		{
		//			//set[0] = (unsigned char)atoi(argv[idx + 1]);
		//			//single = false;
		//		}
		//		if ((strcmp(argv[idx], "-t") == 0) && (idx < argc - 1))
		//		{
		//			//set[1] = (unsigned char)atoi(argv[idx + 1]);
		//			//single = false;
		//		}

		//		if ((strcmp(argv[idx], "-d") == 0) && (idx < argc - 1))
		//		{
		//			//delay = atof(argv[idx + 1]);
		//			//single = false;
		//		}
		//	}

		//}
	
}	// end 


int getOptionCount(string options, vector<string> &option_vector)
{

		//vector<string> option_vector;
		//string temp;
		stringstream ss(options);

		while (ss.good())
		{
			string substr;
			getline(ss, substr, ':');
			option_vector.push_back(substr);

		}
		

		//option_count = (int *)option_vector.size();
		
		return option_vector.size();		

}