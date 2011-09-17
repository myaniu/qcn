<?php
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");

 
$file = $_REQUEST["dat"];

// Check if this is a continual or sensor network station
if (preg_match("/conti/i","substr($file,0,4)")) {
 $url = UPLOADURL . "/trigger/continual/";
} else {
 $url = UPLOADURL . "/trigger/";
}
// Set file name
$file_orig = $url.$file;

// Set base directory:
$dir_base = BASEPATH . "/qcn/earthquakes/view";
$dir_sub  = basename($file,".zip");
$dir = $dir_base.'/'.$dir_sub;

// If already viewed (and image already generated), then dont remake the image:
 $jpgfile = "$dir/waveform.jpg";
if (file_exists($jpgfile)) { // CMC note - test the file, not just the dir
 rotateImage($jpgfile);
} else {

// If not already made, make image:
if (!file_exists($dir))  {
  mkdir($dir);
}

 $file_new = $dir.'/'.$file;
 //echo $file . $file_new. $file_orig;

 copy($file_orig,$file_new);
 $degrees = 90;

/*
echo "<BR>Shell Command:<BR>";
echo $dir;
echo "<BR><BR>";
echo $url;
echo "<BR><BR>";
echo $file_orig;
echo "<BR><BR>";
echo $file_new;
echo "<BR><BR>";
echo CSHELL_CMD . " " . BASEPATH . "/qcn/earthquakes/view/plot_data.sh $dir > $dir/temp.txt";
echo "<BR><BR>";
die();
*/

 if(!file_exists($file_new)) {
    echo "File does not exist: $file...\n";
 } else {
    system(SHELL_CMD . " " . BASEPATH . "/qcn/earthquakes/view/plot_data.sh $dir > $dir/temp.txt");
    $jpgfile = "$dir/waveform.jpg";
   rotateImage($jpgfile);
 }


}

// Remove all old data so it doesn't build up over time
clear_old_views($dir_base);

// Rotate the image:
function rotateImage($filename) {
// File and rotation
$degrees = 270;

// Content type
header('Content-type: image/jpeg');

// Load
$source = imagecreatefromjpeg($filename);

// Rotate
$rotate = imagerotate($source, $degrees, 0);

// Output
imagejpeg($rotate);

// Clear up memory:
imagedestroy($im);

}


function clear_old_views($dir_in) {
  $days = "3";                                 // Maximum age of "continual" or "qcn" directories. 
  $seconds = ( $days * 24 * 60 * 60 );         // Max age in seconds 
  $files = scandir($dir_in);                   // Examine directory

  foreach ($files as $num => $fname) {         // for each file / directories
    $fname2 = $dir_in."/".$fname;              // Full path & file name
    if ((preg_match("/conti/i","substr($fname,0,4)"))||(preg_match("/qcn/i","substr($fname,0,2)"))) {
      if (is_dir($fname2)) {                   // If it is a directory
        if (file_exists("$fname2")) {
         $dtime = (time()-filemtime("$fname2"));
          if ($dtime  > $seconds) {            // If the directory is older than 3 days

            system("rm -r $fname2");
          }
        }
      }
    }
  }
}


?>

