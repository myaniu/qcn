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

$query = "select id, fname, lname, email_addr, addr1, addr2, city, region, postcode, country, latitude, longitude, phone, fax, bshare_coord, bshare_map, bshare_ups, cpu_type, cpu_os, cpu_age, cpu_floor, cpu_admin, cpu_permission, cpu_firewall, cpu_proxy, cpu_internet, cpu_unint_power, sensor_distribute, comments,
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
  liquefaction
from qcn_ramp_participant WHERE active=1";
$order = "order by country, lname, fname";

$detail = null;
$show_aggregate = false;

$q = new SqlQueryString();

// start $_GET

$nresults = get_int("nresults", true);
$last_pos = get_int("last_pos", true);

$bUseCSV = get_int("cbUseCSV", true);
$bUseRegional = get_int("cbUseRegional", true);
/*$bUseArchive = get_int("cbUseArchive", true);
$bUseFile  = get_int("cbUseFile", true);
$bUseQuake = get_int("cbUseQuake", true);
$bUseQCNQuake = get_int("cbUseQCNQuake", true);
$bUseLat   = get_int("cbUseLat", true);
$bUseSensor = get_int("cbUseSensor", true);
$bUseTime  = get_int("cbUseTime", true);
$bUseHost = get_int("cbUseHost", true);
$strHostID = get_int("HostID", true);
*/
$strCountry = get_str("db_country", true);

/*
$qcn_sensorid = get_int("qcn_sensorid", true);
$dateStart = get_str("date_start", true);
$dateEnd   = get_str("date_end", true);
$strLonMin = get_str("LonMin", true);
$strLonMax = get_str("LonMax", true);
$strLatMin = get_str("LatMin", true);
$strLatMax = get_str("LatMax", true);

$timeHourStart   = get_int("time_hour_start", true);
$timeMinuteStart = get_int("time_minute_start", true);

$timeHourEnd   = get_int("time_hour_end", true);
$timeMinuteEnd = get_int("time_minute_end", true);
*/

$sortOrder = get_str("rb_sort", true);

// end $_GET

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

if (!$nresults) $nresults = 1000;
if ($nresults) {
    $entries_to_show = $nresults;
} else {
    $entries_to_show = 100;
}
$page_entries_to_show = $entries_to_show;

if ($last_pos) {
    $start_at = $last_pos;
} else {
    $start_at = 0;
}

page_head("QCN RAMP Participants");

echo "<H2>QCN Ramp Participants</H2>";
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

echo "<form name=\"formFilter\" method=\"get\" action=\"ramp.php\" >";

if (!$strCountry || $strCountry == "None") $strCountry = "International";
echo "Filter by Country:
<select name=db_country id=db_country>";
     print_country_select($strCountry);
echo "
     </select></td></tr>
<BR><BR>
<input type=\"checkbox\" id=\"cbUseRegional\" name=\"cbUseRegional\" value=\"1\" " . ($bUseRegional? "checked" : "") . "> Show only Regional RAMP Signups?
<BR><BR>
<input type=\"checkbox\" id=\"cbUseCSV\" name=\"cbUseCSV\" value=\"1\" " . ($bUseCSV? "checked" : "") . "> Create Text/CSV File of Triggers?
<BR><BR>
   <input type=\"submit\" value=\"Submit Constraints\" />
   </form> <H7>";

if ($strCountry != "International" && $strCountry != "None") $query .= " AND country='$strCountry' ";

if ($bUseRegional) $query .= " AND ramp_type != 'G' ";

//print "<BR><BR>$query<BR><BR>";

/*
$whereString = "t.varietyid=0 ";

if ($bUseFile) {
   $whereString .= " AND t.received_file = 100 ";
}

if ($bUseHost) {
  if ($strHostID) {
     $whereString .= " AND t.hostid = " . $strHostID;
  }
  else if ($strHostName) {
     $whereString .= " AND h.domain_name = '" . $strHostName . "'";
  }
}

if ($bUseQuake) {
   $whereString .= " AND t.qcn_quakeid>0 AND q.magnitude >= " . $quake_mag_min;
}

if ($bUseQCNQuake) {
   $whereString .= " AND t.qcn_quakeid>0 AND q.guid like 'QCN_%' ";
}

if ($bUseLat) {
   $whereString .= " AND t.latitude BETWEEN $strLatMin AND $strLatMax AND t.longitude BETWEEN $strLonMin AND $strLonMax ";
}

if ($bUseSensor) {
   $whereString .= " AND t.qcn_sensorid=$qcn_sensorid ";
}

if ($bUseTime) {
   $whereString .= " AND t.time_received BETWEEN unix_timestamp('" . $dateStart . " " . sprintf("%02d", $timeHourStart) . ":" . sprintf("%02d", $timeMinuteStart) . ":00') " 
        . " AND unix_timestamp('" . $dateEnd . " " . sprintf("%02d", $timeHourEnd) . ":" . sprintf("%02d", $timeMinuteEnd) . ":00') ";
}
*/

/*$queryNew = "select q.id as quakeid, q.time_utc as quake_time, q.magnitude as quake_magnitude, 
q.depth_km as quake_depth, q.latitude as quake_lat,
q.longitude as quake_lon, q.description, q.url, q.guid,
t.id as triggerid, t.hostid, t.ipaddr, t.result_name, t.time_trigger as trigger_time, 
(t.time_received-t.time_trigger) as delay_time, t.time_sync as trigger_sync,
t.sync_offset, t.significance, t.magnitude as trigger_mag, 
t.latitude as trigger_lat, t.longitude as trigger_lon, t.file as trigger_file, t.dt as delta_t,
t.numreset, s.description as sensor_description, t.sw_version, t.qcn_quakeid, t.time_filereq as trigger_timereq, 
t.received_file, t.file_url
FROM
  sensor.qcn_trigger t LEFT OUTER JOIN qcn_quake q ON t.qcn_quakeid = q.id
   LEFT JOIN qcn_sensor s ON t.qcn_sensorid = s.id 
";
*/

/*
$sortString = "trigger_time DESC";
switch($sortOrder)
{
   case "maga":
      $sortString = "quake_magnitude ASC, trigger_time DESC";
      break;
   case "magd":
      $sortString = "quake_magnitude DESC, trigger_time DESC";
      break;
   case "tta":
      $sortString = "trigger_time ASC";
      break;
   case "ttd":
      $sortString = "trigger_time DESC";
      break;
   case "lata":
      $sortString = "trigger_lat ASC, trigger_lon ASC";
      break;
   case "latd":
      $sortString = "trigger_lat DESC, trigger_lon DESC";
      break;
   case "lona":
      $sortString = "trigger_lon ASC, trigger_lat ASC";
      break;
   case "lond":
      $sortString = "trigger_lon DESC, trigger_lat DESC";
      break;
   case "hosta":
      $sortString = "hostid ASC";
      break;
   case "hostd":
      $sortString = "hostid DESC";
      break;
   case "qda":
      $sortString = "quake_distance_km ASC, trigger_time DESC";
      break;
   case "qdd":
      $sortString = "quake_distance_km DESC, trigger_time DESC";
      break;
}

// CMC really need to look at archive table too
if ($bUseArchive) {
  $query .=
   $queryNew . " WHERE " . $whereString
       . " UNION "
       . $queryOld . " WHERE " . $whereString
       . " ORDER BY " . $sortString
     ;
}
else {
  $query .=
   $queryNew . " WHERE " . $whereString
       . " ORDER BY " . $sortString
     ;
}
*/

//print "<BR><BR>$query<BR><BR>";

//$main_query = $q->get_select_query($entries_to_show, $start_at);
        if (!$bUseCSV && $entries_to_show) {
            if ($start_at) {
                $main_query = $query . " limit $start_at,$entries_to_show";
            } else {
                $main_query = $query . " limit $entries_to_show";
            }
        } else {
                $main_query = $query;
        }

$count = 0;
$last = 0;

if (!$bUseCSV) {
  $count = query_count($query);

  if ($count < $start_at + $entries_to_show) {
      $entries_to_show = $count - $start_at;
  }

  $last = $start_at + $entries_to_show;
}

// For display, convert query string characters < and > into 'html form' so
// that they will be displayed.
//
//$html_text=str_replace('<', '&lt;', str_replace('>', '&gt;', $main_query));
//echo "<p>Query: <b>$html_text</b><p>\n";
/*
echo "
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
<HR>
";
*/
 
$start_1_offset = $start_at + 1;
if (!$bUseCSV) {
echo "
    <p>$count records match the query.
    Displaying $start_1_offset to $last.<p>
";
}

$url = $q->get_url("ramp.php");
if ($detail) {
    $url .= "&detail=$detail";
}

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
*/
$queryString = "&nresults=$page_entries_to_show"
       . "&cbUseCSV=$bUseCSV"
       . "&country=$strCountry"
       . "&rb_sort=$sortOrder";

//echo "<hr>$url<hr><br>\n";
if ($start_at || $last < $count) {
    echo "<table border=\"1\"><tr><td width=\"100\">";
    if ($start_at) {
        $prev_pos = $start_at - $page_entries_to_show;
        if ($prev_pos < 0) {
            $prev_pos = 0;
        }
        echo "
            <a href=\"$url&last_pos=$prev_pos" . $queryString . "\">Previous $page_entries_to_show</a><br>
        ";
    }
    echo "</td><td width=100>";
    if ($last < $count) {
        echo "
            <a href=\"$url&last_pos=$last" . $queryString . "\">Next $page_entries_to_show</a><br>
        ";
    }
    echo "</td></tr></table>";
}

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

$result = mysql_query($main_query);
if ($result) {
    echo "<form name=\"formDelete\" method=\"get\" action=\"ramp.php\" >";
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
  <input type=\"submit\" value=\"Delete Checked?\" disabled />
  </form>";

  if ($start_at || $last < $count) {
    echo "<table border=\"1\"><tr><td width=\"100\">";
    if ($start_at) {
        $prev_pos = $start_at - $page_entries_to_show;
        if ($prev_pos < 0) {
            $prev_pos = 0;
        }
        echo "
            <a href=\"$url&last_pos=$prev_pos" . $queryString . "\">Previous $page_entries_to_show</a><br>
        ";
    }
    echo "</td><td width=100>";
    if ($last < $count) {
        echo "
            <a href=\"$url&last_pos=$last" . $queryString . "\">Next $page_entries_to_show</a><br>
        ";
    }
    echo "</td></tr></table>";
  }
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
     . "Thursday, ThursdayTime, Friday, FridayTime, Saturday, SatTime, YearsHost, RampType, QuakeDamage, Liquefaction, Comments"
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
  $entry . "\"\n";

}

function qcn_ramp_header() {
   echo "
       <tr>
       <th>Delete?</th>
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
       <td><input type=\"checkbox\" name=\"cb_delete[]\" id=\"cb_delete[]\" value=\"$res->id\" disabled>" . "</td>
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
     nl2br($res->comments) . "</td>
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
