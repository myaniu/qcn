<?
/* XML format is:

<?xml version="1.0" encoding="UTF-8"?>
<Quake>
<QuakeMagnitude>6.9</QuakeMagnitude>
<QuakeLon>-30</QuakeLon>
<QuakeLat>180</QuakeLat>
<QuakeDepth>10</QuakeDepth>
<QuakeOriginTime>”UNIXTIMEHERE”</QuakeOriginTime>
<QuakeID>12345</QuakeID>
</Quake>

*/

$xml = $_POST["xml"];

if ($xml) {
  echo $xml;
}
else {
   header("HTTP/1.0 400 Bad Request");
   echo "Invalid data";
}

?>
