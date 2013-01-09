CFLAGS = -O2 -Wall
#CFLAGS = -g -O0 -Wall
CC = g++
BOINCDIR = ../../../boinc
MYSQLDIR = /usr/local/mysql
SSLDIR = /usr/local/ssl

ifeq ($(shell hostname),qcn-web.stanford.edu)
  INSTALLDIR = /var/www/boinc/sensor/bin
else
  INSTALLDIR = /home/boinc/web/boinc/sensor/bin
endif


LIBS = -L$(MYSQLDIR)/lib -lmysqlclient_r \
  -L$(BOINCDIR)/lib -lboinc \
  -L$(SSLDIR)/lib -lcrypto -lssl \
  -L/usr/local/lib -lcurl

INCLUDES = -I$(BOINCDIR) -I$(BOINCDIR)/lib -I$(BOINCDIR)/tools -I$(BOINCDIR)/db \
           -I$(MYSQLDIR)/include -I$(BOINCDIR)/sched -I/usr/local/include -I../../client/util

SRCS = $(BOINCDIR)/db/db_base.cpp \
  $(BOINCDIR)/sched/sched_msgs.cpp \
  $(BOINCDIR)/sched/sched_util.cpp \
  $(BOINCDIR)/sched/sched_limit.cpp \
  $(BOINCDIR)/sched/sched_config.cpp \
  $(BOINCDIR)/lib/parse.cpp \
  $(BOINCDIR)/db/boinc_db.cpp 

MAINMON  = trigmon.cpp
MAINDISP =  display.cpp

all: clean trigmonitor trigdisplay
#all: clean trigdisplay
#all: clean trigmonitor

trigdisplay: 
	$(CC) $(CFLAGS) $(INCLUDES) $(MAINDISP) $(SRCS) $(LIBS) -o trigdisplay
	strip trigdisplay
	cp trigdisplay $(INSTALLDIR)


trigmonitor: 
	$(CC) $(CFLAGS) $(INCLUDES) $(MAINMON) $(SRCS) $(LIBS) -o trigmonitor
	strip trigmonitor
	cp trigmonitor $(INSTALLDIR)


clean:
	rm -f *.o core *~ trigmonitor trigdisplay $(INSTALLDIR)/trigmonitor $(INSTALLDIR)/trigdisplay


