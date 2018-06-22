#!/bin/sh

DEVICE=my_chardevice

if [ls -l /dev/ |grep $DEVICE -gt 0 ]; then
	sudo rmmod $DEVICE.ko
	echo "MODULE REMOVED"
	sudo rm -rf /dev/$DEVICE
	echo "DEVICE REMOVED"
fi
