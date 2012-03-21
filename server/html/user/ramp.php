<?php
require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/db_ops.inc");
require_once("../project/common.inc");

db_init();

set_time_limit(600);

$user = get_logged_in_user(true);
// authenticate admin-level user
qcn_admin_user_auth($user, true);


$query = "SELECT id, fname, lname, email_addr, addr1, addr2, city, region, postcode, country, latitude, longitude, phone, fax, bshare_coord, bshare_map, bshare_ups, cpu_type, cpu_os, cpu_age, cpu_floor, cpu_admin, cpu_permission, cpu_firewall, cpu_proxy, cpu_internet, cpu_unint_power, sensor_distribute, comments,
 loc_home,
 loc_business,
 loc_affix_perm,
 loc_self_install,
 loc_day_install_sunday,
 loc_time_install_sunday,
 loc_day_install_monday,
 loc_time_install_monday,
 loc_day_install_tuesday,
 loc_time_install_tuesday,
 loc_day_install_wednesday,
 loc_time_install_wednesday,
 loc_day_install_thursday,
 loc_time_install_thursday,
 loc_day_install_friday,
 loc_time_install_friday,
 loc_day_install_saturday,
 loc_time_install_saturday,
 loc_years_host, 
  ramp_type, 
  quake_damage, 
  liquefaction,
 from_unixtime(time_added) time_add, 
 from_unixtime(time_edit) time_ed,
 completed,
 from_unixtime(time_completed) time_comp
from qcn_ramp_participant WHERE active=1 ";
$order = "order by country, lname, fname";

$show_aggregate = false;

// start $_POST

$nresults = post_int("nresults", true);
$last_pos = post_int("last_pos", true);

$bUseCSV = post_int("cbUseCSV", true);
$bUseRegional = post_int("cbUseRegional", true);
$bUseComp = post_int("cbUseComp", true);
/*$bUseArchive = post_int("cbUseArchive", true);
$bUseFile  = post_int("cbUseFile", true);
$bUseQuake = post_int("cbUseQuake", true);
$bUseQCNQuake = post_int("cbUseQCNQuake", true);
$bUseLat   = post_int("cbUseLat", true);
$bUseSensor = post_int("cbUseSensor", true);
$bUseTime  = post_int("cbUseTime", true);
$bUseHost = post_int("cbUseHost", true);
$strHostID = post_int("HostID", true);
*/
$strCountry = post_str("db_country", true);

/*
$qcn_sensorid = post_int("qcn_sensorid", true);
$dateStart = post_str("date_start", true);
$dateEnd   = post_str("date_end", true);
$strLonMin = post_str("LonMin", true);
$strLonMax = post_str("LonMax", true);
$strLatMin = post_str("LatMin", true);
$strLatMax = post_str("LatMax", true);

$timeHourStart   = post_int("time_hour_start", true);
$timeMinuteStart = post_int("time_minute_start", true);

$timeHourEnd   = post_int("time_hour_end", true);
$timeMinuteEnd = post_int("time_minute_end", true);
*/

$sortOrder = post_str("rb_sort", true);

// end $_POST

/*
// make sure these are in the right order, as the sql "between" will fail if max < min!
// people may forget that lon -76 is less than -72 as it may make more sense to think -72 to -76
if ($strLonMax < $strLonMin) {
   $temp = $strLonMax;
   $strLonMax = $strLonMin;
   $strLonMin = $temp;
}

if ($strLatMax < $strLatMin) {
   $temp = $strLatMax;
   $strLatMax = $strLatMin;
   $strLatMin = $temp;
}

if (!$sortOrder) $sortOrder = "ttd";  // triger time desc is default sort order
*/

page_head("QCN RAMP Participants");

/*
  for ($i = 0; $i < sizeof($arrSensor); $i++)  {
     echo "<option value=" . $arrSensor[$i][0];
     if ($qcn_sensorid == $arrSensor[$i][0]) echo " selected";
     echo ">" . $arrSensor[$i][1] . "\n";
  }
echo "</select>
<BR><BR>
  <input type=\"checkbox\" id=\"cbUseArchive\" name=\"cbUseArchive\" value=\"1\" " . ($bUseArchive ? "checked" : "") . "> 
Include the Archive Database (Triggers Older Than Two Months - May Take Awhile!)
  <BR><BR>
  <input type=\"checkbox\" id=\"cbUseTime\" name=\"cbUseTime\" value=\"1\" " . ($bUseTime ? "checked" : "") . "> Use Time Constraint
<BR>
";


echo "<ul><table><tr><td>
Start Time (UTC):";

// set last four hours for start, current time + 1 for end
$timeStart = gmdate("U", time() - (3600*4));
$timeEnd = gmdate("U", time() + 3600);
if (!$dateStart) {
  $dateStart = date("Y/m/d", $timeStart);
  // now set the times based on timeStart & timeEnd
  $timeHourStart = date("H", $timeStart);
  $timeMinuteStart = "00"; //date("i", $timeStart);
}
if (!$dateEnd) {
  $dateEnd = date("Y/m/d", $timeEnd);
  $timeHourEnd   = date("H", $timeEnd);
  $timeMinuteEnd = "00";
}

echo "<script>DateInput('date_start', true, 'YYYY-MM-DD', '$dateStart')</script>";

echo "<select name=\"time_hour_start\" id=\"time_hour_start\">
";

for ($i = 0; $i < 24; $i++) {
   echo "<option value=$i ";
   if ($i == $timeHourStart) echo "selected";
   echo ">" . sprintf("%02d", $i);
}

echo "</select>
:
<select name=\"time_minute_start\" id=\"time_minute_start\">";

for ($i = 0; $i < 60; $i++) {
   echo "<option value=$i ";
   if ($i == $timeMinuteStart) echo "selected";
   echo ">" . sprintf("%02d", $i);
}

echo "
</select>

</td><td>

End Time (UTC):";

  echo "<script>DateInput('date_end', true, 'YYYY-MM-DD', '$dateEnd')</script>";

echo "<select name=\"time_hour_end\">
";

for ($i = 0; $i < 24; $i++) {
   echo "<option value=$i ";
   if ($i == $timeHourEnd) echo "selected";
   echo ">" . sprintf("%02d", $i);
}

echo "</select>
:
<select name=\"time_minute_end\" id=\"time_minute_end\">
";

for ($i = 0; $i < 60; $i++) {
   echo "<option value=$i ";
   if ($i == $timeMinuteEnd) echo "selected";
   echo ">" . sprintf("%02d", $i);
}

echo "
</select> </tr></table> </UL>
";

echo "<BR>Sort Order: ";

echo "<H7>";

echo "<select name=\"rb_sort\" id=\"rb_sort\">
";
   echo "<option value=\"maga\" ";
   if ($sortOrder == "maga") echo "selected";
   echo ">Magnitude (Ascending)";

   echo "<option value=\"magd\" ";
   if ($sortOrder == "magd") echo "selected";
   echo ">Magnitude (Descending)";

   echo "<option value=\"tta\" ";
   if ($sortOrder == "tta") echo "selected";
   echo ">Trigger Time (Earliest First)";

   echo "<option value=\"ttd\" ";
   if ($sortOrder == "ttd") echo "selected";
   echo ">Trigger Time (Latest First)";

   echo "<option value=\"lata\" ";
   if ($sortOrder == "lata") echo "selected";
   echo ">Latitude (Ascending)";

   echo "<option value=\"latd\" ";
   if ($sortOrder == "latd") echo "selected";
   echo ">Latitude (Descending)";

   echo "<option value=\"lona\" ";
   if ($sortOrder == "lona") echo "selected";
   echo ">Longitude (Ascending)";

   echo "<option value=\"lond\" ";
   if ($sortOrder == "lond") echo "selected";
   echo ">Longitude (Descending)";

   echo "<option value=\"hosta\" ";
   if ($sortOrder == "hosta") echo "selected";
   echo ">Host ID (Ascending)";

   echo "<option value=\"hostd\" ";
   if ($sortOrder == "hostd") echo "selected";
   echo ">Host ID (Descending)";

   echo "<option value=\"qda\" ";
   if ($sortOrder == "qda") echo "selected";
   echo ">Quake Distance (Ascending)";

   echo "<option value=\"qdd\" ";
   if ($sortOrder == "qdd") echo "selected";
   echo ">Quake Distance (Decending)";

   echo "</select>";
*/

// end the form

$aryComp = array();
$numComp = 0;
if(!empty($_REQUEST['submitComplete'])) {
  if (isset($_POST["cb_comp"])) $aryComp = $_POST["cb_comp"];
  $numComp = count($aryComp);
}
if ($numComp) { // we have completion requests to process
   $updateComp = "UPDATE sensor.qcn_ramp_participant SET completed=1, time_completed=unix_timestamp() WHERE id=";
   for ($i = 0; $i < $numComp; $i++) {
      mysql_query($updateComp . $aryComp[$i]); 
   }
}

echo "<form name=\"formRAMP\" method=\"post\" action=\"ramp.php\" >";

if (!$strCountry || $strCountry == "None") $strCountry = "International";
echo "Filter by Country:
<select name=db_country id=db_country>";
     print_country_select($strCountry);
echo "
     </select></td></tr>
<BR><BR>
<input type=\"checkbox\" id=\"cbUseRegional\" name=\"cbUseRegional\" value=\"1\" " . ($bUseRegional? "checked" : "") . "> Show only Regional RAMP Signups?
<BR>
<input type=\"checkbox\" id=\"cbUseComp\" name=\"cbUseComp\" value=\"1\" " . ($bUseComp? "checked" : "") . "> Show Completed Signups?
<BR><BR>
<input type=\"checkbox\" id=\"cbUseCSV\" name=\"cbUseCSV\" value=\"1\" " . ($bUseCSV? "checked" : "") . "> Create Text/CSV File of Triggers?
<BR><BR>
   <input type=\"submit\" name=\"submitConstraint\" id=\"submitConstraint\" value=\"Submit Constraints\" />
    <H7>";
   //</form> <H7>";

if ($strCountry != "International" && $strCountry != "None") $query .= " AND country='$strCountry' ";

if ($bUseRegional) $query .= " AND ramp_type != 'G' ";

if (! $bUseComp)
   $query .= " AND completed = 0 ";

$count = query_count($query);

echo "
<BR><BR>$count records retrieved.<BR><BR>
";

if ($count) {
echo "
<HR>
<script type=\"text/javascript\">
   
function SetAllCheckBoxes(FormName, FieldName, CheckValue)
{
	if(!document.forms[FormName])
		return;
	var objCheckBoxes = document.forms[FormName].elements[FieldName];
	if(!objCheckBoxes)
		return;
	var countCheckBoxes = objCheckBoxes.length;
	if(!countCheckBoxes)
		objCheckBoxes.checked = CheckValue;
	else
		// set the check value for all check boxes
		for(var i = 0; i < countCheckBoxes; i++)
	           if (! objCheckBoxes[i].disabled)
                      objCheckBoxes[i].checked = CheckValue;
}
</script>
  <input type=\"button\" value=\"Check All Completed\" onclick=\"SetAllCheckBoxes('formRAMP', 'cb_comp[]', true); SetAllCheckBoxes('formRAMP', 'cb_comp[]', true);\" >\n
  <input type=\"button\" value=\"Uncheck All Completed\" onclick=\"SetAllCheckBoxes('formRAMP', 'cb_comp[]', false); SetAllCheckBoxes('formRAMP', 'cb_comp[]', false);\" >\n
  <BR><BR>\n
";
} // just show if there's a bunch of recs to show
 
/*
$queryString = "&nresults=$page_entries_to_show"
       . "&cbUseHost=$bUseHost"
       . "&cbUseArchive=$bUseArchive"
       . "&cbUseFile=$bUseFile"
       . "&cbUseCSV=$bUseCSV"
       . "&cbUseQuake=$bUseQuake"
       . "&cbUseQCNQuake=$bUseQCNQuake"
       . "&cbUseLat=$bUseLat"
       . "&cbUseTime=$bUseTime"
       . "&cbUseSensor=$bUseSensor"
       . "&qcn_sensorid=$qcn_sensorid"
       . "&date_start=$dateStart"
       . "&date_end=$dateEnd"
       . "&LonMin=$strLonMin"
       . "&LonMax=$strLonMax"
       . "&LatMin=$strLatMin"
       . "&LatMax=$strLatMax"
       . "&quake_mag_min=$quake_mag_min"
       . "&time_hour_start=$timeHourStart"
       . "&time_minute_start=$timeMinuteStart"
       . "&time_hour_end=$timeHourEnd"
       . "&time_minute_end=$timeMinuteEnd"
       . "&HostID=$strHostID"
       . "&HostName=$strHostName"
       . "&rb_sort=$sortOrder";
$queryString = "&nresults=$page_entries_to_show"
       . "&cbUseCSV=$bUseCSV"
       . "&country=$strCountry"
       . "&rb_sort=$sortOrder";
*/

echo "<p>\n";

$ftmp = 0;

if ($bUseCSV) {   
   // tmp file name tied to user ID & server time
   $fileTemp = sprintf("data/%ld_u%d.csv", time(), $user->id);
   $ftmp = fopen($fileTemp, "w");
   if ($ftmp) {
      fwrite($ftmp, qcn_ramp_header_csv());
   }
   else {
      $fileTemp = ""; // to check for status later on down
   }
}

$result = mysql_query($query);
if ($result) {
    start_table();
    if (!$bUseCSV && !$ftmp) qcn_ramp_header();
    while ($res = mysql_fetch_object($result)) {
        if ($bUseCSV && $ftmp) {
           fwrite($ftmp, qcn_ramp_detail_csv($res));
        }
        else { 
           qcn_ramp_detail($res);
        }
    }
    end_table();
    mysql_free_result($result);
} else {
    echo "<h2>No results found</h2>";
}

if ($bUseCSV && $ftmp) {
  echo "<BR><BR><A HREF=\"" . $fileTemp . "\">Download CSV/Text File Here (File Size " . sprintf("%7.2f", (filesize($fileTemp) / 1e6)) . " MB)</A> (you may want to right-click to save locally)<BR><BR>";
  echo "NB:  In Excel, you may have to make a new worksheet, then select 'Data', then 'Get External Data', then 'Import Text File'.  Select the file you have
downloaded (i.e. 123431_u15.csv), then select 'Delimited' and then ',' (comma) on the 'Next' screen.
   ";
}
else {
 echo "<BR><BR>
  <input type=\"submit\" name=\"submitComplete\" id=\"submitComplete\" value=\"Submit Completions?\" />
  </form>";
}
    if ($bUseCSV && $ftmp) {
      fclose($ftmp);
    }


page_tail();

function qcn_logical($log)
{
  return ($log == 0 ? "N" : ($log == 1 ? "Y" : "M"));
}

function qcn_ramp_header_csv() {
  // note microsoft excel has a bug - crashes if first two chars are "ID"!
   return "id, FirstName, LastName, Email, Address1, Address2, City, Region, "
     . "PostCode, Country, Latitude, Longitude, Phone, Fax, ShareCoord, ShareMap, ShareUPS, "
     . "CPUType, OpSys, CPUAgeYrs, CPUFloor#, AdminRts, Permission, Firewall, Proxy, Internet, UnintPower, DistribSensor, "
     . "Home, Business, AffixPerm, SelfInstall, Sunday, SundayTime, Monday, MondayTime, Tuesday, TuesdayTime, Wednesday, WedsTime, "
     . "Thursday, ThursdayTime, Friday, FridayTime, Saturday, SatTime, YearsHost, RampType, QuakeDamage, Liquefaction, Comments, "
     . "Time Added, Time Edited, Completed?, Time Completed"
     . "\n";
}

function qcn_ramp_detail_csv($res)
{
  $entry = $res->comments;
  $entry = str_replace("\r\n\r\n", "</p><p>", $entry);
  $entry = str_replace("\n", "<br />", $entry);
    return 
       $res->id . "," . 
       "\"$res->fname\"" . "," . 
       "\"$res->lname\"" . "," .
       "\"$res->email_addr\"" . "," .
       "\"$res->addr1\"" . "," .
       "\"$res->addr2\"" . "," .
       "\"$res->city\"" . "," .
       "\"$res->region\"" . "," .
       "\"$res->postcode\"" . "," .
       "\"$res->country\"" . "," .
       "$res->latitude" . "," .
       "$res->longitude" . "," .
       "\"$res->phone\"" . "," .
       "\"$res->fax\"" . ",\"" .
       qcn_logical($res->bshare_coord) . "\",\"" .
       qcn_logical($res->bshare_map) . "\",\"" .
       qcn_logical($res->bshare_ups) . "\"," .
       "\"$res->cpu_type\"" . "," .
       "\"$res->cpu_os\"" . "," .
       "$res->cpu_age" . "," .
       "$res->cpu_floor" . ",\"" .
       qcn_logical($res->cpu_admin) . "\",\"" .
       qcn_logical($res->cpu_permission) . "\",\"" .
       qcn_logical($res->cpu_firewall) . "\",\"" .
       qcn_logical($res->cpu_proxy) . "\",\"" .
       qcn_logical($res->cpu_internet) . "\",\"" .
       qcn_logical($res->cpu_unint_power) . "\",\"" .
       qcn_logical($res->sensor_distribute) . "\",\"" .
  qcn_logical($res->loc_home) . "\",\"" .
  qcn_logical($res->loc_business) . "\",\"" .
  qcn_logical($res->loc_affix_perm) . "\",\"" .
  qcn_logical($res->loc_self_install) . "\",\"" .
  qcn_logical($res->loc_day_install_sunday) . "\",\"" .
  $res->loc_time_install_sunday . "\",\"" .
  qcn_logical($res->loc_day_install_monday) . "\",\"" .
  $res->loc_time_install_monday . "\",\"" .
  qcn_logical($res->loc_day_install_tuesday) . "\",\"" .
  $res->loc_time_install_tuesday . "\",\"" .
  qcn_logical($res->loc_day_install_wednesday) . "\",\"" .
  $res->loc_time_install_wednesday . "\",\"" .
  qcn_logical($res->loc_day_install_thursday) . "\",\"" .
  $res->loc_time_install_thursday . "\",\"" .
  qcn_logical($res->loc_day_install_friday) . "\",\"" .
  $res->loc_time_install_friday . "\",\"" .
  qcn_logical($res->loc_day_install_saturday) . "\",\"" .
  $res->loc_time_install_saturday . "\"," .
  $res->loc_years_host . ", \"" . 
  $res->ramp_type . "\", \"" .
  $res->quake_damage . "\", \"" .
  qcn_logical($res->liquefaction) . "\", \"" .
  $entry . "\", \"" .
  $res->time_add . "\", \"" .
  $res->time_ed . "\", \"" .
  qcn_logical($res->completed) . "\", \"" .
  $res->time_comp . "\", \"" .
    "\n";

}

function qcn_ramp_header() {
   echo "
       <tr>
       <th>Complete?</th>
       <th>FirstName</th>
       <th>LastName</th>
       <th>Email</th>
       <th>Address1</th>
       <th>Address2</th>
       <th>City</th>
       <th>Region</th>
       <th>PostCode</th>
       <th>Country</th>
       <th>Latitude</th>
       <th>Longitude</th>
       <th>Phone</th>
       <th>Fax</th>
       <th>ShareCoord</th>
       <th>ShareMap</th>
       <th>ShareUPS</th>
       <th>CPUType</th>
       <th>OpSys</th>
       <th>CPUAge</th>
       <th>CPUFloor</th>
       <th>AdminRts</th>
       <th>Permission</th>
       <th>Firewall</th>
       <th>Proxy</th>
       <th>Internet</th>
       <th>UnintPower</th>
       <th>DistribSensor</th>
       <th>Home</th>
       <th>Business</th>
       <th>AffixPerm</th>
       <th>SelfInstall</th>
       <th>Sunday</th>
       <th>SundayTime</th>
       <th>Monday</th>
       <th>MondayTime</th>
       <th>Tuesday</th>
       <th>TuesdayTime</th>
       <th>Wednesday</th>
       <th>WedsTime</th>
       <th>Thursday</th>
       <th>ThursdayTime</th>
       <th>Friday</th>
       <th>FridayTime</th>
       <th>Saturday</th>
       <th>SatTime</th>
       <th>YearsHost</th>
       <th>RampType</th>
       <th>QuakeDamage</th>
       <th>Liquefaction</th>
       <th>Comments</th>
       <th>Time Added</th>
       <th>Time Edited</th>
       <th>Completed?</th>
       <th>Time Completed</th>
       </tr>
     ";
}

/*
select id, fname, lname, email_addr, addr1, addr2, city, region, postcode, country, latitude, longitude, phone, fax bshare_coord, bshare_map, bshare_ups, cpu_type, cpu_os, cpu_age, cpu_floor, cpu_admin, cpu_permission, cpu_firewall, cpu_proxy, cpu_internet, cpu_unint_power, sensor_distribute, comments from qcn_ramp_participant WHE
*/
function qcn_ramp_detail($res) 
{
    echo "
       <tr>
       <td><input type=\"checkbox\" name=\"cb_comp[]\" id=\"cb_comp[]\" value=\"$res->id\" " 
          . ($res->completed ? " disabled " : "") . ">" . "</td>
       <td>$res->fname</td> 
       <td>$res->lname</td>
       <td>$res->email_addr</td>
       <td>$res->addr1</td>
       <td>$res->addr2</td>
       <td>$res->city</td>
       <td>$res->region </td>
       <td>$res->postcode</td>
       <td>$res->country</td>
       <td>$res->latitude</td>
       <td>$res->longitude</td>
       <td>$res->phone</td>
       <td>$res->fax</td>
       <td>$res->bshare_coord</td>
       <td>$res->bshare_map</td>
       <td>$res->bshare_ups</td>
       <td>$res->cpu_type</td>
       <td>$res->cpu_os</td>
       <td>$res->cpu_age</td>
       <td>$res->cpu_floor</td>
       <td>$res->cpu_admin</td>
       <td>$res->cpu_permission</td>
       <td>$res->cpu_firewall</td>
       <td>$res->cpu_proxy</td>
       <td>$res->cpu_internet</td>
       <td>$res->cpu_unint_power</td>
       <td>$res->sensor_distribute</td><td>" .
  qcn_logical($res->loc_home) . "</td><td>" .
  qcn_logical($res->loc_business) . "</td><td>" .
  qcn_logical($res->loc_affix_perm) . "</td><td>" .
  qcn_logical($res->loc_self_install) . "</td><td>" .
  qcn_logical($res->loc_day_install_sunday) . "</td><td>" .
  $res->loc_time_install_sunday . "</td><td>" .
  qcn_logical($res->loc_day_install_monday) . "</td><td>" .
  $res->loc_time_install_monday . "</td><td>" .
  qcn_logical($res->loc_day_install_tuesday) . "</td><td>" .
  $res->loc_time_install_tuesday . "</td><td>" .
  qcn_logical($res->loc_day_install_wednesday) . "</td><td>" .
  $res->loc_time_install_wednesday . "</td><td>" .
  qcn_logical($res->loc_day_install_thursday) . "</td><td>" .
  $res->loc_time_install_thursday . "</td><td>" .
  qcn_logical($res->loc_day_install_friday) . "</td><td>" .
  $res->loc_time_install_friday . "</td><td>" .
  qcn_logical($res->loc_day_install_saturday) . "</td><td>" .
  $res->loc_time_install_saturday . "</td><td>" .
  $res->loc_years_host . "</td><td>" .
  $res->ramp_type . "</td><td>" .
  $res->quake_damage . "</td><td>" .
  qcn_logical($res->liquefaction) . "</td>" .
   "<td width=\"15%\">" .
     nl2br($res->comments) . "</td><td>" .
  $res->time_add . "</td><td>" .
  $res->time_ed . "</td><td>" .
  qcn_logical($res->completed) . "</td><td>" .
  $res->time_comp . "</td>" . "
       </tr>
    ";
}

function query_count($myquery) {
        $count_query = "select count(*) as cnt from ( $myquery ) mydb ";
        $result = mysql_query($count_query);
        if (!$result) return 0;
        $res = mysql_fetch_object($result);
        mysql_free_result($result);
        return $res->cnt;
}

function time_str_csv($x) {
    if (!$x) return "";
    return gmdate('Y/m/d H:i:s', $x); // . " UTC";
}

?>
