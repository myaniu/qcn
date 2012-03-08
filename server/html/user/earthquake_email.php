<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.7; 
$longitude = -122.219; 
$latitude = 37.884; 
$depth = 15.4; 
$n_stations = 9; 
$etime = 1330954399.874645; 
$dtime = 1330954409; 
$dt_detect  = 9.1; 
$edir       = 1330954399; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
