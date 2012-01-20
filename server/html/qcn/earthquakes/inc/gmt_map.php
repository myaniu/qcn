<?php

require_once("common.inc");

//$qlon  = floatval(172.6700);
//$qlat  = floatval(-43.6500);
//$r_squared = floatval(0.75);
//$misfit    = floatval(0.71);

date_default_timezone_set('UTC');         // Set the default time zone to UTC

intensity();

map_quake_with_intensity();


function map_quake_with_intensity() {

$edir      = getcwd();
$efile = $edir."/event.xy";
$contents = file($efile);
$string = implode($contents);
list($qlon,$qlat,$qdep,$qmag,$ntrig,$etime,$dtime,$qstd,$r_squared,$misfit) = split('[,]',$string);


if ($qmag > 7) {
 $dec_mag = 2;
} else {
 $dec_mag = 1;
}
$dec = 100;
$dI  = 1./$dec;
$dy = ((int) (log10($qmag*$qmag)*$dec) / $dec * $dec_mag );

$lon_fact = abs(cos($qlat*pi()/180.))*1.05; //$lon_fact*=$lon_fact;
$dx = ((int) (log10($qmag*$qmag)/$lon_fact*$dec)) / $dec * $dec_mag;
//$dx = ((int) ($dy/$lon_fact*$dec)) / $dec;
echo "$lon_fact $dx $dy\n";


$ln_mn = ((int) ($qlon*$dec))/$dec-(int)($dx/2*$dec+10)/$dec;
$ln_mx = ((int) ($qlon*$dec))/$dec+(int)($dx/2*$dec+10)/$dec;
$lt_mn = ((int) ($qlat*$dec))/$dec-(int)($dy/2*$dec+10)/$dec;
$lt_mx = ((int) ($qlat*$dec))/$dec+(int)($dy/2*$dec+10)/$dec;

echo "$ln_mn,$ln_mx,$lt_mn,$lt_mx\n";

$GMT      = GMTPATH . "/bin";
$GRID     = "-I$dI/$dI" ;
$BOUNDS   = "-R$ln_mn/$ln_mx/$lt_mn/$lt_mx";
$QDIR     = BASEPATH . "/qcnwp/earthquakes";
$OUTDIR   = $edir;
$GRDFILE  = "$OUTDIR/grid.grd" ;
$GRADFILE = "$OUTDIR/grad.grd" ;
$TOPO     = GMTPATH . "/share/topo/topo30_gradI.grd" ;
$EVENT    = "$OUTDIR/event.xy";
$STATIONS = "$OUTDIR/stations.xyz"; 
$TCONTOUR = "$OUTDIR/t_contour.xy";
$TXTCON   = "$OUTDIR/t_contour.txt" ;
$IFILE    = "$OUTDIR/intensity_map.xyz" ;
$TEMP     = "$OUTDIR/.temp";
$CPTFILE  = "$QDIR/inc/int.cpt";
$X1Y1     = "-X3 -Y10";
$PROJ     = "-JM4.0i";
$FLAGS1   = "-K -P ";
$FLAGS2   = "-K -O ";
$FLAGS3   = "-O ";
$COASTS   = "-Df -N2 -N1 -W1.0p/0 "; 

echo "GMT=$GMT\n";
echo "pre cut:\n";

exec("$GMT/grdcut $TOPO -G$GRADFILE $BOUNDS"); 
exec("$GMT/surface $IFILE -S0.05 -T0.5 $GRID $BOUNDS -G$GRDFILE"); 


echo "pre\n";

 for ($i = 1; $i<=2;$i++) {
   echo $i;
   if ($i ==2) $i == 3;
   $ii=$i; //if ($ii>2) {$ii=$ii-2;}
   $ln_mn = ((int) ($qlon*$dec))/$dec-($ii)*$dx/2;
   $ln_mx = ((int) ($qlon*$dec))/$dec+($ii)*$dx/2;
   $lt_mn = ((int) ($qlat*$dec))/$dec-($ii)*$dy/2;
   $lt_mx = ((int) ($qlat*$dec))/$dec+($ii)*$dy/2;

   $PSFILE   = "$OUTDIR/intensity_0$i.ps";
   $BOUNDS   = "-R$ln_mn/$ln_mx/$lt_mn/$lt_mx";

// Title & Axes: //
   $title = "QCN Earthquake - ";    // Start of title
   if ($i<=2) { $title=$title."M".number_format($qmag,1,'.','');} // Report Magnitude
   $title = $title." Lon=".number_format($qlon,2,'.','')." Lat=".number_format($qlat,2,'.',''); // Longitude and Latitude
   exec("$GMT/gmtset HEADER_FONT_SIZE 16"); // Set title font size
   exec("$GMT/gmtset HEADER_OFFSET 0.0c");  // Set title vertical offset from top of map
   $B        = "-B1/0.5:.\"$title\":WSne";  // Set grid/annotation interval
   $ts_x   = ($ln_mn+$ln_mx)/2.;            // Middle x position of map
   $ts_y   = $lt_mx + 0.03*$ii*$dy;         // Top of plot plus a little

// Sub-title //
   $title_sub = "$ts_x $ts_y 12 0 1 CB Depth=".number_format($qdep,1,'.','')." ".date('M d Y H:i:s',$etime)."\n";
   $ts_file = "$OUTDIR/ts_file.txt";
   $fts = fopen($ts_file,"w");
   fwrite($fts,$title_sub);
   $ts_y = $lt_mn + 0.02*$ii*$dy;
   $title_sub = "$ts_x $ts_y 12 0 1 CB Detected: ".date('M d Y H:i:s',$dtime);
   fwrite($fts,$title_sub);
   fclose($fts);

   exec("$GMT/gmtset ANOT_FONT_SIZE 12");

   $txtfile = "$OUTDIR/qcn_date_0$ii.txt";
   if ($i<3){

    exec("$GMT/grdimage $GRDFILE -I$GRADFILE -C$CPTFILE $BOUNDS $PROJ $X1Y1 $FLAGS1 > $PSFILE");
    exec("$GMT/pscoast $COASTS $PROJ $BOUNDS $FLAGS2 -S100/150/255>> $PSFILE");


   } else {
   

    exec("$GMT/pscoast $COASTS $PROJ $BOUNDS $FLAGS1 -S100/150/255 > $PSFILE");
   }

// Select a sector for the name plotting ~4 times as fast by dividing it
   $ix = (int) ( ($qlon+180.)/90.) + 1;
   $iy = (int) ( ($qlat+ 90.)/45.) + 1;

   
   $CITIES   = "$QDIR/inc/worldcitiespop/worldcities_pop_".$ix.".".$iy.".gmt";
   $CITY_NAMES = "$QDIR/inc/worldcitiespop/worldcities_names_".$ix.".".$iy.".gmt";
   echo $CITIES."   ".$CITY_NAMES;

   exec("$GMT/psxy $EVENT $BOUNDS $PROJ $FLAGS2 -Sa0.75 -W1p/255/0/0 >> $PSFILE"); 
   exec("$GMT/psxy $STATIONS $BOUNDS $PROJ $FLAGS2 -St0.25 -C$CPTFILE -W0.5p/100 >> $PSFILE"); 
   exec("$GMT/psxy $CITIES $BOUNDS $PROJ $FLAGS2 -Sc0.2 -G0 -W1p/255 >> $PSFILE");
   exec("$GMT/pstext $CITY_NAMES $BOUNDS $PROJ $FLAGS2 -G0 >> $PSFILE");
   exec("$GMT/psxy $TCONTOUR $BOUNDS $PROJ $FLAGS2 -m -W1p/175 >> $PSFILE"); 
   exec("$GMT/pstext $txtfile $BOUNDS $PROJ $FLAGS2 -S0.5p >> $PSFILE");
   exec("$GMT/pstext $ts_file -N $BOUNDS $PROJ $FLAGS2 >> $PSFILE");
   exec("$GMT/pstext $TXTCON $BOUNDS $PROJ $FLAGS3 $B >> $PSFILE");
   exec("$GMT/ps2raster $PSFILE -D$OUTDIR -E150 -A -P -Tj");
   exec("cp $OUTDIR/intensity_0$i.jpg $OUTDIR/../.");
   exec("cp $OUTDIR/intensity_0$i.jpg $QDIR/images/.");

 }

copy($OUTDIR."/event.xy",$OUTDIR."/../event.xy");                 // Copy detection to group directory
copy($QDIR."/inc/index_earthquake.php",$OUTDIR."/../index.php");  // Copy wep page over to new location
copy($QDIR."/inc/index_earthquake_sub.php",$OUTDIR."/index.php"); // Copy individual index page here



echo "plot t scatter plot:\n";

 t_scatter_plot($GMT,$OUTDIR,$r_squared,$misfit);

echo "plot amp v. dist plot\n";
 amp_v_dist_plot($GMT,$OUTDIR,$QDIR,$qlon,$qlat);

echo "plot seismograms";
// CMC NOTE - where is this function?
 //  plot_seismograms($GMT,$OUTDIR,$QDIR,$qlon,$qlat);

}

function t_scatter_plot($GMT,$OUTDIR,$r_squared,$misfit) {
// Travel time (obs v. est) scatter plot //

// Get maximum time from scatter file://
   $efile="./t_scatter.xy";
   if (file_exists($efile)) {
   } else {
    return;
   }
   $contents = file($efile);
   $t_max=0;
   if ($r_squared<0.1) {$get_r2="y";$tos=0;$tes=0;$cnt=0;}
   foreach ($contents as $line_num => $line) {
      list($t_obs,$t_est) = split('[,]',$line);
      if ($t_obs>$t_max) {$t_max=$t_obs;}
      if ($t_est>$t_max) {$t_max=$t_est;}
      if ($get_r2=="y") {
         $r_squared+=($t_obs*$t_est);
         $tos+=($t_obs*$t_obs);
         $tes+=($t_est*$t_est);
         $misfit+=($t_obs-$t_est)^2;
         $cnt++;
      }
      
   }
   $t_max = ((int) ($t_max/10)+1)*10;
   if ($get_r2=="y") {
      $r_squared = $r_squared/($tes*$tos)^0.5;
      $misfit= ( $misfit / $cnt )^0.5;
   }

// Set plot parameters
   $BOUNDS   = "-R0/$t_max/0/$t_max";
   $T_SCAT   = "$OUTDIR/t_scatter.xy"; 
   $PROJ     = "-JX3i/3i";
   $PSFILE   = "$OUTDIR/t_scatter.ps";
   $FLAGS1   = "-K -P ";
   $FLAGS2   = "-K -O ";
   $FLAGS3   = "-O ";
   $B        = "-Ba10f10:T_observed\(s\):/a10f10:T_estimated\(s\):WSne";

// Text file for expected lines through scatter plot //
// Text file for expected lines through scatter plot //
   $SCATLINE = "/var/www/qcnwp/earthquakes/inc/scatter_lines.xy";
   
/*   $tps = $t_max/1.87;   // Time difference between P and S (or S and P) waves.
   $fh = fopen($SCATLINE,'w');  // Open file for writing out lines
   fwrite($fh,"0,0\n");         
   fwrite($fh,"$tps,$t_max\n");
   fwrite($fh,">\n");

   fwrite($fh,"0,0\n");
   fwrite($fh,"$t_max,$t_max\n");
   fwrite($fh,">\n");

   fwrite($fh,"0,0\n");
   fwrite($fh,"$t_max,$tps\n");
   fwrite($fh,">\n");
   fclose($fh);                // Close file for writing out lines
*/
// Create plot using GMT psxy: //
   exec("$GMT/psxy $T_SCAT $BOUNDS $PROJ $FLAGS1 -Sx0.2 -W1p/255/0/0 > $PSFILE"); // Plot points
   exec("$GMT/psxy $SCATLINE $BOUNDS $PROJ $FLAGS2 $B -m -Wthick,-   >> $PSFILE");// Plot lines

// Label correlation (R^2) and Misfit: //
   $STATS    = "$OUTDIR/stats.txt";
   $x = 0.05*$t_max;
   $y = 0.87*$t_max;
   $fh = fopen($STATS,'w');
   fwrite($fh,"$x $y 18 0 0 5 \ R^2 = ".number_format($r_squared,2,'.','')."\n");
   $y = 0.80*$t_max;
   fwrite($fh,"$x $y 18 0 0 5 \ Misfit = ".number_format($misfit,2,'.','')."\n");
   fclose($fh);

   exec("$GMT/pstext $STATS $BOUNDS $PROJ $FLAGS3 $B >> $PSFILE ");
   exec("$GMT/ps2raster $PSFILE -D$OUTDIR -E150 -A -P -Tj");
}


function amp_v_dist_plot($GMT,$OUTDIR,$QDIR,$qlon,$qlat) {
// Travel time (obs v. est) scatter plot //

// Get maximum time from scatter file://
   $efile="stations.xyz";
   $contents = file($efile);
   $dis_max=-999;
   $mag_max=-999;
   $mag_min=999;
   $avd_file = $OUTDIR."/amp_v_dist.txt";
   $fh = fopen($avd_file,'w');
   foreach ($contents as $line_num => $line) {
    list($slon, $slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis) = split('[,]',$line);
    $smag = log($smag);

    if ($dis <= 0) {
      $dis = distance($slon, $slat, $qlon, $qlat);

    }
    if ($dis  > $dis_max) $dis_max = $dis;
    if ($smag > $mag_max) $mag_max = $smag;
    if ($smag < $mag_min) $mag_min = $smag;
    fwrite($fh,"$dis,$smag\n");
   }
   fclose($fh);
   $dis_max = ((int) ($dis_max/10)+1)*10;


   $mag_max = ((int) ($mag_max+0.5   )+2);
   $mag_min = ((int) ($mag_min-0.5   )-2);

// Set plot parameters
   $BOUNDS   = "-R0/$dis_max/$mag_min/$mag_max";
   $PROJ     = "-JX3i/3i";
   $PSFILE   = "$OUTDIR/amp_v_dist.ps";
   $FLAGS1   = "-K -P ";
   $FLAGS2   = "-K -O ";
   $FLAGS3   = "-O ";
   $B        = "-Ba5f5:Distance\(km\):/a1f1:Log\(Amplitude\)\(m/s/s\):WSne";
   $avd_line = $QDIR."/inc/amp_v_dist.theo";
//   $fh = fopen($avd_line,'w');
   $avd_text = $QDIR."/inc/amp_v_dist.text";
//   $ft = fopen($avd_text,'w');
//   for ($i=3;$i<=7;$i++) {
//      for ($j=0;$j<=21;$j++) {
//         $dis = $dis_max*($j)/20.;
//         $lpga = -0.05*$dis + 1.8385*($i)-7.8671;
//         fwrite($fh,"$dis,$lpga\n");
//         if ($j==0) {fwrite($ft,"1 $lpga 18 0 0 5 \ M$i\n");}
//      }
//     fwrite($fh,">\n");
//   }
//   fclose($fh);
//   fclose($ft);


// Create plot using GMT psxy: //
   exec("$GMT/psxy $avd_file $BOUNDS $PROJ $FLAGS1 -Sx0.2 -W1p/255/0/0 > $PSFILE"); // Plot points
   exec("$GMT/psxy $avd_line $BOUNDS $PROJ $FLAGS2 -m -Wthick,-    >> $PSFILE");// Plot lines
   exec("$GMT/pstext $avd_text $BOUNDS $PROJ $FLAGS3 $B >> $PSFILE "); // 
   exec("$GMT/ps2raster $PSFILE -D$OUTDIR -E150 -A -P -Tj");

}



function intensity() {

   $edir      = getcwd();
   $efile = $edir."/event.xy";
   $contents = file($efile);
   $string = implode($contents);
   list($qlon,$qlat,$qdep,$qmag,$ntrig,$etime,$dtime,$qstd,$r_squared,$misfit) = split('[,]',$string);

   $b =  1.8385; // Magnitude term
   $a = -0.03085*$b;  // Distance term
   $c = -4.28; // Constant

   $dec = 100;
   $dI  = 1./$dec;

   if ($qmag >= 7.) {
    $dec_mag = 2;
   } else { 
    $dec_mag = 1;
   }

   $dy = ((int) (log10($qmag*$qmag)*$dec) / $dec)*$dec_mag; 

   $lon_fact = abs(cos($qlat*pi()/180.))*1.1; //$lon_fact*=$lon_fact;
//   $lon_fact = abs(cos($qlat*pi()/180.)); //$lon_fact*=$lon_fact;
   $dx = ((int) (log10($qmag*$qmag)/$lon_fact*$dec)) / $dec * $dec_mag;


   $x_mn = ((int) ($qlon*$dec))/$dec-$dx;
   $x_mx = ((int) ($qlon*$dec))/$dec+$dx;
   $y_mn = ((int) ($qlat*$dec))/$dec-$dy;
   $y_mx = ((int) ($qlat*$dec))/$dec+$dy;

   $dx2 = 0.1;  

   $nx = (int) ($dx*2/$dx2) + 1;   
   $ny = (int) ($dy*2/$dx2) + 1;
/*   for ($ix =0; $ix<100000; $ix++) {
   echo "$x_mn,$x_mx,$y_mn,$y_mx\n";
   }*/
   
   $intensity_file = "intensity_map.xyz";
   $fh = fopen($intensity_file,'w');
   for ($ix =0; $ix <$nx; $ix++) {
     $x = $x_mn+$dx2*(float)$ix;
     for ($iy = 0; $iy < $ny; $iy++) {
       $y = $y_mn+$dx2*(float)$iy;
       
       $dist_xy = distance($qlon,$qlat,$x,$y);
       $dist_xyz = sqrt($dist_xy*$dist_xy + $qdep*$qdep);
       

       $pga = exp($a*$dist_xyz + $b*$qmag + $c*1.8)/5.;
       fwrite($fh,"$x , $y , $pga, $dist_xyz \n");
     }

   }

   $sfile=$edir."/stations.xyz";

   $contents = file($sfile);
  
   foreach ($contents as $line_num => $line) {
     list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],$pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
     for ($j=0;$j<=4;$j++) {
       if ($pgah[$j]>$smag) {$smag=$pgah[$j];}
       if ($pgaz[$j]>$smag) {$smag=$pgaz[$j];}
     }
     for ($ix =-1; $ix<=1 ; $ix++) {
      $x = $slon + $dx2/3.*(float) $ix;
      for ($iy = -1; $iy<=1 ; $iy++) {
       $y = $slat + $dx2/3.*(float) $iy;
       $ssmag = $smag*2.;
       fwrite($fh,"$x , $y, $ssmag\n");
       fwrite($fh,"$x , $y, $ssmag\n");
       fwrite($fh,"$x , $y, $ssmag\n");
       fwrite($fh,"$x , $y, $ssmag\n");
       fwrite($fh,"$x , $y, $ssmag\n");
      }
     }
   }
   fclose($fh);

}

function distance($lng1, $lat1, $lng2, $lat2)
{
      $pi80 = M_PI / 180;
      $lat3 = $lat1*$pi80;
      $lng3 = $lng1*$pi80;
      $lat4 = $lat2*$pi80;
      $lng4 = $lng2*$pi80;
       
      $r = 6372.797; // mean radius of Earth in km
      $dlat = $lat4 - $lat3;
      $dlng = $lng4 - $lng3;
      $a = sin($dlat / 2) * sin($dlat / 2) + cos($lat3) * cos($lat3) * sin($dlng / 2) * sin($dlng / 2);
      $c = 2 * atan2(sqrt($a), sqrt(1 - $a));
      $km = $r * $c;
      return $km;
}


?>


