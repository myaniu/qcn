#! /usr/bin/env python

# this program will bundle up files from a continual_download.job request
# into a single file for downloading

# this will of course be run on the upload server as a cron task, mysqldb for
# python should be installed on the server as well as a mysql acct from the
# upload server to the database server

# CMC note -- need to install 3rd party MySQLdb libraries for python
import smtplib, traceback, sys, os, tempfile, string, MySQLdb, shutil, zipfile
from datetime import datetime
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

def sendEmail(Username, ToEmailAddr, DLURL):
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


def processSingleZipFile(dbconn, myzipfile):
   # process a single zip file, i.e. test, unzip into myTempDir, list all the filenames,
   # test the zips within the zip file perhaps?, and of course update the qcn_trigger
   # table that we have received this zipfile
   fullzippath = os.path.join(UPLOAD_BOINC_DIR, myzipfile)

   try:  # catch zipfile exceptions if any
      myCursor = dbconn.cursor()
      
      myzip = zipfile.ZipFile(fullzippath, "r")
      # test for valid zip file
      if myzip.testzip() != None:
         # move out invalid zip file by raising the zipfile.error exception (caught below)
         raise zipfile.error

      # get the files within the zip
      infiles = myzip.namelist()
         
      for name in infiles:
        if name.endswith("_usb.zip"):
            # this is an upload from a usb test zip file
            outfile = open(os.path.join(UPLOAD_USB_WEB_DIR, name), 'wb')
            outfile.write(myzip.read(name))
            outfile.close()
        elif name.startswith("continual_"):
            # this is an upload from a usb test zip file
            outfile = open(os.path.join(UPLOAD_CONTINUAL_WEB_DIR, name), 'wb')
            outfile.write(myzip.read(name))
            outfile.close()

            # now update the qcn_trigger table!
            strSQL = "UPDATE continual.qcn_trigger SET received_file=100, " +\
                          "file_url='" + URL_DOWNLOAD_BASE + "continual/" + name + "' " +\
                          "WHERE file='" + name + "'"
            #print strSQL
            myCursor.execute(strSQL)
            dbconn.commit()
        else: 
            # this is a regular trigger
            outfile = open(os.path.join(UPLOAD_WEB_DIR, name), 'wb')
            outfile.write(myzip.read(name))
            outfile.close()

            # now update the qcn_trigger table!
            myCursor.execute("UPDATE qcnalpha.qcn_trigger SET received_file=100, " +\
                          "file_url='" + URL_DOWNLOAD_BASE + name + "' " +\
                          "WHERE file='" + name + "'")
            dbconn.commit()

      myzip.close()
      shutil.move(fullzippath, UPLOAD_BACKUP_DIR)
      print "Successfully processed " + fullzippath
      
   except zipfile.error:
      print fullzippath + " is an invalid zip file"
      # move out invalid zip file
      shutil.move(fullzippath, UPLOAD_BACKUP_DIR)
      dbconn.rollback()
      traceback.print_exc()
   except:
      print "Error 2 in " + fullzippath
      dbconn.rollback()
      traceback.print_exc()
   
def processContinualJobs(dbconn):
   # read the continual_download.job table for unfinished jobs, then process the upload files into a single bundle
   myCursor = dbconn.cursor()
   query = "SELECT j.id, j.userid, u.name, u.email_addr, j.create_time, j.list_triggerid " +\
      "FROM continual_download.job j, continual.user u " +\
      "WHERE j.userid=u.id AND finish_time IS NULL"  

   myCursor.execute(query)

   # get the resultset as a tuple
   result = myCursor.fetchall()

   # iterate through resultset
   for rec in result:
      outfilename = "u" + str(rec[1]) + "_j" + str(rec[0]) + ".zip"
      print rec[0] , "-->", rec[1], "  ", rec[2], "  ", rec[3]
      sendEmail(rec[2], rec[3], URL_DOWNLOAD_BASE + outfilename)

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


