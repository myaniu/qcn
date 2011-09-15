<?php

require_once("../project/project.inc");

$test=NULL;

function earthquake_email($mag,$elon,$elat,$edep,$n_stations,$etime,$edir,$dtime,$dt_detect,$test=null)
{
 chdir(BASEPATH . "/boinc/sensor/html/user/");      // Only works from within this directory (relative paths)

 require_once(BASEPATH . "/boinc/sensor/html/inc/email.inc"); // Include email info
 require_once(BASEPATH . "/boinc/sensor/html/inc/distance.inc"); // Include distance function (km)


// Set the regional distance (maximum distance for using regional magnitude):
 $dist_local = 100;



// html table cell formatting similar to USGS ENS formatting
 $tr = "<tr>";
 $td1 = "<tr><td width=\"30\" bgcolor=\"#eeeeee\" padding=\"1\">";
 $td2 = "</strong></td><td width=\"100\" bgcolor=\"#eeeeee\" padding=\"1\">";
 $rt  = "</td></tr>";



//Set Message:
$message = "<h1><strong>QCN Rapid Earthquake Detection: M$mag</strong></h1></p>
 <table width=\"290\" border=\"0\">                      
 $td1 Date:  $td2 ".date('M d Y H:i:s',$etime)." $rt
 $td1 Lon:   $td2 $elon $rt
 $td1 Lat:   $td2 $elat $rt
 $td1 Depth: $td2 $edep $rt
 $td1 Detected: $td2 ".date('M d Y H:i:s',$dtime)." at $dt_detect seconds after the origin time. $rt
 $td1 Stations: $td2  $n_stations $rt
 $td1 Link:  $td2 <a href=\"" . BASEURL . "/earthquakes/$edir\">" . BASEURL . "/earthquakes/$edir</a> $rt
 $td1        $td2<img src=\"" . BASEURL . "/earthquakes/$edir/intensity_03.jpg\" width=\"300\"> $rt
 </table>
 <p>This notice is for scientific use only (not for use as an earthquake early warning tool). This message is generated automatically by the Quake-Catcher Network&#39s Rapid Earthquake Detection System (REDS).  In some cases, REDS compromises accuracy for speed.  Updates to the magnitude are usually available even before you receive the message.  For more information about the earthquake, please click <a href=\"" . BASEURL . "/earthquakes/$edir\">here</a>. For more information about REDS, <a href=\"" . BASEURL . 
    "/earthquakes/REDS.php\">click here</a>.";



// Set email recipient Name:
 $name = "QCN Participant";



// Email list for Rapid Earthquake Detection System
 if($test) {
    $file_handle = fopen(BASEPATH . "/qcn/earthquakes/inc/earthquake_email_test.list",  "r"); // File with list of emails/parametrs for testing
    $message     = "<p>This is a TEST. This is NOT an earthquake.</h1></p>\n".$message;       // Add note at top that this is only a test
 
} else {
    $file_handle = fopen(BASEPATH . "/qcn/earthquakes/inc/earthquake_email.list",  "r"); // File with list of emails/parametrs
 }


    // setup an object to simulate the boinc user record (just need email addr, name, & id)
    class tmpUser {
      public $email_addr;
      public $name;
      public $id;
      public function __construct($em, $name, $id)
      {
            $this->email_addr = $em;
            $this->name = $name;
            $this->id = $id;
      }
    };

// For each entry in the email list, determine if we should email or not: 
 while (!feof($file_handle)) {

    $line = fgets($file_handle);                                          // Read line from file

    $line = str_replace("\n","",$line);                                   // Remove hard enter at end of line

    list($email,$slon,$slat,$mag_global,$mag_local) = split('[,]',$line); // Parse line into variables

    if (strlen($email)<6) { break 1; }                                    // Skip email if too short
    //echo "$email\n$slon\n$slat\n$mag_global\n$mag_local\n";               // Status report

    $dist_km = distance($slon,$slat,$elon,$elat);
 
    $myuser = new tmpUser($email, $name, $id);

    if (($dist_km<$dist_local)&&($mag>$mag_local)) { 

       $subject = "QCN Local Quake: M$mag - ".date('M d Y H:i:s',$etime);// Set email subject:
       if ($test) { $subject = "TEST ONLY:".$subject;}                     // Note this is only a test
       send_email($myuser, $subject, "", $message);
       //print "Sending email to " . $myuser->email_addr . "\n";

    } else {

      if  ($mag>$mag_global) {

         $subject = "QCN Global Quake: M$mag - ".date('M d Y H:i:s',$etime);// Set email subject:
         if ($test) { $subject = "TEST ONLY:".$subject;}                     // Note this is only a test
         send_email($myuser, $subject, "", $message);
        //  print "Sending email to " . $myuser->email_addr . "\n";
      }
    }
}

}
?>
