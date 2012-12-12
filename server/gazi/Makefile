#CFLAGS = -O2 -Wall
CFLAGS = -g -O0 -Wall
CC = g++
BOINCDIR = ../../../boinc
MYSQLDIR = /usr/local/mysql
SSLDIR = /usr/local/ssl
INSTALLDIR = /var/www/boinc/sensor/bin

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
MAINLOC  = qcn_locate.cpp
MAINDISP =  display.cpp

all: clean trigmonitor trigdisplay
#all: clean trigdisplay
#all: clean trigmonitor

trigdisplay: 
	$(CC) $(CFLAGS) $(INCLUDES) $(MAINDISP) $(SRCS) $(LIBS) -o gazi_trigdisplay
	cp gazi_trigdisplay $(INSTALLDIR)


trigmonitor: 
	$(CC) $(CFLAGS) $(INCLUDES) $(MAINMON) $(SRCS) $(LIBS) -o gazi_trigmonitor
	cp gazi_trigmonitor $(INSTALLDIR)

qcn_locate: 
	$(CC) $(CFLAGS) $(INCLUDES) $(MAINLOC) $(SRCS) $(LIBS) -o gazi_qcn_locate
	cp gazi_qcn_locate $(INSTALLDIR)


clean:
	rm -f *.o core *~ gazi_trigmonitor gazi_trigdisplay $(INSTALLDIR)/gazi_trigmonitor $(INSTALLDIR)/gazi_trigdisplay


