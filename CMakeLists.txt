cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# meta data
set(META_PROJECT_NAME dbus-soundrecorder)
set(META_APP_NAME "D-Bus Sound Recorder")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META APP_DESCRIPTION "Records sound from Pulse Audio with ffmpeg while watching D-Bus to determine tracks and meta information.")
set(META_VERSION_MAJOR 1)
set(META_VERSION_MINOR 1)
set(META_VERSION_PATCH 0)

# define project
project(${META_PROJECT_NAME})

# stringification of meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(APP_DESCRIPTION_STR "\"${APP_DESCRIPTION}\"")
set(META_APP_VERSION_STR "\"${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}\"")

# add configuration header
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

# add source and header files
file(GLOB_RECURSE SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
file(GLOB_RECURSE DBUS_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "org.*.xml")

# check required Qt 5 modules
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)

# enable moc
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# generate DBus interfaces
qt5_add_dbus_interfaces(SRC_FILES ${DBUS_FILES})
add_definitions(-DCMAKE_BUILD)

# executable and linking
add_executable(${META_PROJECT_NAME} ${HEADER_FILES} ${SRC_FILES})
target_link_libraries(${META_PROJECT_NAME} c++utilities Qt5::Core Qt5::DBus)

# enable C++11
set_property(TARGET ${META_PROJECT_NAME} PROPERTY CXX_STANDARD 11)

# add install target
install(TARGETS ${META_PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
