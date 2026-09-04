###  cmake 필요 최소 버전
cmake_minimum_required(VERSION 2.8.3)
###  ${PROJECT_NAME}이라는 변수에 저장
project(ad1_sound)


find_package(catkin REQUIRED COMPONENTS
    roscpp
    std_msgs
)

catkin_package(
  LIBRARIES ad1_sound
  CATKIN_DEPENDS roscpp std_msgs
  DEPENDS system_lib
)



include_directories(
  include
#  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(agv_sound src/agv_sound.cpp)
add_dependencies(agv_sound ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_sound ${catkin_LIBRARIES})
