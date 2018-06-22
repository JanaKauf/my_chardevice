#!/bin/sh

DEVICE=my_chardevice

sudo insmod $DEVICE.ko
sudo mknod /dev/$DEVICE c 240 0
sudo chmod 777 /dev/$DEVICE
echo "DONE"