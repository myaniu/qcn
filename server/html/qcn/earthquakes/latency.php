<?php
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");


require_once(BASEPATH . "/qcn/inc/utils.inc");
require_once(BASEPATH . "/qcn/inc/qcn_auto_detect.inc");
page_top();
$show_mg = $_GET["show_mag"];

//require_once(BASEPATH . "/qcn/earthquakes/inc/gmt_quakes.inc");
//gmt_quake_map();   // Generate the earthquake map.

  echo "<p><h1>Earthquake Statistics:</h1></p>\n";

  echo "<p align=\"justify\">The Quake-Catcher Network now locates earthquakes based on the data collected through volunteer distrbuted computing. Below are some plots describing our detections.</p>";

  echo "<hr/>\n";

  if ($f_age = file_age($file_name=BASEPATH . "/qcn/earthquakes/images/earthquake_through_time_hist.jpg") > 0.){//4*60*60) {
    make_latency_stats();
  }
  show_latency_stats();


show_viewed_on();  // Show the date page viewed on

page_end();


function file_age($file_name) {

  if (file_exists($file_name)) {
     return time()-filemtime($file_name);
  } else {
     return null;
  }

}

function show_stats_figure($figure,$blurb) {

$width = "200";
$height= "200";

// Plot # of earthquakes per day:

echo "<table><tr><td width=\"50%\">\n";
echo "           <img src=\"" . BASEURL . "/earthquakes/images/".$figure."\" width=\"".$width."\" height=\"".$height."\" align=\"center\">\n";
echo "       </td>\n";
echo "       <td width=\"50%\">\n";
echo "           <p align=\"justify\">".$blurb."\n";
echo "       </td>\n";
echo "       </tr>\n";
echo "</table>\n";
echo "<hr/>\n";

}

function make_latency_stats(){
// This function plots the distribution of trigger latencies.

// Use the earthquakes directory
   if ($qmax<100) {chdir(BASEPATH . "/qcn/earthquakes/");}
   $path = "./";
   $dir_handle = @opendir($path) or die("Unable to open folder");

   $files = array();
   while (false !== ($file1 = readdir($dir_handle))) {
      $files[]=$file1;
   }

   sort($files);
   $files = array_reverse($files);

   $ABC = array('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z');

//Count # of files that are not junk 
   $aa = 0;$aaa = 0;
   for ($j = 0; $j < 16; $j++) {
     $latency  = array();
     $i = 0;
     foreach ($files as $file1) {
        if(is_dir($file1)){
         if (  ($file1!=".")&&($file1!="..")&&($file1!="JUNK")&&($file1!="SAFE")&&($file1!="view")&&($file1!="inc")
                       &&($file1!="images")&&($file1!="stats")&&($file1!="aichung") ){
           $aa++;
           $sfile = $file1."/".$ABC[$j]."/stations.xyz";
           if (false == file_exists($sfile)) {break;}
// Read trigger file content:
           $contents = file($sfile);
         


// List each line:
           foreach ($contents as $line_num => $line) {
             list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],
                  $pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
             if ( ($rtime!==null)&&($rtime !== 0)&&($ttime!==0) ) {
               $latency[$i] = $rtime-$ttime;
               $i++;
             } 
           }
         }
       }
     }
     $med_m = plot_histogram($latency,$xmin=0,$xmax=15,$ymin=0.01,$ymax=20,$dx=0.5,"Latency","Frequency","Latency Distribution",
           $file_name="trigger_latency_".$ABC[$j],$plt_med="y");
   }
}


function show_latency_stats() {

echo "<h2>Trigger Latency:</h2>\n";
show_stats_figure($figure = "trigger_latency.jpg",$blurb="Triggers from detected earthquakes upload with the following latencies.");




$width = "200";
$height= "200";


$ABC = array('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z');

 for ($i = -1;$i<7;$i++) {
  $i++;
  echo "<table width=\"100%\"><tr>\n"; 
  echo "<tr><td><h2>Detection $ABC[$i] (Iteration ".($i+1)."):</h2></td><td></td><td></td></tr>\n";
  echo "<tr>\n";
  echo "<td width=\"45%\"><img src=\"" . BASEURL . "/earthquakes/images/trigger_latency_".$ABC[$i]."_hist.jpg\" width=\"$width\" height=\"$height\"></td>";
  echo "<td width=\"10%\"> </td>\n";
  echo "<td width=\"45%\"> </td>";
  echo "</tr>\n";
  echo "</table>\n";
  echo "<hr/>\n";
 }


}






function make_quake_stats(){
// This function list the quakes //

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
        &&($file1!="images") ){
        $aa++;

    }
  }
}

 $ABC = array('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z');


 $time_file = BASEPATH . "/qcn/earthquakes/stats/time_v_iteration.txt";
 $mag_file  = BASEPATH . "/qcn/earthquakes/stats/mag_v_iteration.txt";
 $mt_file   = BASEPATH . "/qcn/earthquakes/stats/mag_v_time.txt";
 
 $time_now = time();
  
 $ft = fopen($time_file,'w');
 $fm = fopen($mag_file,'w');
 $fmt = fopen($mt_file,'w');

// For each event direcory, output a line of of a table //
 for ($i=0;$i<7;$i++) {
  $aaa=0;
//  $i++;
  $qmag_arr  = array();
  $dt_detect = array();
  foreach ($files as $file1) {
   $a++;
   if(is_dir($file1)){ 

    if (  ($file1!=".")&&($file1!="..")&&($file1!="JUNK")&&($file1!="SAFE")&&($file1!="view")&&($file1!="inc")
        &&($file1!="images") ){

     $efile = "$file1/$ABC[$i]/event.xy";
     if (file_exists($efile)) {
      
      $aaa++;
      
      $etime = (int) $file1;
      $contents = file($efile);
      $string = implode($contents);
      list($qlon,$qlat,$qdep,$qmag,$ntrig,$etime,$dtime,$qstd) = split('[,]',$string);

      $qmag_arr[$aaa-1]=$qmag;
      $dt_detect[$aaa-1]=$dtime-$etime;
      $days[$aaa-1] = (int) (($etime - $time_now)/60./60./24.);

     }
    }
   }
  }

  $med_m = plot_histogram($qmag_arr,$xmin=3,$xmax=7,$ymin=0.1,$ymax=35,$dx=0.25,"Magnitude","Frequency","Magnitude Distribution",$file_name="magnitude_".$i,$plt_med="y");
  $med_t = plot_histogram($dt_detect,$xmin=0,$xmax=30,$ymin=0.1,$ymax=25,$dx=1,"Detection Times","Frequency","Detection Time Distribution",$file_name="dt_detect_".$i,$plt_med="y");

// Histogram of # of earthquakes detected:
  if ($i==0) {
   $xmin = min($days); if ($xmin<-90) {$xmin=-90;}
   $xmax = 0;
//   echo "$xmin, $xmax\n";
   $med_eq = plot_histogram($days,$xmin,$xmax,$ymin=0,$ymax=15,$dx=1,"Days Ago","Frequency","Earthquake Distribution",$file_name="earthquake_through_time",$plt_med="n");
  }
  fwrite($fm,($i+1).",$med_m\n");
  fwrite($ft,($i+1).",$med_t\n");
  fwrite($fmt,"$med_t,$med_m\n");
 }
 fclose($fm);
 fclose($ft);
 fclose($fmt);
 closedir($dir_handle);

 psxy($time_file,$xmn=0,$xmx=10,$ymn=0,$ymx=20,"Iteration","Time","Detection Time per Iteration","time_v_iter");
 psxy($mag_file,$xmn=0,$xmx=10,$ymn=3,$ymx=7,"Iteration","Magnitude","Magnitude per Iteration","mag_v_iter");
 psxy($mt_file,$xmn=0,$xmx=20,$ymn=2,$ymx=7,"Time (s)","Magnitude","Magitude v. Detection Time","mag_v_time");

}



function psxy($ifile,$xmn,$xmx,$ymn,$ymx,$xtitle,$ytitle,$title,$name_base,$log=null){
   $GMT = GMTPATH . "/bin";
   $dax = (($xmx-$xmn)/5);   
   $daxy = (($ymx-$ymn)/5);
   $axes = "-Ba".$dax."f".$dax.":\"$xtitle\":/a".$daxy."f".$daxy.":\"$ytitle\"::.\"$title\":WSne";

   $bounds = "-R$xmn/$xmx/$ymn/$ymx";
   $proj   = "-JX2i/2i"; if ($log) {$proj=$proj."l";}
   $ps_file = BASEPATH . "/qcn/earthquakes/images/".$name_base.".ps";
   if (!$log) {$log=1;}
   exec("$GMT/gmtset HEADER_FONT_SIZE 16"); // Set title font size
   exec("$GMT/gmtset ANNOT_FONT_SIZE_PRIMARY 12"); // Set title font size
   exec("$GMT/gmtset LABEL_FONT_SIZE 12"); // Set title font size
   exec("$GMT/gmtset HEADER_OFFSET -0.2c");  // Set title vertical offset from top of map
   exec("$GMT/psxy $ifile -m $bounds $proj $axes -W10t30_10:0 > $ps_file");
   exec("$GMT/ps2raster $ps_file -D" . BASEPATH . "/qcn/earthquakes/images -A -P -Tj");
}


?>
