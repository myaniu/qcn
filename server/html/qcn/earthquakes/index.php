<?php
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");


require_once(BASEPATH . "/qcn/inc/utils.inc");
require_once(BASEPATH . "/qcn/inc/qcn_auto_detect.inc");
page_top();
$show_mg = $_GET["show_mag"];

gmt_quake_map();   // Generate the earthquake map.

echo "<table width=\"98%\"><tr>\n";
echo "<td width=\"68%\"><p><h1>Recent Earthquakes:</h1></p></td>";
echo "<td width=\"30%\"><p><h2>Latest Earthquake:</h2></p></td>";
echo "</tr><tr>\n";
echo "<td width=\"68%\"><p><img src=\"" . BASEURL . "/earthquakes/images/events.jpg\" width=\"430\"></td>\n";
echo "<td width=\"30%\"><p>\n";
if ( ($show_mg=="y")||($show_mg=="Y") ) {show_last_eq("y");} else { show_last_eq();; }
echo "</td>\n</tr></table>\n";

echo "<p align=\"justify\">The Quake-Catcher Network now locates earthquakes based on the data collected through volunteer distrbuted computing. Above is a map of the events recently detected by QCN.  Below is a list of earthquakes with links to each page.</p>";



if ( ($show_mg== "y")||($show_mg=="Y") ) {list_quakes("y"); } else { list_quakes(); } 

show_viewed_on();  // Show the date page viewed on

page_end();

?>
