<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.1; 
$longitude = 172.728; 
$latitude = -43.609; 
$depth = 2.8; 
$n_stations = 8; 
$etime = 1331985325.941137; 
$dtime = 1331985351; 
$dt_detect  = 25.1; 
$edir       = 1331985325; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
