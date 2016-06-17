################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/home/odroid/BicycleCamera/Camera_Shared_Code/Chameleon_Utilities.cpp \
/home/odroid/BicycleCamera/Camera_Shared_Code/Lens_Driver.cpp \
/home/odroid/BicycleCamera/Camera_Shared_Code/imageCapture.cpp \
/home/odroid/BicycleCamera/Camera_Shared_Code/stdafx.cpp \
/home/odroid/BicycleCamera/Camera_Shared_Code/videoCapture.cpp \
/home/odroid/BicycleCamera/Camera_Shared_Code/videoCapture_2.cpp 

OBJS += \
./Camera_Shared_Code/Chameleon_Utilities.o \
./Camera_Shared_Code/Lens_Driver.o \
./Camera_Shared_Code/imageCapture.o \
./Camera_Shared_Code/stdafx.o \
./Camera_Shared_Code/videoCapture.o \
./Camera_Shared_Code/videoCapture_2.o 

CPP_DEPS += \
./Camera_Shared_Code/Chameleon_Utilities.d \
./Camera_Shared_Code/Lens_Driver.d \
./Camera_Shared_Code/imageCapture.d \
./Camera_Shared_Code/stdafx.d \
./Camera_Shared_Code/videoCapture.d \
./Camera_Shared_Code/videoCapture_2.d 


# Each subdirectory must supply rules for building sources it contributes
Camera_Shared_Code/Chameleon_Utilities.o: /home/odroid/BicycleCamera/Camera_Shared_Code/Chameleon_Utilities.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Camera_Shared_Code/Lens_Driver.o: /home/odroid/BicycleCamera/Camera_Shared_Code/Lens_Driver.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Camera_Shared_Code/imageCapture.o: /home/odroid/BicycleCamera/Camera_Shared_Code/imageCapture.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Camera_Shared_Code/stdafx.o: /home/odroid/BicycleCamera/Camera_Shared_Code/stdafx.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Camera_Shared_Code/videoCapture.o: /home/odroid/BicycleCamera/Camera_Shared_Code/videoCapture.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Camera_Shared_Code/videoCapture_2.o: /home/odroid/BicycleCamera/Camera_Shared_Code/videoCapture_2.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/local/include/opencv2 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


