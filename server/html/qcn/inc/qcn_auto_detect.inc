<?php
// env vars ie global defines should be set already?
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");

require_once(BASEPATH . "/qcn/inc/inc/utils.inc");

function gmt_quake_map() {
  $event_file = "./events.xy";

  touch($event_file);

  system("cat */event.xy > $event_file");

  $gmt  = GMTPATH . "/";
  $topo = GMTPATH . "/share/topo/topo_coarse.grd";
  $cpt  = GMTPATH . "/share/topo/GMT_globe.cpt";
  $proj = "-JM15";
  $bounds = "-R-180/180/-70/70";
  $psfile = BASEPATH . "/qcn/earthquakes/images/events.ps";
  $outdir = BASEPATH . "/qcn/earthquakes/images";

  system(GMTPATH . "/bin/grdimage $topo -C$cpt $proj $bounds -X4 -Y4 -P -K > $psfile");
  system(GMTPATH . "/bin/pscoast -Dc $proj $bounds -W0.5p/0 -S100/150/255 -O -K >> $psfile");
  system(GMTPATH . "/bin/psxy $event_file $proj $bounds -O -Sa0.5 -W0.5p/255/0/0 -B60g0/30g0WSne >> $psfile");
  system(GMTPATH . "/bin/ps2raster $psfile -D$outdir -A -P -Tj");
}

function gmt_quake_map_lon_lat($lon=null,$lat=null,$dlon=null,$dlat=null) {
  $event_file = "./events.xy";

  touch($event_file);

  system("cat */event.xy > $event_file");

  $gmt  = GMTPATH . "/";
  $topo = GMTPATH . "/share/topo/topo30.grd";
  $cpt  = GMTPATH . "/share/topo/GMT_globe.cpt";
  $proj = "-JM15";
  if ($lon==null) {
     $bounds = "-R-180/180/-90/90";
     echo "NO BOUNDS\n";
  } else {
     if ($dlon==null) {$dlon=5;};
     if ($dlat==null) {$dlat=4;};
     $ln_mn = ((int) (($lon-$dlon/2.)*10.) ) / 10.;
     $ln_mx = ((int) (($lon+$dlon/2.)*10.) ) / 10.;
     $lt_mn = ((int) (($lat-$dlat/2.)*10.) ) / 10.;
     $lt_mx = ((int) (($lat+$dlat/2.)*10.) ) / 10.;
     $bounds = "-R$ln_mn/$ln_mx/$lt_mn/$lt_mx";
     echo $bounds;
  } 
  $psfile = BASEPATH . "/qcn/earthquakes/images/events.ps";
  $outdir = BASEPATH . "/qcn/earthquakes/images";

  system(GMTPATH . "/bin/grdimage $topo -C$cpt $proj $bounds -X4 -Y4 -P -K > $psfile");
  system(GMTPATH . "/bin/pscoast -Df $proj $bounds -W0.5p/0 -O -K -S100/150/200 >> $psfile");
  system(GMTPATH . "/bin/psxy $event_file $proj $bounds -O -Sa0.25 -G255/0/0 -B0.5g0/0.5g0WSne >> $psfile");
  system(GMTPATH . "/bin/ps2raster $psfile -D$outdir -A -P -Tj");
}

function gmt_quake_map_lon_lat2($ln_mn=null,$ln_mx=null,$lt_mn=null,$lt_mx=null,$topo=null) {
  $event_file = "./events.xy";

  touch($event_file);

  system("cat */event.xy > $event_file");

  $gmt  = GMTPATH . "/";
  $cpt  = GMTPATH . "/share/topo/GMT_globe.cpt";
  $proj = "-JM15";
  if ($ln_mx==null) {
     $ln_mn = -180;
     $ln_mx =  180;
     $lt_mn =  -90;
     $lt_mx =   90;

  } 
  if ($lt_mx<$lt_mn) {
   $temp = $lt_mx;
   $lt_mx = $lt_mn;
   $lt_mn = $temp;
  }
  if ($ln_mx<$ln_mn) {
   $temp = $lt_mx;
   $ln_mx = $ln_mn;
   $ln_mn = $temp;
  }
  $bounds = "-R$ln_mn/$ln_mx/$lt_mn/$lt_mx";
  $psfile = BASEPATH . "/qcn/earthquakes/images/events_map.ps";
  $outdir = BASEPATH . "/qcn/earthquakes/images";
  $xtick   = (int) (($ln_mx-$ln_mn)*2)/10.;
  $ytick   = (int) (($lt_mx-$lt_mn)*2)/10.;
  $ticks   = "-B".$xtick."g0/".$ytick."g0WSne";
  $sea     = "-S100/150/200";
  
   if ($xtick<30) {
    $topog = GMTPATH . "/share/topo/topo30.grd";
    $coast   = "-Df";
   } else {
    $topog = GMTPATH . "/share/topo/topo_coarse.grd";
   }
  echo $topo;
  if ($topo=="y") {
   system(GMTPATH . "/bin/grdimage $topog -C$cpt $proj $bounds -X4 -Y4 -P -K > $psfile");
   system(GMTPATH . "/bin/pscoast $coast $proj $bounds -W0.5p/0 -O -K $sea >> $psfile");
  } else {
   system(GMTPATH . "/bin/pscoast $coast $proj $bounds -W0.5p/0 -K $sea > $psfile");
  }
  system(GMTPATH . "/bin/psxy $event_file $proj $bounds -O -Sa0.25 -G255/0/0 -W0.5p $ticks >> $psfile");
  system(GMTPATH . "/bin/ps2raster $psfile -D$outdir -A -P -Tj");
}


function show_last_eq($show_mag=null) {

 if ($show_mag) {
  $qfile = BASEURL . "/earthquakes/images/intensity_01.jpg";
 } else {
  $qfile = BASEURL . "/earthquakes/images/intensity_03.jpg";
 }

 echo "<a href=\"".$qfile."\"><img src=\"".$qfile."\" width=\"225\" align=\"center\"></a>\n";

}

function list_quakes($show_mag=null) {
$qmax=100000;
list_quakes_n($qmax,$show_mag);
}



function list_quakes_n($qmax,$show_mag=null){
// This function list the quakes //
if ($qmax<100) {chdir(BASEPATH . "/qcn/earthquakes/");}
$path = "./";

$dir_handle = @opendir($path) or die("Unable to open folder");

$a = 0;

$files = array();
while (false !== ($file1 = readdir($dir_handle))) {
$files[]=$file1;
}

sort($files);
$files = array_reverse($files);


//Count # of files that are not junk 
$aa = 0;$aaa = 0;
foreach ($files as $file1) {
  if(is_dir($file1)){
    if (  ($file1!=".")&&($file1!="..")&&($file1!="JUNK")&&($file1!="SAFE")&&($file1!="view")&&($file1!="inc")
        &&($file1!="images")&&($file1!="stats")&&($file1!="aichung") ){
        $aa++;
    }
  }
}


// If there are other event directories, then create start of header
if ($aa > 0) {
  if ( ($pwd = getcwd() == BASEPATH . "/qcn/earthquakes")||(($pwd = getcwd() == BASEPATH . "/qcn/earthquakes")) ) {
     echo "<h2>Recent Preliminary Earthquake Estimates:</h2>"; 
  }else{
     echo "<h2>Earlier Preliminary Earthquake Estimates:</h2>";
  }
  echo "<p><table border=1 width=\"100%\"><tr><td>";
  echo "<table paddig=2 width=\"95%\">";
  echo "<tr><td><b>Event ID:</b></td>\n
            <td><b>Date</b></td>\n
            <td><b>Time (UTC)</b></td>\n
            <td><b>Longitude</b></td>\n
            <td><b>Latitude</b></td>\n
            <td><b>Depth</b></td>\n";
  if ($show_mag) { 
     echo "    <td><b>Magnitude</b></td>\n"; 
  }  // Only show magnitude if requested
  echo "    <td><b>Triggers</b></td>\n
            <td><b>Detected</b></td>\n
            <td><b>View</b></td></tr>\n";

// For each event direcory, output a line of of a table //
 foreach ($files as $file1) {
  $a++;
  if($aaa>=$qmax) {break 1;}
  if(is_dir($file1)){

  if (  ($file1!=".")&&($file1!="..")&&($file1!="JUNK")&&($file1!="SAFE")&&($file1!="view")&&($file1!="inc")
      &&($file1!="images")&&($file1!="stats")&&($file1!="aichung") ){

  $aaa++;

  if ($a>1) {
   $bg_color="#ffffff";
   $a = 0;
  } else {
   $bg_color="#DDDDDD";
  }
  
  echo "<tr bgcolor=\"$bg_color\"><td>\n";
  if ( ($pwd = getcwd() == BASEPATH . "/qcn/earthquakes")||(($pwd = getcwd() == BASEPATH . "/qcn/earthquakes")) ) {
     echo "<a href=\"" . BASEURL . "/earthquakes/$file1/";
  } else {
     echo "<a href=\"" . $file1 . "/";
  }
  if ($show_mag) { echo "?show_mag=y"; }
   echo "\">$file1</a></td>\n";
   $efile = "$file1/event.xy"; 

   $etime = (int) $file1;
   $contents = file($efile);
   $string = implode($contents);
   list($qlon,$qlat,$qdep,$qmag,$ntrig,$etime,$dtime,$qstd) = split('[,]',$string);
   echo "</td><td>";
   echo date('M d Y',$etime)."\n";
   echo "</td><td>";
   echo date('H:i:s',$etime)."\n";
   echo "</td><td>";
   echo number_format($qlon,2,'.','')."\n";
   echo "</td><td>";
   echo number_format($qlat,2,'.','')."\n";
   echo "</td><td>";
   echo number_format($qdep,1,'.','')."\n";
   echo "</td><td>";
   if ($show_mag) {
    echo number_format($qmag,1,'.','')." &plusmn ".number_format($qstd,1,'.','')."\n";
    echo "</td><td>";
   }
   echo $ntrig."\n";
   echo "</td><td>";
   echo date('H:i:s',$dtime)."\n";
   echo "</td><td>";

   list($dir1,$dir2,$dir3,$dir4,$dir5,$dir6,$dir7) = split('[/]',getcwd());
   if ($dir6) {
    echo "<a href=\"" . BASEURL . "/earthquakes/view_events.php?event=".$dir6."&amp;sub=".$file1."\">View</a>\n";
   } else {
    echo "<a href=\"" . BASEURL . "/earthquakes/view_events.php?event=".$file1."\">View</a>\n";
   }
   echo "</td></tr>\n"; 
  }

  $qmag_arr[$aaa-1]=$qmag;
  $dt_detect[$aaa-1]=$dtime-$etime;
//   echo "$qmag_arr[$aaa],$aaa\n";

  }

 }
 echo "</table>";
 echo "</td></tr></table>";
}
closedir($dir_handle);

if ( ($pwd = getcwd() == BASEPATH . "/qcn/earthquakes")||(($pwd = getcwd() == BASEPATH . "/qcn/earthquakes")) ) {
  plot_histogram($qmag_arr,$xmin=3,$xmax=7,$ymin=0.1,$ymax=10,$dx=0.25,"Magnitude","Frequency","Magnitude Distribution",$file_name="magnitude",$log=4);
  plot_histogram($dt_detect,$xmin=0,$xmax=30,$ymin=0.1,$ymax=10,$dx=1,"Detection Time","Frequency","Detection Time Distribution",$file_name="dt_detect",$log=4);
}
}





function show_maps($show_mag=null) {

if ($show_mag) {
  $img1 = "intensity_02";
  $img2 = "intensity_01";
} else {
  $img1 = "intensity_04";
  $img2 = "intensity_03";
}

if (file_exists("./$img1.ps") ) {
  $img1 = "./".$img1;
  $img2 = "./".$img2;
} else {
  $img1 = "./A/".$img1;
  $img2 = "./A/".$img2;
}

echo "
<p><table align=\"center\"><tr>

<td width=\"50\" align=\"center\"><a href=\"$img1.jpg\"><img src=\"$img1.jpg\" width=\"325\"></a><br>Download: <a href=\"$img1.ps\">PS</a> or <a href=\"$img1.jpg\">JPEG</a> file.</td>\n

<td width=\"50\" align=\"center\"><a href=\"$img2.jpg\"><img src=\"$img2.jpg\" width=\"325\"></a><br>Download: <a href=\"$img2.ps\">PS</a> or <a href=\"$img2.jpg\">JPEG</a> file.</td>\n

</tr></table>\n

<a href=\"" . BASEURL . "/images/ShakeMap_Scale.png\"><center><img src=\"" . BASEURL . "/images/ShakeMap_Scale.png\" width=\"600\" align=\"center\"></center></a> 
\n";

}


function plot_histogram($val_array,$xmin,$xmax,$ymin,$ymax,$dx,$xtitle,$ytitle,$title,$name_base,$plot_med,$log=null) {
   
//   f
   $n_val = count($val_array); //echo "COUNT: $n_val\n";
//   echo "<p>$xmin,$xmax,$ymin,$ymax,$dx</p>\n";
//   echo $n_val;
   if ($n_val<3) {return;}
   $hist_file = BASEPATH . "/qcn/earthquakes/stats/".$name_base."_hist.txt";
//   echo "<p>".$histfile."</p>\n";
   $fh = fopen($hist_file,'w');

   for ($i = 0; $i <= $n_val; $i++) {
      fwrite($fh,"$val_array[$i]\n");
//      echo "<p>HI: $val_array[$i]\n";
   }

   fclose($fh);

   $GMT = GMTPATH . "/bin";
   $dax = (int) (($xmax-$xmin)/5);//$dx*4;
   if ($dax <=0)   $dax = (int) (($xmax-$xmin))/5;//$dx*4;
   $daxy = (int) (($ymax-$ymin)/5);
   $axes = "-Ba$daxf$dax:\"$xtitle\":/a".$daxy."f".$daxy.":\"$ytitle\"::.\"$title\":WSne";
//   echo "<p>Axes: $axes</p>\n";
   $bounds = "-R$xmin/$xmax/$ymin/$ymax";
//   echo "<p>Bounds: $bounds</p>\n";
   $proj   = "-JX2i/2i"; if ($log) {$proj=$proj."l";}
//   echo "<p>Projection: $proj</p>\n";
   $ps_hist = BASEPATH . "/qcn/earthquakes/images/".$name_base."_hist.ps";
//   echo "<p>PS Histogram: $ps_hist</p>\n";
   if (!$log) {$log=1;}
  exec("$GMT/gmtset HEADER_FONT_SIZE 16"); // Set title font size
   exec("$GMT/gmtset ANNOT_FONT_SIZE_PRIMARY 12"); // Set title font size
   exec("$GMT/gmtset LABEL_FONT_SIZE 12"); // Set title font size
   exec("$GMT/gmtset HEADER_OFFSET -0.2c");  // Set title vertical offset from top of map
   if ($plot_med=="y") {
    $med_file = BASEPATH . "/qcn/earthquakes/images/median.xy";
    $med = median_arr($val_array);
    exec("echo \"$med $ymin\" >  $med_file");
    exec("echo \"$med $ymax\" >> $med_file");
    $FLAG1="-K";
   }
   exec("$GMT/pshistogram $axes $hist_file $bounds $proj -G150/0/0 -Y5.5i -X0.5i -f -Lthinner -Z$log -W$dx $FLAG1 -P > $ps_hist");
   if ($plot_med=="y") {exec("$GMT/psxy $med_file -m $bounds $proj -W10t30_10:0 -O >> $ps_hist");}
   exec("$GMT/ps2raster $ps_hist -D" . BASEPATH . "/qcn/earthquakes/images -A -P -Tj");
//   echo "<p>$GMT/pshistogram $axes $hist_file $bounds $proj -Ggray -Y5.5i -X0.5i -Lthinner -Z$log -W0.25 > $ps_hist\n";
//   echo "<p>$GMT/ps2raster $ps_hist -D" . BASEPATH . "/qcn/earthquakes/images -A -P -Tj\n";
//   exec("rm $hist_file\n");
//   exec("rm $ps_hist\n");
   return $med;
}


function median_arr($numbers=array())
{
	rsort($numbers);
	$mid = (count($numbers) / 2);
	return ($mid % 2 != 0) ? $numbers{$mid-1} : (($numbers{$mid-1}) + $numbers{$mid}) / 2;
}


function average_arr($numbers=array())
{
	$ave = (array_sum($numbers) / count($numbers));
	return $ave;
}

function dl_link($elon,$elat,$etime) {
// Shows the links to the download webpages //
 $lnmn = $elon - 2.;
 $lnmx = $elon + 2.;
 $ltmn = $elat - 2.;
 $ltmx = $elat + 2.;

 $etime_s=$etime-(   30);          // Date/time  1 minute before earthquake
 $etime_e=$etime+(10*60);          // Date/Time 10 minutes after eathquake 

 $dys = date('Y-m-d',$etime_s);   // Date of event
 $hrs = date('H',    $etime_s);       // Hour of event
 $mns = date('i',    $etime_s);       // closest 10 minute of event

 $dye = date('Y-m-d',$etime_e);   // Date of event
 $hre = date('H',    $etime_e);       // Hour of event
 $mne = date('i',    $etime_e);       // closest 10 minute of event
 
 $path = BASEURL . "/continual/trdl.php?";
 $use_file = "cbUseFile=1";
 $loc_box = "&cbUseLat=1&LatMin=$ltmn&LatMax=$ltmx&LonMin=$lnmn&LonMax=$lnmx";
 $time_start = "&cbUseTime=1&date_start=$dys&time_hour_start=$hrs&time_minute_start=$mns";
 $time_end   =             "&date_end=$dye&time_hour_end=$hre&time_minute_end=$mne";
 $flags    = "&rbUseContinual=Continual&rb_sort=ttd&nresult=1000";
 echo "<p><strong>Download: </strong>";
 echo "<a href=\"$path$use_file$loc_box$time_start$time_end$flags\"> Continuous</a> or ";

 $path = BASEURL . "/sensor/trdl.php?";
 $flags    = "&rb_sort=ttd&nresult=1000";

 echo "<a href=\"$path$loc_box$time_start$time_end$flags\">triggered</a> data\n";
}



function event_info($show_mag=null){

echo "<h1>Earthquake:</h1>";

if (file_exists("event.xy")) {
 $efile = "event.xy";
} else {
 $efile="A/event.xy";
 echo "HI";
}

$contents = file($efile);
$string = implode($contents);
list($qlon, $qlat,$qdep,$qmag,$ntrig,$qtime,$dtime,$qstd) = split('[,]',$string);
echo "<p><strong>Date and Time:</strong>". date('M d Y H:i:s',$qtime)."</p>";
echo "<p><strong>Longitude:</strong>". $qlon;
echo "<strong> Latitude:</strong>". $qlat;
echo "<strong> Depth:</strong>". $qdep. " km </p> \n";
if ($show_mag) { echo "<p><strong>Magnitude:</strong>".number_format($qmag,1,'.','')." &plusmn; ".number_format($qstd,1,'.','')." (Local estimate - for scientific use only)</p> \n";}
dl_link($qlon, $qlat, $qtime);
show_created_on($qtime,$dtime);
echo "<hr>\n";
}





function show_scatter_tvt() {
// Shows the scatter plot of theoretical travel time v. observed travel time.
  echo "<hr><table>\n<tr>
  <td><img src=\"./t_scatter.jpg\" width=\"325\"></td>\n 
  <td><p><h2>Travel Time Fit:</h2>\n
  <p>The quality of the earthquake location depends on the match between the travel times estimated from the earthquake location 
  to the observed travel times. The observed travel times come from the time the earthquake triggered the volunteer sensor 
  computer.  The estimated times are determined from the event distance divided by the speed of the wave.
  The correlation (R) is a measure of the similarity between observed and estimated travel times. The misfit is another measure of 
  the similarity (the greater the misfit, the less similar the estimated and observed times are).</td></tr>\n
  <tr><td align=\"center\">Download <a href=\"./t_scatter.ps\">PS</a> or <a href=\"./t_scatter.jpg\">JPEG</a>\n</td><td></td></tr>\n
  </table>\n";
}


function show_scatter_avd() {
// Shows the scatter plot of theoretical travel time v. observed travel time.
  echo "<hr><table>\n<tr>
  <td><img src=\"./amp_v_dist.jpg\" width=\"325\"></td>\n 
  <td><p><h2>Amplitude v. Distance:</h2>\n
  <p>The amplitude of earthquake ground shaking should decrease with distance.  The plot to the left shows this relationship.
  The dashed lines show the (rough) amplitudes you would expect for different magnitudes.</td></td></tr>\n
  <tr><td align=\"center\">Download <a href=\"./t_scatter.ps\">PS</a> or <a href=\"./t_scatter.jpg\">JPEG</a>\n</td><td></td></tr>\n
  </table>\n";
}





function show_disclaimer() {
// Shows the disclaimer //
  echo "<hr><p>The information contained on this page is not intended for official use.  This is a scientific project aiming to validate the methods used to produce these results/data.  For official earthquake characterization, please obtain the appropriate information from your national earthquake program or the <a href=\"http://earthquake.usgs.gov/earthquakes/\">USGS</a>.</p><hr> 
  \n";
}





function show_created_on($qtime,$dtime) {
// Shows the date/time the page was created: // 
  $ddtime = (int) ($dtime-$qtime);
  echo "<p aling=\"justify\"><strong>Page created</strong>: ".$ddtime." seconds after the earthquake origin.\n"; 
}





function show_viewed_on() {
// Shows the date/time the page was viewed on //
  echo "<p>Page viewed on: ". date('M d Y'). " at ". date('h:i:s'); echo " (UTC)\n";
}





function trigger_info(){
// This function shows the trigger information for an event.


// Trigger file name:
  $efile="stations.xyz";
  echo "<table align=\"center\">\n";

// Header of trigger table

  echo "<tr align=\"center\"><td><strong>Host ID</strong></td>\n
            <td><strong>Trigger ID</strong></td>\n
            <td><strong>View</strong></td>\n
            <td><strong>Long</strong></td>\n
            <td><strong>Lat</strong></td>\n
            <td><strong>Trig Time</strong></td>\n
            <td><strong>Time Received</strong></td>\n
            <td><strong>Significance</strong></td>\n
            <td><strong>|accel| (m/s/s)</strong></td>\n
            <td><strong>Distance (km)</strong></td></tr>\n";

// Read trigger file content:
  $contents = file($efile);


// List each line:
  $i = 0;
  foreach ($contents as $line_num => $line) {
   list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],$pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
   for ($j=0;$j<=4;$j++) {
    if ($pgah[$j]>$smag) {$smag=$pgah[$j];}
    if ($pgaz[$j]>$smag) {$smag=$pgaz[$j];}
   }
   $i++; 

   

   if (preg_match("/conti/i","substr($sfile,0,4)")) {
      $sfile = next_ten_min($ttime,$sfile);
       $url = UPLOADURL . "/trigger/continual/";
      $urlhid = BASEURL . "/continual/show_host_detail.php?hostid=".$hid;
   } else {

     $url = UPLOADURL . "/trigger/";
     $urlhid = BASEURL . "/sensor/show_host_detail.php?hostid=".$hid;

   }
     $urlfile = $url.$sfile;
   if ($i>=1) {$bgcolor = "bgcolor=\"#DDDDDD\""; $i=-1;} else {$bgcolor = "bgcolor=\"#FFFFFF\"";}
   echo "<tr align=\"center\" $bgcolor >\n";
   echo "<td><a href=\"$urlhid\">$hid</a></td>\n";
   $fp = @fopen($urlfile,'r');
   if ($fp) {
    fclose($fp);
    echo "<td><a href=\"$urlfile\">$tid</a></td>\n";
    echo "<td><a href=\"javascript:void(0)
         \"onclick=\"window.open(BASEURL . '/earthquakes/view/view_data.php?dat=$sfile','linkname','height=500,width=400,scrollbars=no')\">View</a></td>\n";
   } else {
    echo "<td>$tid</td>\n";
    echo "<td>$sfile</td>\n";
   }
   echo "<td>".number_format($slon ,2,'.','')."</td>\n
         <td>".number_format($slat ,2,'.','')."</td>\n
         <td>".number_format($ttime,1,'.','')."</td>\n
         <td>$rtime</td>\n
         <td>".number_format($sig  ,1,'.','')."</td>\n
         <td>".number_format($smag ,1,'.','')."</td>\n
         <td>".number_format($dis  ,1,'.','')."</td></tr>\n";
   }

  echo "</table>\n";

}


function get_waveformss($event,$sub=null){
// This function shows the trigger information for an event.




// Get event info
if (file_exists("event.xy")) {
 $efile = "event.xy";
} else {
 $efile="A/event.xy";
 echo "HI";
}

$contents = file($efile);

$string = implode($contents);

list($qlon,$qlat,$qdep,$qmag,$ntrig,$qtime,$dtime,$qstd) = split('[,]',$string);

// Trigger file name:
//  $efile="A/stations.xyz";
   if ($sub) {
     $efile=$sub."/stations.xyz";
   } else {
     $efile="A/stations.xyz";
   }

// Header of trigger table

// Read trigger file content:
  $contents = file($efile);

//  echo "<html><head></head><body>";
#  echo "HI\n";

  $dir_base = BASEPATH . "/qcn/earthquakes/view/".round($qtime);

  if (!file_exists($dir_base)) {

      mkdir($dir_base);                    // Make new directory (since it doesn't exist

  }

//  echo "<p>BASE: $dir_base\n";
// List each line:
  $i = 0;
  $dis_max = 0;
  $dis_min = 9999999;
  $ferr_log = $dir_base."/error.log";
  $ferr = fopen($ferr_log,"w");
  $cnt = 0;
  foreach ($contents as $line_num => $line) {
    list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],$pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
    for ($j=0;$j<=4;$j++) {
      if ($pgah[$j]>$smag) {$smag=$pgah[$j];}
      if ($pgaz[$j]>$smag) {$smag=$pgaz[$j];}
    }
    $i++; 

    if ($dis > $dis_max) {$dis_max = $dis;}  
    if ($dis < $dis_min) {$dis_min = $dis;} 

    $dir_sub  = basename($sfile,".zip");

// If continual then use 10 minute time sections:
    if (preg_match("/conti/i","substr($sfile,0,4)")) {

       $sfile = next_ten_min($ttime,$sfile);
       $url = UPLOADURL . "/trigger/continual/";

    } else {

       $url = UPLOADURL . "/trigger/";

    }


// Set base directory:
    $dir = $dir_base.'/'.$dir_sub;
//    echo "<p>DIR: $dir\n";
// If already copied and processed, then don't copy and process:
    if (!@file_exists($dir)) {

// Set file name
      $file_orig = $url.$sfile;
      $file_new = $dir.'/'.$sfile;     // New file name with directory path

//echo "<BR><BR>file_orig=" . $file_orig . "<BR>file_new=" . $file_new . "<BR>dir=$dir<BR>";

      //if (@$fp = fopen($file_orig,"r")) {
      if (@$fp = fopen($file_orig,"r")) {
        @mkdir($dir);                    // Make new directory (since it doesn't exist
        @fclose($fp);
        @copy($file_orig,$file_new); // Copy file to new file location
        $dt = $ttime;
//echo SHELL_CMD . " " . BASEPATH . "/qcn/earthquakes/view/update_waveform.sh $dir $dis > $dir/temp.txt" . "<BR>";
        @system(SHELL_CMD . " " . BASEPATH . "/qcn/earthquakes/view/update_waveform.sh $dir $dis > $dir/temp.txt");
        fprintf($ferr,"y: %s\n",$file_orig);
        $cnt = $cnt + 1;
      } else {
       fprintf($ferr,"n: %s\n",$file_orig);
      }
    }

  }

  fclose($ferr);
  $dis_max = $dis_max + 1; $dis_min = $dis_min - 1;
  $jpgfile = "$dir_base/waveform.jpg";   
//echo SHELL_CMD . " " . BASEPATH . "/qcn/earthquakes/view/plot_all_data.sh $dir_base $dis_max $dis_min > $dir_base/temp.txt" . "<BR>";
  if (!@file_exists($jpgfile)||$cnt>0) {
    @system(SHELL_CMD . " " . BASEPATH . "/qcn/earthquakes/view/plot_all_data.sh $dir_base $dis_max $dis_min > $dir_base/temp.txt");//> $dir_base/temp.txt");
  }
  rotateImage($jpgfile);
}

function next_ten_min($ttime,$sfile) {

   $t_min = date('i',$ttime); // Trigger minutes
   $t_sec = date('s',$ttime); // Trigger seconds
   $t_min_n = (  ceil($t_min / 10) * 10);
   $t_dt = ($t_min_n - $t_min)*60 -$t_sec;
   $ttime2 = (int) ($ttime + $t_dt);   
   $ttime  = (int) ($ttime);
   $sfile = str_replace($ttime,$ttime2,$sfile);
   $sfile = str_replace($ttime+1,$ttime2,$sfile);
   $sfile = str_replace($ttime-1,$ttime2,$sfile);
   $sfile = str_replace($ttime+2,$ttime2,$sfile);

   return $sfile;
}


function show_seismograms() {

return; // CMC here - this doesn't seem to be used?

   $ps_yn = plot_seismograms();
   if ($ps_yn >= 0) {
    echo "HI\n";
    echo "<p><hr/>\n";
    echo "<table width=\"680\"><tr>\n";
    echo "<td width=\"400\"><img src=\"seis/waveform.jpg\" width=\"390\"></td>\n";//
    echo "<td></td>\n";
    echo "</tr></table>\n";
   } else { echo "HO\n";}

}


function plot_seismograms() {
// Travel time (obs v. est) scatter plot //

// Get maximum time from scatter file://
   $st_file="stations.xyz";
   if (file_exists($st_file)) {} else {return -1;}
   $efile = "event.xy";
   $contents = file($efile);
   $string = implode($contents);
   list($qlon,$qlat,$qdep,$qmag,$ntrig,$etime,$dtime,$qstd) = split('[,]',$string);

   $GMT = GMTPATH . "/bin";
   $contents = file($st_file);
   $dis_max=-999;
   $dis_min= 999;
   $sac = SACPATH . "/bin/sac";
   $sgf = SACPATH . "/bin/sgftops";
   $script_file = "plot_seismogram.sh";
//   echo $script_file;
   $fh = fopen($script_file,'w');
   $pltdir = "./seis"; 
   $jpgfile = $pltdir."/waveform.jpg";
   $psfile  = $pltdir."/waveform.ps";
   fwrite($fh,"#!" . SHELL_CMD . " -f\n");
   fwrite($fh,"source ../../inc/bash.env\n");

//   fwrite($fh,"cd ".$pltdir."\n");  // change directories to plot directory

   if (file_exists($jpgfile)) {      // Check if directory/image already exists
//      rotateImage($jpgfile);
   } else {  // If not already made, make image:
      @mkdir($pltdir, 0777);
      chmod($pltdir,0777);
   }
   
//   chdir("$pltdir");
   $iii = 0;
   foreach ($contents as $line_num => $line) {
    list($slon, $slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis) = split('[,]',$line);

    $iii++;
    $smag = log($smag);
    if ($dis <= 0) {
      $dis = distance($slon, $slat, $qlon, $qlat); // Distance from quake
    }
    if ($dis  > $dis_max) $dis_max = $dis;
//    if ($dis < $dis_min) $dis_min = $dis;
    $sfile1 = next_ten_min($ttime,$sfile);
    $sfileo = full_sac_file_name($sfile,$ttime);
    $sfilen= "$pltdir/$sfile1";
    $sfileb = basename($sfile1,".zip");
    if (@copy("$sfileo","$sfilen")) {
     exec("/usr/bin/unzip -o $sfilen");
     exec("mv $sfileb.*.sac $pltdir/.");
     exec("chmod 777 $pltdir/$sfileb.*.sac");
     $sfile_s  = basename($sfile1,".zip");
     $sac_file = "$pltdir/$sfile_s.Z.sac";
//     echo $sac_file;


//     fwrite($fh,"chmod 777 $sfileb*.sac\n");

     fwrite($fh,$sac." << EOF$iii > temp.txt \n");    // start sac
     $t_cut = next_ten_min_f($ttime);
     $t_cut1 = $t_cut - 20;
     $t_cut2 = $t_cut + 20;
     $k_cut = "cut ".$t_cut1." ".$t_cut2;
     fwrite($fh,"$k_cut\n");
     fwrite($fh,"r $sac_file \n");  // read each file and
     fwrite($fh,"div 1000000000\n");
//     fwrite($fh,"chnhdr gcarc ".$dis/111.19."\n"); // Change distance (Deg)
//     fwrite($fh,"chnhdr dist ".$dis."\n");         // Change distance (km)
//     fwrite($fh,"chnhdr evlo ".$qlon."\n");        // Change longitude of quake
//     fwrite($fh,"chnhdr evla ".$qlat."\n");        // Change latitude of quake
     fwrite($fh,"rmean \n");                        // Remove mean of trace
//     fwrite($fh,"rtrend \n");
     fwrite($fh,"lp n 2 co 7\n");
//     fwrite($fh,"whiten\n");
     fwrite($fh,"add $dis\n");                     // Add distance to each trace
     //fwrite($fh,"write $iii.ZZ.sac\n");                    // write over
     fwrite($fh,"write $iii.Z.sac\n");    // is it really ZZ?                // write over
     fwrite($fh,"q\n");
     fwrite($fh,"EOF$iii\n\n");
    }
   }
   $dis_min = 0;
   fwrite($fh,$sac." << EOF\n");    // start sac
   //fwrite($fh,"r *.ZZ.sac\n");                      // Read and plot all files
   fwrite($fh,"r *.Z.sac\n");                      // Read and plot all files

   fwrite($fh,"qdp off\n");
   fwrite($fh,"fileid off\n");
   fwrite($fh,"begg sgf\n");                      // Begin plotting 
//   fwrite($fh,"ylim ".$dis_min." ".$dis_max."\n");// Set y limits
   fwrite($fh,"p2 r\n");                            // Plot all in one window
   fwrite($fh,"endg sgf\n");                      // End plotting
   fwrite($fh,"sgftops f001.sgf $psfile\n");  // Convert plotting to ps file
   fwrite($fh,"rm f001.sgf\n");

   fwrite($fh,"q\n");
   fwrite($fh,"EOF\n");
   fwrite($fh,"$GMT/ps2raster $psfile -D$pltdir -A -P -Tj\n");
//   fwrite($fh,"rm *.sac\n");
//   fwrite($fh,"rm *.zip\n");
   fwrite($fh,"rm $pltdir/*.ps\n");
   fwrite($fh,"chmod 777 $pltdir/*.jpg\n");
   fclose($fh);

   exec(SHELL_CMD . " " . $script_file);  // Run the script

//   rotateImage($jpgfile);
   return 1;
}


// Rotate the image:
function rotateImage($filename) {
// File and rotation
$degrees = 270;

// Content type
header('Content-type: image/jpeg');

// Load
$source = imagecreatefromjpeg($filename);

// Rotate
$rotate = imagerotate($source, $degrees, 0);

// Output
imagejpeg($rotate);

// Clear up memory:
imagedestroy($im);

}



function full_sac_file_name($sfile,$ttime) {
   if (preg_match("/conti/i","substr($sfile,0,4)")) {
      $sfile = next_ten_min($ttime,$sfile);
      $url = UPLOADURL . "/trigger/continual/";
      $urlhid = BASEURL . "/continual/show_host_detail.php?hostid=".$hid;
   } else {

     $url = UPLOADURL . "/trigger/";
     $urlhid = BASEURL . "/sensor/show_host_detail.php?hostid=".$hid;

   }
     return $full_file = $url.$sfile;
}

function next_ten_min_f($ttime) {

   $t_min = date('i',$ttime); // Trigger minutes
   $t_sec = date('s',$ttime); // Trigger seconds
   $t_min_n = (  ceil($t_min / 10) * 10);
   $t_dt = ($t_min_n - $t_min)*60 -$t_sec;
   return $t_dt = 600.-$t_dt;
}

?>
