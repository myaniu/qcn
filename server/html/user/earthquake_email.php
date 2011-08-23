<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.9; 
$longitude = 172.646; 
$latitude = -43.561; 
$depth = 13.1; 
$n_stations = 9; 
$etime = 1313825471.936475; 
$dtime = 1313825483; 
$dt_detect  = 11.1; 
$edir       = 1313825471; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
