#!/usr/local/bin/php
<?php
require_once("../project/common.inc");

include (BASEPATH . "/qcn/jpgraph-2.3.4/src/jpgraph.php");
include (BASEPATH . "/qcn/jpgraph-2.3.4/src/jpgraph_line.php");

require_once(BASEPATH . "/boinc/sensor/html/inc/db.inc");
require_once(BASEPATH . "/boinc/sensor/html/inc/util.inc");

db_init();

$result = mysql_query("CREATE TEMPORARY TABLE tmptrigger (time_received double, hostid int(11))");
if (!$result) exit(1);

$result = mysql_query("INSERT INTO tmptrigger 
      select time_received, hostid from qcnalpha.qcn_trigger
     ");
if (!$result) exit(2);

$result = mysql_query("INSERT INTO tmptrigger 
      select time_received, hostid from qcnarchive.qcn_trigger
         where time_received>(unix_timestamp()-(3600*24*190)) 
     ");
if (!$result) exit(3);

$query_weekly = "select yearweek(from_unixtime(time_received)) yw, count(distinct hostid) ctr
from tmptrigger
where yearweek(from_unixtime(time_received))
  between 
      yearweek(date(date_sub(date(date_sub(now(), interval 180 day)), 
        interval dayofweek(date(date_sub(now(), interval 180 day)))-1 day) )  )
    and 
      yearweek(date(date_sub(now(), interval dayofweek(now()) day)))
group by yearweek(from_unixtime(time_received)) 
order by yearweek(from_unixtime(time_received))";

$query_daily = "select date(from_unixtime(time_received)) yw, count(distinct hostid) ctr
from tmptrigger
where date(from_unixtime(time_received))
  between 
      date(date_sub(date(date_sub(now(), interval 180 day)), 
        interval dayofweek(date(date_sub(now(), interval 180 day)))-1 day) ) 
    and 
      date(date_sub(now(), interval dayofweek(now()) day))
group by date(from_unixtime(time_received)) 
order by date(from_unixtime(time_received))";

$filename = BASEPATH . "/boinc/sensor/html/user/img/weekly.png";

$datax = array();
$datay = array();
$dataz = array();

$ictr = 0;

$result = mysql_query($query_daily);
if (!$result) exit(10);

while ($res = mysql_fetch_array($result)) {
   $dataz[$ictr] = $res[1];
   //$datax[$ictr] = substr($res[0],-2,2) . "-" . substr($res[0],2,2);
   $datax[$ictr] = substr($res[0],-5); // substr($res[0],-2,2) . "-" . substr($res[0],2,2);
   $ictr++;
}

//print_r($dataz);

mysql_free_result($result);

$result = mysql_query($query_weekly);
if (!$result) exit(11);

$total = $ictr;
$ictr = 0;
while ($ictr < $total && $res = mysql_fetch_array($result)) {
   for ($j = 0; $j < 7; $j++) {
     $datay[$ictr] = $res[1];
     $ictr++;
   }
}

//print_r($datay);

mysql_free_result($result);

$graph = new Graph(800, 600);
$graph->img->SetMargin(70,70,80,80);
$graph->SetScale("textlin");
$graph->SetShadow();
$graph->title->Set("Number of Reporting Machines Over the Past 6 Months");
$graph->title->SetFont(FF_FONT1,FS_BOLD);

$graph->yaxis->SetTitleMargin(40);
$graph->yaxis->SetTitleSide(SIDE_LEFT);
$graph->yaxis->SetTitle("# of Unique Machines");

$graph->xaxis->SetTitleMargin(40);
$graph->xaxis->SetTitleSide(SIDE_BOTTOM);
$graph->xaxis->SetTitle("Month-Day");

$graph->xaxis->SetTickLabels($datax);
$graph->xaxis->SetLabelAngle(90);
$graph->xaxis->SetTextLabelInterval(7);

$lp1 = new LinePlot($datay);
$lp1->SetFillColor("blue");
$lp1->SetLegend("By Week");

$lp2 = new LinePlot($dataz);
$lp2->SetFillColor("red");
$lp2->SetLegend("By Day");

/*
$lp1->mark->SetType(MARK_FILLEDCIRCLE);
$lp1->mark->SetFillColor("gray");
$lp1->mark->SetWidth(4);

$lp2->mark->SetType(MARK_FILLEDCIRCLE);
$lp2->mark->SetFillColor("white");
$lp2->mark->SetWidth(4);
*/

$graph->Add($lp1);
$graph->Add($lp2);
$graph->legend->SetAbsPos(20,20,'right','top');

$graph->Stroke($filename);

?>
