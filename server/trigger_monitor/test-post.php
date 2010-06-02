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
$filename = "trigger.xml";

if ($xml) {
  //echo $xml;
  $fs = fopen($filename, "a");
  if ($fs) {
     $numbytes = fwrite($fs, $xml);
     fclose($fs);
     if (!$numbytes) {
         header("HTTP/1.0 400 File Open Error");
         echo "Error: File Open";
     }
     else {
         header("HTTP/1.0 200 OK");  // we wrote the xml to our local file
         echo "OK";
     }
  }
  else {
     header("HTTP/1.0 400 File Open Error");
     echo "Error: File Open 2";
  } 
}
else {
   header("HTTP/1.0 400 Bad Request");
   echo "Error: Bad Request";
}

/*
print("<pre>");
print_r($_POST);
print("</pre>");
*/
?>
