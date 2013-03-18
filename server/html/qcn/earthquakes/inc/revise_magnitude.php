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

    //echo "distance:".$dis."\n";
    //echo "slon:".$slon."\n";

    if($dis < 0.01) $dis = 0.01;

    $ptime = $ttime - $etime - $dis/4.9; if ($ptime < 0.01) $ptime = 0.01;

    $tau_dis =  $ptime/sqrt($dis);

    //echo "P-time:".$ptime."\n";

        $x = $tau_dis;
        $pi = 3.1415927; 
        $a = (8*($pi - 3))/(3*$pi*(4 - $pi)); 
        $x2 = $x * $x; 

        $ax2 = $a * $x2; 
        $num = (4/$pi) + $ax2; 
        $denom = 1 + $ax2; 

        $inner = (-$x2)*$num/$denom; 
        $erf2 = 1 - exp($inner); 

        $erf2 = sqrt($erf2);
        $erf_tau = $erf2;

        if($erf_tau < 0) $erf_tau = -$erf_tau;

    //echo "erf_tau:".$erf_tau."\n";
    //echo "tau_dis:".$tau_dis."\n";





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


    //#7 (Updated 1/11/12):
    //$qmag = $qmag + exp((log($smag) + 0.0011*$dis*$dis - 0.2414*log($ptime) + 7.1426)/4.0736);

    //#8 (Updated 1/11/12):
    //$qmag = $qmag + ((log($smag) + 0.0011*$dis*$dis - 0.2414 * log($ptime) + 5.0244)/0.8770);

   //#9 - Mag inversion (Updated 1/11/12):
    // >> Using previously: $qmag2 = $qmag2 + ((0.2469*log($smag) + 0.0244*$dis - 0.0653 * log($ptime) + 4.1165));
   
    //#10 - Mag inversion (Updated 1/11/12):
    //$qmag = $qmag + ((0.2398*log($smag) + 0.2156*sqrt($dis) - 0.0635 * log($ptime) + 3.6529));

    //$qmag = ($qmag + 1*$qmag2)/2;


    //#11 (Updated 6/29/12):
    //$qmag2 = $qmag2 + ((0.1527*log($smag) + 0.2643*sqrt($dis) - 0.0073*sqrt($ptime) + 3.7102));

    //#12 (Updated 10/08/12):
    //$qmag2 = $qmag2 + exp((0.0449*log($smag) + 0.0508*sqrt($dis) - 0.0044*sqrt($ptime/(sqrt($dis))) + 1.3857));

    

    //#13 (Updated 10/22/12) <- previous best:
    //$qmag2 = $qmag2 + ((0.2064*log($smag) + 0.2762*sqrt($dis) - 0.1054*sqrt($ptime) + 4.0789));



    //#14 (Updated 10/23/12):
    //$qmag2 = $qmag2 + exp((0.0476*log($smag) + 0.0586*sqrt($dis) - 0.0463*sqrt($ptime/sqrt($dis)) + 1.42));

    //#15 (Updated 10/24/12):
    //$qmag2 = $qmag2 + exp((0.0473*log($smag) + 0.0517*sqrt($dis) - 0.0845*sqrt($ptime/($dis)) + 1.4429));

    //#16 (Updated 10/25/12) using fine inversion:
    //$qmag2 = $qmag2 + exp((0.0514*log($smag) + 0.0584*sqrt($dis) - 0.0927*$erf_tau + 1.4538));

    //#17 (Updated 11/02/12) - using fine inversion & solve for tau separately:
    //$qmag2 = $qmag2 + exp((0.0449*log($smag) + 0.0575*sqrt($dis) - 0.0320*$erf_tau + 1.3822));
    
    //#18 (Updated 11/02/12) - solve for tau separately; CHCH data only:
    //$qmag2 = $qmag2 + exp((0.0451*log($smag) + 0.0575*sqrt($dis) - 0.336*$erf_tau + 1.3967));

    //#19 (Updated 11/13/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only:
    //$qmag2 = $qmag2 + exp(0.0466*log($smag) + 0.0725*sqrt($dis) - 0.0293*$erf_tau + 1.3527);

    //#20 (Updated 11/14/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events <- works well for large M, terrible overall:
    //$qmag2 = $qmag2 + exp(0.0498*log($smag) + 0.0584*sqrt($dis) - 0.0663*$erf_tau + 1.4887);

    //#21 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#2):
    //$qmag2 = $qmag2 + exp(0.0588*log($smag) + 0.0673*sqrt($dis) - 0.0727*$erf_tau + 1.4351);

    //#22 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#3) <- best for large Mag:
    //$qmag2 = $qmag2 + exp(0.0565*log($smag) + 0.0689*sqrt($dis) - 0.0689*$erf_tau + 1.4195);

    //#23 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#4):
    //$qmag2 = $qmag2 + exp(0.0566*log($smag) + 0.0663*sqrt($dis) - 0.0586*$erf_tau + 1.4205);

    //#23.2 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#4.2):
    //$qmag2 = $qmag2 + exp(0.0622*log($smag) + 0.0697*sqrt($dis) - 0.0463*$erf_tau + 1.4177);

    //#24 (Updated 11/16/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#5):
    //$qmag2 = $qmag2 + exp(0.0501*log($smag) + 0.0702*sqrt($dis) - 0.0613*$erf_tau + 1.3928);

    //#25 (Updated 11/20/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#6):
    //$qmag2 = $qmag2 + exp(0.0695*log($smag) + 0.0650*sqrt($dis) - 0.1070*$erf_tau + 1.5343);

    //#26 (Updated 11/20/12) - using fine inversion & solve for tau separately; GNS events (#6):
    //$qmag2 = $qmag2 + exp(0*log($smag) + 0.0550*sqrt($dis) - 0.0055*$erf_tau + 1.4233);

    //#27 (Updated 11/28/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#7)):
    //$qmag2 = $qmag2 + exp(0.0499*log($smag) + 0.0641*sqrt($dis) - 0.0172*$erf_tau + 1.3561);

    //#28 (Updated 11/28/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#8)) <- very bad:
    //$qmag2 = $qmag2 + exp(0.0035*log($smag) + 0.0591*sqrt($dis) - 0.0204*$erf_tau + 1.2278);

    //#29 (Updated 11/28/12) - using fine inversion & solve for tau separately; ALL QCN & Mexico events, revised (#8)) <- very bad:
    //$qmag2 = $qmag2 + exp(0.0381*log($smag) + 0.0620*sqrt($dis) - 0.0296*$erf_tau + 1.345);

    //#30 (Updated 12/1/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#8)) <- very bad:
    //$qmag2 = $qmag2 + exp(0.0380*log($smag) + 0.0611*sqrt($dis) - 0.0306*$erf_tau + 1.3472);

    //#31 (Updated 12/1/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#8)):
    //$qmag2 = $qmag2 + exp(0.0401*log($smag) + 0.0622*sqrt($dis) - 0.0312*$erf_tau + 1.3527);

    //#32 (Updated 02/1/13) - using fine inversion & solve for tau separately; FEW QCN events, revised (#8)):
    //$qmag2 = $qmag2 + exp(0.0194*log($smag) + 0.0455*sqrt($dis) - 0.0487*$erf_tau + 1.3503);

    //#33 (Updated 02/8/13) - using fine inversion; Picked QCN events, revised (#8)):
    //$qmag2 = $qmag2 + exp(0.0433*log($smag) + 0.0667*sqrt($dis) - 0.0775*$erf_tau + 1.3819);

    //#34 (Updated 02/11/13) - using fine inversion & solve for tau separately; Picked QCN events, revised (#8)):
    //$qmag2 = $qmag2 + exp(0.0399*log($smag) + 0.0669*sqrt($dis) - 0.0193*$erf_tau + 1.3159);

    //#35 (Updated 02/14/13) - using fine inversion; Picked QCN events #2, revised <- BAD:
    //$qmag2 = $qmag2 + exp(0.0560*log($smag) + 0.1250*sqrt($dis) - 0.1044*$erf_tau + 1.3845);

    //#36 (Updated 02/20/13) - Totally new inversion technique. Solve each part separately. Use Picked data:
    //$qmag2 = $qmag2 + ((log($smag)/($erf_tau)) + 0.59*(sqrt($dis)-sqrt(10)) + 8.328 )/(1.275);

    //#37 (Updated 03/04/13) - using fine inversion; Picked GNS events) <- BEST:
    $qmag2 = $qmag2 + exp(0.0211*log($smag) + 0.0173*sqrt($dis) - 0.0267*$erf_tau + 1.4444);

    //#38 (Updated 03/04/13) - GNS: Totally new inversion technique. Solve each part separately. Use Picked data <-not working: divide by zero?:
    //$qmag2 = $qmag2 + ((log($smag)/($erf_tau)) + 0.596*(sqrt($dis)-sqrt(10)) + 6.607 )/(1.137);

    //#39 (Updated 03/17/13) - QCN w/ QCN locations: Totally new inversion technique. Solve each part separately. Use Picked data <- not working:
    //$qmag2 = $qmag2 + ((log($smag)/($erf_tau)) + 0.596*(sqrt($dis)-sqrt(10)) + 7.882 )/(1.161);

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

   //#3:
   //$qstd = $qstd + (((log($smag) + 0.00081543*$dis*$dis - 0.2752 * log($ptime) + 5.9461)/1.1479) - $qmag)^2;

   //#4:
   //$qstd = $qstd + (exp((log($smag) + 0.0432*$dis - 0.2752 * log($ptime) + 8.5438)/5.52) - $qmag)^2;

   //#5 - Mag inversion:
   //$qstd2 = $qstd2 + ((0.3199*log($smag) + 0.0334*$dis - 0.0880 * log($ptime) + 4.086) - $qmag)^2;

  //#6 - Mag inversion:
   //$qstd = $qstd + ((0.3096*log($smag) + 0.2914*sqrt($dis) - 0.0852 * log($ptime) + 3.4591) - $qmag)^2;



   //#7 (Updated 1/11/12):
   //$qstd = $qstd + (exp((log($smag) + 0.0011*$dis*$dis - 0.2414 * log($ptime) + 7.1426)/4.0736) - $qmag)^2;

   //#8 (Updated 1/11/12):
   //$qstd = $qstd + (((log($smag) + 0.0011*$dis*$dis - 0.2414 * log($ptime) + 5.0244)/0.8770) - $qmag)^2;

   //#9 - Mag inversion (Updated 1/11/12):
   // >> Used previously: $qstd2 = $qstd2 + ((0.2469*log($smag) + 0.0244*$dis - 0.0653 * log($ptime) + 4.1165) - $qmag)^2;

  //#10 - Mag inversion (Updated 1/24/12):
   //$qstd = $qstd + ((0.2398*log($smag) + 0.2156*sqrt($dis) - 0.0635 * log($ptime) + 3.6529) - $qmag)^2;

   //$qstd = ($qstd + 1*$qstd2)/2;


   //#11 (Updated 6/29/12):
   //$qstd2 = $qstd2 + (0.1527*log($smag) + 0.2643*sqrt($dis) - 0.0073*sqrt($ptime) + 3.7102);


   //#12 (Updated 10/08/12) - using all events:
   //$qstd2 = $qstd2 + exp(0.0449*log($smag) + 0.0508*sqrt($dis) - 0.0044*sqrt($ptime/(sqrt($dis))) + 1.3857);




   //#13 (Updated 10/22/12) - using fine inversion <- previous best:
   //$qstd2 = $qstd2 + (0.2064*log($smag) + 0.02762*sqrt($dis) - 0.1054*sqrt($ptime) + 4.0789);
   



   //#14 (Updated 10/23/12) - using fine inversion:
   //$qstd2 = $qstd2 + exp(0.0476*log($smag) + 0.0586*sqrt($dis) - 0.0463*sqrt($ptime/sqrt($dis)) + 1.42);

   //#15 (Updated 10/24/12)
   //$qstd2 = $qstd2 + exp(0.0473*log($smag) + 0.0517*sqrt($dis) - 0.0845*sqrt($ptime/($dis)) + 1.4429);


   //#16 (Updated 10/25/12) - using fine inversion:
   //$qstd2 = $qstd2 + exp(0.0514*log($smag) + 0.0584*sqrt($dis) - 0.0927*$erf_tau + 1.4538);

   //#17 (Updated 11/02/12) - using fine inversion & solve for tau separately:
   //$qstd2 = $qstd2 + exp(0.0449*log($smag) + 0.0575*sqrt($dis) - 0.0320*$erf_tau + 1.1322);

   //#18 (Updated 11/02/12) - using fine inversion & solve for tau separately; CHCH data only:
   //$qstd2 = $qstd2 + exp(0.0451*log($smag) + 0.0575*sqrt($dis) - 0.0336*$erf_tau + 1.3967);

   //#19 (Updated 11/13/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only:
   //$qstd2 = $qstd2 + exp(0.0466*log($smag) + 0.0725*sqrt($dis) - 0.0293*$erf_tau + 1.3527);

    //#20 (Updated 11/14/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events <- works well for large M, terrible overall:
    //$qstd2 = $qstd2 + exp(0.0498*log($smag) + 0.0584*sqrt($dis) - 0.0663*$erf_tau + 1.4887);

    //#21 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#2):
    //$qstd2 = $qstd2 + exp(0.0588*log($smag) + 0.0673*sqrt($dis) - 0.0727*$erf_tau + 1.4351);

    //#22 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#3):
    //$qstd2 = $qstd2 + exp(0.0565*log($smag) + 0.0689*sqrt($dis) - 0.0689*$erf_tau + 1.4195);

    //#23 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#4):
    //$qstd2 = $qstd2 + exp(0.0566*log($smag) + 0.0663*sqrt($dis) - 0.0586*$erf_tau + 1.4205);

    //#23.2 (Updated 11/15/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#4.2):
    //$qstd2 = $qstd2 + exp(0.0622*log($smag) + 0.0697*sqrt($dis) - 0.0463*$erf_tau + 1.4177);

    //#24 (Updated 11/16/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#5):
    //$qstd2 = $qstd2 + exp(0.0501*log($smag) + 0.0702*sqrt($dis) - 0.0613*$erf_tau + 1.3928);

    //#25 (Updated 11/20/12) - using fine inversion & solve for tau separately; Limited # of CHCH events only & Mexico events (#6):
    //$qstd2 = $qstd2 + exp(0.0695*log($smag) + 0.0650*sqrt($dis) - 0.1070*$erf_tau + 1.5343);

    //#26 (Updated 11/20/12) - using fine inversion & solve for tau separately; GNS events (#6):
    //$qstd2 = $qstd2 + exp(0*log($smag) + 0.0550*sqrt($dis) - 0.0055*$erf_tau + 1.4233);

    //#27 (Updated 11/28/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#7)):
    //$qstd2 = $qstd2 + exp(0.0499*log($smag) + 0.0641*sqrt($dis) - 0.0172*$erf_tau + 1.3561);


    //#28 (Updated 11/28/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#8)) <- very bad:
    //$qstd2 = $qstd2 + exp(0.0035*log($smag) + 0.0591*sqrt($dis) - 0.0204*$erf_tau + 1.2278);

    //#29 (Updated 11/28/12) - using fine inversion & solve for tau separately; ALL QCN & Mexico events, revised (#8)) <- very bad:
    //$qstd2 = $qstd2 + exp(0.0037*log($smag) + 0.0571*sqrt($dis) - 0.0300*$erf_tau + 1.241);
    //$qstd2 = $qstd2 + exp(0.0381*log($smag) + 0.0620*sqrt($dis) - 0.0296*$erf_tau + 1.345);

    //#30 (Updated 12/1/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#8)) <- very bad:
    //$qstd2 = $qstd2 + exp(0.0380*log($smag) + 0.0611*sqrt($dis) - 0.0306*$erf_tau + 1.3472);

    //#31 (Updated 12/1/12) - using fine inversion & solve for tau separately; ALL QCN events, revised (#8)):
    //$qstd2 = $qstd2 + exp(0.0401*log($smag) + 0.0622*sqrt($dis) - 0.0312*$erf_tau + 1.3527);

    //#32 (Updated 02/1/13) - using fine inversion & solve for tau separately; FEW QCN events, revised (#8)):
  //  $qstd2 = $qstd2 + exp(0.0194*log($smag) + 0.0455*sqrt($dis) - 0.0487*$erf_tau + 1.3503);

//----- 

    //#33 (Updated 02/8/13) - using fine inversion; Picked QCN events, revised (#8)):
    //$qstd2 = $qstd2 + exp(0.0433*log($smag) + 0.0667*sqrt($dis) - 0.0775*$erf_tau + 1.3819);

    //#34 (Updated 02/11/13) - using fine inversion & solve for tau separately; Picked QCN events, revised (#8)):
    //$qstd2 = $qstd2 + exp(0.0399*log($smag) + 0.0669*sqrt($dis) - 0.0193*$erf_tau + 1.3159);

    //#35 (Updated 02/14/13) - using fine inversion; Picked QCN events #2, revised <- BAD:
    //$qstd2 = $qstd2 + exp(0.0560*log($smag) + 0.1250*sqrt($dis) - 0.1044*$erf_tau + 1.3845);

    //#36 (Updated 02/20/13) - Totally new inversion technique. Solve each part separately. Use Picked data:
    //$qstd2 = $qstd2 + ((log($smag)/($erf_tau)) + 0.59*(sqrt($dis)-sqrt(10)) + 8.328 )/(1.275);

    //#37 (Updated 03/04/13) - using fine inversion; Picked GNS events) <- BEST:
    $qstd2 = $qstd2 + exp(0.0211*log($smag) + 0.0173*sqrt($dis) - 0.0267*$erf_tau + 1.4444);

    //#38 (Updated 03/04/13) - GNS: Totally new inversion technique. Solve each part separately. Use Picked data <- not working: divide by zero?:
    //$qstd2 = $qstd2 + ((log($smag)/($erf_tau)) + 0.596*(sqrt($dis)-sqrt(10)) + 6.607 )/(1.137);

    //#39 (Updated 03/17/13) - QCN w/ QCN locations: Totally new inversion technique. Solve each part separately. Use Picked data <- not working:
    //$qstd2 = $qstd2 + ((log($smag)/($erf_tau)) + 0.596*(sqrt($dis)-sqrt(10)) + 7.882 )/(1.161);

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

