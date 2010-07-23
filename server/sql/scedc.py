#! /usr/local/bin/python
#// #! /usr/bin/env python

# this program will bundle up files for SCEDC, also make a CSV file of the triggers/files

# this will be run manually for now, can be extended for other areas of course

# CMC note -- need to install 3rd party MySQLdb libraries for python
import math, tempfile, smtplib, traceback, sys, os, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
from zipfile import ZIP_STORED
from time import strptime, mktime

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


# the next 5 globals will be set by the appropriate run type in SetRunType()
global URL_DOWNLOAD_BASE
global UPLOAD_WEB_DIR
global DOWNLOAD_WEB_DIR
global DBNAME
global DBNAME_JOB
global sqlQuery

# use fast zip -1 compression as the files are already compressed
global ZIP_CMD
ZIP_CMD  = "/usr/bin/zip -1 "
global UNZIP_CMD
global typeRunning
typeRunning = ""

sqlQuery = "select
  concat(m.mydb, ',' , m.id, ',', m.hostid, ',',
    from_unixtime(m.time_trigger), ',', FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6),
     ',',
    m.magnitude, ',', m.significance, ',',
    m.latitude, ',', m.longitude, ',', m.file,',', m.numreset, ',',
    s.description, ',', IFNULL(a.description,''), ',' , IFNULL(m.levelvalue,''), ',', IFNULL(l.description,'')
  )
from
(
select 'Q' mydb, t.*
from qcnalpha.qcn_trigger t
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00')
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between 31.5 and 37.5 and longitude between -121 and -114

UNION

select 'C' mydb, tt.*
from continual.qcn_trigger tt
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00')
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between 31.5 and 37.5 and longitude between -121 and -114
) m
LEFT JOIN qcnalpha.qcn_sensor s ON m.type_sensor = s.id
LEFT OUTER JOIN qcnalpha.qcn_align a ON m.alignid = a.id
LEFT OUTER JOIN qcnalpha.qcn_level l ON m.levelid = l.id
where m.type_sensor=s.id
order by time_trigger,hostid
"

def SetRunType():
  global URL_DOWNLOAD_BASE
  global UPLOAD_WEB_DIR
  global DOWNLOAD_WEB_DIR
  global DBNAME
  global DBNAME_JOB
  global typeRunning
  icnt = 0
  typeRunning = ""
  for arg in sys.argv:
    if icnt == 1:
      typeRunning = arg
    icnt = icnt + 1

  if typeRunning != "C" and typeRunning != "S":
    print "Must pass in C for Continual jobs, S for regular qcnalpha/sensor DB jobs"
    sys.exit(3)

  if typeRunning == "C":  # continual
    URL_DOWNLOAD_BASE = "http://qcn-upl.stanford.edu/trigger/continual/job/"
    # CMC note -- make sure these paths exist
    UPLOAD_WEB_DIR = "/var/www/trigger/continual/"
    DOWNLOAD_WEB_DIR = "/var/www/trigger/continual/job/"
    DBNAME = "continual"
    DBNAME_JOB = "continual_download"
  else:   #qcnalpha/sensor database
    URL_DOWNLOAD_BASE = "http://qcn-upl.stanford.edu/trigger/job/"
    UPLOAD_WEB_DIR = "/var/www/trigger/"
    DOWNLOAD_WEB_DIR = "/var/www/trigger/job/"
    DBNAME = "qcnalpha"
    DBNAME_JOB = "sensor_download"

  UNZIP_CMD = "/usr/bin/unzip -o -d " + UPLOAD_WEB_DIR + " "

def procDownloadRequest(dbconn, outfilename, url, jobid, userid, trigidlist):
 tmpdir = tempfile.mkdtemp()
 myCursor = dbconn.cursor()
 query = "SELECT id,hostid,latitude,longitude,levelvalue,levelid,file " +\
              "FROM " + DBNAME + ".qcn_trigger " +\
              "WHERE received_file=100 AND id IN " + trigidlist
 myCursor.execute(query)

 zipoutpath = os.path.join(DOWNLOAD_WEB_DIR, outfilename)
 zipinpath = ""

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

      # test for valid zip file
      try:
        zipinpath = os.path.join(UPLOAD_WEB_DIR, rec[6])
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
             myzipout.write(zipinname)
             os.remove(zipinname)
      except:
        print "Error " + str(errlevel) + " in myzipin " + zipinpath
        continue

   os.chdir(curdir)   # go back to regular directory so tmpdir can be erased
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
   global DOWNLOAD_WEB_DIR
   if not os.access(UPLOAD_WEB_DIR, os.F_OK | os.W_OK):
      print UPLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR does not exist or not writable!"
      return 1
   
   if not os.access(DOWNLOAD_WEB_DIR, os.F_OK | os.W_OK):
      print DOWNLOAD_WEB_DIR + " directory for UPLOAD_WEB_DIR does not exist or not writable!"
      return 1
   
   return 0
      
def main():
   global typeRunning
   try:
      # set appropriate global vars for run type (i.e. continual or sensor)
      SetRunType() 

      # first make sure all the necessary paths are in place
      if (checkPaths() != 0):
         sys.exit(2)

      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)

      totalmb = processContinualJobs(dbconn)

      print str(totalmb) + " MB of zip files processed"

      dbconn.close()

   except:
      traceback.print_exc()
      sys.exit(1)

if __name__ == '__main__':
    main()


