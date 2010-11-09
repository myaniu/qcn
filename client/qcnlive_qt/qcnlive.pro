HEADERS       = glwidget.h \
                qcnqt.h \
                myframe.h
SOURCES       = glwidget.cpp \
                qcnqt.cpp \
                myframe.cpp \
           ../util/sac.cpp \
           ../util/qcn_thread.cpp \
           ../util/trickleup.cpp \
           ../util/trickledown.cpp \
           ../util/md5.cpp \
           ../util/execproc.cpp \
           ../util/qcn_util.cpp \
           ../util/qcn_signal.cpp \
           ../util/qcn_curl.cpp \
           ../util/gzstream.cpp \
      ../graphics/qcn_graphics.cpp \
      ../graphics/qcn_2dplot.cpp \
      ../graphics/qcn_earth.cpp \
      ../graphics/qcn_cube.cpp \
      ../graphics/nation_boundary.cpp \
      ../graphics/coastline.cpp \
      ../graphics/plate_boundary.cpp \
           ../util//cserialize.cpp \
           ../main/qcn_shmem.cpp \
           ../main/qcn_thread_sensor.cpp \
           ../main/qcn_thread_sensor_util.cpp \
           ../main/qcn_thread_sensor_loop.cpp \
           ../main/qcn_thread_time.cpp \
           ../main/main.cpp \
           ../sensor/csensor.cpp \
           ../sensor/csensor_mac_laptop.cpp \
           ../sensor/csensor_usb_motionnodeaccel.cpp \
           ../sensor/csensor_mac_usb_jw.cpp \
           ../sensor/csensor_mac_usb_jw24f14.cpp 

QT           += opengl

target.path = ../bin
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS qcnlive.pro
INSTALLS += target sources

