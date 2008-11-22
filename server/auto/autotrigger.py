#! /usr/bin/env python

# this file will update the qcn database triggers (qcn_trigger table)
# to reflect any matches with actual seismic activity from the
# usgs_quake table

# it should be run on the BOINC scheduler server as a task in config.xml
# from the bin/ subdir, i.e. in config.xml <tasks> section:

#    <task>
#      <output>
#        autotrigger.out
#      </output>
#      <cmd>
#        autotrigger.py
#      </cmd>
#      <period>
#        24 hours
#      </period>
#    </task>
#
'''

note this now uses the built-in-to-mysql-server functions in qcn/server/sql_functions

basically it provides a more intelligent way to test for the 'closeness' of an earthquake
to a QCN trigger

'''

# CMC note -- need to install 3rd party MySQLdb & mxDateTime libraries for python
import traceback, sys, string, MySQLdb, mx.DateTime
from datetime import datetime
from time import strptime, mktime

QUERY_QUAKE_PROCESSED = "select id, time_utc, latitude, longitude, magnitude " +\
                        "from usgs_quake where processed is null or not processed"

QUERY_TRIGGER_HOST_LIST = "select hostid,count(*) from qcn_trigger "

# note that we want to get host & trigger records matching a quake
# or it was a prior request that we haven't received anything in the past 3 days with up to 10 tries
# also note we give up for triggers older than 30 days as they would have been deleted by QCN 
# (also the retry every 3 days for 10 times should have given enough chances to get them)
QUERY_TRIGGER_HOST_WHERE = " (" +\
                         "usgs_quakeid>0 and " +\
                         "(time_filereq is null or time_filereq=0) " +\
                         " OR " +\
                         " ((time_filereq + (3600.0*24.0*3.0)) < unix_timestamp() " +\
                         " AND received_file <= 10) " +\
                         " AND (time_trigger + (3600.0*24.0*30.0)) > unix_timestamp() " +\
                         ") "

DBNAME = "qcnalpha"
DBHOST = "db-private"
DBUSER = "root"
DBPASSWD = ""

def updateQuakeTrigger(dbconn):
   # this will update the trigger table with usgs quake events
   # the quake table has a bool that gets saved so we know we processed it (usgs_quake.processed)
   try:
      cMain = dbconn.cursor()
      cMain.execute(QUERY_QUAKE_PROCESSED)
      i = 0
      while (1):
         rowQuake = cMain.fetchone()
         if rowQuake == None:
            break

         # for each "unprocessed" quake, set matches in trigger table
         print "Updating triggers for quake # " + str(rowQuake[0])
         cTrig = dbconn.cursor()

         # note this query constrains (for optimization) triggers within a 4 minute window of the quake
         # and also only gets triggers that were time sync'd to the server in that time
         # last but not least - it uses a custom mysql function (see qcn/server/sql_functions) to bring
         # back a 'score' of how close the trigger was to this event (>0 means the quake was near the trigger host)
         strSQL = "update qcn_trigger t set t.usgs_quakeid = " + str(rowQuake[0]) +\
            " WHERE t.time_trigger BETWEEN " + str(rowQuake[1]-120.0) + " AND " + str(rowQuake[1]+120.0) +\
            " AND t.time_sync > 0 " +\
            " AND quake_hit_test(t.latitude, t.longitude, t.time_trigger, t.type_sensor, " +\
               str(rowQuake[2]) + ", " +\
               str(rowQuake[3]) + ", " +\
               str(rowQuake[1]) + ", " +\
               str(rowQuake[4]) +\
            ") > 0 "

         #print strSQL;
         cTrig.execute(strSQL);
         cTrig.execute("update usgs_quake set processed=true where id = " + str(rowQuake[0]))
         
         dbconn.commit()
         cTrig.close()
         i = i + 1
         
      cMain.close()
      print str(i) + " Quakes updated in qcn_trigger table"
   except:
      dbconn.rollback()
      traceback.print_exc()


def generateFileRequestTrickleDown(dbconn):
   # generate the trickle down request for a trigger that may have occurred near a USGS event
   try:
      cMain = dbconn.cursor()
#      print QUERY_TRIGGER_HOST_LIST + " WHERE " + QUERY_TRIGGER_HOST_WHERE + " GROUP BY hostid"
      cMain.execute(QUERY_TRIGGER_HOST_LIST + " WHERE " + QUERY_TRIGGER_HOST_WHERE + " GROUP BY hostid")
      i = 0
      while (1):
         rowTrig = cMain.fetchone()
         if rowTrig == None:
            break

         # for each "unprocessed" quake, set matches in trigger table
         print "Requesting files for host # " + str(rowTrig[0])
         cTrig = dbconn.cursor()
         cTrig.execute("select concat('<sendme>',t.file,'</sendme>\n') from qcn_trigger t " +\
            "where hostid=" + str(rowTrig[0]) + " AND " + QUERY_TRIGGER_HOST_WHERE )

         strSendMe = ""
         while (1):
            rowTrickle = cTrig.fetchone()
            if rowTrickle == None:
               break
            strSendMe += rowTrickle[0]
            # check that string isn't too big, 256K is max msg_to_host size, 64K should be plenty
            if len(strSendMe) > 65536:
               break


         # now make the full trickle down insert
         if len(strSendMe) > 0:
            cTrig.execute("insert into msg_to_host " +\
               "(create_time,hostid,variety,handled,xml) " +\
               "select unix_timestamp(), " + str(rowTrig[0]) + ", 'filelist', 0, " +\
               "concat('<trickle_down>\n<result_name>', r.name, '</result_name>\n<filelist>\n" +\
               strSendMe + "</filelist>\n</trickle_down>\n') " +\
               "from result r " +\
               "where r.hostid=" + str(rowTrig[0])  +\
               "  and r.sent_time=(select max(rr.sent_time) from result rr where rr.hostid=r.hostid) " )
                       
         cTrig.execute("update qcn_trigger set time_filereq=unix_timestamp(), received_file=IF(ISNULL(received_file), 1, received_file+1) " +\
                       "where hostid=" + str(rowTrig[0]) + " AND " + QUERY_TRIGGER_HOST_WHERE )
         
         dbconn.commit()
         cTrig.close()
         i = i + 1
         
      cMain.close()
      print str(i) + " hosts sent file requests (trickle down)"
   except:
      dbconn.rollback()
      traceback.print_exc()

      
def main():
   try:
      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)

      updateQuakeTrigger(dbconn)

      # OK, at this point we have updated the qcn_trigger table with any matching
      # earthquakes from the usgs_quake table

      # next step will be to request the matching triggers be uploaded
      generateFileRequestTrickleDown(dbconn)

      dbconn.close()

   except:
      traceback.print_exc()
      sys.exit(1)

if __name__ == '__main__':
    main()

