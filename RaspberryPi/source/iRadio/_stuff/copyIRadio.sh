#!/bin/bash

echo "Removing old stuff..."
echo chris | sudo rm -rf /media/christian/rootfs/usr/IRadio/*  

echo "Copy IRadio files..."
sudo mkdir /media/christian/rootfs/usr/IRadio
sudo cp /usr/development/iRadio/output/ARMV6/* /media/christian/rootfs/usr/IRadio

echo "Done...!"
