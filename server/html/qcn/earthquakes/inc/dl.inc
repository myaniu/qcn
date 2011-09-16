<?php
require_once("common.inc");
function dl_link($elon,$elat,$etime) {

 $lnmn = $elon - 2.;
 $lnmx = $elon + 2.;
 $ltmn = $elat - 2.;
 $ltmx = $elat + 2.;

 $ds = date('Y-m-d',$etime);   // Date of event
 $hr = date('H',$etime);       // Hour of event
 $mn = date('i',$etime);       // closest 10 minute of event
 $mn = (int) $mn/10; $mn = 10*$mn+10;
 if ($mn > 50) { $mn = 00; $hr=$hr+1; if ($hr>23) { $hr=00; }}

 $path = BASEURL . "/sensor/trdl.php?";
 $loc_box = "cbUseLat=1&LatMin=$ltmn&LatMax=$ltmx&LonMin=$lnmn&LonMax=$lnmx";
 $time_start = "&cbUseTime=1&date_start=$ds&time_hour_start=$hr&time_minute_start=$mn";
 $time_end   = "&date_end=$ds&time_hour_end=$hr&time_min_end=$mn";
 echo "<a href="\n";

}


?>
