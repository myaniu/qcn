<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$longitude = TEST; 
$latitude = -43.556; 
$depth = 2.2; 
$n_stations = 8; 
$etime = 1315696751.700613; 
$dtime = 1315696761; 
$dt_detect  = IGNORE; 
$edir       = 1315696751; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
