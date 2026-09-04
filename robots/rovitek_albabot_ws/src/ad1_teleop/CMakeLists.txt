###  cmake 필요 최소 버전
cmake_minimum_required(VERSION 2.8.3)
###  ${PROJECT_NAME}이라는 변수에 저장
project(ad1_teleop)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    rospy
    std_msgs
    tf
    nav_msgs
    message_generation
)

find_package(Boost REQUIRED COMPONENTS thread)

catkin_package(
  LIBRARIES ad1_teleop
  CATKIN_DEPENDS roscpp std_msgs
  DEPENDS system_lib
)

include_directories(
  include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(joystick src/joystick.cpp)
#add_dependencies(joystick ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(joystick ${catkin_LIBRARIES})

add_executable(keyboard src/keyboard.cpp)
#add_dependencies(keyboard ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(keyboard ${catkin_LIBRARIES})
