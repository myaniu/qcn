<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$longitude = 172.542; 
$latitude = -43.466; 
$depth = 11.4; 
$n_stations = 7; 
$etime = 1316493016.571349; 
$dtime = 1316493028; 
$dt_detect  = 11.4; 
$edir       = 1316493016; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
