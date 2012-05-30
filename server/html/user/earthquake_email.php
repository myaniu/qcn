<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.9; 
$longitude = 172.723; 
$latitude = -43.507; 
$depth = 6.8; 
$n_stations = 8; 
$etime = 1338138033.610069; 
$dtime = 1338138043; 
$dt_detect  = 9.4; 
$edir       = 1338138033; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
