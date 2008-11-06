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

Regarding the 1-degree box: is this 1 degree on each side of the eq, or 1/2
degree on each side?

Several Comments:

1) Can we account for the longitudes shrinking at the poles (actual distance
per degree longitude decreases as cos(latitude)). I'd hate to miss data at
higher latitude just because the longitudes converge.

2) Can we make the spatial window around the eq a circle rather than a
square? Square is a fine first cut (because it is fast), but distance from
the eq is circular. I have subroutines that do this sort of thing if you
want.

3) Is there a way to scale the magnitude-distance (M-D) relationship? The
relation, [ D <= (M-4) ], would use a bigger data collection area for bigger
earthquakes. We'd probably want to collect 4.0 and smaller if the eq is less
than 0.1 degree away.

M <=  4.0:    D <= 0.1 Deg
M <=  4.5:    D <= 0.5 Deg
M <=  5.0:    D <= 1.0 Deg
M <=  5.5:    D <= 1.5 Deg
M <=  6.0:    D <= 2.0 Deg
M <=  6.5:    D <= 2.5 Deg
M <=  7.0:    D <= 3.0 Deg
M <=  7.5:    D <= 3.5 Deg
M <=  8.0:    D <= 4.0 Deg
M <=  8.5:    D <= 4.5 Deg
M <=  9.0:    D <= 5.0 Deg
M <=  9.5:    D <= 5.5 Deg
M <= 10.0:    D <= 6.0 Deg

4) Can we do a similar thing for distance-timing? Timing should fall in a
window after the earthquake. The timing should depend on the distance from
the earthquake and the velocities of the slowest S wave and the fastest P
wave [13(sec/deg)*D(deg) <= T (sec) <= 38(sec/deg)*D(deg)]:

D <= 0.0 Deg:    0.0 <= T <=   0.0 sec
D <= 0.5 Deg:    6.5 <= T <=  18.5 sec
D <= 1.0 Deg:   13.0 <= T <=  37.1 sec
D <= 1.5 Deg:   19.5 <= T <=  55.6 sec
D <= 2.0 Deg:   26.0 <= T <=  74.1 sec
D <= 2.5 Deg:   32.5 <= T <=  92.7 sec
D <= 3.0 Deg:   39.0 <= T <= 111.2 sec
D <= 3.5 Deg:   45.5 <= T <= 129.7 sec
D <= 4.0 Deg:   52.0 <= T <= 148.3 sec
D <= 4.5 Deg:   58.5 <= T <= 166.8 sec
D <= 5.0 Deg:   65.0 <= T <= 185.3 sec
D <= 5.5 Deg:   71.5 <= T <= 203.8 sec
D <= 6.0 Deg:   78.0 <= T <= 222.4 sec

This should hopefully cut down on the number of false positives because of
the shortened time window and lesser distance for smaller eq's.


'''

# CMC note -- need to install 3rd party MySQLdb & mxDateTime libraries for python
import traceback, sys, string, MySQLdb, mx.DateTime
from datetime import datetime
from time import strptime, mktime

# set latitude & longitude "window" in degree (i.e. quake +/- LAT_WINDOW & LON_WINDOW away)
LAT_WINDOW = 1.5
LON_WINDOW = 1.5

# time window of 120 seconds before & after UTC time of USGS event?
TIME_WINDOW = 120.0

QUERY_QUAKE_PROCESSED = "select id, time_utc, latitude, longitude " +\
                        "from usgs_quake where processed is null or not processed"

QUERY_TRIGGER_HOST_LIST = "select hostid,count(*) from qcn_trigger "

# note that we want to get host & trigger records matching a quake
# or it was a prior request that we haven't received anything in the past 3 days with up to 10 tries
QUERY_TRIGGER_HOST_WHERE = " (usgs_quakeid>0 and " +\
                         "(time_filereq is null or time_filereq=0) " +\
                         " OR " +\
                         " ((time_filereq + (3600.0*24.0*3.0)) < unix_timestamp() " +\
                         " AND received_file <= 10) ) "

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
         cTrig.execute("update qcn_trigger t set t.usgs_quakeid = " + str(rowQuake[0]) +\
            " where t.latitude between " + str(rowQuake[2] - LAT_WINDOW) +\
                       " and " + str(rowQuake[2] + LAT_WINDOW) +\
                    " and t.longitude between "  + str(rowQuake[3] - LON_WINDOW) +\
                       " and " + str(rowQuake[3] + LON_WINDOW) +\
                    " and t.time_trigger between "  + str(rowQuake[1] - TIME_WINDOW) +\
                       " and " + str(rowQuake[1] + TIME_WINDOW) )

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

