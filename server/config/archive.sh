#!/bin/bash
#archive.sql is stored in svn:/qcn/server/sql/archive.sql
FILE_BACKUP=/home/boinc/qcn-archive-backup.sql
#first set the constant for the archive time i.e. current time - two months
/usr/local/mysql/bin/mysql -h db-private -u root -pPWD qcnalpha -e "UPDATE qcnalpha.qcn_constant SET value_int=unix_timestamp()-(3600*24*60) WHERE description='ArchiveTime'"
/usr/local/mysql/bin/mysql -h db-private -u root -pPWD qcnalpha < /home/boinc/archive.sql
# now that the archive is done, make a copy of it, so we don't have to dump this all out every night
/bin/rm -f $FILE_BACKUP
/bin/nice -n19 /usr/local/mysql/bin/mysqldump -h db-private -u root -pPWD --databases sensor_archive continual_archive > $FILE_BACKUP
/bin/nice -n19 /bin/gzip -f $FILE_BACKUP
/usr/local/bin/python /home/boinc/trigger_archive.py
