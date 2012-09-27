<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.5; 
$longitude = 172.835; 
$latitude = -43.381; 
$depth = 11.8; 
$n_stations = 8; 
$etime = 1348423875.622742; 
$dtime = 1348423888; 
$dt_detect  = 12.4; 
$edir       = 1348423875; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
