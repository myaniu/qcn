<?php
chdir("/var/www/boinc/sensor/html/user/");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.8; 
$elon = 172.816; 
$elat = -43.599; 
$edep = 3.2; 
$n_stations = 13; 
$etime = 1308249447.516028; 
$dtime = 1308249459; 
$dt_detect  = 11.5; 
$edir       = 1308249447; 

earthquake_email($mag,$elon,$elat,$edep,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
