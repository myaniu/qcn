#!/bin/bash
#archive.sql is stored in svn:/qcn/server/sql/archive.sql
/usr/local/mysql/bin/mysql -u root -pPWD qcnalpha < /root/archive.sql
# now that the archive is done, make a copy of it, so we don't have to dump this all out every night
/bin/rm -f /tmp/qcn-archive-backup.sql
/bin/nice -n19 /usr/local/mysql/bin/mysqldump -u root -pPWD --databases qcnarchive contarchive > /tmp/qcn-archive-backup.sql
/bin/nice -n19 /usr/bin/scp -C /tmp/qcn-archive-backup.sql carlgt1@upl-private:
/bin/rm -f /tmp/qcn-archive-backup.sql
/usr/local/bin/python /home/boinc/trigger_arcive.py
