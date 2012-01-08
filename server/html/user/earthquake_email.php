<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.2; 
$longitude = 172.708; 
$latitude = -43.441; 
$depth = 0.0; 
$n_stations = 8; 
$etime = 1325898120.443065; 
$dtime = 1325898137; 
$dt_detect  = 16.6; 
$edir       = 1325898120; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
