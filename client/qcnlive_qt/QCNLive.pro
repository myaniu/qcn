BASEDIR = /Users/carlgt1/projects

BASEDIRBOINC = $$BASEDIR/boinc
BASEDIRQCN = $$BASEDIR/qcn

UTILDIR = $$BASEDIRQCN/client/util
MAINDIR = $$BASEDIRQCN/client/main
SENSORDIR = $$BASEDIRQCN/client/sensor
GRAPHICSDIR = $$BASEDIRQCN/client/graphics
BINDIR = $$BASEDIRQCN/client/bin
BAPIDIR = $$BASEDIRBOINC/api
BLIBDIR = $$BASEDIRBOINC/lib

CPP = gcc-4.0
CXX = g++-4.0
CFLAGS = -Wno-deprecated
QMAKE_CFLAGS_DEBUG = $$CFLAGS -D_DEBUG -g -O0
QMAKE_CFLAGS_RELEASE = $$CFLAGS -O2
QMAKE_CXXFLAGS_DEBUG = $$QMAKE_CFLAGS_DEBUG
QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE
LIBS += -framework IOKit -framework Carbon \
   -L$$BASEDIRQCN/client/mac_build \
     -lboinc_zip -ljpeg-universal -lcurl-universal -lz-universal

#   LIBS += -bind_at_load -framework IOKit -framework Foundation \
#          -framework ScreenSaver -framework Carbon -framework Cocoa

SRC_SENSOR = $$SENSORDIR/csensor_mac_laptop.cpp \
           $$SENSORDIR/csensor_usb_motionnodeaccel.cpp \
           $$SENSORDIR/csensor_mac_usb_onavi01.cpp \
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

SRC_BOINC = $$BAPIDIR/boinc_api.cpp \
   $$BAPIDIR/graphics2_util.cpp \
   $$BAPIDIR/graphics2.cpp \
   $$BAPIDIR/gutil.cpp \
   $$BAPIDIR/texfont.cpp \
   $$BAPIDIR/texture.cpp \
   $$BLIBDIR/parse.cpp \
   $$BLIBDIR/shmem.cpp \
   $$BLIBDIR/str_util.cpp \
   $$BLIBDIR/util.cpp \
   $$BLIBDIR/diagnostics.cpp \
   $$BLIBDIR/filesys.cpp \
   $$BLIBDIR/mfile.cpp \
   $$BLIBDIR/miofile.cpp \
   $$BLIBDIR/app_ipc.cpp \
   $$BLIBDIR/hostinfo.cpp \
   $$BLIBDIR/proxy_info.cpp \
   $$BLIBDIR/prefs.cpp \
   $$BLIBDIR/url.cpp \
   $$BLIBDIR/coproc.cpp \
   $$BLIBDIR/mac/mac_backtrace.cpp \
   $$BLIBDIR/mac/QBacktrace.c \
   $$BLIBDIR/mac/QCrashReport.c \
   $$BLIBDIR/mac/QMachOImage.c \
   $$BLIBDIR/mac/QMachOImageList.c \
   $$BLIBDIR/mac/QSymbols.c \
   $$BLIBDIR/mac/QTaskMemory.c

LIBS += -framework IOKit -framework Foundation

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
                $$SRC_GRAPHICS \
                $$SRC_BOINC


QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
CONFIG+=x86 ppc
QT           += opengl


# install
target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS QCNLive.pro
sources.path = ./
INSTALLS += target sources
