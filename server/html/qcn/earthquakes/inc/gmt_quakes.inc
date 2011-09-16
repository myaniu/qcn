<?php
require_once("common.inc");
require_once(BASEPATH . "/qcn/inc/utils.inc");

function gmt_quakes_map() {
  $event_file = "./events.xy";

  touch($event_file);

  system("cat */event.xy > $event_file");

  $gmt  = GMTPATH;
  $topo = GMTPATH . "/share/topo/topo_coarse.grd";
  $cpt  = GMTPATH . "/share/topo/GMT_globe.cpt";
  $proj = "-JM15";
  $bounds = "-R-180/180/-70/70";
  $psfile = BASEPATH . "/qcn/earthquakes/images/events.ps";
  $outdir = BASEPATH . "/qcn/earthquakes/images";

  system(GMTPATH . "/bin/grdimage $topo -C$cpt $proj $bounds -X4 -Y4 -P -K > $psfile");
  system(GMTPATH . "/bin/pscoast -Dc $proj $bounds -W0.5p/0 -O -K >> $psfile");
  system(GMTPATH . "/bin/psxy $event_file $proj $bounds -O -Sa0.5 -W0.5p/255/0/0 -B60g0/30g0WSne >> $psfile");
  system(GMTPATH . "/bin/ps2raster $psfile -D$outdir -A -P -Tj");
}


function gmt_quakes_map_lon_lat($lon=null,$lat=null,$dlon=null,$dlat=null) {
  $event_file = "./events.xy";

  touch($event_file);

  system("cat */event.xy > $event_file");

  $gmt  = GMTPATH . "/";
  $topo = GMTPATH . "/share/topo/topo_coarse.grd";
  $cpt  = GMTPATH . "/share/topo/GMT_globe.cpt";
  $proj = "-JM15";
  if ($lon==null) {
     $bounds = "-R-180/180/-90/90";
  } else {
     if ($dlon==null) {$dlon=5;};
     if ($dlat==null) {$dlat=4;};
     $ln_mn = ((int) ($lon-$dlon/2.*10.) ) / 10.;
     $ln_mx = ((int) ($lon+$dlon/2.*10.) ) / 10.;
     $lt_mn = ((int) ($lat-$dlat/2.*10.) ) / 10.;
     $lt_mx = ((int) ($lat+$dlat/2.*10.) ) / 10.;
     $bounds = "-R$ln_mn/$ln_mx/$lt_mn/$lt_mx";
  } 
  $psfile = BASEPATH . "/qcn/earthquakes/images/events.ps";
  $outdir = BASEPATH . "/qcn/earthquakes/images";

  system(GMTPATH . "/bin/grdimage $topo -C$cpt $proj $bounds -X4 -Y4 -P -K > $psfile");
  system(GMTPATH . "/bin/pscoast -Dc $proj $bounds -W0.5p/0 -O -K >> $psfile");
  system(GMTPATH . "/bin/psxy $event_file $proj $bounds -O -Sa0.5 -W0.5p/255/0/0 -B60g0/30g0WSne >> $psfile");
  system(GMTPATH . "/bin/ps2raster $psfile -D$outdir -A -P -Tj");
}

?>
