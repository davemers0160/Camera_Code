################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Camera_Main_Linux.cpp \
../Chameleon_Utilities.cpp \
../GPIO_Ctrl.cpp \
../GPS_Ctrl.cpp \
../Lens_Driver.cpp \
../imageCapture.cpp \
../stdafx.cpp \
../videoCapture.cpp \
../videoCaptureInterleave.cpp 

OBJS += \
./Camera_Main_Linux.o \
./Chameleon_Utilities.o \
./GPIO_Ctrl.o \
./GPS_Ctrl.o \
./Lens_Driver.o \
./imageCapture.o \
./stdafx.o \
./videoCapture.o \
./videoCaptureInterleave.o 

CPP_DEPS += \
./Camera_Main_Linux.d \
./Chameleon_Utilities.d \
./GPIO_Ctrl.d \
./GPS_Ctrl.d \
./Lens_Driver.d \
./imageCapture.d \
./stdafx.d \
./videoCapture.d \
./videoCaptureInterleave.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


