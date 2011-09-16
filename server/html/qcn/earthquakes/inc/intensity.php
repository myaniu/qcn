<?php

function intensity() {

   $edir      = getcwd();
   $efile = $edir."/event.xy";
   $contents = file($efile);
   $string = implode($contents);
   list($qlon,$qlat,$qdep,$qmag,$ntrig,$etime,$dtime,$qstd,$r_squared,$misfit) = split('[,]',$string);

   $a = -0.05;  // Distance term
   $b =  1.8385; // Magnitude term
   $c = -7.8671; // Constant

   $width = ((int) (log10($qmag*$qmag)*100.) / 100.);

   $dec = 100;
   $dI  = 1./$dec;
   $dy = ((int) (log10($qmag*$qmag)*$dec) / $dec); 

   $lon_fact = (cos($qlat*pi()/180.)); $lon_fact*=$lon_fact;
   $dx = ((int) ($dy/$lon_fact)*$dec) / $dec;

   $x_mn = ((int) ($qlon*$dec))/$dec-$dx;
   $x_mx = ((int) ($qlon*$dec))/$dec+$dx;
   $y_mn = ((int) ($qlat*$dec))/$dec-$dy;
   $y_mx = ((int) ($qlat*$dec))/$dec+$dy;

   $dx2 = 0.1;  

   $nx = (int) ($dx/$dx2*2) + 1;   
   $ny = (int) ($dy/$dx2*2) + 1;

   $intensity_file = "intensity_map.xyz";
   $fh = fopen($intensity_file,'w');
   for ($ix =0; $ix <$nx; $ix++) {
     $x = $x_mn+$dx2*(float)$ix;
     for ($iy = 0; $iy < $ny; $iy++) {
       $y = $y_mn+$dx2*(float)$iy;
       
       $dist_xy = distance($qlon,$qlat,$x,$y);
       $dist_xyz = sqrt($dist_xy*$dist_xy + $qdep*$qdep);
       
       $pga = exp($a*$dist_xyz + $b*$qmag + $c);
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
     fwrite($fh,"$slon , $slat, $smag\n");
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
