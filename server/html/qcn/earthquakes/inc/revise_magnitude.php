<?php

$edir      = getcwd();

revise_magnitude($edir);

function revise_magnitude($edir) {

   $efile = $edir."/event_safe.xy";
   if (file_exists($efile)) {}else{return;}
   $contents = file($efile);
   $string = implode($contents);
   $string = str_replace("\n",'',$string);
   list($qlon,$qlat,$qdep,$qmag,$ntrig,$etime,$dtime,$qstd,$r_squared,$misfit) = split('[,]',$string);
   echo "HI $qstd\n";
   if ($qstd==null) {
      $qstd=0.0;$r_squared=0.0;$misfit=0.0;
   }
   $sfile=$edir."/stations.xyz";

   $contents = file($sfile);
  echo $qmag;
  $qmag = 0;
  $i = 0;
  
  foreach ($contents as $line_num => $line) {
   $i++;
   list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],$pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
   for ($j=0;$j<=4;$j++) {
    if ($pgah[$j]>$smag) {$smag=$pgah[$j];}
    if ($pgaz[$j]>$smag) {$smag=$pgaz[$j];}
   }
   $qmag = $qmag + 0.03085*$dis + 4.28 + log($smag*5.)/1.838;
  }

  $qmag = $qmag / ($i+1.);
  if ($i < 9) {$qmag=$qmag*.9;}
  foreach ($contents as $line_num => $line) {
   list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],$pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
   for ($j=0;$j<=4;$j++) {
    if ($pgah[$j]>$smag) {$smag=$pgah[$j];}
    if ($pgaz[$j]>$smag) {$smag=$pgaz[$j];}
  }
   $qstd = $qstd + (0.03085*$dis + 4.28 + log($smag*5.)/1.838 - $qmag)^2;
  }
  $qstd = sqrt($qstd/$i);
  echo "MAG=$qmag +/- $qstd\n";
  $fh = fopen($efile,'w');
  $txtout = "$qlon,$qlat,$qdep,".number_format($qmag,2,'.','').",$ntrig,$etime,$dtime,".number_format($qstd,2,'.','');
  $txtout = $txtout.",".number_format($r_squared,2,'.','').",".number_format($misfit,2,'.','');
  $txtout = $txtout."\n";
  fwrite($fh,$txtout);
  fclose($fh);
}


?>

