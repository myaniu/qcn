BASEDIR = /Users/carlgt1/projects

BASEDIRBOINC = $$BASEDIR/boinc
BASEDIRQCN = $$BASEDIR/qcn

UTILDIR = $$BASEDIRQCN/client/util
MAINDIR = $$BASEDIRQCN/client/main
SENSORDIR = $$BASEDIRQCN/client/sensor
GRAPHICSDIR = $$BASEDIRQCN/client/graphics
BINDIR = $$BASEDIRQCN/client/bin

#CFLAGS = -Wno-deprecated
#QMAKE_CFLAGS_DEBUG += $$CFLAGS -D_DEBUG
#QMAKE_CFLAGS_RELEASE += $$CFLAGS
#QMAKE_CXXFLAGS_DEBUG += $$CFLAGS -D_DEBUG
#QMAKE_CXXFLAGS_RELEASE += $$CFLAGS
#   LIBS += -bind_at_load -framework IOKit -framework Foundation \
#          -framework ScreenSaver -framework Carbon -framework Cocoa

SRC_SENSOR = $$SENSORDIR/csensor_mac_laptop.cpp \
           $$SENSORDIR/csensor_usb_motionnodeaccel.cpp \
           $$SENSORDIR/csensor_mac_usb_jw.cpp \
           $$SENSORDIR/csensor_mac_usb_jw24f14.cpp \
           $$SENSORDIR/csensor.cpp

SRC_MAIN =  $$MAINDIR/qcn_shmem.cpp \
           $$MAINDIR/qcn_thread_sensor.cpp \
           $$MAINDIR/qcn_thread_sensor_util.cpp \
           $$MAINDIR/qcn_thread_sensor_loop.cpp \
           $$MAINDIR/qcn_thread_time.cpp \
           $$MAINDIR/main.cpp

SRC_UTIL = $$UTILDIR/cserialize.cpp \
           $$UTILDIR/sac.cpp \
           $$UTILDIR/qcn_thread.cpp \
           $$UTILDIR/trickleup.cpp \
           $$UTILDIR/trickledown.cpp \
           $$UTILDIR/md5.cpp \
           $$UTILDIR/execproc.cpp \
           $$UTILDIR/qcn_util.cpp \
           $$UTILDIR/qcn_signal.cpp \
           $$UTILDIR/qcn_curl.cpp \
           $$UTILDIR/gzstream.cpp

SRC_GRAPHICS = $$GRAPHICSDIR/qcn_graphics.cpp \
      $$GRAPHICSDIR/qcn_2dplot.cpp \
      $$GRAPHICSDIR/qcn_earth.cpp \
      $$GRAPHICSDIR/qcn_cube.cpp \
      $$GRAPHICSDIR/nation_boundary.cpp \
      $$GRAPHICSDIR/coastline.cpp \
      $$GRAPHICSDIR/plate_boundary.cpp

LIBS += -bind_at_load -framework IOKit -framework Foundation -framework Screensaver -framework Cocoa

DEFINES += _USE_NTPDATE_EXEC_ QCNLIVE GRAPHICS_PROGRAM APP_GRAPHICS _ZLIB QCN _THREAD_SAFE

INCLUDEPATH += \
        $$BASEDIRQCN \
        $$BASEDIRQCN/jpeg-6b \
        $$BASEDIRQCN/zlib \
        $$BASEDIRQCN/curl \
        $$MAINDIR \
        $$SENSORDIR \
        $$UTILDIR \
        $$BASEDIRBOINC/lib \
        $$BASEDIRBOINC/api \
        $$BASEDIRBOINC/zip \
        $$GRAPHICSDIR


HEADERS       += qcnqt.h \
                glwidget.h \
                myframe.h \
                $$MAINDIR/main.h \
                $$MAINDIR/define.h \



SOURCES       = glwidget.cpp \
                qcnqt.cpp \
                myframe.cpp \
                $$SRC_MAIN \
                $$SRC_UTIL \
                $$SRC_SENSOR \
                $$SRC_GRAPHICS

QT           += opengl


# install
target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qcnlive.pro
sources.path = ./
INSTALLS += target sources
