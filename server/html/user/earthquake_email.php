<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.5; 
$longitude = 172.455; 
$latitude = -43.587; 
$depth = 5.9; 
$n_stations = 9; 
$etime = 1358583310.045698; 
$dtime = 1358583319; 
$dt_detect  = 9.0; 
$edir       = 1358583310; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
