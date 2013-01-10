<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.3; 
$longitude = 172.503; 
$latitude = -43.595; 
$depth = 9.4; 
$n_stations = 7; 
$etime = 1355979285.391479; 
$dtime = 1355979300; 
$dt_detect  = 14.6; 
$edir       = 1355979285; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
