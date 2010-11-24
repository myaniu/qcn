#!/bin/bash
#archive.sql is stored in svn:/qcn/server/sql/archive.sql
FILE_BACKUP=/home/boinc/qcn-archive-backup.sql
/usr/local/mysql/bin/mysql -h db-private -u root -pPWD qcnalpha < /home/boinc/archive.sql
# now that the archive is done, make a copy of it, so we don't have to dump this all out every night
/bin/rm -f $FILE_BACKUP
/bin/nice -n19 /usr/local/mysql/bin/mysqldump -h db-private -u root -pPWD --databases qcnarchive contarchive > $FILE_BACKUP
/usr/local/bin/python /home/boinc/trigger_archive.py
