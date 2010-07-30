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
       "<input name=\"db_fname\" type=\"text\" id=\"db_fname\" size=\"30\" value=\"" . $row["fname"] . "\">"
       . "&nbsp;&nbsp;<input name=\"db_lname\" type=\"text\" id=\"db_lname\" size=\"60\" value=\"" 
           . $row["lname"] . "\">");
     row2("Street Address (Line 1)",
              "<input name=\"db_addr1\" type=\"text\" id=\"db_addr1\" size=\"60\" value=\"" . $row["addr1"] . "\">");
     row2("Street Address (Line 2)",
              "<input name=\"db_addr2\" type=\"text\" id=\"db_addr2\" size=\"60\" value=\"" . $row["addr2"] . "\">");
     row2("City",
              "<input name=\"db_city\" type=\"text\" id=\"db_city\" size=\"60\" value=\"" . $row["city"] . "\">");
     row2("Region/State/Province",
              "<input name=\"db_region\" type=\"text\" id=\"db_region\" size=\"40\" value=\"" . $row["region"] . "\">");

     row2("Post Code", "<input name=\"db_postcode \" type=\"text\" id=\"db_postcode\" size=\"20\" value=\"" . $row["postcoe"] . "\">");

     row2_init("Country", "<select name=country>");
     print_country_select($row["country"]);
     echo "</select></td></tr>";

     
     echo "<tr><td colspan=2><hr></td></tr>";
     row_heading_array(array("Enter Your Contact Information (Phone / Fax/ Email)"));
     row2("Phone Number (include country code)",
              "<input name=\"db_phone\" type=\"text\" id=\"db_phone\" size=\"40\" value=\"" . $row["phone"] . "\">");
     row2("FAX Number (include country code)",
              "<input name=\"db_fax\" type=\"text\" id=\"db_fax\" size=\"40\" value=\"" . $row["fax"] . "\">");

     $defEmail = $user->email_addr;
     if (strlen($row["email_addr"])>0) {
          $defEmail = $row["email_addr"];
     }
     row2("E-mail address (default is your QCN Account email address)",
              "<input name=\"db_fax\" type=\"text\" id=\"db_fax\" size=\"40\" value=\"" . $defEmail . "\">");

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

     $bshare = " checked ";
     if (strlen($row["email_addr"]) > 0 && $row["bshare_map"] == 0) { // don't want to share
        $bshare = "";
     }
     row2("Share This Location on the QCN Participant Map?", "<input type=\"checkbox\" name=\"db_bshare_map\" id=\"db_bshare_map\" $bshare>");
   
/* remaining questions
    ->     bshare_coord boolean,
    ->     bshare_ups boolean,
    ->     cpu_type varchar(20),
    ->     cpu_os varchar(20),
    ->     cpu_age int,
    ->     cpu_admin boolean,
    ->     cpu_permission boolean,
    ->     cpu_firewall varchar(20),
    ->     cpu_floor int,
    ->     internet_access boolean,
    ->     unint_power boolean,
    ->     comments varchar(255),

1.f) Will the volunteer allow QCN-RAMP to share information with:
1.f.i) A volunteer RAMP coordinator after a major earthquake?
1.f.ii) As a point on the RAMP volunteer map?
1.f.iii) With UPS for more rapid delivery?
1.g) What kind of computer?
1.g.i) Windows XP? Vista? OS X?
1.g.ii) Is it connected to the internet always?
1.g.iii) Does the computer have an uninteruptable power supply?
1.g.iv) How old is the computer (years)?
1.h) What floor is the computer on? Basement, ground floor, 1, 2, 3, 4, ...?
1.i) ask if they have Administrative privileges on their computer
1.j) ask about firewall issues for a computer?  describe...

*/
 
echo "
              <tr><td colspan=2><hr></tr>
            <tr>
               <td>&nbsp;</td>
               <td>Computer Age:<br>(Years)</td>
               <td><input name=\"cage\" type=\"text\" id=\"cage\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Operating System:</td>
               <td>OS X:<input name=\"OS\" type=\"radio\" id=\"OS\" value=\"OSX\"><br>
                   Windows XP:<input name=\"OS\" type=\"radio\" id=\"OS\" value=\"WindowsXP\"><br></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Regional Coordinator:</td>
               <td>Are you willing to help distribute sensors to participants in your area?<br>
               
               <dd>Yes:<input name=\"RC\" type=\"radio\" id=\"RC\" value=\"YES\"><br>
               <dd>No: <input name=\"RC\" type=\"radio\" id=\"RC\" value=\"NO\"><br></td>
            </tr>


            <tr>
               <td>&nbsp;</td>
               <td colspan=\"2\"><p>If you have any comments or concerns, please let us know.</p><textarea name=\"essay\" id=\"essay\" rows=\"15\" cols=\"64\"></textarea></td>

            </tr>
            <tr>   <td colspan=3><hr></td>  </tr>
            <tr>
               <td>&nbsp;</td>
               <td>&nbsp;</td>

               <td><input type=\"submit\" name=\"Submit\" value=\"Submit\"></td>
            </tr>

</table>
</ul>
";

page_tail();

?>

