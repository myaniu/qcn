CFLAGS = -g -Wall -DQCN_SERVER
CC = g++
BOINCDIR = ../../../boinc
MYSQLDIR = /usr/local/mysql

SSLDIR = /usr/local/ssl
#QCNDIR = /var/www/boinc/continual
QCNDIR = /var/www/boinc/sensor
CONTDIR = /var/www/boinc/continual

LIBS = -L$(MYSQLDIR)/lib -lmysqlclient_r \
  -L$(BOINCDIR)/lib -lboinc \
  -L$(SSLDIR)/lib64 -lcrypto -lssl \
  -L/usr/local/lib -lcurl

INCLUDES = -I$(SSLDIR)/include -I$(BOINCDIR) -I$(BOINCDIR)/lib -I$(BOINCDIR)/tools -I$(BOINCDIR)/db \
           -I$(MYSQLDIR)/include -I$(BOINCDIR)/sched

SRCS = $(BOINCDIR)/db/db_base.cpp \
  $(BOINCDIR)/db/boinc_db.cpp \
  $(BOINCDIR)/sched/sched_config.cpp \
  $(BOINCDIR)/sched/sched_msgs.cpp \
  $(BOINCDIR)/sched/sched_util.cpp \
  $(BOINCDIR)/tools/backend_lib.cpp \
  $(BOINCDIR)/tools/process_result_template.cpp

MAIN = work.cpp

all: qcn_workgen

qcn_workgen: 
	$(CC) $(CFLAGS) $(INCLUDES) $(SRCS) $(LIBS) $(MAIN) -o qcn_workgen
	cp -rf templates/ $(QCNDIR)/
	cp qcn_workgen $(QCNDIR)/bin/
	cp -rf templates/ $(CONTDIR)/
	mv qcn_workgen $(CONTDIR)/bin/

clean:
	rm -f *.o core *~ qcn_workgen


