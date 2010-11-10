BASEDIRBOINC = /Users/carlgt1/projects/boinc
BASEDIRQCN = /Users/carlgt1/projects/qcn

UTILDIR = $$BASEDIRQCN/client/util
MAINDIR = $$BASEDIRQCN/client/main
SENSORDIR = $$BASEDIRQCN/client/sensor
GRAPHICSDIR = $$BASEDIRQCN/client/graphics
BINDIR = $$BASEDIRQCN/client/bin

CPPFLAGS = \
        -Wall \
        -DQCN -D_THREAD_SAFE -D_ZLIB \
        -O2

LIBS +=

DEFINES += _ZLIB QCN _THREAD_SAFE

INCLUDEPATH += \
        $$BASEDIRQCN \
        $$MAINDIR \
        $$SENSORDIR \
        $$UTILDIR \
        $$BASEDIRBOINC/lib \
        $$BASEDIRBOINC/api \
        $$BASEDIRBOINC/zip \
        $$GRAPHICSDIR


HEADERS       += glwidget.h \
                myframe.h \
                $$MAINDIR/main.h \
                $$MAINDIR/define.h \



SOURCES       = glwidget.cpp \
                qcnqt.cpp \
                myframe.cpp


QT           += opengl


# install
target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qcnlive.pro
sources.path = ./
INSTALLS += target sources
