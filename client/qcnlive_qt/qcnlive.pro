HEADERS       = glwidget.h \
                qcnqt.h \
                myframe.h
SOURCES       = glwidget.cpp \
                qcnqt.cpp \
                myframe.cpp
QT           += opengl

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/opengl/qcnlive
#sources.path = $$[QT_INSTALL_EXAMPLES]/opengl/qcnlive
target.path = ../bin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qcnlive.pro
INSTALLS += target sources

#symbian: include($$QT_SOURCE_TREE/examples/symbianpkgrules.pri)
