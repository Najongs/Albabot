#!/bin/bash

#installation
#cp albabot.desktop  ~/.config/autostart/

export ROS_MASTER_URI=http://192.168.1.82:11311;
#export ROS_MASTER_URI=http://albabot-s0002:11311;
#rostopic list;
rostopic hz /robot_info
