###  cmake 필요 최소 버전
cmake_minimum_required(VERSION 2.8.3)
###  ${PROJECT_NAME}이라는 변수에 저장
project(tb2_base_pkg)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    serial
    std_msgs

)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES tb2_base_pkg
  CATKIN_DEPENDS roscpp std_msgs
  DEPENDS system_lib
)

include_directories(
  include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(gen_vel_node src/gen_vel_node.cpp)
add_dependencies(gen_vel_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(gen_vel_node ${catkin_LIBRARIES})

add_executable(joy2vel src/joy2vel.cpp)
add_dependencies(joy2vel ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(joy2vel ${catkin_LIBRARIES})

add_executable(head_controller src/head_controller.cpp)
add_dependencies(head_controller ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(head_controller ${catkin_LIBRARIES})
