cmake_minimum_required(VERSION 3.0.2)
project(albabot_autoscan)

# 순수 파이썬(rospy) 패키지: 메시지는 런타임에 import(albabot_msgs 는 exec 의존).
# 빌드 의존 없이 파이썬 스크립트만 설치한다.
find_package(catkin REQUIRED)

catkin_package()

catkin_install_python(PROGRAMS
  scripts/safety_guard.py
  scripts/explorer.py
  scripts/set_remote_mode.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
