cmake_minimum_required(VERSION 3.0.2)
project(ad1_socket)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    std_msgs
)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES ad1_socket
  CATKIN_DEPENDS roscpp std_msgs albabot_msgs
  DEPENDS system_lib
)

include_directories(
  include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(ad1_ui_socket src/ad1_ui_socket.cpp)
add_dependencies(ad1_ui_socket ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ad1_ui_socket ${catkin_LIBRARIES})
