<?php
include ("/var/www/qcn/jpgraph-2.3.4/src/jpgraph.php");
include ("/var/www/qcn/jpgraph-2.3.4/src/jpgraph_line.php");
require_once("/var/www/boinc/sensor/html/inc/db.inc");
require_once("/var/www/boinc/sensor/html/inc/util.inc");

db_init();

$query = "select count(distinct hostid) ctr, yearweek(from_unixtime(time_received)) yw
from qcn_trigger
where yearweek(from_unixtime(time_received))
  between 200826 and yearweek(now())-1
group by yearweek(from_unixtime(time_received))
order by yearweek(from_unixtime(time_received)) desc
limit 26";

$filename = "/var/www/boinc/sensor/html/user/img/weekly.png";

$datax = array();
$datay = array();
$ictr = 0;

$result = mysql_query($query);
if (!$result) return 0;

while ($res = mysql_fetch_array($result)) {
   $datax[$ictr] = $res->yw;
   $datay[$ictr] = $res->ctr;
   $ictr++;
}

mysql_free_result($result);

$graph = new Graph(1024,768,'auto');
$graph->img->SetMargin(20,20,20,20);
$graph->SetScale("textlin");
$graph->SetShadow();
$graph->title->Set("Number of Active Machines by Year/Week");
$graph->title->SetFont(FF_FONT1,FS_BOLD);

$p1 = new LinePlot($datay,$datax);
$p1->SetFillColor("orange");
$p1->mark->SetType(MARK_FILLEDCIRCLE);
$p1->mark->SetFillColor("red");
$p1->mark->SetWidth(4);
$graph->Add($p1);

$graph->Stroke($filename);

?>
