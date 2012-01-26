<?php

$edir      = getcwd();

echo $edir;

revise_magnitude($edir);

function revise_magnitude($edir) {
   
   $efile = $edir."/event.xy";
   //$efile = $edir."/event_safe.xy";
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
  echo "Initial Mag:".$qmag."\n";
  $qmag = 0;
  $i = 0;
  
  foreach ($contents as $line_num => $line) {
   $i++;
   echo $i;
   list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],$pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
   for ($j=0;$j<=4;$j++) {
    if ($pgah[$j]>$smag) {$smag=$pgah[$j];}
    if ($pgaz[$j]>$smag) {$smag=$pgaz[$j];}
   }

    if($dis < 0.1) $dis = 0.1;

    $ptime = $ttime - $etime - $dis/4.9; if ($ptime < 0.1) $ptime = 0.1; 


   //Orig:
    //$qmag = $qmag + 0.03085*$dis + 4.28 + log($smag*5.)/1.838;

    //#1:
    //$qmag = $qmag + exp((log($smag) + 0.00046*$dis*$dis - 0.2381*log($ptime/$dis) + 7.9727)/5.691);

    //#2:
    //$qmag = $qmag + exp((log($smag) + 0.0008171*$dis*$dis - 0.2752*log($ptime) + 8.6591)/5.2864);

    //#3:
    //$qmag = $qmag + ((log($smag) + 0.00081543*$dis*$dis - 0.2752 * log($ptime) + 5.9461)/1.1479);

    //#4:
    //$qmag = $qmag + exp((log($smag) + 0.0432*$dis - 0.2752 * ($ptime) + 8.5438)/5.52);

    //#5 - Mag inversion:
    //$qmag2 = $qmag2 + ((0.3199*log($smag) + 0.0334*$dis - 0.0880 * log($ptime) + 4.086));

    //#6 - Mag inversion:
    //$qmag = $qmag + ((0.3096*log($smag) + 0.2914*sqrt($dis) - 0.0852 * log($ptime) + 3.4591));


    //#2 (Updated 1/11/12):
    //$qmag = $qmag + exp((log($smag) + 0.0011*$dis*$dis - 0.2414*log($ptime) + 7.1426)/4.0736);

    //#3 (Updated 1/11/12):
    //$qmag = $qmag + ((log($smag) + 0.0011*$dis*$dis - 0.2414 * log($ptime) + 5.0244)/0.8770);

   //#5 - Mag inversion (Updated 1/11/12):
    $qmag2 = $qmag2 + ((0.2469*log($smag) + 0.0244*$dis - 0.0653 * log($ptime) + 4.1165));
   
    //#6 - Mag inversion (Updated 1/11/12):
    //$qmag = $qmag + ((0.2398*log($smag) + 0.2156*sqrt($dis) - 0.0635 * log($ptime) + 3.6529));

    //$qmag = ($qmag + 1*$qmag2)/2;

    $qmag = $qmag2;


    echo "mag=".$qmag."  \n";
  }

  $qmag = $qmag / ($i+1.);
  //if ($i < 9) {$qmag=$qmag*.9;}
  $qstd = 0;
  foreach ($contents as $line_num => $line) {
   list($slon,$slat,$smag,$hid,$tid,$sfile,$ttime,$rtime,$sig,$dis,$pgah[0],$pgaz[0],$pgah[1],$pgaz[1],$pgah[2],$pgaz[2],$pgah[3],$pgaz[3],$pgah[4],$pgaz[4]) = split('[,]',$line);
   for ($j=0;$j<=4;$j++) {
    if ($pgah[$j]>$smag) {$smag=$pgah[$j];}
    if ($pgaz[$j]>$smag) {$smag=$pgaz[$j];}
  }

    if ($dis < 0.1) $dis = 0.1;
    $ptime = $ttime - $etime - $dis/4.9; if ($ptime < 0.1) $ptime = 0.1; 



   //Orig:
   //$qstd = $qstd + (0.03085*$dis + 4.28 + log($smag*5.)/1.838 - $qmag)^2;

   //#1:
   //$qstd = $qstd + (exp((log($smag) + 0.00046*$dis*$dis - 0.2381 * log($ptime/$dis) + 7.9727)/5.691) - $qmag)^2;

   //#2:
   //$qstd = $qstd + (exp((log($smag) + 0.0008171*$dis*$dis - 0.2752 * log($ptime) + 8.6591)/5.2864) - $qmag)^2;

   //3:
   //$qstd = $qstd + (((log($smag) + 0.00081543*$dis*$dis - 0.2752 * log($ptime) + 5.9461)/1.1479) - $qmag)^2;

   //#4:
   //$qstd = $qstd + (exp((log($smag) + 0.0432*$dis - 0.2752 * log($ptime) + 8.5438)/5.52) - $qmag)^2;

   //#5 - Mag inversion:
   //$qstd2 = $qstd2 + ((0.3199*log($smag) + 0.0334*$dis - 0.0880 * log($ptime) + 4.086) - $qmag)^2;

  //#6 - Mag inversion:
   //$qstd = $qstd + ((0.3096*log($smag) + 0.2914*sqrt($dis) - 0.0852 * log($ptime) + 3.4591) - $qmag)^2;



   //#2 (Updated 1/11/12):
   //$qstd = $qstd + (exp((log($smag) + 0.0011*$dis*$dis - 0.2414 * log($ptime) + 7.1426)/4.0736) - $qmag)^2;

   //3 (Updated 1/11/12):
   //$qstd = $qstd + (((log($smag) + 0.0011*$dis*$dis - 0.2414 * log($ptime) + 5.0244)/0.8770) - $qmag)^2;

   //#5 - Mag inversion (Updated 1/11/12):
   $qstd2 = $qstd2 + ((0.2469*log($smag) + 0.0244*$dis - 0.0653 * log($ptime) + 4.1165) - $qmag)^2;

  //#6 - Mag inversion (Updated 1/24/12):
   //$qstd = $qstd + ((0.2398*log($smag) + 0.2156*sqrt($dis) - 0.0635 * log($ptime) + 3.6529) - $qmag)^2;

   //$qstd = ($qstd + 1*$qstd2)/2;

   $qstd = $qstd2;


   echo "std=".$qstd."\n";
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

