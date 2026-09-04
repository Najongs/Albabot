###  cmake 필요 최소 버전
cmake_minimum_required(VERSION 2.8.3)
###  ${PROJECT_NAME}이라는 변수에 저장
project(ad1_sensor)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    std_msgs
    tf
    nav_msgs
    sensor_msgs
)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES ad1_sensor
  CATKIN_DEPENDS roscpp std_msgs albabot_msgs tf nav_msgs sensor_msgs
  DEPENDS system_lib
)

include_directories(
  include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(lidar_merge src/lidar_merge.cpp)
add_dependencies(lidar_merge ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(lidar_merge ${catkin_LIBRARIES})


add_executable(agv_lidar_obstacle src/agv_lidar_obstacle.cpp)
add_dependencies(agv_lidar_obstacle ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_lidar_obstacle ${catkin_LIBRARIES})

add_executable(agv_odom src/agv_odom.cpp)
add_dependencies(agv_odom ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_odom ${catkin_LIBRARIES})
