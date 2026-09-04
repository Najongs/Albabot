#!/bin/bash

echo  'KERNEL=="ttyUSB*", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6014", ATTRS{serial}=="ROVITEK", MODE:="0666", GROUP:="dialout",  SYMLINK+="ttyUSB-Albabot"' >/etc/udev/rules.d/99-albabot.rules

echo  'KERNEL=="ttyUSB*", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6014", ATTRS{serial}=="FT4TCL28", MODE:="0666", GROUP:="dialout",  SYMLINK+="ttyUSB-Head"' >/etc/udev/rules.d/99-head.rules

echo  'KERNEL=="video*", ATTRS{idVendor}=="1e4e", ATTRS{idProduct}=="0100", SYMLINK+="video_thermal"' >/etc/udev/rules.d/99-albabot_termal.rules



service udev reload
sleep 2
service udev restart

