<?php

require_once("../project/project.inc");
require_once("../project/project_specific_prefs.inc");
require_once("../inc/utils.inc");
require_once("../inc/db.inc");
require_once("../inc/countries.inc");
require_once("../inc/translation.inc");
require_once("../inc/google_translate.inc");

db_init();

$user = get_logged_in_user(true, true);
$psprefs = project_specific_prefs_parse($user->project_prefs);

//$db = BoincDb::get();
$row["id"] = post_int("db_id", true);

echo "
<center><h1>Rapid Aftershock Mobilization Program (RAMP)</h1></center>
";
//<h2>Welcome Back " . $user->name . "</h2>

google_translate_new();

switch ($_POST["submit"]) { 

case "Submit":
   doRAMPSubmit($user->id, $row["id"]);
   break;
case "Delete":
   //break; -- no break, want to "refresh the fields"
  $sqlRAMP = "DELETE FROM qcn_ramp_participant WHERE id=" . $row["id"] . " AND userid=" . $user->id;
  $result = mysql_query($sqlRAMP);
  if ($result) {
     echo "<BR><BR><B><center><font color=black>Your RAMP information has been deleted, thanks for participating!!</font></center></b><BR><BR>";
     $row["id"] = 0;
  }
  else {
     echo "<BR><BR><B><center><font color=red>Error in deleting record - please try later</font></center></b><BR><BR>";
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

// note this has google stuff  
page_head("QCN RAMP Information", null, null, "", true, $psprefs, false, 1, $zoomout);

echo "
<script type=\"text/javascript\">
   function clickedAddressCopy() {
       getElement(\"addrlookup\").value = getElement(\"db_addr1\").value + \", \" + getElement(\"db_addr2\").value + \", \" + getElement(\"db_city\").value + \", \" + getElement(\"db_region\").value + \", \" + getElement(\"db_country\").value;
   }
</script>

<!-- 
   <img src=\"http://qcn.stanford.edu/images/QCN-USB-Aftershocks_Cut.png\" align=\"right\" width=\"260\" height=\"320\" margin=\"6\">
-->
</a>
";

echo "<ul><p align=\"justify\">You can add yourself to QCN RAMP by submitting the following information,
    or edit a previous submission.
<BR>Please enter as much of the following information as you can:</p>
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

     row2("Post Code", "<input name=\"db_postcode\" type=\"text\" id=\"db_postcode\" size=\"20\" value=\"" . stripslashes($row["postcode"]) . "\">");

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
     row2("Share This Location on the QCN Participant Map?", 
       "<input type=\"checkbox\" name=\"db_bshare_map\" id=\"db_bshare_map\" $bshare>");
    
     // bshare_coord == share info with RAMP coordinator
     $bshare = " checked ";
     if ($row["id"] > 0 && $row["bshare_coord"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share Your Information With a Volunteer RAMP coordinator After A Major Earthquake?",
       "<input type=\"checkbox\" name=\"db_bshare_coord\" id=\"db_bshare_coord \" $bshare>");

     // bshare_ups == share info with UPS
     $bshare = " checked ";
     if ($row["id"] > 0 && $row["bshare_ups"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share Your Information With UPS For Faster Sensor Delivery?",
       "<input type=\"checkbox\" name=\"db_bshare_ups\" id=\"db_bshare_ups\" $bshare>");
 
     row2("Are you able to distribute sensors to (and help setup) other local participants?",
       "<input type=\"checkbox\" name=\"db_sensor_distribute\" id=\"db_sensor_distribute\" " . ($row["sensor_distribute"] ? "checked" : "") . ">");

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
     row2("Computer Age In Years<BR><i><font color=red>(0=<1 yr old)</font></i>", $cpuage);

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
     row2("Floor Location of Computer<BR><i><font color=red>(-1 = Basement, 0 = Ground Floor, 1 = First Floor etc)<font></i>", $cpufl);

     row2("Do You Have Administrator Rights On This Computer?",
       "<input type=\"checkbox\" name=\"db_cpu_admin\" id=\"db_cpu_admin\" "   . ($row["cpu_admin"] ? "checked" : "") . ">");

     row2("Do You Have Permission To Send Seismic Data Out Of Your Country From This Computer?",
       "<input type=\"checkbox\" name=\"db_cpu_permission\" id=\"db_cpu_permission\" "   . ($row["cpu_permission"] ? "checked" : "") . ">");

     row2("Is This Computer Behind A Firewall?",
       "<input type=\"checkbox\" name=\"db_cpu_firewall\" id=\"db_cpu_firewall\" " . ($row["cpu_firewall"] ? "checked" : "") . ">");

     row2("Is This Computer Usually Connected To The Internet?",
       "<input type=\"checkbox\" name=\"db_cpu_internet\" id=\"db_cpu_internet\" " . ($row["cpu_internet"] ? "checked" : "") . ">");

     row2("Does This Computer Use A Proxy for Internet Access?",
       "<input type=\"checkbox\" name=\"db_cpu_proxy\" id=\"db_cpu_proxy\" " . ($row["cpu_proxy"] ? "checked" : "") . ">");

     row2("Do You Have An Uninterruptible Power Supply Attached To This Computer?",
       "<input type=\"checkbox\" name=\"db_cpu_unint_power\" id=\"db_cpu_unint_power\" " . ($row["cpu_unint_power"] ? "checked" : "") . ">");

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

function doRAMPSubmit($userid, $rampid)
{
/*   print_r($_POST);Array ( [db_id] => 0 [lnm0] => [db_fname] => car [db_lname] => Christensen [db_addr1] => 14525 SW Millikan #76902 [db_addr2] => [db_city] => Beaverton [db_region] => OR [db_postcode_] => [db_country] => United States [db_phone] => +1 215 989 4276 [db_fax] => carlgt1@yahoo.com [db_email_addr] => carlgt6@hotmail.com [lat0] => [lng0] => [addrlookup] => 14525 SW Millikan #76902, , Beaverton, OR, United States [db_bshare_map] => on [db_bshare_coord] => on [db_bshare_ups] => on [db_sensor_distribute] => on [db_cpu_os] => Mac OS X (Intel) [db_cpu_age] => 5 [db_cpu_floor] => 6 [db_cpu_admi
n] => on [db_cpu_permission] => on [db_cpu_firewall] => on [db_cpu_internet] => on [db_cpu_proxy] => on [db_cpu_unint_power] => on [db_comments] =>
 hkhk [submit] => Submit )
*/

    // copy over post variables to reuse in the fields below, and for the sql insert/update!
    $row["id"] = $rampid;
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
    $row["cpu_admin"] = ($_POST["db_cpu_admin"] == "on") ? 1 : 0;
    $row["cpu_permission"] = ($_POST["db_cpu_permission"] == "on") ? 1 : 0;
    $row["cpu_firewall"] = ($_POST["db_cpu_firewall"] == "on") ? 1 : 0;
    $row["cpu_proxy"] = ($_POST["db_cpu_proxy"] == "on") ? 1 : 0;
    $row["cpu_internet"] = ($_POST["db_cpu_internet"] == "on") ? 1 : 0;
    $row["cpu_unint_power"] = ($_POST["db_cpu_unint_power"] == "on") ? 1 : 0;
    $row["sensor_distribute"] = ($_POST["db_sensor_distribute"] == "on") ? 1 : 0;
    $row["comments"] = mysql_real_escape_string(post_str("db_comments", true));

    $mylat = $row["latitude"];
    $mylng = $row["longitude"];
    $zoomout = 0;

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
            comments='" . $row["comments"] . "`', 
            active=1, time_edit=unix_timestamp() ";

//echo "<BR><BR>" . $sqlStart . $sqlSet . $sqlEnd . "<BR><BR>";

      $result = mysql_query($sqlStart . $sqlSet . $sqlEnd);
      if ($result) {
         if ($bInsert) { // get the insert id
            $row["id"] = mysql_insert_id();
            if (!$row["id"]) {
              echo "<BR><BR><B><center><font color=red>Database Error in inserting information - please try later or review your submission!</font></center></b><BR><BR>";
            }
         }
         //mysql_free_result($result);
         echo "<BR><BR><center><B>Your submission has been saved.  Thank you for taking part in QCN RAMP!</font></center></b><BR><BR>";
      }
      else {
//echo $sqlStart . $sqlSet . $sqlEnd;

         echo "<BR><BR><B><center><font color=red>Error in updating information - please try later or review your submission!</font></center></b><BR><BR>";
      }

 echo "<br><center><a href=\"http://qcn.stanford.edu/sensor/\">Return to Quake-Catcher Network Seismic Monitoring main page</a></center><br>";

}

?>

