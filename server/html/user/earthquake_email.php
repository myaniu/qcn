<?php
chdir("/var/www/boinc/sensor/html/user/");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 4.0; 
$elon = 172.596; 
$elat = -43.622; 
$edep = 1.8; 
$n_stations = 7; 
$etime = 1308671536.384101; 
$dtime = 1308671601; 
$dt_detect  = 64.6; 
$edir       = 1308671536; 

earthquake_email($mag,$elon,$elat,$edep,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
