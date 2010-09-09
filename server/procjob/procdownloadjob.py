#! /usr/local/bin/python
#// #! /usr/bin/env python

# this program will bundle up files from a continual_download.job request
# into a single file for downloading

# this will of course be run on the upload server as a cron task, mysqldb for
# python should be installed on the server as well as a mysql acct from the
# upload server to the database server

# CMC note -- need to install 3rd party MySQLdb libraries for python

# the layout of the program is to unzip each requested zip, insert metadata into the unzipped 
# SAC files (from the qcn_trigger record), and then after all requested zip files are processed, zip them all up and 
# move to a download location, with an email to the user of the download link


import math, time, tempfile, smtplib, traceback, sys, os, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
from zipfile import ZIP_STORED
from time import strptime, mktime
from subprocess import Popen, PIPE, STDOUT

global DBHOST 
global DBUSER
global DBPASSWD
global SAC_CMD, SACSWAP_CMD, GRD_CMD
global SMTPS_HOST, SMTPS_PORT, SMTPS_LOCAL_HOSTNAME, SMTPS_KEYFILE, SMTPS_CERTFILE, SMTPS_TIMEOUT

DBHOST = "db-private"
DBUSER = "qcn"
DBPASSWD = ""

SAC_CMD = "/usr/local/sac/bin/sac"
SACSWAP_CMD = "/usr/local/sac/bin/sacswap"
GRD_CMD = "/data/cees2/QCN/GMT/bin/grd2point /data/cees2/QCN/GMT/share/topo/topo30.grd -R"

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

# use fast zip -1 compression as the files are already compressed
global ZIP_CMD
ZIP_CMD  = "/usr/bin/zip -1 "
global UNZIP_CMD
global typeRunning
typeRunning = ""

# delete old job files > 7 days old
def delFilesPath(path):
  now = time.time()
  for f in os.listdir(path):
    fname = os.path.join(path, f)
    if os.stat(fname).st_mtime < now - 7 * 86400:
      if os.path.isfile(fname) and f.find("qcn_scedc") == -1:
        os.remove(fname)


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
 query = "SELECT t.id,t.hostid,t.latitude,t.longitude,t.levelvalue,t.levelid,t.file, " +\
            "t.qcn_quakeid, q.time_utc quake_time, q.depth_km quake_depth_km, " +\
            "q.latitude quake_lat, q.longitude quake_lon, q.magnitude quake_mag " +\
              "FROM " + DBNAME + ".qcn_trigger t " +\
              "LEFT OUTER JOIN qcnalpha.qcn_quake q ON q.id = t.qcn_quakeid " +\
              "WHERE t.received_file=100 AND t.id IN " + trigidlist
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
             # OK - at this point the zip file requested has been unzipped, so we need to process metadata here
             getSACMetadata(zipinname, rec[2], rec[3], rec[4], rec[5], rec[7], rec[8], rec[9], rec[10], rec[11], rec[12])

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

# this will put metadata into the SAC file using values from the database for this trigger
# it's very "quick & dirty" and just uses SAC as a cmd line program via a script
def getSACMetadata(zipinname, latTrig, lonTrig, lvlTrig, lvlType, idQuake, timeQuake, depthKmQuake, latQuake, lonQuake, magQuake):
  global SAC_CMD, SACSWAP_CMD, GRD_CMD


# elevation data - usage of GRD_CMD -Rlon_min/lon_max/lat_min/lat_max
#grd2point /data/cees2/QCN/GMT/share/topo/topo30.grd -R$lng/$lng2/$lat/$lat2
#> temp.xyz
#output
#/data/cees2/QCN/GMT/bin/grd2point /data/cees2/QCN/GMT/share/topo/topo30.grd -R-75.01/-75.00/40.00/40.01
#grd2point: GMT WARNING: (w - x_min) must equal (NX + eps) * x_inc), where NX is an integer and |eps| <= 0.0001.
#grd2point: GMT WARNING: w reset to -75.0083
#grd2point: GMT WARNING: (n - y_min) must equal (NY + eps) * y_inc), where NY is an integer and |eps| <= 0.0001.
#grd2point: GMT WARNING: n reset to 40.0083
#-75.0041666667	40.0041666667	19

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

      delFilesPath(DOWNLOAD_WEB_DIR)

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


