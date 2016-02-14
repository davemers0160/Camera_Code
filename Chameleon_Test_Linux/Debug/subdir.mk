################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Camera_Main.cpp \
../Config_Chameleon.cpp \
../Varioptic_Class.cpp \
../stdafx.cpp 

OBJS += \
./Camera_Main.o \
./Config_Chameleon.o \
./Varioptic_Class.o \
./stdafx.o 

CPP_DEPS += \
./Camera_Main.d \
./Config_Chameleon.d \
./Varioptic_Class.d \
./stdafx.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv2 -O0 -g3 -Wall -c -fmessage-length=0 -v -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


