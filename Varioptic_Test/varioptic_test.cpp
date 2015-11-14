#include <string>
#include <iostream>
#include <stdio.h> 
#include <windows.h> 

using namespace std;

#define FOCUS_VOLTAGE_MIN 24.0
#define FOCUS_VOLTAGE_MAX 70.0


typedef int(__stdcall *Casp_OpenCOM_ptr)();
typedef int(__stdcall *Casp_CloseCOM_ptr)();
typedef int(__stdcall *Casp_SetFocus_ptr)(double paFocus);
typedef int(__stdcall *Casp_GetFocus_ptr)(double *paVoltage);

void print_usage()
{
	cout << "Usage: varioptic_test [-v <voltage level ranging from 20.0 to 70.0>]" << endl;
}


int main(int argc, char ** argv)
{
	int Casp_retVal;
	double voltage = 0.0;

	HINSTANCE dllHandle = NULL;
	//int* Casp_OpenCOM_ptr = NULL;

	dllHandle = LoadLibrary(TEXT("ComCasp64.dll"));

	if (dllHandle == NULL)
	{

		cout << "Error: could not link to ComCaspXX.dll." << endl;
		return EXIT_FAILURE;
	}
	else
	{

		// try to get the several functions we need out of the dll...

		Casp_OpenCOM_ptr Casp_OpenCOM = (Casp_OpenCOM_ptr)GetProcAddress(dllHandle, "Casp_OpenCOM");
		Casp_CloseCOM_ptr Casp_CloseCOM = (Casp_OpenCOM_ptr)GetProcAddress(dllHandle, "Casp_CloseCOM");
		Casp_SetFocus_ptr Casp_SetFocus = (Casp_SetFocus_ptr)GetProcAddress(dllHandle, "Casp_SetFocus");
		Casp_GetFocus_ptr Casp_GetFocus = (Casp_GetFocus_ptr)GetProcAddress(dllHandle, "Casp_GetFocus");

		Casp_retVal = Casp_OpenCOM();

		cout << "reval:" << Casp_retVal << endl;

		if (argc == 1)
		{
			print_usage();
			exit(0);
		}

		else if (argc == 3)
		{
			voltage = atof(argv[2]);
			if (voltage <= FOCUS_VOLTAGE_MIN)
				voltage = FOCUS_VOLTAGE_MIN;
			else if (voltage >= FOCUS_VOLTAGE_MAX)
				voltage = FOCUS_VOLTAGE_MAX;

			cout << "Voltage: " << voltage << endl;

			Casp_retVal = Casp_SetFocus(voltage);

			cout << "reval:" << Casp_retVal << endl;
		}

		// close the comm port before exiting
		Casp_retVal = Casp_CloseCOM();

	}

	
	// free up any resources used by loading the dll
	FreeLibrary(dllHandle);
	return EXIT_SUCCESS;

}	// end of main