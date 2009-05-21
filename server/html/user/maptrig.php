<?php

require_once('../inc/boinc_db.inc');
require_once('../inc/util.inc');
require_once('../inc/translation.inc');
require_once('../inc/phoogle.inc');
require_once('../project/project_specific_prefs.inc');

$cx = get_int("cx", true);
$cy = get_int("cy", true);
$zoom = get_int("zoom", true);
$mapwidth = get_int("width", true);
$mapheight = get_int("height", true);
$timeint = $_GET["timeint"];  // H / D / M / W - blank or H is 4-hourly, D = day, M=month, W = week

//see if hostid gets a value
$hostid = get_int("hostid", true);  // note the (int) so it's a safe sql statement i.e. no sql injection string
if ($hostid>0) {
  db_init();
  $sqlhost = "select latitude, longitude, round(latitude,2) lat, round(longitude,2) lng, ifnull(q.hostid, 0) from qcn_trigger t left outer join qcn_showhostlocation q ON t.hostid=q.hostid 
  where t.hostid=$hostid order by time_trigger desc limit 1";
  $result = mysql_query($sqlhost);
  if ($result && ($res = mysql_fetch_array($result))) {
      if ($res[4]) {
      $cx = $res[0];
      $cy = $res[1];
      }
      else { // round
      $cx = $res[2];
      $cy = $res[3];
      }
      mysql_free_result($result); 
  }  
  else { // no host info, set zoom level to default
      $zoom = 0;
  }
}

$title = "";

    $mapimg = MAP_TRIGGER;
    $cachedatafile = CACHE_PATH_MAPTRIG;
    $legendbase = "Legend: <IMG SRC=\"img/qcn_32_laptop.png\"> = QCN participant laptop, <IMG SRC=\"img/qcn_32_usb.png\"> = QCN participant USB sensor, <IMG SRC=\"img/qcn_32_quake.png\"> = USGS-reported Earthquake of minimum magnitude ";
    switch($timeint) {
       case "D":
          $mapimg = MAP_TRIGGER_D;
          $cachedatafile = CACHE_PATH_MAPTRIG_D;
          $title = "Trigger Map for the Last Day";
          $legend = $legendbase . MIN_MAGNITUDE_D;
          break;
       case "W":
          $mapimg = MAP_TRIGGER_W;
          $cachedatafile = CACHE_PATH_MAPTRIG_W;
          $title = "Trigger Map for the Last Week";
          $legend = $legendbase . MIN_MAGNITUDE_W;
          break;
       case "M":
          $mapimg = MAP_TRIGGER_M;
          $cachedatafile = CACHE_PATH_MAPTRIG_M;
          $title = "Trigger Map for the Last Month";
          $legend = $legendbase . MIN_MAGNITUDE_M;
          break;
       default:
          $mapimg = MAP_TRIGGER;
          $cachedatafile = CACHE_PATH_MAPTRIG;
          $title = "Trigger Map for the Last 4 Hours";
          $legend = $legendbase . MIN_MAGNITUDE;
          break;
    }

    $title .= " (Generated on " . date("F d Y H:i:s", filectime($cachedatafile)) . " UTC)";

page_head(tra($title), null, $title, "", true, null, true);

echo "<h3>" . $title . "</h3>";
echo "<h5>" . $legend . "</h5>";
echo "<h7>Note: locations changed at the kilometer-level to protect privacy, unless participant authorized exact location be used</h7><BR>";
echo "<I>click and drag to move map; on empty region - left dbl-click to zoom in, right dbl-click to zoom out</I><BR>";

$pm = new PhoogleMap();

if ($zoom) {
   $pm->zoomLevel = $zoom;
}
else {
   $pm->zoomLevel = 5;
}

if ($mapwidth) {
   $pm->setWidth($mapwidth);
}
else {
   $pm->setWidth(1);
}

if ($mapheight) {
   $pm->setHeight($mapheight);
}
else {
   $pm->setHeight(1);
}

if ($cx && $cy) {
   $pm->centerMap($cx, $cy);
}
else {  // default to northern California
   $pm->centerMap(38, -115);
}

$data = array();

$cacheddata=get_cached_data(MAPTRIG_TTL, "maptrig", $cachedatafile);  // regenerate every 15 minutes
if ($cacheddata){
        $data = unserialize($cacheddata); // use the cached data
} 
/*else {
    echo "Generating a new map, please wait...<BR>";
    qcn_generate_world_map($data);
}*/

$i = 0;
for ($i = 0; $i < sizeof($data); $i++) {
  if ($data[$i]) {
     $pm->addGeoPoint($data[$i]->lat, $data[$i]->lng, $data[$i]->descript, $data[$i]->is_quake, $data[$i]->is_usb);
  }
}

$pm->showMap();

echo "<center><h5><a href=maptrig.php?timeint=H>Hour</a> - <a href=maptrig.php?timeint=D>Day</a> - <a href=maptrig.php?timeint=W>Week</a> - <a href=maptrig.php?timeint=M>Month</a>";

page_tail();

?>
