#!/bin/bash
echo Started on `date`
cd /var/www/boinc/qcnalpha/html/user
/usr/local/bin/php ../ops/generate_maptrig.php
echo Finished on `date`
