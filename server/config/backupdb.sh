#!/bin/bash
/bin/rm -f /tmp/qcn-backup.sql
##/bin/nice -n19 /usr/local/mysql/bin/mysqldump -u root -pPWD --all-databases > /data/cees2/QCN/qcn-backup.sql
/bin/nice -n19 /usr/local/mysql/bin/mysqldump -u root -pPWD --databases qcnalpha continual mysql sensor_download continual_download qcn_ecommerce todolist information_schema drupal > /tmp/qcn-backup.sql
#/bin/nice -n19 /bin/gzip /tmp/qcn-backup.sql
# note for the following line need to have .ssh id between root & carlgt1 setup
# also make sure that '.ssh/known_hosts' is setup to receive connections
/bin/nice -n19 /usr/bin/scp -C /tmp/qcn-backup.sql carlgt1@upl-private:
/bin/nice -n19 /bin/mv -f /tmp/qcn-backup.sql /data/cees2/QCN/
