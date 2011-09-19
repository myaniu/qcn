<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.1; 
$longitude = 172.758; 
$latitude = -43.619; 
$depth = 3.8; 
$n_stations = 9; 
$etime = 1316397094.013279; 
$dtime = 1316397105; 
$dt_detect  = 11.0; 
$edir       = 1316397094; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
