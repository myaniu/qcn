<?php
if (file_exists("../../inc/common.inc"))
   require_once("../../inc/common.inc");
elseif (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("../../../inc/common.inc"))
   require_once("../../../inc/common.inc");

require_once(BASEPATH . '/qcn/inc/utils.inc');
require_once(BASEPATH . '/qcn/inc/qcn_auto_detect.inc');
$show_mg = $_GET["show_mag"];
page_top();

if ( ($show_mg == "y")||($show_mg=="Y") ) { event_info("y"); } else { event_info();}

if ( ($show_mg=="y")||($show_mg=="Y") ) { show_maps("y"); } else { show_maps();}

echo "<hr> 
<h2>Triggers:</h2>\n";

trigger_info();

show_scatter_tvt(); // Show Estimated v. Observed Travel time  scatter plot
show_scatter_avd(); // Show Amplitude v. Distance scatter plot
show_triggers_db(); // Show the trigger database
show_disclaimer();
show_viewed_on();
show_seismograms();
page_end();
?>
