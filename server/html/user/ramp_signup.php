<?php

require_once("../project/project.inc");
require_once("../project/project_specific_prefs.inc");
require_once("../inc/utils.inc");
require_once("../inc/db.inc");
require_once("../inc/countries.inc");
require_once("../inc/translation.inc");

db_init();

$user = get_logged_in_user(true, true);
$psprefs = project_specific_prefs_parse($user->project_prefs);

//$db = BoincDb::get();
$row = array();
$action = $_POST["submit"];

if ($action) { // we're updating ramp info
   print_r($_POST);
/*
  Array ( [db_id] => [lnm0] => haga sofia, istanbul, turkey [db_fname] => first [db_lname] => last [db_addr1] => add 1 [db_addr2] => add 2 [db_city] => city 3 [db_region] => state 4 [db_postcode_] => postc 5 [db_country] => Turkey [db_phone] => phon3 [db_fax] => fax4 [db_email_addr] => carlgt1@yahoo.com [lat0] => 41.00527 [lng0] => 28.97696 [addrlookup] => haga sofia, istanbul, turkey [db_bshare_map] => on [db_bshare_coord] => on [db_bshare_ups] => on [db_cpu_os] => Mac OS X (Intel) [db_cpu_age] => 1 [db_cpu_admin] => on [db_cpu_firewall] => on [db_internet_access] => on [db_unint_power] => on [db_cpu_floor] => 1 [db_comments] => testing!!!!!! ok [submit] => Submit )
*/
}
else { // we're coming into the page to add or edit ramp info
  // get the row for this user
  $sqlRAMP = "SELECT * FROM qcn_ramp WHERE userid=$user->id";
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


  $mylat = $row["latitude"];
  $mylng = $row["longitude"];
  $zoomout = 0;

  if (!$mylat || !$mylng) { // use geoip
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
          $zoomout = 1;
       }
    }
  }
  if (!$mylat || !$mylng){ // default to Turkey?
       $mylat = 39;
       $mylng = 35;
       $zoomout = 1;
  }

}

// note this has google stuff  
page_head("QCN RAMP Information", null, null, "", true, $psprefs, false, 1, $zoomout);

echo "
<script type=\"text/javascript\">
   function clickedAddressCopy() {
       getElement(\"addrlookup\").value = getElement(\"address1\").value + \", \" + getElement(\"address2\").value + \", \" + getElement(\"city\").value + \", \" + getElement(\"region\").value + \", \" + getElement(\"country\").value;
   }
</script>

<h1>Rapid Aftershock Mobilization Program (RAMP)</h1>
<a href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-38&cy=-70&timeint=W\">
<!-- 
   <img src=\"http://qcn.stanford.edu/images/QCN-USB-Aftershocks_Cut.png\" align=\"right\" width=\"260\" height=\"320\" margin=\"6\">
-->
</a>

<h2>Welcome Back " . $user->name . "</h2>
<BR>
<ul><p align=\"justify\">You can add yourself to QCN RAMP by submitting the following information,
    or edit a previous submission:</p>
         <form name=\"ramp_form\" method=\"post\" action=\"ramp_signup.php\">

    <input name=\"db_id\" type=\"hidden\" id=\"db_id\" size=\"20\" value=\"" . $row["id"] . "\">
    <input name=\"lnm0\" type=\"hidden\" id=\"lnm0\" size=\"64\">

<table>";

     row_heading_array(array("Enter Your Postal Address (for UPS Delivery)"));
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

     row2("Post Code", "<input name=\"db_postcode \" type=\"text\" id=\"db_postcode\" size=\"20\" value=\"" . stripslashes($row["postcode"]) . "\">");

     row2_init("Country", "<select name=db_country id=db_country>");
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
<tr><td colspan=2><div name=\"map\" id=\"map\" style=\"width: 800px; height: 480px\"></div></td></tr>
<tr><td colspan-2>Use the following box to lookup an address (i.e. 360 Panama Mall, Stanford, CA)</td></tr>
<tr><td colspan=2>Click on the map for exact placement of the computer (zoom in if necessary)</td></tr>
<tr><td colspan=2 width=\"50\"><input type=\"text\" name=\"addrlookup\" id=\"addrlookup\" size=64 value=\""
    . $row["gmap_placename"] . "\"> 
    <input type=\"button\" name=\"btnaddress\" id=\"btnaddress\" onclick=\"clickedAddressLookup(addrlookup.value)\" value=\"Lookup Address\" size=20>
    <input type=\"button\" name=\"btncopyaddr\" id=\"btncopyaddr\" onclick=\"clickedAddressCopy()\"
   value=\"Copy Address From Above\" size=20>
</td></tr>";
   
     // extra info
     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Extra Options"));

     // more checkboxes to default to true unless there is a record and it was set to false

     // bshare_map == share map location
     $bshare = " checked ";
     if (strlen($row["email_addr"]) > 0 && $row["bshare_map"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share This Location on the QCN Participant Map?", 
       "<input type=\"checkbox\" name=\"db_bshare_map\" id=\"db_bshare_map\" $bshare>");
    
     // bshare_coord == share info with RAMP coordinator
     $bshare = " checked ";
     if (strlen($row["email_addr"]) > 0 && $row["bshare_coord"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share Your Information With a Volunteer RAMP coordinator After A Major Earthquake?",
       "<input type=\"checkbox\" name=\"db_bshare_coord\" id=\"db_bshare_coord \" $bshare>");

     // bshare_ups == share info with UPS
     $bshare = " checked ";
     if (strlen($row["email_addr"]) > 0 && $row["bshare_ups"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share Your Information With UPS For Faster Sensor Delivery?",
       "<input type=\"checkbox\" name=\"db_bshare_ups\" id=\"db_bshare_ups\" $bshare>");
 
     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Computer Information"));

     $os = array("Mac OS X (Intel)", 
                 "Mac OS X (PPC)", "Mac (Other)",
             "Windows XP", "Windows Vista", "Windows 7", "Windows (other i.e. 98)",
             "Linux", "Other");
     $os_select = "<select name=\"db_cpu_os\" id=\"db_cpu_os\">";
     for ($i = 0; $i < count($os); $i++) {
        $os_select .= "<option";
        if ($row["db_cpu_os"] == $os[$i]) {
           $os_select .= " selected";
        }
        $os_select .= (">" . $os[$i] . "</option>\n");
     }
     $os_select .= "</select>";

     row2("Operating System", $os_select);

     $cpuage = "<select name=\"db_cpu_age\" id=\"db_cpu_age\">";
     for ($i = 0; $i <=20; $i++) {  
         $cpuage .= "<option";
         if ($row["db_cpu_age"] == $i) {
            $cpuage .= " selected";
         }
         $cpuage .= (">" . $i . "</option>\n");
     }
     $cpuage .= "</select>";
     row2("Computer Age In Years<BR><i><font color=red>(0=<1 yr old)</font></i>", $cpuage);

     row2("Do You Have Administrator Rights On This Computer?",
       "<input type=\"checkbox\" name=\"db_cpu_admin\" id=\"db_cpu_admin\" "   . ($row["cpu_admin"] ? "checked" : "") . ">");

     row2("Is This Computer Behind A Firewall?",
       "<input type=\"checkbox\" name=\"db_cpu_firewall\" id=\"db_cpu_firewall\" " . ($row["cpu_firewall"] ? "checked" : "") . ">");

     row2("Is This Computer Usually Connected To The Internet?",
       "<input type=\"checkbox\" name=\"db_internet_access\" id=\"db_internet_access\" " . ($row["internet_access"] ? "checked" : "") . ">");

     row2("Do You Have An Uninterruptible Power Supply Attached To This Computer?",
       "<input type=\"checkbox\" name=\"db_unint_power\" id=\"db_unint_power\" " . ($row["unint_power"] ? "checked" : "") . ">");

     $cpufl = "<select name=\"db_cpu_floor\" id=\"db_cpu_floor\">";
     if (!$row["db_cpu_floor"]) $row["db_cpu_floor"] = 0;
     for ($i = -10; $i <= 100; $i++) {
         $cpufl .= "<option";
         if ($row["db_cpu_floor"] == $i) {
            $cpufl .= " selected";
         }
         $cpufl .= (">" . $i . "</option>\n");
     }
     $cpufl .= "</select>";
     row2("Floor Location of Computer<BR><i><font color=red>(-1 = Basement, 0 = Ground Floor, 1 = First Floor etc)<font></i>", $cpufl);

     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Comments (Firewall type, Proxy type, Can you distribute sensors etc)"));
     echo "<tr><td colspan=2><textarea name=\"db_comments\" id=\"db_comments\" cols=\"60\" rows=\"4\">" 
        . stripslashes($row["comments"]) . "</textarea></td></tr>";

echo "<tr>
         <td colspan=2 align=center><input type=\"submit\" id=\"submit\" name=\"submit\" value=\"Submit\"></td>
      </tr>
</table>
";

page_tail();

?>

