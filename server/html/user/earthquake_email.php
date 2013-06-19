<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.2; 
$longitude = -119.866; 
$latitude = 36.455; 
$depth = 37.2; 
$n_stations = 7; 
$etime = 1370918371.276176; 
$dtime = 1370918456; 
$dt_detect  = 84.7; 
$edir       = 1370918371; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
