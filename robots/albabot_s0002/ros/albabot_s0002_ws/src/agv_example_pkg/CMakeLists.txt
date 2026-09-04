cmake_minimum_required(VERSION 2.8.3)
project(agv_example_pkg)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    std_msgs
    tf
)

catkin_package(
  #INCLUDE_DIRS include
  LIBRARIES albabot_base_pkg
  CATKIN_DEPENDS roscpp std_msgs albabot_msgs
  DEPENDS system_lib
)

include_directories(
  #include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)


add_executable(albabot_mapping src/albabot_mapping.cpp)
add_dependencies(albabot_mapping ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(albabot_mapping ${catkin_LIBRARIES} )

add_executable(ex1_node src/ex1_node.cpp)
add_dependencies(ex1_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ex1_node ${catkin_LIBRARIES} )
