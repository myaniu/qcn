<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$longitude = 172.799; 
$latitude = -43.523; 
$depth = 4.4; 
$n_stations = 8; 
$etime = 1325705337.403757; 
$dtime = 1325705350; 
$dt_detect  = 12.6; 
$edir       = 1325705337; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
