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

# meta data
set(META_PROJECT_NAME dbus-soundrecorder)
set(META_APP_NAME "D-Bus Sound Recorder")
set(META_APP_AUTHOR "Martchus")
set(META_APP_URL "https://github.com/${META_APP_AUTHOR}/${META_PROJECT_NAME}")
set(META_APP_DESCRIPTION "Records sound from Pulse Audio with ffmpeg while watching D-Bus to determine tracks and meta information.")
set(META_VERSION_MAJOR 1)
set(META_VERSION_MINOR 1)
set(META_VERSION_PATCH 0)

# stringification of meta data
set(META_PROJECT_NAME_STR "\"${META_PROJECT_NAME}\"")
set(META_APP_NAME_STR "\"${META_APP_NAME}\"")
set(META_APP_AUTHOR_STR "\"${META_APP_AUTHOR}\"")
set(META_APP_URL_STR "\"${META_APP_URL}\"")
set(META_APP_DESCRIPTION_STR "\"${META_APP_DESCRIPTION}\"")
set(META_APP_VERSION ${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH})
set(META_APP_VERSION_STR "\"${META_APP_VERSION}\"")

# define project
project(${META_PROJECT_NAME})

# add configuration header
configure_file(
    "${PROJECT_SOURCE_DIR}/resources/config.h.in"
    "${PROJECT_BINARY_DIR}/resources/config.h"
)
include_directories("${PROJECT_BINARY_DIR}")

# add windows resource file
if(MINGW)
    # creation windows icon from png with ffmpeg if available
    find_program(FFMPEG_BIN ffmpeg avconv)
    if(FFMPEG_BIN)
        set(PNG_ICON_PATH "${PROJECT_SOURCE_DIR}/resources/icons/hicolor/128x128/apps/${META_PROJECT_NAME}.png")
        if(EXISTS ${PNG_ICON_PATH})
            set(WINDOWS_ICON_PATH "${PROJECT_BINARY_DIR}/${META_PROJECT_NAME}.ico")
            set(WINDOWS_ICON_RC_ENTRY "IDI_ICON1   ICON    DISCARDABLE \"${WINDOWS_ICON_PATH}\"")
            add_custom_command(
                OUTPUT "${WINDOWS_ICON_PATH}"
                COMMAND ${FFMPEG_BIN} -y -i "${PNG_ICON_PATH}" -vf crop=iw-20:ih-20:10:10,scale=64:64 "${WINDOWS_ICON_PATH}"
            )
        endif()
    endif(FFMPEG_BIN)
    # create windows rc file from template
    set(WINDOWS_EXT "exe")
    configure_file(
        "${PROJECT_SOURCE_DIR}/resources/windows.rc.in"
        "${PROJECT_BINARY_DIR}/resources/windows.rc"
    )
    # set windres as resource compiler
    set(RES_FILES "${PROJECT_BINARY_DIR}/resources/windows.rc")
    set(CMAKE_RC_COMPILER_INIT windres)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    enable_language(RC)
endif(MINGW)

# check required Qt 5 modules
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)

# enable moc
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# generate DBus interfaces
qt5_add_dbus_interfaces(SRC_FILES ${DBUS_FILES})

# disable new ABI (can't catch ios_base::failure with new ABI)
add_definitions(
    -D_GLIBCXX_USE_CXX11_ABI=0
    -DCMAKE_BUILD
)

# executable and linking
add_executable(${META_PROJECT_NAME} ${HEADER_FILES} ${SRC_FILES} ${RES_FILES} ${WINDOWS_ICON_PATH})
target_link_libraries(${META_PROJECT_NAME} c++utilities Qt5::Core Qt5::DBus)
set_target_properties(${META_PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
)

# add install target
install(TARGETS ${META_PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT binary
)
add_custom_target(install-binary
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-mingw-w64
    DEPENDS install-binary
)
add_custom_target(install-binary-strip
    DEPENDS ${META_PROJECT_NAME}
    COMMAND "${CMAKE_COMMAND}" -DCMAKE_INSTALL_DO_STRIP=1 -DCMAKE_INSTALL_COMPONENT=binary -P "${CMAKE_BINARY_DIR}/cmake_install.cmake"
)
add_custom_target(install-mingw-w64-strip
    DEPENDS install-binary-strip
)
