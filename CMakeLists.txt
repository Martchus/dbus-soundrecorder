cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

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

# meta data
set(META_PROJECT_NAME dbus-soundrecorder)
set(META_APP_NAME "D-Bus Sound Recorder")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "Records sound from Pulse Audio with ffmpeg while watching D-Bus to determine tracks and meta information.")
set(META_VERSION_MAJOR 1)
set(META_VERSION_MINOR 2)
set(META_VERSION_PATCH 2)

# find c++utilities
find_package(c++utilities 4.0.0 REQUIRED)
use_cpp_utilities()

# find qtutilities
find_package(qtutilities 4.0.0 REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${QT_UTILITIES_MODULE_DIRS})

# include modules to apply configuration
include(BasicConfig)
include(QtConfig)
include(WindowsResources)
include(AppTarget)
