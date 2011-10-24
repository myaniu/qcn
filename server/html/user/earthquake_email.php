<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$longitude = -122.045; 
$latitude = 37.823; 
$depth = 26.8; 
$n_stations = 7; 
$etime = 1319166962.516385; 
$dtime = 1319166973; 
$dt_detect  = 10.5; 
$edir       = 1319166962; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
