<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$longitude = 172.542; 
$latitude = -43.561; 
$depth = 4.0; 
$n_stations = 7; 
$etime = 1316410862.574000; 
$dtime = 1316410871; 
$dt_detect  = 8.4; 
$edir       = 1316410862; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
