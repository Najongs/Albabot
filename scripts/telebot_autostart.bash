#!/bin/bash


rosclean purge << @
y
@


source $HOME/albabot_t0001/ros2/devel/setup.bash &
roslaunch tb2_base_pkg tele_presence_manager.launch ;


#node /home/swpark/mywork/albabot_t0001/ui/main.js &
#sleep 10
#/opt/google/chrome/chrome -kiosk http://impala.redirectme.net:3000/welcome.html
