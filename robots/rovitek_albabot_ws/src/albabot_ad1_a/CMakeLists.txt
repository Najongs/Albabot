###  cmake 필요 최소 버전
cmake_minimum_required(VERSION 2.8.3)
###  ${PROJECT_NAME}이라는 변수에 저장
project(albabot_ad1_a)

find_package(catkin REQUIRED COMPONENTS
    roscpp
    rospy
    std_msgs    
    tf
    nav_msgs
    message_generation
)

add_service_files(FILES setnode.srv clearnode.srv savemap.srv)
generate_messages(DEPENDENCIES albabot_ad1_a)

catkin_package(
  #INCLUDE_DIRS include
  #INCLUDE_DIRS ~/mywork/albabot_alpha1/sw/albabot_alpha1_ws/devel/include
  LIBRARIES albabot_ad1_a
  CATKIN_DEPENDS roscpp std_msgs tf nav_msgs  #albabot_msgstf nav_msgs
  DEPENDS system_lib
)

include_directories(
  include
  ../../devel/include/
  ${catkin_INCLUDE_DIRS}
)

add_executable(agv_pathplanner src/agv_pathplanner.cpp include/PathPlanner.cpp)
add_dependencies(agv_pathplanner ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_pathplanner ${catkin_LIBRARIES})

add_executable(agv_mapping src/agv_mapping.cpp include/agvteleop.cpp)
add_dependencies(agv_mapping ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_mapping ${catkin_LIBRARIES})

add_executable(agv_driving_m2 src/agv_driving_m2.cpp)
add_dependencies(agv_driving_m2 ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_driving_m2 ${catkin_LIBRARIES})

add_executable(agv_driving_m1 src/agv_driving_m1.cpp)
add_dependencies(agv_driving_m1 ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(agv_driving_m1 ${catkin_LIBRARIES})

