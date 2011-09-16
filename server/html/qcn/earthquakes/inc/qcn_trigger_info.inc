<?php
require_once("common.inc");
function trigger_info(){
 $efile="stations.xyz";
 echo "<table align=\"center\">";

 echo "<tr align=\"center\"><td><strong>Host ID</strong></td>
           <td><strong>Trigger ID</strong></td>
           <td><strong>View</strong></td>
           <td><strong>Long</strong></td>
           <td><strong>Lat</strong></td>
           <td><strong>Trig Time</strong></td>
           <td><strong>Time Received</strong></td>
           <td><strong>Significance</strong></td>
           <td><strong>|accel| (m/s/s)</strong></td>
           <td><strong>Distance (km)</strong></td></tr>";

 $contents = file($efile);
 foreach ($contents as $line_num => $line) {

//  $string = implode($line);
  list($slon, $slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis) = split('[,]',$line);


  if (preg_match("/conti/i","substr($sfile,0,4)")) {
   $url = UPLOADURL . "/trigger/continual/";
   $urlhid = BASEURL . "/continual/show_host_detail.php?hostid=".$hid;
  } else {
   $url = UPLOADURL . "/trigger/";
   $urlhid = BASEURL . "/trigger/show_host_detail.php?hostid=".$hid;
  }
  $urlfile = $url.$sfile;
  
  echo "<tr align=\"center\"><td><a href=\"$urlhid\">$hid</a></td><td><a href=\"$urlfile\">$tid</a></td>\n";
  
  echo "<td><a href=\"javascript:void(0)\"onclick=\"window.open(BASEURL . '/earthquakes/view/view_data.php?dat=$sfile','linkname','height=500,width=400,scrollbars=no')\">View</a></td>\n";

  echo "<td>".number_format($slon,2,'.','')."</td>
        <td>".number_format($slat,2,'.','')."</td>
        <td>".number_format($ttime,1,'.','')."</td>
        <td>$rtime</td>
        <td>".number_format($sig,1,'.','')."</td>
        <td>".number_format($smag,1,'.','')."</td>
        <td>".number_format($dis,1,'.','')."</td></tr>\n";

 }

 echo "</table>\n";

}





?>
