#!/bin/bash
/usr/local/mysql/bin/mysql -h db-private -u root -pPWD -e 'source /home/boinc/projects/qcn/server/sql/triggers_last_24hrs.sql;' > /var/www/boinc/sensor/trig24.txt
