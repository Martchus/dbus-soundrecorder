# meta data
projectname = dbus-soundrecorder
appname = "D-Bus Sound Recorder"
appauthor = Martchus
appurl = "https://github.com/$${appauthor}/$${projectname}"
QMAKE_TARGET_DESCRIPTION = "Records sound from Pulse Audio with ffmpeg while watching D-Bus to determine tracks and meta information."
VERSION = 1.2.1

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

# basic configuration: console application
TEMPLATE = app
CONFIG += console
QT += core dbus

# add project files
HEADERS += \
    playerwatcher.h \
    ffmpeglauncher.h

SOURCES += \
    main.cpp \
    playerwatcher.cpp \
    ffmpeglauncher.cpp

DBUS_INTERFACES += \
    org.freedesktop.DBus.Properties.xml \
    org.mpris.MediaPlayer2.xml \
    org.mpris.MediaPlayer2.Player.xml

OTHER_FILES += \
    README.md \
    LICENSE \
    CMakeLists.txt \
    resources/config.h.in \
    resources/windows.rc.in

# libs and includepath
CONFIG(debug, debug|release) {
    LIBS += -lc++utilitiesd
} else {
    LIBS += -lc++utilities
}

# installs
target.path = $$(INSTALL_ROOT)/bin
INSTALLS += target


