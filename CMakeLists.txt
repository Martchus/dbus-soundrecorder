cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

# set meta data
project(dbus-soundrecorder)
set(META_PROJECT_NAME ${PROJECT_NAME})
set(META_PROJECT_VARNAME DBUS_SOUNDRECORDER)
set(META_PROJECT_TYPE application)
set(META_APP_NAME "D-Bus Sound Recorder")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "Records sound from Pulse Audio with ffmpeg while watching D-Bus to determine tracks and meta information")
set(META_VERSION_MAJOR 1)
set(META_VERSION_MINOR 2)
set(META_VERSION_PATCH 4)

# add project files
set(HEADER_FILES
    ffmpeglauncher.h
    playerwatcher.h
)
set(SRC_FILES
    ffmpeglauncher.cpp
    main.cpp
    playerwatcher.cpp
)

set(DBUS_FILES
    org.freedesktop.DBus.Properties.xml
    org.mpris.MediaPlayer2.xml
    org.mpris.MediaPlayer2.Player.xml
)

set(DOC_FILES
    README.md
)

# find c++utilities
set(CONFIGURATION_PACKAGE_SUFFIX "" CACHE STRING "sets the suffix for find_package() calls to packages configured via c++utilities")
find_package(c++utilities${CONFIGURATION_PACKAGE_SUFFIX} 5.0.0 REQUIRED)
use_cpp_utilities()

# find qtutilities
find_package(qtutilities${CONFIGURATION_PACKAGE_SUFFIX} 6.0.0 REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${QT_UTILITIES_MODULE_DIRS})

# include modules to apply configuration
include(BasicConfig)
include(QtConfig)
include(WindowsResources)
include(AppTarget)
include(ShellCompletion)
include(ConfigHeader)
