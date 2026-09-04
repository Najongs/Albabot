cmake_minimum_required(VERSION 2.8.3)
project(albabot_ad1_pkg)


find_package(catkin REQUIRED COMPONENTS
    roscpp
    std_msgs
    sensor_msgs
    geometry_msgs
    tf
    tf2
    tf2_ros
    nav_msgs
    sound_play

)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES albabot_base_pkg
  CATKIN_DEPENDS roscpp std_msgs sensor_msgs tf tf2 tf2_ros sound_play geometry_msgs nav_msgs albabot_msgs
  DEPENDS system_lib
)

include_directories(
  #include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)


add_executable(ad1_sound src/ad1_sound.cpp)
add_dependencies(ad1_sound ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ad1_sound ${catkin_LIBRARIES} )


add_executable(ad1_sensor src/ad1_sensor.cpp)
add_dependencies(ad1_sensor ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ad1_sensor ${catkin_LIBRARIES} )


add_executable(joy_teleop src/joy_teleop.cpp)
add_dependencies(joy_teleop ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(joy_teleop ${catkin_LIBRARIES} )

add_executable(joy_agv_teleop src/joy_agv_teleop.cpp)
add_dependencies(joy_agv_teleop ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(joy_agv_teleop ${catkin_LIBRARIES} )

add_executable(ad1_tf src/ad1_tf.cpp)
add_dependencies(ad1_tf ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ad1_tf ${catkin_LIBRARIES} )

add_executable(ad1_lidar src/ad1_lidar.cpp)
add_dependencies(ad1_lidar ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ad1_lidar ${catkin_LIBRARIES} )
