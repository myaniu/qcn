CFLAGS = -O2 -Wall
CC = g++
BOINCDIR = ../../../boinc
MYSQLDIR = /usr/local/mysql
SSLDIR = /usr/local/ssl

LIBS = -L$(MYSQLDIR)/lib -lmysqlclient_r \
  -L$(BOINCDIR)/lib -lboinc \
  -L$(SSLDIR)/lib -lcrypto -lssl

INCLUDES = -I$(BOINCDIR) -I$(BOINCDIR)/lib -I$(BOINCDIR)/tools -I$(BOINCDIR)/db \
           -I$(MYSQLDIR)/include -I$(BOINCDIR)/sched -I../../client/util

SRCS = $(BOINCDIR)/db/db_base.cpp \
  $(BOINCDIR)/sched/sched_msgs.cpp \
  $(BOINCDIR)/sched/sched_util.cpp \
  $(BOINCDIR)/sched/sched_config.cpp \
  $(BOINCDIR)/lib/parse.cpp \
  $(BOINCDIR)/db/boinc_db.cpp 

MAIN = trigmon.cpp

all: trigmon

trigmon: 
	$(CC) $(CFLAGS) $(INCLUDES) $(MAIN) $(SRCS) $(LIBS) -o trigmon 

clean:
	rm -f *.o core *~ trigmon


