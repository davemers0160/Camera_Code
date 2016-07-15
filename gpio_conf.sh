#!/bin/bash

grpname="gpio"
usrname="odroid"

if [ "$(id -u)" = "0" ]
then
    echo
    echo "This script will assist users in configuring their udev rules to allow";
    echo "access to the GPIO pins on the Odroid. The script will create a udev rule";
    echo "which will add the user permissions to the gpio group.";
    echo
    echo
else
    echo
    echo "This script needs to be run as root.";
    echo "eg.";
    echo "sudo gpio_conf";
    echo
    exit 0
fi

groupadd -f $grpname
usermod -a -G $grpname $usrname

UdevFile="/etc/udev/rules.d/39-gpio.rules";
echo
echo "Writing the udev rules file.";

# Add gpio control to odroid user to allow usage of the gpio pins
echo "SUBSYSTEM==\"gpio\", KERNEL==\"gpiochip*\", ACTION==\"add\", PROGRAM=\"/bin/sh -c 'chown root:gpio /sys/class/gpio/export /sys/class/gpio/unexport ; chmod 220 /sys/class/gpio/export /sys/class/gpio/unexport'\"" 1>$UdevFile
echo "SUBSYSTEM==\"gpio\", KERNEL==\"gpio*\", ACTION==\"add\", PROGRAM=\"/bin/sh -c 'chown root:gpio /sys%p/active_low /sys%p/direction /sys%p/edge /sys%p/value ; chmod 660 /sys%p/active_low /sys%p/direction /sys%p/edge /sys%p/value'\"" 1>>$UdevFile


/etc/init.d/udev restart

echo
echo "Configuration complete. A reboot may be required on some systems for changes to take effect";
echo

exit 0
