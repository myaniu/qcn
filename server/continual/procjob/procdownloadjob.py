#! /usr/bin/env python

# this program will bundle up files from a continual_download.job request
# into a single file for downloading

# this will of course be run on the upload server as a cron task, mysqldb for
# python should be installed on the server as well as a mysql acct from the
# upload server to the database server

# CMC note -- need to install 3rd party MySQLdb libraries for python
import tempfile, smtplib, traceback, sys, os, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
from zipfile import ZIP_STORED
from time import strptime, mktime

# trigger file download URL base
URL_DOWNLOAD_BASE = "http://qcn-upl.stanford.edu/trigger/continual/job/"

# CMC note -- make sure these paths exist, or they will be created!
UPLOAD_CONTINUAL_WEB_DIR = "/var/www/trigger/continual/"
DOWNLOAD_CONTINUAL_WEB_DIR = "/var/www/trigger/continual/job/"
UPLOAD_WEB_DIR = "/var/www/trigger/continual/"

UNZIP_CMD = "/usr/bin/unzip -o -d " + UPLOAD_WEB_DIR + " " 

# use fast zip -1 compression as the files are already compressed
ZIP_CMD = "/usr/bin/zip -1 "

DBNAME = "continual"
DBHOST = "db-private"
DBUSER = "qcn"
DBPASSWD = ""
SMTP_HOST = "smtp.stanford.edu"

def procDownloadRequest(dbconn, outfilename, url, jobid, userid, trigidlist):
 tmpdir = tempfile.mkdtemp()
 myCursor = dbconn.cursor()
 query = "SELECT id,hostid,latitude,longitude,levelvalue,levelid,file " +\
              "FROM continual.qcn_trigger " +\
              "WHERE received_file=100 AND id IN " + trigidlist
 myCursor.execute(query)

 zipoutpath = os.path.join(DOWNLOAD_CONTINUAL_WEB_DIR, outfilename)
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
   for rec in result:
      errlevel = 2
      #print "    ", rec[0] , "  ", rec[1], "  ", rec[2], "  ", rec[3], "  ", rec[4], "  ", rec[5], "  ", rec[6]

      zipinpath = os.path.join(UPLOAD_CONTINUAL_WEB_DIR, rec[6])
      myzipin = zipfile.ZipFile(zipinpath, "r")
      # test for valid zip file
      if myzipin.testzip() == None:
         errlevel = 3
         # valid zip file so add to myzipout, first close
         zipinlist = myzipin.namelist()
         myzipin.extractall(tmpdir)
         myzipin.close()
         for zipinname in zipinlist:
            errlevel = 4
            zipinpath = os.path.join(tmpdir, zipinname)
            myzipout.write(zipinpath)
            os.remove(zipinpath)

   myzipout.close() 
   numbyte = os.path.getsize(zipoutpath)
   shutil.rmtree(tmpdir)    # remove temp directory
   return numbyte

 except zipfile.error:
   print "Error " + str(errlevel) + " in " + zipoutpath + " or " + zipinpath +\
        " is an invalid zip file (tmpdir=" + tmpdir + ")"
   #dbconn.rollback()
   traceback.print_exc()
   shutil.rmtree(tmpdir)    # remove temp directory
   if (myzipout != None):
      myzipout.close() 
      os.remove(zipoutpath)
   return 0
 except:
   print "Error " + str(errlevel) + " in " + zipoutpath + " or " + zipinpath + " (tmpdir=" + tmpdir + ")"
   #dbconn.rollback()
   traceback.print_exc()
   shutil.rmtree(tmpdir)    # remove temp directory
   if (myzipout != None):
      myzipout.close() 
      os.remove(zipoutpath)
   return 0
 

def sendEmail(Username, ToEmailAddr, DLURL):
  return
  # sends email that job is done
  FromEmailAddr = "noreply@qcn.stanford.edu"
  server=smtplib.SMTP(SMTP_HOST)
  msg = "Hello " + Username + ":\n\n" + "Your requested files are available for download " +\
    "over the next 24 hours from the following URL:\n\n" + DLURL + "\n\nNote that this email is automated - please do not reply!"
  subj = "QCN Continual Download Archive Completed"

  MessageText = """\
From: %s
To: %s
Subject: %s

%s
""" % (FromEmailAddr, ToEmailAddr, subj, msg)

  server.sendmail(FromEmailAddr, ToEmailAddr, MessageText)
  server.quit()


def processContinualJobs(dbconn):
   # read the continual_download.job table for unfinished jobs, then process the upload files into a single bundle
   myCursor = dbconn.cursor()
   query = "SELECT j.id, j.userid, u.name, u.email_addr, j.create_time, j.list_triggerid " +\
      "FROM continual_download.job j, continual.user u " +\
      "WHERE j.userid=u.id AND finish_time IS NULL AND userid=15"  

   myCursor.execute(query)

   # get the resultset as a tuple
   result = myCursor.fetchall()

   # iterate through resultset
   for rec in result:
      numbyte = 0
      outfilename = "u" + str(rec[1]) + "_j" + str(rec[0]) + ".zip"
      url = URL_DOWNLOAD_BASE + outfilename
      print rec[0] , "  ", rec[1], "  ", rec[2], "  ", rec[3]
      if (procDownloadRequest(dbconn, outfilename, url, rec[0], rec[1], rec[5]) > 0):
         sendEmail(rec[2], rec[3], url)
         updateRequest(rec[0])

   myCursor.close();


# makes sure that the necessary paths are in place as defined above
def checkPaths():
   if not os.access(UPLOAD_CONTINUAL_WEB_DIR, os.F_OK | os.W_OK):
      print UPLOAD_CONTINUAL_WEB_DIR + " directory for UPLOAD_CONTINUAL_WEB_DIR does not exist or not writable!"
      return 1
   
   if not os.access(DOWNLOAD_CONTINUAL_WEB_DIR, os.F_OK | os.W_OK):
      print DOWNLOAD_CONTINUAL_WEB_DIR + " directory for UPLOAD_CONTINUAL_WEB_DIR does not exist or not writable!"
      return 1
   
   return 0
      
def main():
   try:

      # first make sure all the necessary paths are in place
      if (checkPaths() != 0):
         sys.exit(2)
         
      dbconn = MySQLdb.connect (host = DBHOST,
                           user = DBUSER,
                           passwd = DBPASSWD,
                           db = DBNAME)

      processContinualJobs(dbconn)

      dbconn.close()

   except:
      traceback.print_exc()
      sys.exit(1)

if __name__ == '__main__':
    main()


