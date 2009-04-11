include $(top_srcdir)/Makefile.incl

QCNLIVEDIR = $(top_srcdir)/client/qcnlive

OTHER_CPPFLAGS = -D_USE_NTPDATE_EXEC_ -DQCNLIVE -DGRAPHICS_PROGRAM -DAPP_GRAPHICS

if OS_DARWIN
   #  also note need to have the following settings in environment for Leopard and higher OS X (i.e. >=10.5)
   # DEVELOPER_SDK_DIR=/Developer/SDKs
   # LDFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch i386"
   # CPPFLAGS="-I/usr/include -I/usr/local/include -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
   # CFLAGS="-I/usr/include -I/usr/local/include -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
   # CXXFLAGS="-I/usr/include -I/usr/local/include -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386"
   # SDKROOT="/Developer/SDKs/MacOSX10.4u.sdk"
   # MACOSX_DEPLOYMENT_TARGET=10.4
   # export DEVELOPER_SDK_DIR LDFLAGS CPPFLAGS CFLAGS CXXFLAGS SDKROOT MACOSX_DEPLOYMENT_TARGET
   BINARY = $(top_srcdir)/client/bin/QCNLive.app/Contents/MacOS/QCNLive
   STACKSIZE = -Wl,-stack_size -Wl,2000000
   QCN_WX_LIBS = -L/usr/local/lib   -framework IOKit -framework Carbon -framework Cocoa -framework System \
        -framework QuickTime -framework OpenGL -framework AGL  /usr/local/lib/libwx_mac-2.8.a \
        -lz -lwxtiff-2.8 -lz -lpthread -liconv
   LIBS += -bind_at_load -framework IOKit -framework Foundation \
          -framework ScreenSaver -framework Carbon -framework Cocoa
   # LIBS += @CLIENTLIBS@ -L$(LIBNTPDIR) -lntp -ldl $(BOINC_LIB) -lwx_mac_gl-2.8
   LIBS += @CLIENTLIBS@ -ldl $(BOINC_LIB) -lwx_mac_gl-2.8
   # UTIL_CPP = $(UTILDIR)/qcn_util.cpp $(UTILDIR)/qcn_semaphore.cpp
   SENSORS = \
           $(SENSORDIR)/csensor_mac_laptop.cpp \
           $(SENSORDIR)/csensor_mac_usb_jw.cpp \
           $(SENSORDIR)/csensor_usb_motionnodeaccel.cpp
else
   # must be Linux because I'm sure not using autoconf on Windows! ;-)
   BINARY = $(top_srcdir)/client/bin/qcnlive
   STACKSIZE = --stack=2000000
   QCN_WX_LIBS = $(WX_LIBS_STATIC)
   # LIBS += -z lazy -L$(LIBNTPDIR) -lntp -ldl $(BOINC_LIB) $(GLUT_LIBS) -lwx_x11univ_gl-2.8 @CLIENTLIBS@ 
   LIBS += -z lazy -ldl $(BOINC_LIB) $(GLUT_LIBS) -lwx_x11univ_gl-2.8 @CLIENTLIBS@ 
   SENSORS  = \
        $(SENSORDIR)/csensor_linux_usb_jw.cpp \
        $(SENSORDIR)/csensor_usb_motionnodeaccel.cpp 
endif

LIBS += /usr/local/lib/libpng.a /usr/local/lib/libjpeg.a

#bin_PROGRAMS = qcnlive detectaccel
bin_PROGRAMS = qcnlive

#           $(UTILDIR)/qcn_ntpdate.c 

qcnlive_SOURCES = \
           $(QCNLIVEDIR)/qcnwx.cpp \
           $(QCNLIVEDIR)/myframe.cpp \
           $(QCNLIVEDIR)/myglpane.cpp \
           $(QCNLIVEDIR)/dlgsettings.cpp \
           $(UTILDIR)/sac.cpp \
           $(UTILDIR)/qcn_thread.cpp \
           $(UTILDIR)/trickleup.cpp \
           $(UTILDIR)/trickledown.cpp \
           $(UTILDIR)/md5.cpp \
           $(UTILDIR)/execproc.cpp \
           $(UTILDIR)/qcn_util.cpp \
           $(UTILDIR)/qcn_signal.cpp \
           $(UTILDIR)/qcn_curl.cpp \
           $(UTILDIR)/gzstream.cpp \
      $(GRAPHICSDIR)/qcn_graphics.cpp \
      $(GRAPHICSDIR)/qcn_2dplot.cpp \
      $(GRAPHICSDIR)/qcn_earth.cpp \
      $(GRAPHICSDIR)/qcn_cube.cpp \
      $(GRAPHICSDIR)/nation_boundary.cpp \
      $(GRAPHICSDIR)/coastline.cpp \
      $(GRAPHICSDIR)/plate_boundary.cpp \
           $(UTILDIR)/cserialize.cpp \
           $(MAINDIR)/qcn_shmem.cpp \
           $(SENSORDIR)/csensor.cpp \
           $(MAINDIR)/qcn_thread_sensor.cpp \
           $(MAINDIR)/qcn_thread_time.cpp \
           $(MAINDIR)/main.cpp \
           $(SENSORS) 

qcnlive_DEPENDENCIES =
qcnlive_CFLAGS = $(AM_CFLAGS) $(OTHER_CPPFLAGS) $(WX_CFLAGS)
qcnlive_CPPFLAGS = $(AM_CPPFLAGS) $(OTHER_CPPFLAGS) $(WX_CPPFLAGS)
qcnlive_LDFLAGS = $(LIBS) $(AM_LDFLAGS) -lcurl -lstdc++ $(STACKSIZE) $(QCN_WX_LIBS) 
qcnlive_LDADD = 

#detectaccel_DEPENDENCIES =
#detectaccel_CFLAGS = $(AM_CFLAGS) $(OTHER_CPPFLAGS)
#detectaccel_CPPFLAGS = $(AM_CPPFLAGS) $(OTHER_CPPFLAGS)
#detectaccel_LDFLAGS = $(LIBS) $(AM_LDFLAGS) -lstdc++ $(STACKSIZE)
#detectaccel_LDADD = 

.PHONY: moveover

all-local: qcnlive moveover 

moveover:
	mv qcnlive $(BINARY)
	$(STRIP) $(BINARY)

## these source files need to be specified because no rule uses them.

clean-local:
	rm -f @CLIENT_BIN_FILENAME@ $(BINARY) 

