#!/bin/bash

sudo cp -f /etc/modprobe.d/blacklist.conf /etc/modprobe.d/blacklist.conf.orig
echo -e '\nblacklist uas\nblacklist usb_storage\n' | sudo tee -a /etc/modprobe.d/blacklist.conf
sudo update-grub