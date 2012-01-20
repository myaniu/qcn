<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.2; 
$longitude = 172.858; 
$latitude = -43.549; 
$depth = 10.2; 
$n_stations = 9; 
$etime = 1326548872.220695; 
$dtime = 1326548884; 
$dt_detect  = 11.8; 
$edir       = 1326548872; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
