<?php

require_once('../inc/boinc_db.inc');
require_once('../inc/util.inc');
require_once('../inc/translation.inc');
require_once('../project/project_specific_prefs.inc');

db_init();

$title = "Triggers for the Past Day, Week, and Month";
page_head(tra($title), null, $title, "", true, null, true);

echo "<h2>" . $title . "</h2><BR>";

echo "<table>";

printMap("D");
printMap("W");
printMap("M");

echo "</table>";

page_tail();

function printMap($timeint)
{
  $minmag = MIN_MAGNITUDE;
  switch($timeint)
  {
     case "D":
        $interval = "Day";
        $mapfile = MAP_TRIGGER_D;
        $minmag = MIN_MAGNITUDE_D;
        break;
     case "W":
        $interval = "Week";
        $mapfile = MAP_TRIGGER_W;
        $minmag = MIN_MAGNITUDE_W;
        break;
     case "M":
        $interval = "Month";
        $mapfile = MAP_TRIGGER_M;
        $minmag = MIN_MAGNITUDE_M;
        break;
  }

echo "<tr><td>Latest Triggers Recorded in the Past $interval - Generated on " . date("F d Y H:i:s", filectime($mapfile)) . " UTC</td></tr>";

echo "
    <tr><p><td><A HREF=maptrig.php?timeint=$timeint>Click here or on a region of the map for an interactive Google map</A></td></tr>
    <tr><p><td>Legend: Blue triangle = QCN participant trigger, Red circle = Earthquake of minimum magnitude " . $minmag . "</td></tr>
    <tr><p><td><i>Note: locations changed at the kilometer level to protect privacy</i></td</tr>
    ";

echo "
<tr><td><IMG SRC=\"" . $mapfile . "\" usemap=\"#" . $mapfile . "\" border=\"0\"></td></tr>
<map name=\"" . $mapfile . "\">
        <area shape=\"rect\" coords=\"0,2,225,232\" href=\"maptrig.php?cx=38&cy=-120&timeint=$timeint\">
        <area shape=\"rect\" coords=\"2,236,228,511\" href=\"maptrig.php?cx=-20&cy=-120&timeint=$timeint\">
        <area shape=\"rect\" coords=\"227,3,428,234\" href=\"maptrig.php?cx=38&cy=-70&timeint=$timeint\">
        <area shape=\"rect\" coords=\"231,238,442,510\" href=\"maptrig.php?cx=-20&cy=-70&timeint=$timeint\">
        <area shape=\"rect\" coords=\"430,3,605,237\" href=\"maptrig.php?cx=50&cy=1&timeint=$timeint\">
        <area shape=\"rect\" coords=\"445,241,732,510\" href=\"maptrig.php?cx=-10&cy=5&timeint=$timeint\">
        <area shape=\"rect\" coords=\"609,3,803,239\" href=\"maptrig.php?cx=38&cy=80&timeint=$timeint\">
        <area shape=\"rect\" coords=\"735,244,1022,511\" href=\"maptrig.php?cx=-20&cy=140&timeint=$timeint\">
        <area shape=\"rect\" coords=\"806,3,1021,243\" href=\"maptrig.php?cx=38&cy=140&timeint=$timeint\">
</map>
    <tr><td><HR></td></tr>
   <tr><td><BR></td></tr>
  ";

}
?>
