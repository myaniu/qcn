<?php
if (file_exists("../../earthquakes/inc/common.inc"))
   require_once("../../arthqaukes/inc/common.inc");
elseif (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");


function getData(){

 $data = array();
 $file = BASEPATH . "/qcn/rt_image/rt_triggers.xyz";
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
