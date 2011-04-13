#! /usr/local/bin/python
#// #! /usr/bin/env python

# this program will bundle up files for SCEDC, also make a CSV file of the triggers/files

# this will be run manually for now, can be extended for other areas of course

#contacts:
#Ellen Yu 
#eyu@gps.caltech.edu
#
#Aparna Bhaskaran 
#aparnab@gps.caltech.edu
#
# CMC note -- need to install 3rd party MySQLdb libraries for python
import math, tempfile, smtplib, traceback, sys, os, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
from zipfile import ZIP_STORED
from time import strptime, mktime
from qcnutil import getSACMetadata
from qcnutil import getFanoutDirFromZip

global DBHOST 
global DBUSER
global DBPASSWD
global SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT

DBHOST = "db-private"
DBUSER = "qcn"
DBPASSWD = ""

SMTPS_HOST = "smtp.stanford.edu"
SMTPS_PORT = 465
SMTPS_LOCAL_HOSTNAME = "qcn-upl.stanford.edu"
SMTPS_KEYFILE = "/etc/sslcerts/server.key"
SMTPS_CERTFILE = "/etc/sslcerts/server.crt"
SMTPS_TIMEOUT = 60


global URL_DOWNLOAD_BASE
global UPLOAD_WEB_DIR
global DOWNLOAD_WEB_DIR
global UPLOAD_WEB_DIR_CONTINUAL
global DBNAME
global DBNAME_CONTINUAL
global DBNAME_JOB
global IS_ARCHIVE
global sqlQuery
global ZIP_CMD
global UNZIP_CMD
global DATE_MIN
global DATE_MAX
global LAT_MIN
global LAT_MAX
global LNG_MIN
global LNG_MAX
global FILE_CSV
global FILE_ZIP

DOWNLOAD_WEB_DIR         = "/var/www/trigger/job/"
DBNAME = "qcnalpha"
DBNAME_JOB = "sensor_download"

FILE_CSV                 = "qcn_scedc.csv"
FILE_ZIP                 = "qcn_scedc.zip"
  
DATE_MIN = "2011-01-01 00:00:00"
DATE_MAX = "2011-01-08 00:00:00"
LAT_MIN  = 31.5
LAT_MAX  = 37.5
LNG_MIN  = -121.0
LNG_MAX  = -114.0

# use fast zip -1 compression as the files are already compressed
ZIP_CMD  = "/usr/bin/zip -1 "
UNZIP_CMD = "/usr/bin/unzip -o -d "
 
# can use concat for direct query to csv file 
#  concat(m.mydb, ',' , m.id, ',', m.hostid, ',',
#    from_unixtime(m.time_trigger), ',', FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6),
#     ',',
#    m.magnitude, ',', m.significance, ',',
#    m.latitude, ',', m.longitude, ',', m.file,',', m.numreset, ',',
#    s.description, ',', IFNULL(a.description,''), ',' , IFNULL(m.levelvalue,''), ',', IFNULL(l.description,'')
#  )

def procRequest(dbconn):

  sqlQuery = """select
  m.mydb, m.id, m.hostid, 
    from_unixtime(m.time_trigger) time_trig, FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6) utime_trig,
    m.magnitude,  m.significance, 
    m.latitude, m.longitude, m.file, m.numreset, 
    s.description sensor, IFNULL(a.description,'') alignment, 
     IFNULL(m.levelvalue,'') level, IFNULL(l.description,'') level_type,
   q.time_utc quake_time, q.depth_km quake_depth_km, 
   q.latitude quake_lat, q.longitude quake_lon, q.magnitude, q.id
from
(
select 'Q' mydb, 
t.id, t.qcn_quakeid, t.hostid, t.time_trigger, t.magnitude, t.significance, t.latitude, t.longitude,
t.file, t.numreset, t.alignid, t.levelid, t.levelvalue, t.type_sensor
from %s.qcn_trigger t
where time_trigger between %d and %d
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between %f and %f and longitude between %f and %f
UNION
select 'C' mydb, 
tt.id, tt.qcn_quakeid, tt.hostid, tt.time_trigger, tt.magnitude, tt.significance, tt.latitude, tt.longitude,
tt.file, tt.numreset, tt.alignid, tt.levelid, tt.levelvalue, tt.type_sensor
from %s.qcn_trigger tt
where time_trigger between %d and %d
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between %f and %f and longitude between %f and %f
) m
LEFT JOIN qcnalpha.qcn_sensor s ON m.type_sensor = s.id
LEFT OUTER JOIN qcnalpha.qcn_align a ON m.alignid = a.id
LEFT OUTER JOIN qcnalpha.qcn_level l ON m.levelid = l.id
LEFT OUTER JOIN qcnalpha.qcn_quake q ON q.id = m.qcn_quakeid
where m.type_sensor=s.id
order by time_trigger,hostid"""  \
  % ( \
      DBNAME, DATE_MIN, DATE_MAX, LAT_MIN, LAT_MAX, LNG_MIN, LNG_MAX, \
      DBNAME_CONTINUAL, DATE_MIN, DATE_MAX, LAT_MIN, LAT_MAX, LNG_MIN, LNG_MAX  \
    )

  strHeader = "db, triggerid, hostid, time_utc, time_us, magnitude, significance, latitude, longitude, file, " +\
     "numreset, sensor, alignment, level_value, level_type\n"

  tmpdir = tempfile.mkdtemp()
  myCursor = dbconn.cursor()
  myCursor.execute(sqlQuery)

  zipoutpath = os.path.join(DOWNLOAD_WEB_DIR, FILE_ZIP)
  zipinpath = ""

  #strCSVFile = os.path.join(DOWNLOAD_WEB_DIR, FILE_CSV)
  strCSVFile = FILE_CSV
  fileCSV = open(strCSVFile, "w")
  fileCSV.write(strHeader)

  # get the resultset as a tuple
  result = myCursor.fetchall()
  numbyte = 0
  myzipout = None
  errlevel = 0

  try:
 
   # open a zip output file - allow zip64 compression for large (>2GB) files
   errlevel = 1
   #print zipoutpath, "  ", 'w', "  ", str(ZIP_STORED), "  ", str(True)
   myzipout = zipfile.ZipFile(zipoutpath, "w", ZIP_STORED, True)

   # iterate through resultset
   curdir = os.getcwd()   # save current directory and go to the temp dir (so paths aren't stored in zip's)
   os.chdir(tmpdir)
   for rec in result:
      errlevel = 2
      #print "    ", rec[0] , "  ", rec[1], "  ", rec[2], "  ", rec[3], "  ", rec[4], "  ", rec[5], "  ", rec[6]
      #"db, triggerid, hostid, time_utc, time_us, magnitude, significance, latitude, longitude, file, " +\
      # "numreset, sensor, alignment, level_value, level_type\n"

      # test for valid zip file
      try:
        if IS_ARCHIVE:  # need to get fanout directory 
          fandir, dtime = getFanoutDirFromZip(rec[9])
          if (rec[0] == "Q"):
            fullpath = os.path.join(UPLOAD_WEB_DIR, fandir)
            zipinpath = os.path.join(fullpath, rec[9])
          else:
            fullpath = os.path.join(UPLOAD_WEB_DIR_CONTINUAL, fandir)
            zipinpath = os.path.join(fullpath, rec[9])
        else:
          if (rec[0] == "Q"):
            zipinpath = os.path.join(UPLOAD_WEB_DIR, rec[9])
          else:
            zipinpath = os.path.join(UPLOAD_WEB_DIR_CONTINUAL, rec[9])

        myzipin = zipfile.ZipFile(zipinpath, "r")
        if os.path.isfile(zipinpath) and myzipin.testzip() == None:
           errlevel = 3
           # valid zip file so add to myzipout, first close
           zipinlist = myzipin.namelist()
           myzipin.extractall(tmpdir)
           myzipin.close()

           for zipinname in zipinlist:
             errlevel = 4
             #zipinpath = os.path.join(tmpdir, zipinname)
             # OK - at this point the zip file requested has been unzipped, so we need to process metadata here
  #   m.mydb, m.id, m.hostid,   0 1 2
  #  from_unixtime(m.time_trigger) time_trig, FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6) utime_trig, 3 4
  #  m.magnitude,  m.significance,  5 6
  #  m.latitude, m.longitude, m.file, m.numreset,  7 8 9 10
  #  s.description sensor, IFNULL(a.description,'') alignment,  11 12 
  #   IFNULL(m.levelvalue,'') level, IFNULL(l.description,'') level_type 13 14 

        #def getSACMetadata(zipinname, hostid, latTrig, lonTrig, lvlTrig, lvlType, idQuake, timeQuake, depthKmQuake, latQuake, lonQuake, magQuake):
             getSACMetadata(zipinname, rec[3], rec[7], rec[8], rec[13], rec[14], rec[20], rec[15], rec[16], rec[17], rec[18], rec[19])
             myzipout.write(zipinname)
             os.remove(zipinname)


           # valid file - print out line of csv
           for x in range(15):
             fileCSV.write(str(rec[x]))
             if x < 15:
               fileCSV.write(",")
           fileCSV.write("\n")

      except:
        print "Error " + str(errlevel) + " in myzipin " + zipinpath
        traceback.print_exc()
        #exit(3)
        continue

   fileCSV.close()
   os.chdir(curdir)   # go back to regular directory so tmpdir can be erased
   myzipout.write(strCSVFile)
   myzipout.close() 
   numbyte = os.path.getsize(zipoutpath)
   shutil.rmtree(tmpdir)    # remove temp directory
   myCursor.close();
   return numbyte

  except zipfile.error:
   print "Error " + str(errlevel) + " in " + zipoutpath + " or " + zipinpath +\
        " is an invalid zip file (tmpdir=" + tmpdir + ")"
   #dbconn.rollback()
   traceback.print_exc()
   shutil.rmtree(tmpdir)    # remove temp directory
   myCursor.close();
   if (myzipout != None):
      myzipout.close() 
      os.remove(zipoutpath)
   return 0
  except:
   print "Error " + str(errlevel) + " in " + zipoutpath + " or " + zipinpath + " (tmpdir=" + tmpdir + ")"
   #dbconn.rollback()
   traceback.print_exc()
   shutil.rmtree(tmpdir)    # remove temp directory
   myCursor.close();
   if (myzipout != None):
      myzipout.close() 
      os.remove(zipoutpath)
   return 0

def sendEmail(Username, ToEmailAddr, DLURL, NumMB):
  global SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT
  # sends email that job is done
  FromEmailAddr = "noreply@qcn.stanford.edu"
  server=smtplib.SMTP_SSL(SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT)
  msg = "Hello " + Username + ":\n\n" + "Your requested files are available for download " +\
    "over the next 24 hours from the following URL:\n\n" + DLURL +\
    "\n\nThe file size to download is approximately " + str(NumMB) + " megabytes." +\
    "\n\nNote that this email is automated - please do not reply!"
  if typeRunning == "C":
    subj = "QCN Continual Download Archive Completed"
  else:
    subj = "QCN Sensor Download Archive Completed"

  MessageText = """\
From: %s
To: %s
Subject: %s

%s
""" % (FromEmailAddr, ToEmailAddr, subj, msg)

  server.sendmail(FromEmailAddr, ToEmailAddr, MessageText)
  server.quit()

def updateRequest(dbconn, jobid, numbyte, outfilename, url):
   myCursor = dbconn.cursor()
   query = "UPDATE " + DBNAME_JOB + ".job SET finish_time=unix_timestamp(), " +\
                "url='" + url + "', local_path='" + outfilename + "', size=" + str(numbyte) +\
                " WHERE id=" + str(jobid)
   #print query
   myCursor.execute(query)
   dbconn.commit();
   myCursor.close();


def processContinualJobs(dbconn):
   # read the DBNAME_JOB table for unfinished jobs, then process the upload files into a single bundle
   myCursor = dbconn.cursor()
   query = "SELECT j.id, j.userid, u.name, u.email_addr, j.create_time, j.list_triggerid " +\
      "FROM " + DBNAME_JOB + ".job j, " + DBNAME + ".user u " +\
      "WHERE j.userid=u.id AND finish_time IS NULL"

   myCursor.execute(query)

   # get the resultset as a tuple
   result = myCursor.fetchall()
   totalmb = 0

   # iterate through resultset
   for rec in result:
      outfilename = "u" + str(rec[1]) + "_j" + str(rec[0]) + ".zip"
      url = URL_DOWNLOAD_BASE + outfilename
      print rec[0] , "  ", rec[1], "  ", rec[2], "  ", rec[3]
      numbyte = procDownloadRequest(dbconn, outfilename, url, rec[0], rec[1], rec[5])
      if (numbyte > 0):
         nummb = math.floor(numbyte/(1024*1024))
         totalmb += nummb
         sendEmail(rec[2], rec[3], url, nummb)
         updateRequest(dbconn, rec[0], numbyte, outfilename, url)

   myCursor.close();
   return math.ceil(totalmb)

# makes sure that the necessary paths are in place as defined above
def checkPaths():
   global UPLOAD_WEB_DIR
   global UPLOAD_WEB_DIR_CONTINUAL
   global DOWNLOAD_WEB_DIR

   if not os.access(UPLOAD_WEB_DIR, os.F_OK | os.W_OK):
      print UPLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR does not exist or not writable!"
      return 1
   
   if not os.access(UPLOAD_WEB_DIR_CONTINUAL, os.F_OK | os.W_OK):
      print UPLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR_CONTINUAL does not exist or not writable!"
      return 1

   if not os.access(DOWNLOAD_WEB_DIR, os.F_OK | os.W_OK):
      print DOWNLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR does not exist or not writable!"
      return 1
   
   return 0

def main():
   global UPLOAD_WEB_DIR
   global DOWNLOAD_WEB_DIR
   global UPLOAD_WEB_DIR_CONTINUAL
   global DBNAME
   global DBNAME_CONTINUAL
   global IS_ARCHIVE
   global DATE_MIN
   global DATE_MAX

   DATE_MIN = ""
   DATE_MAX = ""

   try:
      cnt = 0
      for s in sys.argv:
        if cnt == 1:
           DATE_MIN = s
        elif cnt == 2:
           DATE_MAX = s
        cnt = cnt + 1

      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)

      if (cnt < 3):  # just use first week of current month
        sqlts = "SELECT UNIX_TIMESTAMP(DATE_ADD(NOW(), INTERVAL -8 DAY)), UNIX_TIMESTAMP(DATE_ADD(NOW(), INTERVAL -1 DAY))"
      else:
        sqlts = "SELECT UNIX_TIMESTAMP('" + DATE_MIN + "'), UNIX_TIMESTAMP('" + DATE_MAX + "')"
    
      myCursor = dbconn.cursor()
      myCursor.execute(sqlts)
      res = myCursor.fetchall()
      myCursor.close()

      if (res[0][0] == 0 or res[0][1] == 0) or (res[0][0] > res[0][1]):
         print "Usage: ./scedc DATE_MIN DATE_MAX\n"
         print "Invalid dates entered - format is YYYY-MM-DD\n"
         dbconn.close()
         return 1

      DATE_MIN = int(res[0][0])
      DATE_MAX = int(res[0][1])

      # now chck if we need archive database or not
      sqlts = "SELECT value_int FROM qcnalpha.qcn_constant WHERE description='ArchiveTime'"
      myCursor = dbconn.cursor()
      myCursor.execute(sqlts)
      res = myCursor.fetchall()
      myCursor.close()
      if int(res[0][0]) == 0:
         print "Usage: ./scedc DATE_MIN DATE_MAX\n"
         print "Cannot get archive timestamp - format is YYYY-MM-DD\n"
         dbconn.close()
         return 2

      if (int(res[0][0]) > DATE_MIN):
        IS_ARCHIVE = 1
        UPLOAD_WEB_DIR           = "/data/cees2/QCN/trigger/archive/"
        UPLOAD_WEB_DIR_CONTINUAL = "/data/cees2/QCN/trigger/archive/continual/"
        DBNAME                   = "qcnarchive"
        DBNAME_CONTINUAL         = "contarchive"
      else: 
        IS_ARCHIVE = 0
        UPLOAD_WEB_DIR           = "/var/www/trigger/"
        UPLOAD_WEB_DIR_CONTINUAL = "/var/www/trigger/continual/"
        DBNAME                   = "qcnalpha"
        DBNAME_CONTINUAL         = "continual"


      # first make sure all the necessary paths are in place
      if (checkPaths() != 0):
         sys.exit(2)
 
      procRequest(dbconn)

      dbconn.close()

   except:
      traceback.print_exc()
      return 3

if __name__ == '__main__':
    main()


