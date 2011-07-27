<?php
chdir("/var/www/boinc/sensor/html/user/");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$longitude = 172.241; 
$latitude = -43.593; 
$depth = 3.8; 
$n_stations = 15; 
$etime = 1311269972.711694; 
$dtime = 1311269993; 
$dt_detect  = 20.3; 
$edir       = 1311269972; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
