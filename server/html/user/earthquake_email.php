<?php
chdir("/var/www/boinc/sensor/html/user/");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.2; 
$longitude = 172.782; 
$latitude = -43.560; 
$depth = 2.2; 
$n_stations = 7; 
$etime = 1310771502.019589; 
$dtime = 1310771515; 
$dt_detect  = 13.0; 
$edir       = 1310771502; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
