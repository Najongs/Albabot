###  cmake 필요 최소 버전
cmake_minimum_required(VERSION 2.8.3)
###  ${PROJECT_NAME}이라는 변수에 저장
project(tb1_base_pkg)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    std_msgs

)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES tb1_base_pkg
  CATKIN_DEPENDS roscpp std_msgs albabot_msgs
  DEPENDS system_lib
)

include_directories(
  include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(gen_vel_node src/gen_vel_node.cpp )
add_dependencies(gen_vel_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(gen_vel_node ${catkin_LIBRARIES})
