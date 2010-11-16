#! /usr/local/bin/python
#// #! /usr/bin/env python

# this program will bundle up files for SCEDC, also make a CSV file of the triggers/files

# this will be run manually for now, can be extended for other areas of course

# CMC note -- need to install 3rd party MySQLdb libraries for python
import math, tempfile, smtplib, traceback, sys, os, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
from zipfile import ZIP_STORED
from time import strptime, mktime
from subprocess import Popen, PIPE, STDOUT

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
global SAC_CMD, SACSWAP_CMD, GRD_CMD

SAC_CMD = "/usr/local/sac/bin/sac"
SACSWAP_CMD = "/usr/local/sac/bin/sacswap"
GRD_CMD = "/usr/local/gmt/bin/grd2point /usr/local/gmt/share/topo/topo30.grd -R"

UPLOAD_WEB_DIR           = "/var/www/trigger/"
DOWNLOAD_WEB_DIR         = "/var/www/trigger/job/"
UPLOAD_WEB_DIR_CONTINUAL = "/var/www/trigger/continual/"
DBNAME                   = "qcnalpha"
DBNAME_CONTINUAL         = "continual"
FILE_CSV                 = "qcn_scedc.csv"
FILE_ZIP                 = "qcn_scedc.zip"
  
DATE_MIN = "2010-11-14 00:00:00"
DATE_MAX = "2010-11-16 00:00:00"
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
  global sqlQuery
  global DOWNLOAD_WEB_DIR
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


  sqlQuery = """select
  m.mydb, m.id, m.hostid, 
    from_unixtime(m.time_trigger) time_trig, FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6) utime_trig,
    m.magnitude,  m.significance, 
    m.latitude, m.longitude, m.file, m.numreset, 
    s.description sensor, IFNULL(a.description,'') alignment, 
     IFNULL(m.levelvalue,'') level, IFNULL(l.description,'') level_type
from
(
select 'Q' mydb, t.*
from qcnalpha.qcn_trigger t
where time_trigger between unix_timestamp('%s') and unix_timestamp('%s')
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between %f and %f and longitude between %f and %f
UNION
select 'C' mydb, tt.*
from continual.qcn_trigger tt
where time_trigger between unix_timestamp('%s') and unix_timestamp('%s')
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between %f and %f and longitude between %f and %f
) m
LEFT JOIN qcnalpha.qcn_sensor s ON m.type_sensor = s.id
LEFT OUTER JOIN qcnalpha.qcn_align a ON m.alignid = a.id
LEFT OUTER JOIN qcnalpha.qcn_level l ON m.levelid = l.id
where m.type_sensor=s.id
order by time_trigger,hostid"""  \
  % ( \
      DATE_MIN, DATE_MAX, LAT_MIN, LAT_MAX, LNG_MIN, LNG_MAX, \
      DATE_MIN, DATE_MAX, LAT_MIN, LAT_MAX, LNG_MIN, LNG_MAX  \
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
       #  global SAC_CMD, SACSWAP_CMD, GRD_CMD
  #   m.mydb, m.id, m.hostid,   0 1 2
  #  from_unixtime(m.time_trigger) time_trig, FLOOR(ROUND((m.time_trigger-FLOOR(m.time_trigger)), 6) * 1e6) utime_trig, 3 4
  #  m.magnitude,  m.significance,  5 6
  #  m.latitude, m.longitude, m.file, m.numreset,  7 8 9 10
  #  s.description sensor, IFNULL(a.description,'') alignment,  11 12 
  #   IFNULL(m.levelvalue,'') level, IFNULL(l.description,'') level_type 13 14 

        #def getSACMetadata(zipinname, latTrig, lonTrig, lvlTrig, lvlType, idQuake, timeQuake, depthKmQuake, latQuake, lonQuake, magQuake):
             getSACMetadata(zipinname, rec[7], rec[8], rec[13], rec[14], 0, 0, 0, 0, 0, 0)
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

# this will put metadata into the SAC file using values from the database for this trigger
# it's very "quick & dirty" and just uses SAC as a cmd line program via a script
def getSACMetadata(zipinname, latTrig, lonTrig, lvlTrig, lvlType, idQuake, timeQuake, depthKmQuake, latQuake, lonQuake, magQuake):
  global SAC_CMD, SACSWAP_CMD, GRD_CMD


# elevation data - usage of GRD_CMD -Rlon_min/lon_max/lat_min/lat_max
#grd2point /usr/local/gmt/share/topo/topo30.grd -R$lng/$lng2/$lat/$lat2
#> temp.xyz
#output
#/usr/local/gmt/bin/grd2point /usr/local/gmt/share/topo/topo30.grd -R-75.01/-75.00/40.00/40.01
#grd2point: gmt WARNING: (w - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= 0.0001.
#grd2point: gmt WARNING: w reset to -75.0083
#grd2point: gmt WARNING: (n - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= 0.0001.
#grd2point: gmt WARNING: n reset to 40.0083
#-75.0041666667 40.0041666667   19

#outputs closest lon/lat point and elevation in meters
  myElev = 0.0
  # lvlType of 4 or 5 means they explicitly put in the elevation, so no need to look up 
  if lvlType not in (4,5):
    grdstr = str(lonTrig - .005) + "/" + str(lonTrig + .005) + "/" + str(latTrig - .005) + "/" + str(latTrig + .005)
    cc = Popen(GRD_CMD + grdstr, shell=True, stdout=PIPE).communicate()[0]
    vals = cc.rstrip("\n").split("\t")
    if len(vals) == 3:
      myElev = float(vals[2])

  # at this point myElev is either 0 or their estimated elevation in meters based on lat/lng

#lvlType should be one of:
#|  1 | Floor (+/- above/below surface)    | 
#|  2 | Meters (above/below surface)       | 
#|  3 | Feet (above/below surface)         | 
#|  4 | Elevation - meters above sea level | 
#|  5 | Elevation - feet above sea level   |  note 4 & 5 they input actual elevation , so use that
#
  # we want level in meters, ideally above sea level, but now just convert to meters (1 floor = 3 m)
  myLevel = myElev
  if lvlType == 1:
    myLevel = myElev + (lvlTrig * 3.0)
  elif lvlType == 2:
    myLevel = myElev + lvlTrig
  elif lvlType == 3:
    myLevel = myElev + (lvlTrig * 0.3048)
  elif lvlType == 4:
    myLevel = lvlTrig
  elif lvlType == 5:
    myLevel = lvlTrig * 0.3048

#  sac values to fill in are: stlo, stla, stel (for station)
#                             evlo, evla, evdp, mag (for quake)

#  print "\n\nmyLevel = " + str(myLevel) + " meters\n\n"

  fullcmd = SAC_CMD + " << EOF\n" +\
    "r " + zipinname + "\n" +\
    "chnhdr stlo " + str(lonTrig) + "\n" +\
    "chnhdr stla " + str(latTrig) + "\n" +\
    "chnhdr stel " + str(myLevel) + "\n"

  #if myLevel != 0.0:
  #  fullcmd = fullcmd + "chnhdr stel " + str(myLevel) + "\n" 

  if idQuake > 0:
    fullcmd = fullcmd +\
      "chnhdr evlo " + str(lonQuake) + "\n" +\
      "chnhdr evla " + str(latQuake) + "\n" +\
      "chnhdr evdp " + str(1000.0 * depthKmQuake) + "\n" +\
      "chnhdr mag "  + str(magQuake) + "\n"

  fullcmd = fullcmd +\
      "chnhdr leven TRUE\n" +\
      "write over \n" +\
      "quit\n" +\
      "EOF\n"

# debug info
#  print fullcmd

  cc = Popen(fullcmd, shell=True, stdout=PIPE).communicate()[0]

# now need to run sacswap for some reason
#  fullcmd = SACSWAP_CMD + " " + zipinname
#  cc = Popen(fullcmd, shell=True, stdout=PIPE).communicate()[0]

# if we have a file named zipinname.swap, then we need to move back over to zipinname 
#  if os.path.isfile(zipinname + ".swap"):
#    shutil.move(zipinname + ".swap", zipinname)

# done metadata updating of SAC files

      
def main():
   try:
      # first make sure all the necessary paths are in place
      if (checkPaths() != 0):
         sys.exit(2)

      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)

      #totalmb = processContinualJobs(dbconn)
      #print str(totalmb) + " MB of zip files processed"

      procRequest(dbconn)


      dbconn.close()

   except:
      traceback.print_exc()
      sys.exit(1)

if __name__ == '__main__':
    main()


