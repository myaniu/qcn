#!/bin/bash
/usr/local/mysql/bin/mysqldump -h db-private -u root -pPWD -d --databases qcnalpha continual contarchive qcnarchive qcnwp sensor_download continual_download > qcn_all_schema.sql
