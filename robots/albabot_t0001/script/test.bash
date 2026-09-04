#!/bin/bash

node ../ui/main.js &

#source {HOME}/mywork/rvt/albabot_ad1/rvt_ad1_ws/devel/setup.bash
source  ../rvt_ad1_ws/devel/setup.bash;

rosclean purge << @
y
@

#roslaunch ad1_sensor ad1_sensor.launch & 
#sleep 5



/opt/google/chrome/chrome --full-screen  --kiost http://192.168.1.253:3000/hello.html
