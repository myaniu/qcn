<?php

require_once("../project/project.inc");

require_once("../inc/prefs.inc");
require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/utils.inc");

db_init();

$user = get_logged_in_user();
$psprefs = project_specific_prefs_parse($user->project_prefs);

$db = BoincDb::get();

//verify this logged in user owns this host!
$hostid = $_GET['hostid'];
$bMapExact = 0;

if ($hostid) {
   $host = BoincHost::lookup_id($hostid);
}
else { // get the first host record for this user i.e. they may be a new user
  $sql = "SELECT id FROM host WHERE userid=" . $db->base_escape_string($user->id) . " ORDER BY id DESC LIMIT 1";
  $result=$db->do_query($sql);
  if ($result && mysql_num_rows($result)>0) {  // we found some records
     if ($row=mysql_fetch_array($result)) {
       $hostid = $row[0];
     }
  }
  if ($hostid) $host = BoincHost::lookup_id($hostid);
}

if (!$hostid || $host->userid != $user->id)
{
   page_head("Invalid Host");

   echo "<BR>It appears that you do not own this host machine, therefore you cannot edit location preferences!<BR>";
   page_tail();
   exit();
}

// first off get the sensor types
$sqlsensor = "select id,description from qcn_level order by id";
$result = mysql_query($sqlsensor);
$i = 0;
$arrLevel = array();
if ($result) {
    while ($res = mysql_fetch_array($result)) {
       $arrLevel[$i] = $res;
       $i++;
    }
    mysql_free_result($result);
}
$clvlmax = sizeof($arrLevel) - 1;

// get host qcn_showhostlocation info
$result = $db->do_query("select count(hostid) from qcn_showhostlocation where hostid=" . $db->base_escape_string($hostid));
  if ($result && mysql_num_rows($result)>0) {  // we found some records
     if ($row=mysql_fetch_array($result)) {
       $bMapExact = $row[0];
     }
  }

// get array of geoipaddrid,ipaddr,location,latitude,longitude
$hll = project_specific_host_latlng_prefs($host->id);

$startlat = "";
$startlng = "";
$bnewuser = false;

// hll is a 2-d array of rows for this host:
$hllsize = $hll ? count($hll) : 0;
$buserset = false;
$iRadioAlign = 0;

for ($i = 0; $i < $hllsize; $i++)
{
   if ($hll[$i][0] == 0) { $buserset=true; break; }  
}

// if $hll is empty, put in their current location via geoip
if (!$hll || !$buserset)
{ // no user set settings, so get ip lookup from their current address
  $ipaddr = $_SERVER['REMOTE_ADDR'];
  $iparr = explode(".", $ipaddr);
  if (count($iparr)==4) {
     $ipaddr = $iparr[0] . "." . $iparr[1] . "." . $iparr[2]; 
  }

  // now ipaddr should be just the first 3, lookup in our table
  $sql = "SELECT latitude,longitude FROM qcn_geo_ipaddr WHERE ipaddr='" . $db->base_escape_string($ipaddr) . "'";
  $result=$db->do_query($sql);
  if ($result && mysql_num_rows($result)>0) {  // we found some records
     // seed the startlat & startlng
     if ($row=mysql_fetch_array($result)) {
       $startlat = $row[0];
       $startlng = $row[1];
     }
  }
  else { // I suppose we should do a geoip lookup and insert into geoip table?
     $url = "http://geoip1.maxmind.com/b?l=0q9qp6z4BS40&i=" . $ipaddr . ".1";
     $html = disguise_curl($url);
     if ($html) { // see if we got a good html response from maxmind for this ipaddr
       $iparr = explode(",", $html);
       if (count($iparr) == 5) { // example line:
          // US,CA,Stanford,37.417801,-122.171997
          $country  = $db->base_escape_string($iparr[0]);
          $region   = $db->base_escape_string($iparr[1]);
          $city     = $db->base_escape_string($iparr[2]);
          $startlat = $db->base_escape_string($iparr[3]);
          $startlng = $db->base_escape_string($iparr[4]);

         $sql = "(ipaddr, time_lookup, country, region, city, latitude, longitude) "
            . " values ('" . $ipaddr . "',unix_timestamp(),"
              . "'" . $country . "', "
              . "'" . $region . "', "
              . "'" . $city . "', "
              . $startlat . ", "
              . $startlng . ")";

         $retval = $db->insert("qcn_geo_ipaddr", $sql);
       }
     } 
  }

   // need to push this into the start of the hll array, i.e. may have geoip entries?
   // select geoipaddrid,ipaddr,location,latitude,longitude,levelvalue,levelid,alignid from qcn_host_ipaddr where hostid=$hostid order by geoipaddrid,location";

   $hll[$hllsize][0] = 0;
   $hll[$hllsize][1] = $ipaddr;
   $hll[$hllsize][2] = "geoip";
   $hll[$hllsize][3] = $startlat;
   $hll[$hllsize][4] = $startlng;
   $hll[$hllsize][5] = "";
   $hll[$hllsize][6] = 0;
   $hll[$hllsize][7] = 0;
   $hllsize++; // increment size of hll
   $bnewuser = true;
}

$iRadioAlign = $hll[0][7];  // the first record is the radio btn alignment

page_head("Edit Host Location/Network Address Map Information", null, null, "", true, $psprefs);
echo "<H1>Edit Host Location/Network Address Map Information</H1>";

echo "
 <script type=\"text/javascript\">
  function validate_form(thisForm)
  {// check ip addresses, lat & lng
     for (i = 0; i < 5; i++) {
       var testlat  = getElement(\"lat\" + i).value;
       var testlng  = getElement(\"lng\" + i).value;
       var testip   = getElement(\"ipa\" + i).value;

       if (testlat < -90 || testlat > 90) {
           alert(\"Line #\" + (i+1) + \":  \" + testlat + \",  \" + testlng + \":\\nLatitude must be between -90 and 90\\n(Note: South of Equator is negative)\");
           return false;
       }
       if (testlng < -180 || testlng > 180) {
           alert(\"Line #\" + (i+1) + \":  \" + testlat + \",  \" + testlng + \":\\nLatitude must be between -180 and 180\\n(Note: West of Greenwich is negative)\");
           return false;
       }
       if ((testlat == \"\" && testlng) || (testlat && testlng == \"\")) { // left one blank
           alert(\"Line #\" + (i+1) + \":  \" + testlat + \",  \" + testlng + \":\\nYou must fill in both Latitude and Longitude (or leave both blank)\");
           return false;
       }
       // now test IP address of form A.B.C.D or A.B.C (we just want A.B.C)
       if (testip) {
         var iparr = testip.split(\".\");
         var arrlen = iparr.length;
         if (testip == \"127.0.0.1\" || testip == \"127.0.0\") {
            alert(\"Line #\" + (i+1) + \":  Loopback addresses are not useful for a lat/lng lookup; external IP address needed\");
            return false;
         }
         if (arrlen < 3 || arrlen > 4) {
            alert(\"Line #\" + (i+1) + \":  Invalid IP Address - should be formatted as X.Y.Z or W.X.Y.Z\");
            return false;
         }   
         var bgood = true;
         for (j = 0; j < arrlen; j++) {
            if (iparr[j] < 1 || iparr[j] > 255) { bgood = false; break; }
         }
         if (!bgood) {
            alert(\"Line #\" + (i+1) + \":  Invalid IP Address - each section should be between 1 and 255\");
            return false;
         }

         // so if we're here, but they didn't put in a lat/lng, they may have put in a good iP address with no lat/lng!
         if (testlat == \"\" || testlng == \"\") { // left one blank
           alert(\"Line #\" + (i+1) + \":  IP Address valid, but no Latitude and Longitude entered!\");
           return false;
         }
       }
     }
     return true;  // if made it to here we're OK
  }

</script>
";

echo "<form method=post action=edit_host_info_action.php onsubmit=\"return validate_form(this)\">";

$COLSPAN = "colspan=9";

start_table();
echo "<tr><td $COLSPAN>Enter Up To Five Locations for Your Computer ID # <B>" . $host->id . "</B> Named:  <b>" . $host->domain_name . "</b><BR>\n";
echo "</td></tr>\n";
echo "<tr><td $COLSPAN><div id=\"map\" style=\"width: 640px; height: 480px\"></div></td></tr>\n";
echo "<tr><td $COLSPAN></td></tr>\n";
echo "<tr><td $COLSPAN>Use the following box to lookup an address (i.e. 360 Panama Mall, Stanford, CA)</td></tr>\n";
echo "<tr><td $COLSPAN width=\"50\"><input type=\"text\" name=\"addrlookup\" id=\"addrlookup\" size=50 value=\"\"> <input type=\"button\" name=\"btnaddress\" id=\"btnaddress\" onclick=\"clickedAddressLookup(addrlookup.value)\" value=\"Lookup Address\" size=20></td></tr>\n";
echo "<tr><td $COLSPAN>Try to be as accurate as possible with your location using the Google Map provided.  It will help us pinpoint events!<BR>\n";
echo "Use a different colored marker (selected below) to search and add a new potential location for this computer.<BR><BR>";
echo "You can also (optionally) enter an IP (network) address to associate with/map to this location, and help us sync up your triggers with the lat/long.
  This allows us to make more accurate calculations with the seismic trigger data received.<BR><BR>";
echo "Use the <B>Clear Net Addr</B> button if you have only one location for this computer and sensor and do not 
need to associate different locations with different IP addresses.  In this case the IP address for the line will be blank.<BR>";
echo "<BR><i>We only store the first 3 bytes of your IP address, and never store any address information.  All information is used solely to track seismic events for your area and will not be sold or shared with any other party.  ";
echo "If you don't add any information, we will use an IP/Lat/Lng lookup as a default; this will be shown at the bottom of this page as triggers occur.</i>\n";
echo "<BR>Tip: You can add a single entry (without an IP address) to always use a particular location for your machine (e.g. in case you always/only run QCN at home for example).<BR>\n";
echo "<BR>If you add multiple locations, they should each have an IP address to associate, or if one is left blank, QCN will always use that location (so you could set one location blank when you move, and QCN will use that instead of relying on IP address matching.<BR>";
echo "You can also optionally enter a height/floor level.  This can be useful for building studies.  You can select your preferred level entry (i.e. floor number or height in feet or meters above ground level or sea level.  \n";
  echo "As an example if your computer/sensor is in your basement you may want to enter -1 and then select the Floor so we know that your units are in floors.  If you know it is 65.5 feet above ground level, put in 65.5 and select Feet.<BR>\n";
echo "<BR>Select a different marker for each separate location you want to add - when you are done click the 'Update Info' button.<BR><BR>";
echo "</td></tr>\n";

 
if ($bnewuser) {
  echo "\n<tr><td $COLSPAN><font color=red>Note that the first row is set based on a guess based on your current IP address, this is not saved until you confirm by pressing the 'Update Info' button</font></td</tr>\n";
}
 
  echo "\n<tr><td $COLSPAN><font color=red><i>" .
      "If you have a single location just enter the lat/lng and 'clear' the IP/Network address field " .
      "</i></font></td</tr>\n";

echo "\n<tr><th width=\"5\">Select</th><th>Location Name (optional)</th><th>Latitude</th><th>Longitude</th><th>Level (Height)</th><th>Level Type</th><th>Net (IP) Addr</th><th>Set Net  Addr</th><th>Clear Net Addr</th></tr>\n";

for ($i=0; $i<5; $i++)
{

   echo "<tr>
            <td width=\"5\"><img src=\"" . constant("MAP_ICON_" . $i) . "\">\n";
   echo "   <input type=\"radio\" name=\"radioSelect\" id=\"radioSelect\" value=\"" . $i . "\" onclick=\"clickedRadio("; 
   echo $i . ", this.value==" . $i . ")\" ";

   if ($i == 0) echo "checked";
   echo " ></td>";

   if ($i < $hllsize && $hll[$i][0] == 0) {
     // geoipaddrid,ipaddr,location,latitude,longitude
     echo "
            <td width=\"20\"><input type=\"text\" name=\"lnm" . $i . "\" id=\"lnm" . $i . "\" size=15 value=\"" . stripslashes($hll[$i][2]) . "\"></td>
            <td width=\"20\"><input type=\"text\" name=\"lat" . $i . "\" id=\"lat" . $i . "\" size=15 value=\"" . $hll[$i][3] . "\">
            </td>
            <td width=\"20\"><input type=\"text\" name=\"lng" . $i . "\" id=\"lng" . $i . "\" size=15 value=\"" . $hll[$i][4] . "\">
            </td>
            <td width=\"20\"><input type=\"text\" name=\"lvlv" . $i . "\" id=\"lvlv" . $i . "\" size=15 value=\"" . $hll[$i][5] . "\">
            </td>";

       echo "
            <td width=\"20\"><select name=\"lvlt" . $i . "\" id=\"lvlt" . $i . "\" >";

         for ($jj = 0; $jj <= $clvlmax; $jj++)  {
            echo "<option value=" . $arrLevel[$jj][0];
            if ($hll[$i][6] == $arrLevel[$jj][0]) echo " selected";
            echo ">" . $arrLevel[$jj][1] . "\n";
          }

          echo "</select>
            </td>
            <td width=\"20\"><input type=\"text\" name=\"ipa" . $i . "\" id=\"ipa" . $i . "\" size=15 value=\"" . $hll[$i][1] . "\">
            </td>
          ";
   }
   else {
     echo "
            <td width=\"20\"><input type=\"text\" name=\"lnm" . $i . "\" id=\"lnm" . $i . "\" size=15 value=\"\"></td>
            <td width=\"20\"><input type=\"text\" name=\"lat" . $i . "\" id=\"lat" . $i . "\" size=15 value=\"\">
            </td>
            <td width=\"20\"><input type=\"text\" name=\"lng" . $i . "\" id=\"lng" . $i . "\" size=15 value=\"\">
            </td>
            <td width=\"20\"><input type=\"text\" name=\"lvlv" . $i . "\" id=\"lvlv" . $i . "\" size=15 value=\"\">
            </td>";

       echo "
            <td width=\"20\"><select name=\"lvlt" . $i . "\" id=\"lvlt" . $i . "\" >";

         for ($jj = 0; $jj <= $clvlmax; $jj++)  {
            echo "<option value=" . $arrLevel[$jj][0];
            //if ($hll[$i][5] == $arrLevel[$jj][0]) echo " selected";
            echo ">" . $arrLevel[$jj][1] . "\n";
          }

          echo "</select></td>
            <td width=\"20\"><input type=\"text\" name=\"ipa" . $i . "\" id=\"ipa" . $i . "\" size=15 value=\"\">
            </td>
          ";
   }
/*  hidden fields if needed
              <input type=\"hidden\" name=\"hlat" . $i . "\" id=\"hlat" . $i . "\" value=\"" . $hll[$i][3] . "\">
              <input type=\"hidden\" name=\"hlng" . $i . "\" id=\"hlng" . $i . "\" value=\"" . $hll[$i][4] . "\">
               <input type=\"hidden\" name=\"hlat" . $i . "\" id=\"hlat" . $i . "\">
               <input type=\"hidden\" name=\"hlng" . $i . "\" id=\"hlng" . $i . "\">
             // <input type=\"hidden\" name=\"hipa" . $i . "\" id=\"hipa" . $i . "\" value=\"" . $hll[$i][1] . "\">
               <input type=\"hidden\" name=\"hipa" . $i . "\" id=\"hipa" . $i . "\">
*/

   echo "
            <td width=\"20\"><input type=\"button\" name=\"bipa" . $i . "\" id=\"bipa" . $i . "\" onclick=\"clickedButton(" . $i . ", false)\" value=\"Set Current\" size=10></td>
            <td width=\"20\"><input type=\"button\" name=\"bipac" . $i . "\" id=\"bipac" . $i . "\" onclick=\"clickedButton(" . $i . ", true)\" value=\"Clear\" size=10></td>
      </tr>
   ";

}

// checkbox for allowing location to be shown exactly on the map
echo "<tr><td colspan=2><BR><input type=\"checkbox\" id=\"cbmapexact\" name=\"cbmapexact\" ";
if ($bMapExact) {
  echo "checked";
}
echo ">(Optional)  Show This Computer's Exact Location on Public QCN Maps and Listings <P></td></tr>";

// add radio buttons for type of alignment
echo "<tr><td colspan=2>";
echo "(Optional)  Sensor alignment state: &nbsp&nbsp&nbsp";
echo "<input type=\"radio\" name=\"radioAlign\" value=\"0\" " . ($iRadioAlign==0 ? "checked" : "") . ">Unaligned  ";
echo "&nbsp&nbsp&nbsp&nbsp";
echo "<input type=\"radio\" name=\"radioAlign\" value=\"1\" " . ($iRadioAlign==1 ? "checked" : "") . ">North      ";
echo "&nbsp&nbsp&nbsp&nbsp";
echo "<input type=\"radio\" name=\"radioAlign\" value=\"5\" " . ($iRadioAlign==5 ? "checked" : "") . ">Wall       ";
echo "</td></tr>";

row2("", "<input type=submit value='Update info'>");

   //print prior geoip lookups
   echo "\n<tr><td $COLSPAN></td></tr><tr><th $COLSPAN align=center>Maxmind/GeoIP Lookups for this Host (based on triggers)</th></tr>\n";
   echo "<tr><th></th><th></th><th>Latitude</th><th>Longitude</th><th>Net (IP) Addr</th><th colspan=3></th></tr>\n";
   for ($i = 0 ; $i < $hllsize ; $i++) {
      if ($hll[$i][0] > 0) { // it's a geoip lookup
         // geoipaddrid,ipaddr,location,latitude,longitude
         echo "<tr><td></td><td></td><td>" . $hll[$i][3] . "</td><td>" . $hll[$i][4] . "</td><td>" . $hll[$i][1] . "</td><td colspan=3></td></tr>\n";
      }
   } 

end_table();
echo "   <input type=\"hidden\" name=\"txthidMAPEXACT\" id=\"txthidMAPEXACT\" value=\"" . $bMapExact . "\">\n";
echo "   <input type=\"hidden\" name=\"txthidHOST\" id=\"txthidHOST\" value=\"" . $host->id . "\">\n";
echo "   <input type=\"hidden\" name=\"txthidIP\" id=\"txthidIP\" value=\"" . $_SERVER['REMOTE_ADDR'] . "\">\n";
echo "   <input type=\"hidden\" name=\"txthidLEVEL\" id=\"txthidLEVEL\" value=\"" . $clvlmax . "\">\n";
echo "</form>\n";

page_tail();

?>
