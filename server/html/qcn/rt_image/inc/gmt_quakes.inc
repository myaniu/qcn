<?php

require_once("/var/www/qcn/inc/utils.inc");

//page_top();

$event_file = "./rt_triggers.xyz";

touch($event_file);

system("cat */event.xy > $event_file");

$gmt  = "/usr/local/gmt/";
$topo = "/usr/local/gmt/share/topo/topo_coarse.grd";
$cpt  = "/usr/local/gmt/share/topo/GMT_globe.cpt";
$proj = "-JM15";
$bounds = "-R-180/180/-70/70";
$psfile = "/var/www/qcn//rt_image/images/events.ps";
$outdir = "/var/www/qcn/rt_image/images";

system("/usr/local/gmt/bin/grdimage $topo -C$cpt $proj $bounds -X4 -Y4 -P -K > $psfile");
//system("/usr/local/gmt/bin/pscoast -Dc $proj $bounds -W0.5p/0 -O -K >> $psfile");
system("/usr/local/gmt/bin/psxy $trig_file $proj $bounds -O -Sa0.5 -W0.5p/255/0/0 -B60g0/30g0 >> $psfile");
system("/usr/local/gmt/bin/ps2raster $psfile -D$outdir -A -P -Tj");

//page_end();

?>
