<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.2; 
$longitude = -117.719; 
$latitude = 33.909; 
$depth = 4.4; 
$n_stations = 8; 
$etime = 1344443601.986206; 
$dtime = 1344443615; 
$dt_detect  = 13.0; 
$edir       = 1344443601; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
