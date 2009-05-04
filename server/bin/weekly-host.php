<?php
include ("/var/www/qcn/jpgraph-2.3.4/jpgraph.php");
include ("/var/www/qcn/jpgraph-2.3.4/jpgraph_line.php");

$filename = "/var/www/boinc/sensor/html/user/img/weekly.png";

$datay = array(1.23,1.9,1.6,3.1,3.4,2.8,2.1,1.9);
$graph = new Graph(300,200,'auto');
$graph->img->SetMargin(40,40,40,40);
$graph->SetScale("textlin");
$graph->SetShadow();
$graph->title->Set("Number of Active Machines by Year/Week");
$graph->title->SetFont(FF_FONT1,FS_BOLD);

$p1 = new LinePlot($datay);
$p1->SetFillColor("orange");
$p1->mark->SetType(MARK_FILLEDCIRCLE);
$p1->mark->SetFillColor("red");
$p1->mark->SetWidth(4);
$graph->Add($p1);

//$graph->Stroke();

$graph->$img->Stream($filename);

?>
