<?php
include ("/var/www/qcn/jpgraph-2.3.4/src/jpgraph.php");
include ("/var/www/qcn/jpgraph-2.3.4/src/jpgraph_line.php");

require_once("/var/www/boinc/sensor/html/inc/db.inc");
require_once("/var/www/boinc/sensor/html/inc/util.inc");

db_init();

$query = "select yearweek(from_unixtime(time_received)) yw, count(distinct hostid) ctr
from qcn_trigger
where yearweek(from_unixtime(time_received))
  between 
      yearweek(date(date_sub(date(date_sub(now(), interval 180 day)), 
        interval dayofweek(date(date_sub(now(), interval 180 day))) day) )  )
    and 
      yearweek(date(date_sub(now(), interval dayofweek(now()) day)))
group by yearweek(from_unixtime(time_received)) 
order by yearweek(from_unixtime(time_received))";

$filename = "/var/www/boinc/sensor/html/user/img/weekly.png";

$datax = array();
$datay = array();
$ictr = 0;

$result = mysql_query($query);
if (!$result) exit(1);

while ($res = mysql_fetch_array($result)) {
   $datax[$ictr] = substr($res[0],-2,2) . "-" . substr($res[0],2,2);
   $datay[$ictr] = $res[1];
   $ictr++;
}

mysql_free_result($result);

$graph = new Graph(800, 600);
$graph->img->SetMargin(40,40,80,80);
$graph->SetScale("textlin");
$graph->SetShadow();
$graph->title->Set("Number of Triggering Machines by Week of the Year");
$graph->title->SetFont(FF_FONT1,FS_BOLD);

$graph->xaxis->SetTickLabels($datax);
$graph->xaxis->SetLabelAngle(90);

$p1 = new LinePlot($datay);
$p1->SetFillColor("orange");
$p1->mark->SetType(MARK_FILLEDCIRCLE);
$p1->mark->SetFillColor("red");
$p1->mark->SetWidth(4);
$graph->Add($p1);

$graph->Stroke($filename);

?>
