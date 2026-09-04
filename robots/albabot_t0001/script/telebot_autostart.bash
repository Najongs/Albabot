#!/bin/bash



rosclean purge << @
y
@


source /home/swpark/mywork/albabot_t0001/ros/devel/setup.bash &
roslaunch tb1_base_pkg albabot_t0001.launch & 


node /home/swpark/mywork/albabot_t0001/ui/main.js &
sleep 10
/opt/google/chrome/chrome -kiosk http://impala.redirectme.net:3000/welcome.html
