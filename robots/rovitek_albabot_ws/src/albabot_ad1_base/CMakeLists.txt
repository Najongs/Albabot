###  cmake 필요 최소 버전
cmake_minimum_required(VERSION 2.8.3)
###  ${PROJECT_NAME}이라는 변수에 저장
project(albabot_ad1_base)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    rospy    
    std_msgs

)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES albabot_ad1_pkg
  CATKIN_DEPENDS roscpp std_msgs albabot_msgs
  DEPENDS system_lib
)

include_directories(
  #include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)


add_executable(agv_lidar_obstacle src/agv_lidar_obstacle.cpp)
add_dependencies(agv_lidar_obstacle ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_lidar_obstacle ${catkin_LIBRARIES})

add_executable(agv_joystick src/agv_joystick.cpp)
add_dependencies(agv_joystick ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_joystick ${catkin_LIBRARIES})


add_executable(ad1_lidar src/ad1_lidar.cpp)
add_dependencies(ad1_lidar ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ad1_lidar ${catkin_LIBRARIES})

add_executable(agv_patrol2 src/agv_patrol2.cpp)
add_dependencies(agv_patrol2 ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_patrol2 ${catkin_LIBRARIES})

add_executable(agv_patrol src/agv_patrol.cpp)
add_dependencies(agv_patrol ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_patrol ${catkin_LIBRARIES} )

add_executable(joy_teleop src/joy_teleop.cpp)
add_dependencies(joy_teleop ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(joy_teleop ${catkin_LIBRARIES} )
