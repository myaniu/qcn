mac:BASEDIR = /Users/carlgt1/projects
win32:BASEDIR = c:/projects

BASEDIRBOINC = $$BASEDIR/boinc
BASEDIRQCN = $$BASEDIR/qcn

UTILDIR = $$BASEDIRQCN/client/util
MAINDIR = $$BASEDIRQCN/client/main
SENSORDIR = $$BASEDIRQCN/client/sensor
GRAPHICSDIR = $$BASEDIRQCN/client/graphics
BINDIR = $$BASEDIRQCN/client/bin
BAPIDIR = $$BASEDIRBOINC/api
BLIBDIR = $$BASEDIRBOINC/lib

CFLAGS = -Wno-deprecated
#QMAKE_CFLAGS_DEBUG += $$CFLAGS -D_DEBUG -D_DEBUG_QCNLIVE -g -O0
#QMAKE_CFLAGS_RELEASE += $$CFLAGS -O2
#QMAKE_CXXFLAGS_DEBUG += $$QMAKE_CFLAGS_DEBUG
#QMAKE_CXXFLAGS_RELEASE += $$QMAKE_CFLAGS_RELEASE

# Mac specific settings
mac:CPP = gcc-4.0
mac:CXX = g++-4.0

mac:LIBS += -framework IOKit -framework Carbon \
   -L$$BASEDIRQCN/client/mac_build \
     -lboinc_zip -ljpeg-universal -lcurl-universal \
     -lz-universal -lfreetype-universal -lftgl-universal

win32:LIBS += -L$$BASEDIRQCN/client/win_build \
   wsock32.lib hid.lib setupapi.lib winmm.lib glu32.lib opengl32.lib \
   comctl32.lib boinc_api.lib boinc_lib.lib boinc_zip.lib curllib.lib \
   jpeglib32.lib zlib32.lib \
   MotionNodeAccelAPI.lib

#win32:WININCLUDEPATH = c:\\Program Files (x86)\\Microsoft #Visual Studio 9.0\\VC\\ATLMFC\\INCLUDE;c:\\Program Files (x86)##\\Microsoft Visual Studio 9.0\\VC\\INCLUDE;C:\\Program Files#\\Microsoft SDKs\\Windows\\v6.0A\\include;
#win32:WINDEFINES = WIN32 _WIN32 _CRT_SECURE_NO_DEPRECATE
#win32:WININCLUDE = windows.h

DEFINES += _USE_NTPDATE_EXEC_ QCNLIVE GRAPHICS_PROGRAM APP_GRAPHICS _ZLIB QCN _THREAD_SAFE CURL_STATICLIB _ZLIB $$WINDEF

# setup proper order of include paths
INCLUDEPATH += \
        $$BASEDIRQCN \
        $$BASEDIRQCN/jpeg-6b \
        $$BASEDIRQCN/zlib-1.2.5 \
        $$BASEDIRQCN/curl-7.19.5/include \
        $$BASEDIRQCN/ftgl-2.1.3/include \
        $$BASEDIRQCN/freetype-2.3.9/include \
        $$MAINDIR \
        $$SENSORDIR \
        $$UTILDIR \
        $$BASEDIRBOINC/lib \
        $$BASEDIRBOINC/api \
        $$BASEDIRBOINC/zip \
        $$WININCLUDEPATH \
        $$GRAPHICSDIR 

mac:ICON = $$BASEDIRQCN/doc/qcnmac.icns
mac:RC_FILE = $$BASEDIRQCN/doc/qcnmac.icns
mac:QMAKE_INFO_PLIST = Info.plist.mac

mac:SRC_SENSOR = $$SENSORDIR/csensor_mac_laptop.cpp \
           $$SENSORDIR/csensor_usb_motionnodeaccel.cpp \
           $$SENSORDIR/csensor_mac_usb_onavi01.cpp \
           $$SENSORDIR/csensor_mac_usb_jw.cpp \
           $$SENSORDIR/csensor_mac_usb_jw24f14.cpp \
           $$SENSORDIR/csensor.cpp

win32:SRC_SENSOR = $$SENSORDIR/csensor_win_laptop_hp.cpp \
           $$SENSORDIR/csensor_win_laptop_thinkpad.cpp \
           $$SENSORDIR/csensor_win_usb_jw.cpp \
           $$SENSORDIR/csensor_win_usb_jw24f14.cpp \
           $$SENSORDIR/csensor_usb_motionnodeaccel.cpp \
           $$SENSORDIR/csensor_win_usb_onavi01.cpp \
           $$SENSORDIR/csensor.cpp

linux:SRC_SENSOR = \
           $$SENSORDIR/csensor_linux_usb_jw.cpp \
           $$SENSORDIR/csensor_linux_usb_jw24f14.cpp \
           $$SENSORDIR/csensor_usb_motionnodeaccel.cpp \
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
           $$UTILDIR/ttfont.cpp \
           $$UTILDIR/gzstream.cpp

SRC_GRAPHICS = $$GRAPHICSDIR/qcn_graphics.cpp \
      $$GRAPHICSDIR/qcn_2dplot.cpp \
      $$GRAPHICSDIR/qcn_earth.cpp \
      $$GRAPHICSDIR/qcn_cube.cpp \
      $$GRAPHICSDIR/nation_boundary.cpp \
      $$GRAPHICSDIR/coastline.cpp \
      $$GRAPHICSDIR/plate_boundary.cpp

mac:MAC_SRC_BOINC = $$BLIBDIR/mac/mac_backtrace.cpp \
   $$BLIBDIR/mac/QBacktrace.c \
   $$BLIBDIR/mac/QCrashReport.c \
   $$BLIBDIR/mac/QMachOImage.c \
   $$BLIBDIR/mac/QMachOImageList.c \
   $$BLIBDIR/mac/QSymbols.c \
   $$BLIBDIR/mac/QTaskMemory.c

SRC_BOINC = $$BAPIDIR/boinc_api.cpp \
   $$BAPIDIR/graphics2_util.cpp \
   $$BAPIDIR/graphics2.cpp \
   $$BAPIDIR/gutil.cpp \
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
   $$BLIBDIR/coproc.cpp $$MAC_SRC_BOINC

HEADERS       += qcnqt.h $$WININCLUDE \
                glwidget.h \
                myframe.h \
                dlgsettings.h \
                dlgmakequake.h \
                qcnlive_define.h \
                icons.h \
                $$MAINDIR/main.h \
                $$MAINDIR/define.h


SOURCES       = glwidget.cpp \
                qcnqt.cpp \
                myframe.cpp \
                dlgsettings.cpp \
                dlgmakequake.cpp \
                $$SRC_MAIN \
                $$SRC_UTIL \
                $$SRC_SENSOR \
                $$SRC_GRAPHICS \
                $$SRC_BOINC


mac:QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
mac:QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
mac:CONFIG += x86 ppc app_bundle
#mac:CONFIG += x86 ppc app_bundle debug_and_release
#mac:CONFIG(debug) {
#  DEFINES += _DEBUG _DEBUG_QCNLIVE -g -O0
#}
#mac:CONFIG(release) {
#  DEFINES += -O2
#}
QT         += opengl


# install
target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS QCNLive.pro
sources.path = $$BINDIR
INSTALLS += target sources


