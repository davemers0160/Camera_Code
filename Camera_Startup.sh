#!/bin/sh
#Start up script for the camera program to begin recording data.
#begin
echo "Starting video capture script."
echo "Sleeping for 20 seconds to allow all drivers to activate..."
sleep 120;
echo "Sleep done."
export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib
mate-terminal --command /home/odroid/BicycleCamera/Chameleon_Test_Linux/Debug/Chameleon_Test_Linux

