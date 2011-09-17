<?php
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");


require_once(BASEPATH . "/qcn/inc/inc/utils.inc");
require_once(BASEPATH . "/qcn/inc/qcn_auto_detect.inc");
page_top();
$show_mg = $_GET["show_mag"];
$ln_mn  = $_REQUEST["ln_mn"];
$ln_mx  = $_REQUEST["ln_mx"];
$lt_mn  = $_REQUEST["lt_mn"];
$lt_mx  = $_REQUEST["lt_mx"];
$topo   = $_REQUEST["topo"];

if ($ln_mn==null) {
 $ln_mn = -180;
 $ln_mx =  180;
 $lt_mn =  -90;
 $lt_mx =   90;
}

$dlon  = $ln_mx-$ln_mn;
$dlat  = $lt_mx-$lt_mn;

if ($topo!="y") {$topo=null;}

//require_once(BASEPATH . "/qcn/earthquakes/inc/gmt_quakes.inc");
gmt_quake_map_lon_lat2($ln_mn,$ln_mx,$lt_mn,$lt_mx,$topo);   // Generate the earthquake map.





echo "<table width=\"98%\"><tr>\n";
echo "<td width=\"68%\"><p><h1>Recent Earthquakes:</h1></p></td>";
echo "<td width=\"30%\"><p><h2>Select Earthquake Range:</h2></p></td>";
echo "</tr><tr>\n";
echo "<td width=\"68%\"><p><img src=\"" . BASEPATH . "/earthquakes/images/events_map.jpg\" width=\"430\"></td>\n";
echo "<td width=\"30%\"><p>\n";
echo "<form name=\"sensor_request\" method=\"post\" action=\"map_quakes.php\">\n";
echo "                  <p>West Lon:<input name=\"ln_mn\"   type=\"text\" id=\"ln_mn\" value=\"$ln_mn\" size=\"10\">\n";
echo "                  <p>East Lon: <input name=\"ln_mx\"  type=\"text\" id=\"ln_mx\" value=\"$ln_mx\" size=\"10\">\n";
echo "                  <p>South Lat:<input name=\"lt_mn\"  type=\"text\" id=\"lt_mn\" value=\"$lt_mn\" size=\"10\">\n";
echo "                  <p>North Lat: <input name=\"lt_mx\" type=\"text\" id=\"lt_mx\" value=\"$lt_mx\" size=\"10\">\n";
echo "                  <p>Topo (slow):<input type=\"radio\" name=\"topo\" value=\"y\">\n";
echo "                  <p>     (fast):<input type=\"radio\" name=\"topo\" value=\"n\" checked>\n";
echo "                  <p><input type=\"submit\" name=\"Submit\" value=\"Send\">\n";
echo "</form>\n";
echo "</td>\n</tr><p></table>\n";

echo "<p align=\"justify\">The Quake-Catcher Network now locates earthquakes based on the data collected through volunteer distrbuted computing. Above is a map of the events recently detected by QCN.  Below is a list of earthquakes with links to each page.</p>";



show_viewed_on();  // Show the date page viewed on

page_end();

function write_onclick(){
echo "
<script type=\"text/javascript\">
   
function update_form(FormName, FieldName, FormValue)
{
	if(!document.forms[FormName])
		return;
	document.forms[FormName].elements[FieldName] = FormValue;
	if(!objCheckBoxes)
}
</script>\n";
}


?>
