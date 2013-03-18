<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 2.9; 
$longitude = -116.846; 
$latitude = 32.782; 
$depth = 84.4; 
$n_stations = 7; 
$etime = 1362439044.139669; 
$dtime = 1362439105; 
$dt_detect  = 60.9; 
$edir       = 1362439044; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
