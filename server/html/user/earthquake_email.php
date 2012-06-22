<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$longitude = 172.692; 
$latitude = -43.527; 
$depth = 2.5; 
$n_stations = 9; 
$etime = 1339425276.844712; 
$dtime = 1339425285; 
$dt_detect  = 8.2; 
$edir       = 1339425276; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
