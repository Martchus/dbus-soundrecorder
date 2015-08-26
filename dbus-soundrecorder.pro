projectname = dbus-soundrecorder
appname = "D-Bus Sound Recorder"
appauthor = Martchus
appurl = "https://github.com/$${appauthor}/$${projectname}"
VERSION = 1.0.0

# include ../../common.pri when building as part of a subdirs project; otherwise include general.pri
!include(../../common.pri) {
    !include(./general.pri) {
        error("Couldn't find the common.pri or the general.pri file!")
    }
}

TEMPLATE = app

CONFIG += console

QT += core dbus

SOURCES += main.cpp \
    playerwatcher.cpp \
    ffmpeglauncher.cpp

HEADERS += \
    playerwatcher.h \
    ffmpeglauncher.h

DBUS_INTERFACES += \
    org.freedesktop.DBus.Properties.xml \
    org.mpris.MediaPlayer2.xml \
    org.mpris.MediaPlayer2.Player.xml

DISTFILES += \
    README.md \
    LICENSE

# libs and includepath
CONFIG(debug, debug|release) {
    LIBS += -lc++utilitiesd
} else {
    LIBS += -lc++utilities
}

# installs
target.path = $$(INSTALL_ROOT)/bin
INSTALLS += target


