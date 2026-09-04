##############################################################################
# CMake
##############################################################################

cmake_minimum_required(VERSION 2.8.0)
project(qtgui_user)

##############################################################################
# Catkin
##############################################################################
# qt_build provides the qt cmake glue, roscpp the comms for a default talker
find_package(catkin REQUIRED COMPONENTS
    qt_build
    roscpp
    std_msgs
    sensor_msgs
    cv_bridge
    image_transport
    message_generation
    dynamic_reconfigure
    geometry_msgs
    message_filters
    rviz
    tf
    )

find_package(OpenCV)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${catkin_INCLUDE_DIRS})
# Use this to define what the package will export (e.g. libs, headers).
# Since the default here is to produce only a binary, we don't worry about
# exporting anything. 
catkin_package()

##############################################################################
# Qt Environment
##############################################################################
find_package(Boost REQUIRED COMPONENTS thread)
find_package(Qt5 COMPONENTS Core Gui Widgets PrintSupport Network)
set(CMAKE_INCLUDE_CURRENT_DIR ON)  ## add ui_mainwindow.h

### Qwt 외부 라이브러리 추가 ############################
#find_library(
#    LIBQWT
#    NAMES libqwt.so
#    HINTS /usr/local/qwt-6.1.3/lib #-lqwt
#    REQUIRED
#)
#################################################

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include

  ${catkin_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include/qtgui_user
)

#link_directories(${catkin_LIBRARY_DIRS})

##############################################################################
# Sections
##############################################################################

file(GLOB QT_FORMS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ui/*.ui)
file(GLOB QT_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    resources/*.qrc)
file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS
    include/qtgui_user/*.h)

QT5_ADD_RESOURCES(QT_RESOURCES_CPP ${QT_RESOURCES})
QT5_WRAP_UI(QT_FORMS_H ${QT_FORMS})
QT5_WRAP_CPP(QT_MOC_H ${QT_MOC} )

set(QT_LIBRARIES
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network
    )

##############################################################################
# Sources
##############################################################################

file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS
    src/*.cpp)

##############################################################################
# Binaries
##############################################################################

add_executable(qtgui_user ${QT_SOURCES} ${QT_RESOURCES_CPP} ${QT_FORMS_H} ${QT_MOC_H})
target_link_libraries(qtgui_user ${QT_LIBRARIES} ${catkin_LIBRARIES})
target_link_libraries(qtgui_user ${OpenCV_LIBRARIES}  )                                     # OpenCV
#target_link_libraries(qtgui_user ${LIBQWT} )                                                # Qwt
install(TARGETS qtgui_user RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

