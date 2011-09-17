<?php
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");

require_once(BASEPATH . '/qcn/inc/utils.inc');
require_once(BASEPATH . '/qcn/inc/qcn_auto_detect.inc');

$show_mg = $_GET["show_mag"];

$event = $_GET["event"];
$sub   = $_GET["sub"];

if ($event ) {
$dir = BASEPATH . "/qcn/earthquakes/" . $event;

chdir($dir);

get_waveformss($event,$sub);

} else {
 page_top();

 echo "Event not found:".$event;

 page_end();
}
?>
