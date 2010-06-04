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

function unserialize_xml($input, $callback = null, $recurse = false)
/* bool/array unserialize_xml ( string $input [ , callback $callback ] )
 * Unserializes an XML string, returning a multi-dimensional associative array, optionally runs a callback on all non-array data
 * Returns false on all failure
 * Notes:
    * Root XML tags are stripped
    * Due to its recursive nature, unserialize_xml() will also support SimpleXMLElement objects and arrays as input
    * Uses simplexml_load_string() for XML parsing, see SimpleXML documentation for more info
 */
{
    // Get input, loading an xml string with simplexml if its the top level of recursion
    $data = ((!$recurse) && is_string($input))? simplexml_load_string($input): $input;
    // Convert SimpleXMLElements to array
    if ($data instanceof SimpleXMLElement) $data = (array) $data;
    // Recurse into arrays
    if (is_array($data)) foreach ($data as &$item) $item = unserialize_xml($item, $callback, true);
    // Run callback and return
    return (!is_array($data) && is_callable($callback))? call_user_func($callback, $data): $data;
}


$xml = $_POST["xml"];
$filename = "trigger.xml";

if ($xml) {
  //echo $xml;
  libxml_use_internal_errors();
  //$x = simplexml_load_string($xml);
  //var_dump($x);
   $x = stripslashes($xml);
   $data = unserialize_xml($x);

  if ($data && is_array($data)) {
  $fs = fopen($filename, "a");
  if ($fs) {
    $xml = sprintf("%f %s %d %f %f %f %f\n",
         $data["time"],
         $data["timeymd"],
         $data["id"],
         $data["lat"],
         $data["lon"],
         $data["dep"],
         $data["mag"]);

     $numbytes = fwrite($fs, $xml);
     fclose($fs);
     if (!$numbytes) {
         header("HTTP/1.0 400 File Open Error");
         echo "Error: File Open";
     }
     else {
         header("HTTP/1.0 200 OK");  // we wrote the xml to our local file
         print $xml;
     }
  }
  else {
     header("HTTP/1.0 400 File Open Error");
     echo "Error: File Open 2";
  } 
  }  //$data
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
