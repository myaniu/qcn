#! /usr/bin/env python

# this program will feed the trigmem_test.qcn_trigger_memory table fake triggers (usually on qcn-data but could be qcn-web)
# it can "mimic" past events such as previously detected QCN or USGS earthquakes from historical trigger data
# or you can enter a date/time range of triggers to "play back"
# note it will create new records in the qcn_trigger table of qcn_data so that they can be updated with the 'trigmon' program

# note -- need to install 3rd party MySQLdb libraries for python
import sys, os, math, traceback, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
#from zipfile import ZIP_STORED
from time import strptime, mktime, sleep
#from qcnutil import getSACMetadata
#from qcnutil import getFanoutDirFromZip
from optparse import OptionParser

global PROGNAME
global DBHOST 
global DBUSER
global DBPASSWD
global DBNAME
global DB_TRIGMEM
global SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT

PROGNAME = "replay-trigger.py"
DBHOST = "localhost"
DBUSER = "qcn"
DBPASSWD = "qcntest"
DBNAME = "sensor"
DB_TRIGMEM = "trigmem_test"

global DBNAME_CONTINUAL
global DBNAME_JOB
global IS_ARCHIVE
global sqlQuery
global ZIP_CMD
global UNZIP_CMD
global DATE_MIN_ORIG
global DATE_MAX_ORIG
global DATE_MIN
global DATE_MAX
global LAT_MIN
global LAT_MAX
global LNG_MIN
global LNG_MAX


# print out last 'numquake' quakes & count of triggers
def printQuakeList(dbconn, numquake):
   print "Last " + str(numquake) + " Quakes"
   print "QuakeID, Num Triggers, Date/Time (UTC), Latitude, Longitude, Magnitude, Description"

   strSQL = """select q.id, from_unixtime(q.time_utc), q.magnitude, q.latitude, q.longitude, q.description,
((select count(*) from sensor.qcn_trigger t1 where t1.qcn_quakeid=q.id) +
(select count(*) from continual.qcn_trigger t2 where t2.qcn_quakeid=q.id)) trigcnt
from sensor.qcn_quake q 
group by q.id having trigcnt>0
order by q.id desc
 limit """ + str(numquake)
   #print strSQL
   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   result = myCursor.fetchall()
   myCursor.close()
   for rec in result:
     print rec[0], rec[6], rec[1], rec[3], rec[4], rec[2], rec[5]

   return 0

# get the tuples of the triggers for a time range
def getTriggerTupleTime(minTime, maxTime, iDelay, dbconn):
   strSQL = "select * from (select 'sensor', s.* from sensor.qcn_trigger s " +\
          "where varietyid=0 AND time_trigger between " +\
            str(minTime) + " AND " + str(maxTime) +\
             " UNION " +\
            "select 'continual', c.* from continual.qcn_trigger c " +\
          "where varietyid=0 AND time_trigger between " +\
            str(minTime) + " AND " + str(maxTime) +\
                ") t order by time_trigger"
 
   # TODO: check archive time and select against archive tables

   #print timeStart, timeEnd 
   #print strSQL
   #sys.exit()

   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   result = myCursor.fetchall()
   myCursor.close()
   return result

# get the tuples of the triggers
def getTriggerTupleQuakeID(idQuake, iDelay, dbconn):
   # strWhere is the where clause ie quakeid=5445 --- iDelay is the delay time # of seconds from first trigger
   #strSQL = "select * from (select 'sensor', s.* from sensor.qcn_trigger s " + strWhere + " UNION " +\
   #         "select 'continual', c.* from continual.qcn_trigger c " + strWhere + ") t order by time_trigger"
   strWhere = "WHERE varietyid=0 and qcn_quakeid=" + str(idQuake)

   # should perhaps get first trigger for start time
   strSQL = "select min(trigtime) from (select min(time_trigger) trigtime from sensor.qcn_trigger s " + strWhere + " UNION " +\
            "select min(time_trigger) trigtime from continual.qcn_trigger c " + strWhere + ") t "

   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   row = myCursor.fetchone()
   myCursor.close()

   if row is None or row[0] is None or len(row) == 0:
     return None

   timeStart = row[0] - iDelay

   # now get end time of event
   strSQL = "select max(trigtime) from (select max(time_trigger) trigtime from sensor.qcn_trigger s " + strWhere + " UNION " +\
            "select max(time_trigger) trigtime from continual.qcn_trigger c " + strWhere + ") t "

   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   row = myCursor.fetchone()
   myCursor.close()
   if row is None or row[0] is None or len(row) == 0:
     return None

   timeEnd = row[0] + iDelay

   return getTriggerTupleTime(timeStart, timeEnd, iDelay, dbconn)


def processTriggerTuple(tt, iDelay, dbconn):
   if tt is None or tt[0] is None or len(tt) == 0:
     print "No triggers found for your criteria!"
     print "Type ./'" + PROGNAME + " -h' for help"
     return 1
   timeStart = tt[0][5]
   play = 0
   print "Trigger replay starting in " + str(iDelay) + " seconds (Ctrl+C to stop)...."
   SLEEP_INTERVAL = 0.01
   for t in tt:
     while play < t[5]-timeStart+iDelay:
        sleep(SLEEP_INTERVAL)
        play = play + SLEEP_INTERVAL
     print t[5]-timeStart+iDelay, t[0], t[1], t[2], t[3], t[4], t[6], t[7], t[8]

     # insert into sensor/continual.qcn_trigger table here
     # get new trigger insert ID # and update t[] for trimem_test insert below

     # insert into trigmem_test.qcn_trigger_memory table here

   return 0

def deleteTrigMemTest(dbconn):
   # delete mem triggers older than 5 minutes
   strSQL = "DELETE FROM trigmem_test.qcn_trigger_memory WHERE time_trigger < unix_timestamp() - 300"
   myCursor = dbconn.cursor()
   myCursor.execute(strSQL)
   myCursor.close()
   return 0

# main proc
def main():
   global UPLOAD_WEB_DIR
   global DOWNLOAD_WEB_DIR
   global DOWNLOAD_URL
   global UPLOAD_WEB_DIR_CONTINUAL
   global DBNAME
   global DBNAME_CONTINUAL
   global IS_ARCHIVE

   global DATE_MIN_ORIG
   global DATE_MAX_ORIG
   global DATE_MIN
   global DATE_MAX
   global TIME_MIN
   global TIME_MAX
   global LAT_MIN
   global LAT_MAX
   global LNG_MIN
   global LNG_MAX
   global DELAY_TIME

   class FinishedException(Exception):
     def __init__(self, value):
       self.value = value
     def __str__(self):
       return repr(self.value)

   strDescription = "This program will 'replay' past triggers into the trigmem_test.qcn_trigger_memory table on qcn-data"

   # get cmd-line arguments
   parser = OptionParser()
   parser = OptionParser(description=strDescription)
   parser.add_option("--date_start", dest="DATE_MIN", type="string", help="Enter Start Date in YYYY-MM-DD format", metavar="DATE_MIN")
   parser.add_option("--time_start", dest="TIME_MIN", type="string", help="Enter Start Time in 24-hr UTC format e.g. HH:MM:SS", metavar="TIME_MIN")
   parser.add_option("--date_end", dest="DATE_MAX", type="string", help="Enter End Date in YYYY-MM-DD format", metavar="DATE_MAX")
   parser.add_option("--time_end", dest="TIME_MAX", type="string", help="Enter End Time in 24-hr UTC format e.g. HH:MM:SS", metavar="TIME_MAX")
   #parser.add_option("--lat_min", dest="LAT_MIN", type="float", help="Enter Minimum Latitude [-90,90]", metavar="LAT_MIN")
   #parser.add_option("--lat_max", dest="LAT_MAX", type="float", help="Enter Maximum Latitude [-90,90]", metavar="LAT_MAX")
   #parser.add_option("--lng_min", dest="LNG_MIN", type="float", help="Enter Minimum Longitude [-180,180]", metavar="LNG_MIN")
   #parser.add_option("--lng_max", dest="LNG_MAX", type="float", help="Enter Maximum Longitude [-180,180]", metavar="LNG_MAX")
   parser.add_option("--delay", dest="DELAY_TIME", type="int", help="Number of seconds to delay until start of first trigger (default 5s)", metavar="sec")
   parser.add_option("--quake_id", dest="QUAKE_ID", type="int", help="Enter Quake ID # (run script with --quake_list n to get last n events", metavar="id")
   parser.add_option("--quake_list", dest="QUAKE_LIST", type="int", help="Show last 'n' earthquakes with matching triggers", metavar="n")
   (options, args) = parser.parse_args();

   #lat/lng
   # default to SCEDC run
   #if options.LAT_MIN is None:
   #  options.LAT_MIN  = 31.5
   #if options.LAT_MAX is None:
   #  options.LAT_MAX  = 37.5
   #if options.LNG_MIN is None:
   #  options.LNG_MIN  = -121.0
   #if options.LNG_MAX is None:
   #  options.LNG_MAX  = -114.0

   #dates
   if options.DATE_MAX is None and options.DATE_MIN is not None:
     options.DATE_MAX = options.DATE_MIN
   if options.DATE_MIN is None and options.DATE_MAX is not None:
     options.DATE_MIN = options.DATE_MAX
   if options.TIME_MIN is None:
     options.TIME_MIN = "00:00:00"
   if options.TIME_MAX is None:
     options.TIME_MAX = "00:05:00"

   #if options.LAT_MIN is not None or options.LAT_MAX is not None:
   #  if options.LAT_MIN < -90 or options.LAT_MIN > 90 or options.LAT_MIN > options.LAT_MAX:
   #    print "Incorrect Minimum Latitude, must be between -90 and 90 and less than Maximum Latitude entered."
   #    print "Type ./'" + PROGNAME + " -h' for help"
   #    sys.exit()
#
#     if options.LAT_MAX < -90 or options.LAT_MAX > 90 or options.LAT_MAX < options.LAT_MIN:
#       print "Incorrect Maximum Latitude, must be between -90 and 90 and greater than Minimum Latitude entered."
#       print "Type ./'" + PROGNAME + " -h' for help"
#       sys.exit()
#
#     if options.LNG_MIN < -180 or options.LNG_MIN > 180 or options.LNG_MIN > options.LNG_MAX:
#       print "Incorrect Minimum Longitude, must be between -180 and 180 and less than Maximum Longitude entered."
#       print "Type ./'" + PROGNAME + " -h' for help"
#       sys.exit()
#
#     if options.LNG_MAX < -180 or options.LNG_MAX > 180 or options.LNG_MAX < options.LNG_MIN:
#       print "Incorrect Maximum Longitude, must be between -180 and 180 and greater than Minimum Longitude entered."
#       print "Type ./'" + PROGNAME + " -h' for help"
#       sys.exit()
#
#   LAT_MIN = options.LAT_MIN
#   LAT_MAX = options.LAT_MAX
#   LNG_MIN = options.LNG_MIN
#   LNG_MAX = options.LNG_MAX

   if options.DELAY_TIME is None:
     options.DELAY_TIME = 5

   try:
      cnt = 0

      #open database connection
      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)

      if dbconn is None:
         print "Database error - check connection settings"
         raise

      deleteTrigMemTest(dbconn)

      # test dates
      if options.DATE_MIN is None and options.DATE_MAX is None:
        sqlts = "SELECT DATE_SUB(CURDATE(), INTERVAL 1 DAY) " 
        myCursor = dbconn.cursor()
        myCursor.execute(sqlts)
        res = myCursor.fetchone()
        myCursor.close()
        options.DATE_MIN = str(res[0])
        options.DATE_MAX = options.DATE_MIN
        # if using the default filename, add the date sub to filename

      DATE_MIN_ORIG = options.DATE_MIN + " " + options.TIME_MIN
      DATE_MAX_ORIG = options.DATE_MAX + " " + options.TIME_MAX
      DATE_MIN = DATE_MIN_ORIG
      DATE_MAX = DATE_MAX_ORIG

      # first get archive time, good spot to get the unix timestamp versions of our dates
      sqlts = "select value_int, unix_timestamp('" + DATE_MIN + "'), unix_timestamp('" + DATE_MAX + "'), DATE_SUB(CURDATE(), INTERVAL 1 DAY) " +\
       "from sensor.qcn_constant where description='ArchiveTime'"
      myCursor = dbconn.cursor()
      myCursor.execute(sqlts)
      res = myCursor.fetchone()
      myCursor.close()

      if len(res) == 0 or res is None or res[0] is None:
         print "Error - cannot retrieve archive trigger time or validate min/max dates"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()

      timeArchive = int(res[0])
      DATE_MIN = int(res[1])
      DATE_MAX = int(res[2])

      if DATE_MIN is None or DATE_MIN < 1e6 or DATE_MAX is None or DATE_MAX < 1e6:
         print "Error - Invalid start or end date"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()

      if DATE_MIN > DATE_MAX:
         print "Error - start date greater than end date"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()

      if DATE_MIN >= timeArchive and DATE_MAX >= timeArchive:
         IS_ARCHIVE = False
         UPLOAD_WEB_DIR           = "/data/QCN/trigger/"
         UPLOAD_WEB_DIR_CONTINUAL = "/data/QCN/trigger/continual/"
         DBNAME                   = "sensor"
         DBNAME_CONTINUAL         = "continual"
      elif DATE_MIN < timeArchive and DATE_MAX <= timeArchive:
         IS_ARCHIVE = True
         UPLOAD_WEB_DIR           = "/data/QCN/trigger/archive/"
         UPLOAD_WEB_DIR_CONTINUAL = "/data/QCN/trigger/archive/continual/"
         DBNAME                   = "sensor_archive"
         DBNAME_CONTINUAL         = "continual_archive"
      else: # error! 
         print "Error - dates straddle the archive time - not yet supported"
         print "Type './trigger_replay.py -h' for help"
         dbconn.close()
         sys.exit()

      # first make sure all the necessary paths are in place
      #if (checkPaths() != 0):
      #   sys.exit(2)

      bDone = False 
      #procRequest(dbconn)
      # quake list supercedes all ie print quake ID & info & return
      if not bDone and options.QUAKE_LIST is not None:
        if options.QUAKE_LIST == 0:
           options.QUAKE_LIST = 10
        printQuakeList(dbconn, options.QUAKE_LIST)
        bDone = True

      if not bDone and options.QUAKE_ID is not None:
        tt = getTriggerTupleQuakeID(options.QUAKE_ID, options.DELAY_TIME, dbconn)
        processTriggerTuple(tt, options.DELAY_TIME, dbconn)
        bDone = True
        #raise FinishedException(2)

      if not bDone and options.DATE_MIN is not None and options.DATE_MAX is not None:
        tt = getTriggerTupleTime(DATE_MIN, DATE_MAX, options.DELAY_TIME, dbconn)
        processTriggerTuple(tt, options.DELAY_TIME, dbconn)
        bDone = True
        #raise FinishedException(3)

      deleteTrigMemTest(dbconn)
      dbconn.close()
      dbconn = None
      print "Finished!"
      return 0


   except FinishedException: #  as e: 
      # just to signify we're done
      if dbconn is not None:
        dbconn.close()
      return 0

   except:
      traceback.print_exc()
      if dbconn is not None:
         dbconn.close()
      return 3

if __name__ == '__main__':
    main()


