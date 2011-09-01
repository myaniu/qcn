<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.6; 
$longitude = 172.868; 
$latitude = -43.609; 
$depth = 4.4; 
$n_stations = 7; 
$etime = 1314890990.984384; 
$dtime = 1314891000; 
$dt_detect  = 9.0; 
$edir       = 1314890990; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
