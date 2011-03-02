<?php

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
