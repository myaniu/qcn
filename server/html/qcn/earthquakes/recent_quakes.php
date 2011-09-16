<?php

require_once("inc/common.inc");
require_once(BASEPATH . "/qcn/inc/utils.inc");
require_once(BASEPATH . "/qcn/inc/qcn_auto_detect.inc");
page_top();
$show_mg = $_GET["show_mag"];

gmt_quake_map();   // Generate the earthquake map.

echo "<p><h1>Earthquakes:</h1></p>";
echo "<p><img src=\"" . BASEURL . "/earthquakes/images/events.jpg\" width=\"650\">";
echo "<p align=\"justify\">The Quake-Catcher Network now locates earthquakes based on the data collected through volunteer distrbuted computing. Above is a map of the events recently detected by QCN.  Below is a list of earthquakes with links to each page.</p>";



if ( ($show_mg== "y")||($show_mg=="Y") ) {list_quakes("y"); } else { list_quakes(); } 

show_viewed_on();  // Show the date page viewed on

page_end();

?>
