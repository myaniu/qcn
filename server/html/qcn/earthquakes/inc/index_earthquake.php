<?php
require_once('common.inc');
require_once(BASEPATH . '/qcn/inc/utils.inc');
require_once(BASEPATH . '/qcn/inc/qcn_auto_detect.inc');
$show_mg = $_GET["show_mag"];
page_top();

if ( ($show_mg == "y")||($show_mg=="Y") ) { event_info("y"); } else { event_info();}

if ( ($show_mg == "y")||($show_mg=="Y") ) { show_maps("y"); } else { show_maps();}

if ( ($show_mg== "y")||($show_mg=="Y") ) {list_quakes("y"); } else { list_quakes(); } 

echo " 
<p>The information contained on this page is not intended for official use.  This is a scientific project aiming to validate the methods used to produce these data.  For official earthquake characterization, please obtain the appropriate information from your national earthquake program or the <a href=\"http://earthquake.usgs.gov/earthquakes/\">USGS.</a>

";
echo "<p>Page viewed on: ". date('M d Y'). " at ". date('h:i:s'); echo " (UTC)";
page_end();
?>
