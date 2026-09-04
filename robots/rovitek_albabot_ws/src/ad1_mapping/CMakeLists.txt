cmake_minimum_required(VERSION 3.0.2)
project(ad1_mapping)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    std_msgs
    tf
    nav_msgs

)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES ad1_mapping
  CATKIN_DEPENDS roscpp std_msgs tf nav_msgs albabot_msgs
  DEPENDS system_lib
)

include_directories(
  include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(node_register src/node_register.cpp )
add_dependencies(node_register ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(node_register ${catkin_LIBRARIES})

add_executable(node_display src/node_display.cpp include/agvmapper.cpp)
add_dependencies(node_display ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(node_display ${catkin_LIBRARIES})

add_executable(odom_gen src/odom_gen.cpp include/agvmapper.cpp)
add_dependencies(odom_gen ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(odom_gen ${catkin_LIBRARIES})

add_executable(node_gen src/node_gen.cpp include/agvmapper.cpp)
add_dependencies(node_gen ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(node_gen ${catkin_LIBRARIES})

add_executable(ad1_mapping src/ad1_mapping.cpp include/agvmapper.cpp)
add_dependencies(ad1_mapping ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(ad1_mapping ${catkin_LIBRARIES})
