#!/bin/bash

usrname="odroid"

if [ "$(id -u)" = "0" ]
then
    echo
    echo "This script will add the ftdi_sio drivers to the blacklist so that the D2XX";
    echo "will work with the lens driver and GPS devices.";
    echo 
    echo
else
    echo
    echo "This script needs to be run as root.";
    echo "eg.";
    echo "sudo ftdi_config.sh";
    echo
    exit 0
fi

BlacklistFile="/etc/modprobe.d/blacklist_ftdi.conf";
echo
echo "Writing the blacklist file.";

# Add gpio control to odroid user to allow usage of the gpio pins
echo "blacklist ftdi_sio" 1>$BlacklistFile



echo
echo "Configuration complete. A reboot may be required on some systems for changes to take effect";
echo

exit 0
