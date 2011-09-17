<?php

if (file_exists("../../earthquakes/inc/common.inc"))
   require_once("../../arthqaukes/inc/common.inc");
elseif (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");

require_once(BASEPATH . "/qcn/inc/utils.inc");

//page_top();

$event_file = "./rt_triggers.xyz";

touch($event_file);

system("cat */event.xy > $event_file");

$gmt  = GMTPATH;
$topo = GMTPATH . "/share/topo/topo_coarse.grd";
$cpt  = GMTPATH . "/share/topo/GMT_globe.cpt";
$proj = "-JM15";
$bounds = "-R-180/180/-70/70";
$psfile = BASEPATH . "/qcn//rt_image/images/events.ps";
$outdir = BASEPATH . "/qcn/rt_image/images";

system(GMTPATH . "/bin/grdimage $topo -C$cpt $proj $bounds -X4 -Y4 -P -K > $psfile");
system(GMTPATH . "/bin/psxy $trig_file $proj $bounds -O -Sa0.5 -W0.5p/255/0/0 -B60g0/30g0 >> $psfile");
system(GMTPATH . "/bin/ps2raster $psfile -D$outdir -A -P -Tj");

//page_end();

?>
