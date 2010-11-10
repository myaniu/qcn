HEADERS       = glwidget.h \
                mainwindow.h
SOURCES       = glwidget.cpp \
                main.cpp \
                mainwindow.cpp
QT           += opengl

QCNBIN = ../bin/
# install
target.path = $$QCNBIN
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qcnlive.pro
sources.path = ./
INSTALLS += target sources
