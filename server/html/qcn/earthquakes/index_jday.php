<?php
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");


require_once(BASEPATH . "/qcn/inc/inc/utils.inc");

page_top();

require_once(BASEPATH . "/qcn/earthquakes/inc/gmt_quakes.inc");

$path = "./";

$dir_handle = @opendir($path) or die("Unable to open folder");

echo "<p><h1>Earthquakes:</h1></p>";
echo "<p><img src=\"" . BASEURL . "/earthquakes/images/events.jpg\" width=\"650\">";
echo "<p align=\"justify\">The Quake-Catcher Network now locates earthquakes based on the data collected through volunteer distrbuted computing. Above is a map of the events recently detected by QCN.  Below is a list of earthquakes with links to each page.</p>";

echo "<p><table border=1 width=\"100%\"><tr><td>";
echo "<table paddig=2 width=\"99%\">";
echo "<tr><td><b>Event ID:</b></td><td><b>Date</b></td><td><b>Time (UTC)</b></td><td><b>Longitude</b></td><td><b>Latitude</b></td><td><b>Depth</b></td><td><b>Magnitude</b></td><td><b>Triggers</b></td><td><b>Detected</b></tr></tr>";

$a = 0;

$files = array();
while (false !== ($file1 = readdir($dir_handle))) {
$files[]=$file1;
}

sort($files);
$files = array_reverse($files);
foreach ($files as $file1) {
$a++;
if(is_dir($file1)){
if (($file1!=".")&&($file1!="..")&&($file1!="JUNK")&&($file1!="SAFE")&&($file1!="inc")){

if ($a>1) {
 $bg_color="#ffffff";
 $a = 0;
} else {
 $bg_color="#DDDDDD";
}

$efile = "$file1/event.xy";
if (file_exists($efile)) {
echo "<tr bgcolor=\"$bg_color\"><td><a href=\"" . BASEPATH . "/earthquakes/$file1\">$file1</a></td>";
$etime = (int) $file1;
$contents = file($efile);
$string = implode($contents);
list($qlon, $qlat,$qdep,$qmag,$ntrig,$etime,$tdet,$qstd) = split('[,]',$string);


echo "</td><td>";
echo date('Y z',$etime);
echo "</td><td>";
echo date('H i s',$etime);

echo "</td><td>";
echo $qlon;
echo "</td><td>";
echo $qlat;
echo "</td><td>";
echo number_format($qdep,2,'.','');
echo "</td><td>";
echo number_format($qmag,2,'.','')." &plusmn ".number_format($qstd,2,'.','');
echo "</td><td>";
echo $ntrig;
echo "</td><td>";
echo date('H:i:s',$tdet);
echo "</td></tr>";
}
}
}

}
echo "</table>";
echo "</td></tr></table>";
closedir($dir_handle);


echo "Page created on \n". date('M d Y'). " at "; echo date('h:i:s'); echo " (UTC)";


page_end();

?>
