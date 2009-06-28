CFLAGS = -DQCN_CONTINUAL -DQCN_SERVER -O3 -Wall
#CFLAGS = -DQCN_SERVER -O3 -Wall
CC = g++
BOINCDIR = ../../../boinc
MYSQLDIR = /usr/local/mysql
SSLDIR = /usr/local/ssl
QCNDIR = /var/www/boinc/continual
#QCNDIR = /var/www/boinc/sensor

# CMC note - ssl lib used seems to be in /usr/kerberos/lib64, not the one in /usr/local/ssl !
LIBS = -L$(BOINCDIR)/lib -lboinc \
   -L$(MYSQLDIR)/lib -lmysqlclient \
   -L/usr/kerberos/lib64 -lidn -lssl -lcrypto -lldap -lrt -lcrypto -ldl -lz

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
	mv qcn_workgen $(QCNDIR)/bin/

clean:
	rm -f *.o core *~ qcn_workgen


