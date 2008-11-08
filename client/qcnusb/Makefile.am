include $(top_srcdir)/Makefile.incl

QCNUSB = qcnusb
BINARY = $(QCNUSB)_$(QCN_VERSION_STRING)_$(build)
QCNUSBDIR = $(BASEDIR)/client/qcnusb

client-bin: $(QCNUSB)

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
   ZIP  = /usr/bin/zip
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Carbon -framework Cocoa
   # UTIL_CPP = $(UTILDIR)/qcn_util.cpp $(UTILDIR)/qcn_semaphore.cpp $(UTILDIR)/qcn_signal.cpp
   UTIL_CPP = $(UTILDIR)/qcn_util.cpp $(UTILDIR)/qcn_signal.cpp
endif

LIBS += @CLIENTLIBS@ $(BOINC_LIB)

bin_PROGRAMS = qcnusb test

qcnusb_SOURCES = \
           $(SENSORDIR)/csensor.cpp \
           $(SENSORDIR)/csensor_mac_usb_jw.cpp \
           $(UTIL_CPP) \
           ./qcn_shmem_usb.cpp \
           ./qcnusb.cpp 

qcnusb_DEPENDENCIES =
qcnusb_CFLAGS = -I$(QCNUSBDIR) -DQCN_USB $(AM_CPPFLAGS) $(PTHREAD_CFLAGS)
qcnusb_CPPFLAGS = -I$(QCNUSBDIR) -DQCN_USB $(AM_CPPFLAGS) $(PTHREAD_CFLAGS)
qcnusb_LDFLAGS = $(LIBS) $(AM_LDFLAGS) $(PTHREAD_LIBS)
qcnusb_LDADD = 

test_SOURCES = \
           $(UTIL_CPP) \
           ./qcn_shmem_usb.cpp \
           ./test.cpp 
test_DEPENDENCIES =
test_CPPFLAGS = $(qcnusb_CPPFLAGS)
test_LDFLAGS = $(qcnusb_LDFLAGS)
test_LDADD = 

.PHONY: moveover

qcndir = $(BASEDIR)/bin

all-local: client-bin moveover

moveover:
	$(STRIP) $(QCNUSB)
	mv $(QCNUSB) install/
#	mv $(QCNUSB) install/$(BINARY)

clean-local:
	rm -f $(QCNUSB) $(BINARY) test install/$(QCNUSB) install/$(BINARY)


