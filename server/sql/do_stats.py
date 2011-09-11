#! /usr/bin/env python

import sys, string, MySQLdb
from time import time

DBNAME = "qcnalpha"
DBHOST = "db-private"
DBUSER = "qcn"
DBPASSWD = "PASSWORD"

def main():
   try:
      t1 = time()
      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)
      cMain = dbconn.cursor()
      cMain.execute("update qcnalpha.result set validate_state=1 where validate_state=3 and outcome=1")
      cMain.execute("update continual.result set validate_state=1 where validate_state=3 and outcome=1")
      cMain.execute("call do_stats();")
      dbconn.close()
      t2 = time()
      print "do_stats Successful on " + str(t2) + " - Elapsed Time = " + str(t2-t1) + " seconds"
   except:
      t2 = time()
      print "do_stats Database error at " + str(t2) + " - Elapsed Time = " + str(t2-t1) + " seconds"
      sys.exit(1)

if __name__ == '__main__':
    main()


