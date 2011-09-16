<?php


function getData(){

 $data = array();
 $file = "/var/www/qcn/rt_image/rt_triggers.xyz";
 $fp1 = fopen($file,'r'); 
 $content = file($file);
 foreach($content as $value) {
  list($lon,$lat,$mag,$hid) = split('[,]',$value);
  echo "<p>$lon $lat $mag $hid<br>";
 }
 
  return 0;
  settimeout("getData()",500);
}
?>
