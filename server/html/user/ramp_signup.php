<?php

require_once("../project/project.inc");
require_once("../project/project_specific_prefs.inc");
require_once("../inc/utils.inc");
require_once("../inc/db.inc");
require_once("../inc/countries.inc");
require_once("../inc/translation.inc");
require_once("../inc/google_translate.inc");
require_once("../project/common.inc");


db_init();

$user = get_logged_in_user(true, true);
$psprefs = project_specific_prefs_parse($user->project_prefs);

//$db = BoincDb::get();
$row["id"] = post_int("db_id", true);

// check the ramp type  (database field ramp_type)
// valid types are G for global (default), R for regional, C for christchurch nz, M for Mexico
$row["ramp_type"] = $GLOBALS["ramp_type"];  // from a redirected page global i.e. rampnz_signup.php
if (empty($row["ramp_type"])) { // from a POST
  $row["ramp_type"] = post_str("db_ramp_type", true);
}
if (empty($row["ramp_type"])) { // from a GET
   $row["ramp_type"] = get_str("ramp_type", true);
}

//<h2>Welcome Back " . $user->name . "</h2>
$strMessage = "";

switch ($_POST["submit"]) { 

case "Submit":
   $strMessage = doRAMPSubmit($user->id, $row["id"], $row["ramp_type"]);
   break;
case "Delete":
   //break; -- no break, want to "refresh the fields"
  $sqlRAMP = "DELETE FROM qcn_ramp_participant WHERE id=" . $row["id"] . " AND userid=" . $user->id;
  $result = mysql_query($sqlRAMP);
  if ($result) {
     $strMessage = "<BR><BR><B><center><font color=black>Your RAMP information has been deleted, thanks for participating!!</font></center></b><BR><BR>";
     $row["id"] = 0;
  }
  else {
     $strMessage = "<BR><BR><B><center><font color=red>Error in deleting record - please try later</font></center></b><BR><BR>";
  }
  break;
}


  // get the row for this user
  $sqlRAMP = "SELECT * FROM qcn_ramp_participant WHERE userid=" . $user->id;
  $result = mysql_query($sqlRAMP);
  /*if (!$result) {
     echo "<BR><BR><font color=\"red\">Database Error - Try Again Later!</font><BR><BR>";
     page_tail();
     exit();
  }*/
  if ($result && mysql_num_rows($result)==1) {
      $row = mysql_fetch_assoc($result); // assoc array i.e. $row["userid"]
      mysql_free_result($result);
  }

 if (empty($row["ramp_type"])) $row["ramp_type"] = "G";
 $strRampType = $row["ramp_type"];
 if ($strRampType != "G") {
    $row["ramp_type"] = $strRampType; // get regional status from post or get
 }
  $mylat = $row["latitude"];
  $mylng = $row["longitude"];
  $zoomout = 12;

  if (!$row["id"] && ($strRampType == "R" || $strRampType == "C")) {  // use New Zealand
     $mylat = -43.5;
     $mylng = 172.6;
     $zoomout = 12;
  }
  if (!$row["id"] && ($strRampType == "M")) { // Mexico
     $mylat = 18.0;
     $mylng = -98.0;
     $zoomout = 7;
  }

//echo "<BR><BR>[" . $mylat . " , " . $mylng . "]<BR><BR>";;

  if (!$mylat || !$mylng) { 
    // use geoip
    //See if we can find the user's country and select it as default:
    require_once("../inc/geoip.inc");
    $gi = geoip_open("../inc/GeoIP.dat",GEOIP_STANDARD);
    $countrycode = geoip_country_code_by_addr($gi,$_SERVER["REMOTE_ADDR"]);
    geoip_close($gi);
    if ($countrycode) { // lookup country lat/lng
       $sqlCC = "SELECT latitude,longitude FROM qcn_country_latlng WHERE id='$countrycode'";
       $rowCC = array();
       $result = mysql_query($sqlCC);
       if ($result && mysql_num_rows($result)>0) {
          $rowCC = mysql_fetch_assoc($result); // assoc array i.e. $row["userid"]
          mysql_free_result($result);
          $mylat = $rowCC["latitude"];
          $mylng = $rowCC["longitude"];
          $zoomout = 6;
       }
    }
  }

// note this has google stuff  
page_head(null, null, null, "", true, $psprefs, false, 1, $zoomout);
google_translate_new();

$prefix = "";
if ($strRampType == "R") $prefix = " - Regional";
else if ($strRampType == "C") $prefix = " - Christchurch, NZ";
else if ($strRampType == "M") $prefix = " - Mexico";

echo "
  <center><h1>Rapid Array Mobilization Program (RAMP)$prefix</h1></center>
";

echo "
<script type=\"text/javascript\">
   function clickedAddressCopy() {
       getElement(\"addrlookup\").value = getElement(\"db_addr1\").value + \", \" + getElement(\"db_addr2\").value + \", \" + getElement(\"db_city\").value + \", \" + getElement(\"db_region\").value + \", \" + getElement(\"db_country\").value;
   }
</script>

<!-- 
   <img src=\"" . BASEURL . "/images/QCN-USB-Aftershocks_Cut.png\" align=\"right\" width=\"260\" height=\"320\" margin=\"6\">
-->
</a>
";

if ($strMessage) { // print status/error message if any
   echo $strMessage . "\n";
}

if ($strRampType == "R" || $strRampType == "C" || $strRampType == "M") echo "<center><h3>Apply for a free USB sensor if you are in a region of interest</h3></center>\n";

echo "<ul><p align=\"justify\">You can add yourself to QCN RAMP by submitting the following information,
    or edit a previous submission.
<BR>Please enter as much of the following information as you can:</p>
         <form name=\"ramp_form\" method=\"post\" action=\"ramp_signup.php\">

    <input name=\"db_id\" type=\"hidden\" id=\"db_id\" size=\"20\" value=\"" . $row["id"] . "\">
    <input name=\"db_ramp_type\" type=\"hidden\" id=\"db_ramp_type\" size=\"20\" value=\"" . $row["ramp_type"] . "\">
    <input name=\"lnm0\" type=\"hidden\" id=\"lnm0\" size=\"64\">

<table>";

     row_heading_array(array("Enter Your Postal Address (for UPS, Mail, or Hand Delivery)"));
     row2("Name (First and Last/Surname)" , 
       "<input name=\"db_fname\" type=\"text\" id=\"db_fname\" size=\"30\" value=\"" . stripslashes($row["fname"]) . "\">"
       . "&nbsp;&nbsp;<input name=\"db_lname\" type=\"text\" id=\"db_lname\" size=\"60\" value=\"" 
           . stripslashes($row["lname"]) . "\">");
     row2("Street Address (Line 1)",
              "<input name=\"db_addr1\" type=\"text\" id=\"db_addr1\" size=\"60\" value=\"" . stripslashes($row["addr1"]) . "\">");
     row2("Street Address (Line 2)",
              "<input name=\"db_addr2\" type=\"text\" id=\"db_addr2\" size=\"60\" value=\"" . stripslashes($row["addr2"]) . "\">");
     row2("City",
              "<input name=\"db_city\" type=\"text\" id=\"db_city\" size=\"60\" value=\"" . stripslashes($row["city"]) . "\">");
     row2("Region/State/Province",
              "<input name=\"db_region\" type=\"text\" id=\"db_region\" size=\"40\" value=\"" . stripslashes($row["region"]) . "\">");

     row2("Post Code", "<input name=\"db_postcode\" type=\"text\" id=\"db_postcode\" size=\"20\" value=\"" . stripslashes($row["postcode"]) . "\">");

     row2_init("Country", "<select name=db_country id=db_country>");
     if (empty($row["country"]) && ($strRampType == "R" || $strRampType == "C")) $row["country"] = "New Zealand";
     if (empty($row["country"]) && $strRampType == "M") $row["country"] = "Mexico";
     print_country_select($row["country"]);
     echo "</select></td></tr>";

     
     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Enter Your Contact Information"));
     row2("Phone Number (include country code)",
              "<input name=\"db_phone\" type=\"text\" id=\"db_phone\" size=\"40\" value=\"" . $row["phone"] . "\">");
     row2("FAX Number (include country code)",
              "<input name=\"db_fax\" type=\"text\" id=\"db_fax\" size=\"40\" value=\"" . $row["fax"] . "\">");

     $defEmail = $user->email_addr;
     if (strlen($row["email_addr"])>0) {
          $defEmail = $row["email_addr"];
     }
     row2("E-mail address (default is your QCN Account email address)",
              "<input name=\"db_email_addr\" type=\"text\" id=\"db_email_addr\" size=\"40\" value=\"" . stripslashes($defEmail) . "\">");

     // note the form field names change frm db_* as using google map api the names are lat0, lng0
     // this will ensure the contact lat/lng will be displayed in the google map below
     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Enter Your Location (Latitude, Longitude) or Use the Google Map"));
     row2("Latitude, Longitude<BR><font color=\"red\"><i>(use '-' sign for south of equator latitude or west of Greenwich longitude)</i></font>",
              "<input name=\"lat0\" type=\"text\" id=\"lat0\" size=\"20\" value=\"" . $mylat . "\">" . " , " .
              "<input name=\"lng0\" type=\"text\" id=\"lng0\" size=\"20\" value=\"" . $mylng . "\">"
     );

   // google map stuff (that isn't in ../project/project.inc
   // note the field name addrlookup should map/be stored in the database table as gmap_placename
echo "
<tr><td colspan-2>Use the following box to lookup an address (i.e. 360 Panama Mall, Stanford, CA)</td></tr>
<tr><td colspan=2>Click on the map for exact placement of the computer (zoom in if necessary)</td></tr>
<tr><td colspan=2 width=\"50\"><input type=\"text\" name=\"addrlookup\" id=\"addrlookup\" size=64 value=\""
    . $row["gmap_placename"] . "\"> 
    <input type=\"button\" name=\"btnaddress\" id=\"btnaddress\" onclick=\"clickedAddressLookup(addrlookup.value)\" value=\"Lookup Address\" size=20>
    <input type=\"button\" name=\"btncopyaddr\" id=\"btncopyaddr\" onclick=\"clickedAddressCopy()\"
   value=\"Copy Address From Above\" size=20>
</td></tr>
<tr><td colspan=2><div name=\"map\" id=\"map\" style=\"width: 800px; height: 480px\"></div></td></tr>
  ";
 
     // extra info
     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Extra Options"));

     // more checkboxes to default to true unless there is a record and it was set to false

     // bshare_map == share map location
     $bshare = " checked ";
     if ($row["id"] > 0 && $row["bshare_map"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Will you share this computer's location on the QCN Participant Map?", 
       "<input type=\"checkbox\" name=\"db_bshare_map\" id=\"db_bshare_map\" $bshare> (Contact & personal information is not shared)");
    
     // bshare_coord == share info with RAMP coordinator
     $bshare = " checked ";
     if ($row["id"] > 0 && $row["bshare_coord"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share Your Information With a Volunteer RAMP coordinator for installation?",
       "<input type=\"checkbox\" name=\"db_bshare_coord\" id=\"db_bshare_coord \" $bshare> (some sensors are installed by QCN volunteers)" );

   if ($row["ramp_type"] != "R") {  // don't use for regional ramp
     // bshare_ups == share info with UPS
     $bshare = " checked ";
     if ($row["id"] > 0 && $row["bshare_ups"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share Your Information With UPS For Faster Sensor Delivery?",
       "<input type=\"checkbox\" name=\"db_bshare_ups\" id=\"db_bshare_ups\" $bshare> (only following major earthquakes)");
  
     row2("Are you able to distribute sensors to (and help setup) other local participants?",
       "<input type=\"checkbox\" name=\"db_sensor_distribute\" id=\"db_sensor_distribute\" " . ($row["sensor_distribute"] ? "checked" : "") . "> (if so, we may contact you to help install sensors after a major earthquake)");
   }

     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Computer Information"));

     $os = array("Mac OS X (Intel)", 
                 "Mac OS X (PPC)", "Mac (Other)",
             "Windows XP", "Windows Vista", "Windows 7", "Windows (other i.e. 98)",
             "Linux", "Other");
     $os_select = "<select name=\"db_cpu_os\" id=\"db_cpu_os\">";
     for ($i = 0; $i < count($os); $i++) {
        $os_select .= "<option";
        if ($row["cpu_os"] == $os[$i]) {
           $os_select .= " selected";
        }
        $os_select .= (">" . $os[$i] . "</option>\n");
     }
     $os_select .= "</select>";

     row2("Operating System", $os_select);

     $cpuage = "<select name=\"db_cpu_age\" id=\"db_cpu_age\">";
     for ($i = 0; $i <=20; $i++) {  
         $cpuage .= "<option";
         if ($row["cpu_age"] == $i) {
            $cpuage .= " selected";
         }
         $cpuage .= (">" . $i . "</option>\n");
     }
     $cpuage .= "</select>";
     row2("Computer Age In Years<BR><i><font color=red>(0=<1 yr old)</font></i>", $cpuage." (Some older computers may have difficulty with QCN software)");

     $cpufl = "<select name=\"db_cpu_floor\" id=\"db_cpu_floor\">";
     if (!$row["db_cpu_floor"]) $row["db_cpu_floor"] = 0;
     for ($i = -10; $i <= 100; $i++) {
         $cpufl .= "<option";
         if ($row["cpu_floor"] == $i) {
            $cpufl .= " selected";
         }
         $cpufl .= (">" . $i . "</option>\n");
     }
     $cpufl .= "</select>";
     row2("Floor Location of Computer<BR><i><font color=red>(-1 = Basement, 0 = Ground Floor, 1 = First Floor etc)<font></i>", $cpufl." (ground or basement floors are ideal)");

     row2("Do You Have Administrator Rights On This Computer?",
       "<input type=\"radio\" name=\"db_cpu_admin\" id=\"db_cpu_admin\" value=0 " . ($row["cpu_admin"] == 0 ? "checked" : "") . ">No&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_admin\" id=\"db_cpu_admin\" value=1 " . ($row["cpu_admin"] == 1 ? "checked" : "") . ">Yes&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_admin\" id=\"db_cpu_admin\" value=2 " . ($row["cpu_admin"] == 2 ? "checked" : "") . ">Not Sure&nbsp&nbsp  "
       . "  (without administrator rights, one can't install the QCN software/drivers)");

     row2("Do You Have Permission To Send Seismic Data Out Of Your Country From This Computer?",
       "<input type=\"radio\" name=\"db_cpu_permission\" id=\"db_cpu_permission\" value=0 " . ($row["cpu_permission"] == 0 ? "checked" : "") . ">No&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_permission\" id=\"db_cpu_permission\" value=1 " . ($row["cpu_permission"] == 1 ? "checked" : "") . ">Yes&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_permission\" id=\"db_cpu_permission\" value=2 " . ($row["cpu_permission"] == 2 ? "checked" : "") . ">Not Sure&nbsp&nbsp  " 
       . "  (it may be illegal to disseminate seismic data in some countries)");

     row2("Is This Computer Behind A Firewall?",
       "<input type=\"radio\" name=\"db_cpu_firewall\" id=\"db_cpu_firewall\" value=0 " . ($row["cpu_firewall"] == 0 ? "checked" : "") . ">No&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_firewall\" id=\"db_cpu_firewall\" value=1 " . ($row["cpu_firewall"] == 1 ? "checked" : "") . ">Yes&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_firewall\" id=\"db_cpu_firewall\" value=2 " . ($row["cpu_firewall"] == 2 ? "checked" : "") . ">Not Sure&nbsp&nbsp  " 
       . "  (if the firewall blocks regular programs, it may block QCN)");

     row2("Is This Computer Usually Connected To The Internet?",
       "<input type=\"radio\" name=\"db_cpu_internet\" id=\"db_cpu_internet\" value=0 " . ($row["cpu_internet"] == 0 ? "checked" : "") . ">No&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_internet\" id=\"db_cpu_internet\" value=1 " . ($row["cpu_internet"] == 1 ? "checked" : "") . ">Yes&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_internet\" id=\"db_cpu_internet\" value=2 " . ($row["cpu_internet"] == 2 ? "checked" : "") . ">Not Sure&nbsp&nbsp  " 
       . " (without the Internet, QCN cannot upload data to the server) ");

     row2("Does This Computer Use A Proxy for Internet Access?",
       "<input type=\"radio\" name=\"db_cpu_proxy\" id=\"db_cpu_proxy\" value=0 " . ($row["cpu_proxy"] == 0 ? "checked" : "") . ">No&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_proxy\" id=\"db_cpu_proxy\" value=1 " . ($row["cpu_proxy"] == 1 ? "checked" : "") . ">Yes&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_proxy\" id=\"db_cpu_proxy\" value=2 " . ($row["cpu_proxy"] == 2 ? "checked" : "") . ">Not Sure&nbsp&nbsp  " 
       . "  (this can occasionally complicate installation)");

     row2("Do You Have An Uninterruptible Power Supply Attached To This Computer?",
       "<input type=\"radio\" name=\"db_cpu_unint_power\" id=\"db_cpu_unint_power\" value=0 " . ($row["cpu_unint_power"] == 0 ? "checked" : "") . ">No&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_unint_power\" id=\"db_cpu_unint_power\" value=1 " . ($row["cpu_unint_power"] == 1 ? "checked" : "") . ">Yes&nbsp&nbsp  "
       . "<input type=\"radio\" name=\"db_cpu_unint_power\" id=\"db_cpu_unint_power\" value=2 " . ($row["cpu_unint_power"] == 2 ? "checked" : "") . ">Not Sure&nbsp&nbsp  " 
       . "  (with an uninterruptible Power Supply, your computer will record even if the power fails)");



   if ($row["ramp_type"] == "R" || $row["ramp_type"] == "C" || $row["ramp_type"] == "M")  { // regional ramp specific information
     echo "<tr><td colspan=2><hr></td></tr>";
     if ($row["ramp_type"] == "R") {
        row_heading_array(array("Regional RAMP Questions"));
     }
     else if ($row["ramp_type"] == "C") {
        row_heading_array(array("Christchurch RAMP Questions"));
     }
     else if ($row["ramp_type"] == "M") {
        row_heading_array(array("Mexico RAMP Questions"));
     }

  if ($strRampType == "C" || $strRampType == "M") { // christchurch fields
   // CMC HERE - new fields
   //   `quake_damage` varchar(5) NULL DEFAULT '',
   //   `liquefaction` boolean NULL DEFAULT '0',
   // row2("Sunday <input type=\"checkbox\" name=\"db_loc_day_install_sunday\" id=\"db_loc_day_install_sunday\" " . 
   //        ($row["loc_day_install_sunday"] ? "checked" : "") . ">" ,
   //           "<select name=\"db_loc_time_install_sunday\" id=\"db_loc_time_install_sunday\">" . $time_select[0]);


     row2("Please describe damage to your home/property as a result of earthquake shaking",
       "<input type=\"radio\" name=\"db_quake_damage\" id=\"db_quake_damage\" value=\"none\" " . ($row["quake_damage"] == "none" ? "checked" : "") . ">None <BR> "
      . "<input type=\"radio\" name=\"db_quake_damage\" id=\"db_quake_damage\" value=\"low\" " . ($row["quake_damage"] == "low" ? "checked" : "") . ">Mild (little or no damage)&nbsp&nbsp<BR>  "
       . "<input type=\"radio\" name=\"db_quake_damage\" id=\"db_quake_damage\" value=\"med\" " . ($row["quake_damage"] == "med" ? "checked" : "") . ">Moderate (significant damage to home; large cracks, fallen chimney, etc)&nbsp&nbsp<BR>  "
       . "<input type=\"radio\" name=\"db_quake_damage\" id=\"db_quake_damage\" value=\"high\" " . ($row["quake_damage"] == "high" ? "checked" : "") . ">Severe (major structural damage to home)&nbsp&nbsp  ");


    // Liquefaction
    $bshare = "";
    if ($row["id"] > 0 && $row["liquefaction"]) { 
       $bshare = " checked ";
    }
    row2("Did you experience liquefaction on your home/property?",
       "<input type=\"checkbox\" name=\"db_liquefaction\" id=\"db_liquefaction\" $bshare> " .
       "<A HREF=\"http://en.wikipedia.org/wiki/Soil_liquefaction\"><I>Wikipedia on Liquefaction</I></A>");
  



  }  // end christchurch fields



     $affixtype = "we";
     if ($strRampType == "C" || $strRampType == "M") $affixtype = "you";

     row2("Can $affixtype affix a sensor to the floor with adhesive or screws?",
       "<input type=\"checkbox\" name=\"db_loc_affix_perm\" id=\"db_loc_affix_perm\" " . ($row["loc_affix_perm"] ? "checked" : "") . "> (Check if 'Yes' - If
    the sensor is not mounted, it cannot record strong motions as well)");

     row2("Is this a home location?",
      "<input type=\"checkbox\" name=\"db_loc_home\" id=\"db_loc_home\" " . 
        ($row["loc_home"] ? "checked" : "") . "> (Check if this is your residence)");

     row2("Is this a business location?",
      "<input type=\"checkbox\" name=\"db_loc_business\" id=\"db_loc_business\" " . 
        ($row["loc_business"] ? "checked" : "") . "> (Check if this is a business)");

     row2("Are you comfortable installing the sensor yourself?  (<A HREF=\"" . BASEURL . "/manuals/physical/\">Click here for directions</A>)",
      "<input type=\"checkbox\" name=\"db_loc_self_install\" id=\"db_loc_self_install\" " . 
        ($row["loc_self_install"] ? "checked" : "") . "> " .
      "(Check if 'Yes - Volunteer interns can install some sensors, but we mail the rest to participants with instructions)"
      );

     $time_host = "<select name=\"db_loc_years_host\" id=\"db_loc_years_host\">";
     for ($i = 1; $i <=5; $i++) {
         $time_host .= "<option";
         if ($row["loc_years_host"] == $i) {
            $time_host .= " selected";
         }
         $time_host .= (">" . $i . "</option>\n");
     }
     $time_host .= "</select>";
     row2("How many years are you willing to host the sensor?", $time_host);

  if ($row["ramp_type"] == "R") {  // only put installation day/time for regional ie not Christchurch

     //"<select name=\"db_cpu_os\" id=\"db_cpu_os\">";
     $time_ops = array("All Day",
                 "Morning (8am-12pm)", "Afternoon (12pm-4pm)", "Evenings (4pm-8pm)",
                 "8am", "9am", "10am", "11am", "12pm", "1pm", "2pm", "3pm", "4pm", "5pm",
                 "6pm", "7pm", "8pm"
               );
     $time_select = array("","","","","","","");
     $time_test = array(
                         $row["loc_time_install_sunday"],
                         $row["loc_time_install_monday"],
                         $row["loc_time_install_tuesday"],
                         $row["loc_time_install_wednesday"],
                         $row["loc_time_install_thursday"],
                         $row["loc_time_install_friday"],
                         $row["loc_time_install_saturday"]
                     );

//print_r($time_test);

     for ($j = 0; $j < count($time_select); $j++) {
       for ($i = 0; $i < count($time_ops); $i++) {
         $time_select[$j] .= "<option";
         if (strcmp($time_test[$j], $time_ops[$i]) == 0) {
            $time_select[$j] .= " selected";
         }
         $time_select[$j] .= (">" . $time_ops[$i] . "</option>\n");
       }
       $time_select[$j] .= "</select>";
     }

    row_heading_array(array("Preferred Installation Day and Time", "(you can select more than one day)"));

    row2("Sunday <input type=\"checkbox\" name=\"db_loc_day_install_sunday\" id=\"db_loc_day_install_sunday\" " . 
          ($row["loc_day_install_sunday"] ? "checked" : "") . ">" ,
              "<select name=\"db_loc_time_install_sunday\" id=\"db_loc_time_install_sunday\">" . $time_select[0]);
    row2("Monday <input type=\"checkbox\" name=\"db_loc_day_install_monday\" id=\"db_loc_day_install_monday\" " . 
          ($row["loc_day_install_monday"] ? "checked" : "") . ">" ,
              "<select name=\"db_loc_time_install_monday\" id=\"db_loc_time_install_monday\">" . $time_select[1]);
    row2("Tuesday <input type=\"checkbox\" name=\"db_loc_day_install_tuesday\" id=\"db_loc_day_install_tuesday\" " . 
          ($row["loc_day_install_tuesday"] ? "checked" : "") . ">" ,
              "<select name=\"db_loc_time_install_tuesday\" id=\"db_loc_time_install_tuesday\">" . $time_select[2]);
   row2("Wednesday <input type=\"checkbox\" name=\"db_loc_day_install_wednesday\" id=\"db_loc_day_install_wednesday\" " . 
          ($row["loc_day_install_wednesday"] ? "checked" : "") . ">" ,
              "<select name=\"db_loc_time_install_wednesday\" id=\"db_loc_time_install_wednesday\">" . $time_select[3]);
   row2("Thursday <input type=\"checkbox\" name=\"db_loc_day_install_thursday\" id=\"db_loc_day_install_thursday\" " . 
          ($row["loc_day_install_thursday"] ? "checked" : "") . ">" ,
              "<select name=\"db_loc_time_install_thursday\" id=\"db_loc_time_install_thursday\">" . $time_select[4]);
   row2("Friday <input type=\"checkbox\" name=\"db_loc_day_install_friday\" id=\"db_loc_day_install_friday\" " . 
          ($row["loc_day_install_friday"] ? "checked" : "") . ">" ,
              "<select name=\"db_loc_time_install_friday\" id=\"db_loc_time_install_friday\">" . $time_select[5]);
   row2("Saturday <input type=\"checkbox\" name=\"db_loc_day_install_saturday\" id=\"db_loc_day_install_saturday\" " . 
          ($row["loc_day_install_saturday"] ? "checked" : "") . ">" ,
              "<select name=\"db_loc_time_install_saturday\" id=\"db_loc_time_install_saturday\">" . $time_select[6]);

/*  //  echo "<table width=\"80%\"><tr>\n" .
     echo  "<tr colspan=7><th><input type=\"checkbox\" name=\"db_loc_day_install_sunday\" id=\"db_loc_day_install_sunday\" " . 
          ($row["loc_day_install_sunday"] ? "checked" : "") . ">Sunday<BR>" .
              "<select name=\"db_loc_time_install_sunday\" id=\"db_loc_time_install_sunday\">" . $time_select[0] . "</th>\n" .
       "<th><input type=\"checkbox\" name=\"db_loc_day_install_monday\" id=\"db_loc_day_install_monday\" " . 
          ($row["loc_day_install_monday"] ? "checked" : "") . ">Monday<BR>" .
              "<select name=\"db_loc_time_install_monday\" id=\"db_loc_time_install_monday\">" . $time_select[1] . "</th>\n" .
       "<th><input type=\"checkbox\" name=\"db_loc_day_install_tuesday\" id=\"db_loc_day_install_tuesday\" " . 
          ($row["loc_day_install_tuesday"] ? "checked" : "") . ">Tuesday<BR>" .
              "<select name=\"db_loc_time_install_tuesday\" id=\"db_loc_time_install_tuesday\">" . $time_select[2] . "</th>\n" .
       "<th><input type=\"checkbox\" name=\"db_loc_day_install_wednesday\" id=\"db_loc_day_install_wednesday\" " . 
          ($row["loc_day_install_wednesday"] ? "checked" : "") . ">Wednesday<BR>" .
              "<select name=\"db_loc_time_install_wednesday\" id=\"db_loc_time_install_wednesday\">" . $time_select[3] . "</th>\n" .
       "<th><input type=\"checkbox\" name=\"db_loc_day_install_thursday\" id=\"db_loc_day_install_thursday\" " . 
          ($row["loc_day_install_thursday"] ? "checked" : "") . ">Thursday<BR>" .
              "<select name=\"db_loc_time_install_thursday\" id=\"db_loc_time_install_thursday\">" . $time_select[4] . "</th>\n" .
       "<th><input type=\"checkbox\" name=\"db_loc_day_install_friday\" id=\"db_loc_day_install_friday\" " . 
          ($row["loc_day_install_friday"] ? "checked" : "") . ">Friday<BR>" .
              "<select name=\"db_loc_time_install_friday\" id=\"db_loc_time_install_friday\">" . $time_select[5] . "</th>\n" .
       "<th><input type=\"checkbox\" name=\"db_loc_day_install_saturday\" id=\"db_loc_day_install_saturday\" " . 
          ($row["loc_day_install_sunday"] ? "checked" : "") . ">Saturday<BR>" .
              "<select name=\"db_loc_time_install_saturday\" id=\"db_loc_time_install_saturday\">" . $time_select[6] . "</th>\n" .
      "</tr>" ; //</table>\n<BR>";
*/
/*
     row2("Preferred installation day?  (you can select more than one)",
      "<input type=\"checkbox\" name=\"db_loc_day_install_sunday\" id=\"db_loc_day_install_sunday\" " . 
        ($row["loc_day_install_sunday"] ? "checked" : "") . ">Sunday&nbsp&nbsp" .
      "<input type=\"checkbox\" name=\"db_loc_day_install_monday\" id=\"db_loc_day_install_monday\" " . 
        ($row["loc_day_install_monday"] ? "checked" : "") . ">Monday&nbsp&nbsp" .
      "<input type=\"checkbox\" name=\"db_loc_day_install_tuesday\" id=\"db_loc_day_install_tuesday\" " . 
        ($row["loc_day_install_tuesday"] ? "checked" : "") . ">Tuesday&nbsp&nbsp" .
      "<input type=\"checkbox\" name=\"db_loc_day_install_wednesday\" id=\"db_loc_day_install_wednesday\" " . 
        ($row["loc_day_install_wednesday"] ? "checked" : "") . ">Wednesday&nbsp&nbsp" .
      "<input type=\"checkbox\" name=\"db_loc_day_install_thursday\" id=\"db_loc_day_install_thursday\" " . 
        ($row["loc_day_install_thursday"] ? "checked" : "") . ">Thursday&nbsp&nbsp" .
      "<input type=\"checkbox\" name=\"db_loc_day_install_friday\" id=\"db_loc_day_install_friday\" " . 
        ($row["loc_day_install_friday"] ? "checked" : "") . ">Friday&nbsp&nbsp" .
      "<input type=\"checkbox\" name=\"db_loc_day_install_saturday\" id=\"db_loc_day_install_saturday\" " . 
        ($row["loc_day_install_saturday"] ? "checked" : "") . ">Saturday&nbsp&nbsp"
     );
*/
/*
     $time_hour_install= "<select name=\"db_loc_time_hour_install\" id=\"db_loc_time_hour_install\">";
     for ($i = 0; $i <=23; $i++) {
         $time_hour_install .= "<option";
         if ($row["loc_time_hour_install"] == $i) {
            $time_hour_install .= " selected";
         }
         $time_hour_install .= (">" . $i . "</option>\n");
     }
     $time_hour_install .= "</select>";

     $time_minute_install = "<select name=\"db_loc_time_minute_install\" id=\"db_loc_time_minute_install\">";
     for ($i = 0; $i <=59; $i++) {
         $time_minute_install .= "<option";
         if ($row["loc_time_minute_install"] == $i) {
            $time_minute_install .= " selected";
         }
         $time_minute_install .= (">" . $i . "</option>\n");
     }
     $time_minute_install .= "</select>";

     row2("Preferred time of day for installation", $time_hour_install . " : " . $time_minute_install .  " (enter as 24 hour clock 0-23 and minutes 0-59)");
*/

  } // end regional extra questions

  } // end regional installation day/time

     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Comments"));

     // need to put CRLF back to spaces \r\n
     //har\r\n\r\nde\r\n\r\nhar\r\n\r\nhar\r\n\r\n\r\nblah
     //$comments = nl2br( htmlentities( $row["comments"], ENT_QUOTES, "UTF-8" ) );
     //$comments = nl2br( $row["comments"] );
     $comments = str_replace("\\r\\n", "\n", $row["comments"]);
     echo "<tr><td colspan=2><textarea name=\"db_comments\" id=\"db_comments\" cols=\"100\" rows=\"10\">"
        . $comments . "</textarea></td></tr>";

echo "<tr>
         <td align=center><input type=\"submit\" id=\"submit\" name=\"submit\" value=\"Submit\"></td>
         <td align=center><input type=\"submit\" id=\"submit\" name=\"submit\" value=\"Delete\"></td>
      </tr>
</table>
";

page_tail();

function doRAMPSubmit($userid, $rampid, $ramp_type)
{
/*   print_r($_POST);Array ( [db_id] => 0 [lnm0] => [db_fname] => car [db_lname] => Christensen [db_addr1] => 14525 SW Millikan #76902 [db_addr2] => [db_city] => Beaverton [db_region] => OR [db_postcode_] => [db_country] => United States [db_phone] => +1 215 989 4276 [db_fax] => carlgt1@yahoo.com [db_email_addr] => carlgt6@hotmail.com [lat0] => [lng0] => [addrlookup] => 14525 SW Millikan #76902, , Beaverton, OR, United States [db_bshare_map] => on [db_bshare_coord] => on [db_bshare_ups] => on [db_sensor_distribute] => on [db_cpu_os] => Mac OS X (Intel) [db_cpu_age] => 5 [db_cpu_floor] => 6 [db_cpu_admi
n] => on [db_cpu_permission] => on [db_cpu_firewall] => on [db_cpu_internet] => on [db_cpu_proxy] => on [db_cpu_unint_power] => on [db_comments] =>
 hkhk [submit] => Submit )
*/

    // copy over post variables to reuse in the fields below, and for the sql insert/update!
    $row["id"] = $rampid;
    $row["ramp_type"] = $ramp_type;
    $row["userid"] = $userid;
    $row["fname"] = mysql_real_escape_string(post_str("db_fname"));
    $row["lname"] = mysql_real_escape_string(post_str("db_lname"));
    $row["email_addr"] = mysql_real_escape_string(post_str("db_email_addr"));
    $row["addr1"] = mysql_real_escape_string(post_str("db_addr1"));
    $row["addr2"] = mysql_real_escape_string(post_str("db_addr2", true)); // note it's optional
    $row["city"] = mysql_real_escape_string(post_str("db_city"));
    $row["region"] = mysql_real_escape_string(post_str("db_region", true)); // note it's optional
    $row["country"] = mysql_real_escape_string(post_str("db_country"));
    $row["postcode"] = mysql_real_escape_string(post_str("db_postcode", true)); // note it's optional
    $row["latitude"] = post_double("lat0", true);
    $row["longitude"] = post_double("lng0", true);
    $row["gmap_placename"] = mysql_real_escape_string(post_str("addrlookup", true)); // note it's optional
    $row["gmap_view_level"] = 18;
    $row["gmap_view_type"] = 0;
    $row["phone"] = mysql_real_escape_string(post_str("db_phone", true)); // note it's optional
    $row["fax"] = mysql_real_escape_string(post_str("db_fax", true)); // note it's optional
    $row["bshare_coord"] = ($_POST["db_bshare_coord"] == "on") ? 1 : 0;
    $row["bshare_map"] = ($_POST["db_bshare_map"] == "on") ? 1 : 0;
    $row["bshare_ups"] = ($_POST["db_bshare_ups"] == "on") ? 1 : 0;
    $row["cpu_type"] = post_str("db_cpu_os", true);
    $row["cpu_os"] = post_str("db_cpu_os", true);
    $row["cpu_age"] = post_int("db_cpu_age", true);
    $row["cpu_floor"] = post_int("db_cpu_floor", true);
    $row["cpu_admin"] = post_int("db_cpu_admin", true);
    $row["cpu_permission"] = post_int("db_cpu_permission", true);
    $row["cpu_firewall"] = post_int("db_cpu_firewall", true);
    $row["cpu_proxy"] = post_int("db_cpu_proxy", true);
    $row["cpu_internet"] = post_int("db_cpu_internet", true);
    $row["cpu_unint_power"] = post_int("db_cpu_unint_power", true);
    $row["sensor_distribute"] = ($_POST["db_sensor_distribute"] == "on") ? 1 : 0;
    $row["comments"] = mysql_real_escape_string(post_str("db_comments", true));

    $row["loc_home"] = ($_POST["db_loc_home"] == "on") ? 1 : 0;
    $row["loc_business"] = ($_POST["db_loc_business"] == "on") ? 1 : 0;
    $row["loc_affix_perm"] = ($_POST["db_loc_affix_perm"] == "on") ? 1 : 0;
    $row["loc_self_install"] = ($_POST["db_loc_self_install"] == "on") ? 1 : 0;
    
    $row["loc_day_install_sunday"] = ($_POST["db_loc_day_install_sunday"] == "on") ? 1 : 0;
    $row["loc_day_install_monday"] = ($_POST["db_loc_day_install_monday"] == "on") ? 1 : 0;
    $row["loc_day_install_tuesday"] = ($_POST["db_loc_day_install_tuesday"] == "on") ? 1 : 0;
    $row["loc_day_install_wednesday"] = ($_POST["db_loc_day_install_wednesday"] == "on") ? 1 : 0;
    $row["loc_day_install_thursday"] = ($_POST["db_loc_day_install_thursday"] == "on") ? 1 : 0;
    $row["loc_day_install_friday"] = ($_POST["db_loc_day_install_friday"] == "on") ? 1 : 0;
    $row["loc_day_install_saturday"] = ($_POST["db_loc_day_install_saturday"] == "on") ? 1 : 0;

    if ($row["loc_day_install_sunday"])
      $row["loc_time_install_sunday"] = post_str("db_loc_time_install_sunday", true);
    else
      $row["loc_time_install_sunday"] = "";
    if ($row["loc_day_install_monday"])
      $row["loc_time_install_monday"] = post_str("db_loc_time_install_monday", true);
    else
      $row["loc_time_install_monday"] = "";
    if ($row["loc_day_install_tuesday"])
      $row["loc_time_install_tuesday"] = post_str("db_loc_time_install_tuesday", true);
    else
      $row["loc_time_install_tuesday"] = "";
    if ($row["loc_day_install_wednesday"])
      $row["loc_time_install_wednesday"] = post_str("db_loc_time_install_wednesday", true);
    else
      $row["loc_time_install_wednesday"] = "";
    if ($row["loc_day_install_thursday"])
      $row["loc_time_install_thursday"] = post_str("db_loc_time_install_thursday", true);
    else
      $row["loc_time_install_thursday"] = "";
    if ($row["loc_day_install_friday"])
      $row["loc_time_install_friday"] = post_str("db_loc_time_install_friday", true);
    else
      $row["loc_time_install_friday"] = "";
    if ($row["loc_day_install_saturday"])
      $row["loc_time_install_saturday"] = post_str("db_loc_time_install_saturday", true);
    else
      $row["loc_time_install_saturday"] = "";

    $row["loc_time_hour_install"] = post_int("db_loc_time_hour_install", true);
    if ($row["loc_time_hour_install"] == "") $row["loc_time_hour_install"] = "null";
    $row["loc_time_minute_install"] = post_int("db_loc_time_minute_install", true);
    if ($row["loc_time_minute_install"] == "") $row["loc_time_minute_install"] = "null";
    $row["loc_years_host"] = post_int("db_loc_years_host", true);
    if ($row["loc_years_host"] == "") $row["loc_years_host"] = 1;

    $row["quake_damage"] = post_str("db_quake_damage", true);
    $row["liquefaction"] = ($_POST["db_liquefaction"] == "on") ? 1 : 0;

    $mylat = $row["latitude"];
    $mylng = $row["longitude"];
    $zoomout = 1;

    $bInsert = true; // insert if no db_id posted (i.e. record exists for this userid
    $sqlStart = "INSERT INTO qcn_ramp_participant SET ";
    $sqlEnd   = "";

    if ($row["id"] > 0) {
      $bInsert = false;
      $sqlStart = "UPDATE qcn_ramp_participant SET ";
      $sqlEnd   = "WHERE id=" . $row["id"] . " AND userid=" . $row["userid"];  // node the userid check
    }


    $sqlSet = "userid=" . $row["userid"] . ", 
            qcn_ramp_coordinator_id = NULL, 
            fname='" . $row["fname"] . "', 
            lname='" . $row["lname"] . "', 
            email_addr='" . $row["email_addr"] . "', 
            addr1='" . $row["addr1"] . "', 
            addr2='" . $row["addr2"] . "', 
            city='" . $row["city"] . "', 
            region='" . $row["region"] . "', 
            country='" . $row["country"] . "', 
            postcode='" . $row["postcode"] . "', 
            latitude=" . $row["latitude"] . ", 
            longitude=" . $row["longitude"] . ", 
            gmap_placename='" . $row["gmap_placename"] . "', 
            gmap_view_level=" . $row["gmap_view_level"] . ", 
            gmap_view_type=" . $row["gmap_view_type"] . ", 
            phone='" . $row["phone"] . "', 
            fax='" . $row["fax"] . "', 
            bshare_coord=" . $row["bshare_coord"] . ", 
            bshare_map=" . $row["bshare_map"] . ", 
            bshare_ups=" . $row["bshare_ups"] . ", 
            cpu_type='" . $row["cpu_type"] . "', 
            cpu_os='" . $row["cpu_os"] . "', 
            cpu_age=" . $row["cpu_age"] . ", 
            cpu_floor=" . $row["cpu_floor"] . ", 
            cpu_admin=" . $row["cpu_admin"] . ", 
            cpu_permission=" . $row["cpu_permission"] . ", 
            cpu_firewall=" . $row["cpu_firewall"] . ", 
            cpu_proxy=" . $row["cpu_proxy"] . ", 
            cpu_internet=" . $row["cpu_internet"] . ", 
            cpu_unint_power=" . $row["cpu_unint_power"] . ",             
            sensor_distribute=" . $row["sensor_distribute"] . ", 
            loc_home=" . $row["loc_home"] . ",
            loc_business=" . $row["loc_business"] . ",
            loc_affix_perm=" . $row["loc_affix_perm"] . ",
            loc_self_install=" . $row["loc_self_install"] . ",
            loc_day_install_sunday=" . $row["loc_day_install_sunday"] . ",
            loc_day_install_monday=" . $row["loc_day_install_monday"] . ",
            loc_day_install_tuesday=" . $row["loc_day_install_tuesday"] . ",
            loc_day_install_wednesday=" . $row["loc_day_install_wednesday"] . ",
            loc_day_install_thursday=" . $row["loc_day_install_thursday"] . ",
            loc_day_install_friday=" . $row["loc_day_install_friday"] . ",
            loc_day_install_saturday=" . $row["loc_day_install_saturday"] . ",
            loc_time_install_sunday='" . $row["loc_time_install_sunday"] . "',
            loc_time_install_monday='" . $row["loc_time_install_monday"] . "',
            loc_time_install_tuesday='" . $row["loc_time_install_tuesday"] . "',
            loc_time_install_wednesday='" . $row["loc_time_install_wednesday"] . "',
            loc_time_install_thursday='" . $row["loc_time_install_thursday"] . "',
            loc_time_install_friday='" . $row["loc_time_install_friday"] . "',
            loc_time_install_saturday='" . $row["loc_time_install_saturday"] . "',
            loc_years_host=" . $row["loc_years_host"] . ",
            comments='" . $row["comments"] . "', 
            ramp_type='" . $row["ramp_type"] . "', 
            quake_damage='" . $row["quake_damage"] . "', 
            liquefaction=" . $row["liquefaction"] . ", 
            active=1, time_edit=unix_timestamp() ";

//echo "<BR><BR>" . $sqlStart . $sqlSet . $sqlEnd . "<BR><BR>";

      //echo $sqlStart . $sqlSet . $sqlEnd;

      $result = mysql_query($sqlStart . $sqlSet . $sqlEnd);
      if ($result) {
         if ($bInsert) { // get the insert id
            $row["id"] = mysql_insert_id();
            if (!$row["id"]) {
              echo "<BR><BR><B><center><font color=red>Database Error in inserting information - please try later or review your submission!</font></center></b><BR><BR>";
            }
         }
         //mysql_free_result($result);
         return "<BR><BR><center><B>Your submission has been saved.  Thank you for taking part in QCN RAMP!</font></center></b><BR><BR>";
      }
      else {
//echo $sqlStart . $sqlSet . $sqlEnd;

         return "<BR><BR><B><center><font color=red>Error in updating information - please try later or review your submission!</font></center></b><BR><BR>";
      }
   return "";
}

?>
