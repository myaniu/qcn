<?php
chdir("/var/www/boinc/sensor/html/user/");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.9; 
$longitude = 172.655; 
$latitude = -43.628; 
$depth = 4.1; 
$n_stations = 7; 
$etime = 1310530954.384823; 
$dtime = 1310530963; 
$dt_detect  = 8.6; 
$edir       = 1310530954; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
