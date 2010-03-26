<?php
require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/db_ops.inc");

db_init();

$user = get_logged_in_user(true);
// user->donated means they can do download stuff (donated is a SETI@home field reused here)
if (!$user->id || !$user->donated) {
   echo "You are not authorized to use this page.  Please contact a QCN staff member.";
   exit();
}

$queryNew = "select q.id as quakeid, q.time_utc as quake_time, q.magnitude as quake_magnitude, 
q.depth_km as quake_depth, q.latitude as quake_lat,
q.longitude as quake_lon, q.description, q.url, q.guid,
t.id as triggerid, t.hostid, t.ipaddr, t.result_name, t.time_trigger as trigger_time, 
(t.time_received-t.time_trigger) as delay_time, t.time_sync as trigger_sync,
t.sync_offset, t.significance, t.magnitude as trigger_mag, 
t.latitude as trigger_lat, t.longitude as trigger_lon, t.file as trigger_file, t.dt as delta_t,
t.numreset, s.description as sensor_description, t.sw_version, t.usgs_quakeid, t.time_filereq as trigger_timereq, 
t.received_file, t.file_url
FROM
  qcnalpha.qcn_trigger t LEFT OUTER JOIN usgs_quake q ON t.usgs_quakeid = q.id
   LEFT JOIN qcn_sensor s ON t.type_sensor = s.id 
";

$queryOld = "select q.id as quakeid, q.time_utc as quake_time, q.magnitude as quake_magnitude, 
q.depth_km as quake_depth, q.latitude as quake_lat,
q.longitude as quake_lon, q.description, q.url, q.guid,
t.id as triggerid, t.hostid, t.ipaddr, t.result_name, t.time_trigger as trigger_time, 
(t.time_received-t.time_trigger) as delay_time, t.time_sync as trigger_sync,
t.sync_offset, t.significance, t.magnitude as trigger_mag, 
t.latitude as trigger_lat, t.longitude as trigger_lon, t.file as trigger_file, t.dt as delta_t,
t.numreset, s.description as sensor_description, t.sw_version, t.usgs_quakeid, t.time_filereq as trigger_timereq, 
t.received_file, t.file_url
FROM
  qcnarchive.qcn_trigger t LEFT OUTER JOIN usgs_quake q ON t.usgs_quakeid = q.id
   LEFT JOIN qcn_sensor s ON t.type_sensor = s.id 
";

// full querystring
// http://qcn.stanford.edu/continual_dl/trig.php?cbCSV=1&cbUseLat=1&LatMin=-39&LatMax=-30&LonMin=-76&LonMax=-69&cbUseSensor=1&type_sensor=100&cbUseTime=1&date_start=2010-03-24&time_hour_start=0&time_minute_start=0&date_end=2010-03-25&time_hour_end=0&time_minute_end=0&rb_sort=ttd

// sort order options: tta/d  hosta/d  maga/d lata/d lona/d


// first off get the sensor types
$sqlsensor = "select id,description from qcn_sensor order by id";
$result = mysql_query($sqlsensor);
$i = 0;
$arrSensor = array();
if ($result) {
    while ($res = mysql_fetch_array($result)) {
       $arrSensor[$i] = $res;
       $i++;
    }
    mysql_free_result($result);
}

$detail = null;
$show_aggregate = false;

$q = new SqlQueryString();

// start $_GET

$nresults = get_int("nresults", true);
$last_pos = get_int("last_pos", true);

$bUseCSV = get_int("cbUseCSV", true);
$bUseArchive = get_int("cbUseArchive", true);
$bUseFile  = get_int("cbUseFile", true);
$bUseQuake = get_int("cbUseQuake", true);
$bUseLat   = get_int("cbUseLat", true);
$bUseSensor = get_int("cbUseSensor", true);
$bUseTime  = get_int("cbUseTime", true);
$bUseHost = get_int("cbUseHost", true);
$strHostID = get_int("HostID", true);
$strHostName = get_str("HostName", true);

$quake_mag_min = get_str("quake_mag_min", true);

$type_sensor = get_int("type_sensor", true);
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

$sortOrder = get_str("rb_sort", true);

// end $_GET

if (!$quake_mag_min) $quake_mag_min = "3.0";  // set minimum quake mag cutoff


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

//page_head("QCN Trigger Listing");
echo "<html><head>
<script type=\"text/javascript\" src=\"calendarDateInput.js\">

/***********************************************
* Jason's Date Input Calendar- By Jason Moon http://calendar.moonscript.com/dateinput.cfm
* Script featured on and available at http://www.dynamicdrive.com
* Keep this notice intact for use.
***********************************************/

</script>
  <title>QCN Trigger Listing</title>
</head><body " . BODY_COLOR . ">\n";
 echo "<h5>";
  echo "<table><tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=\"6\"><b><a href=\"trig.php\">".PROJECT.":</a>  QCN Trigger Listing </b></font></td></tr></table>\n";


// if no constraints then at least use quakes as otherwise we'll have too many i.e. a million triggers
if (!$bUseFile && !$bUseQuake && !$bUseLat && !$bUseTime && !$bUseSensor) $bUseQuake = 1;

echo "
<form name='formSelect' method=\"get\" action=trig.php >
<HR>
Constraints:<br><br>
  <input type=\"checkbox\" id=\"cbUseFile\" name=\"cbUseFile\" value=\"1\" " . ($bUseFile ? "checked" : "") . "> Only Show If Files Received
 <BR><BR>
  <input type=\"checkbox\" id=\"cbUseQuake\" name=\"cbUseQuake\" value=\"1\" " . ($bUseQuake ? "checked" : "") . "> Show Matching USGS Quakes 
  &nbsp&nbsp
  Minimum Magnitude: <input id=\"quake_mag_min\" name=\"quake_mag_min\" value=\"$quake_mag_min\">
<BR><BR>
  <input type=\"checkbox\" id=\"cbUseHost\" name=\"cbUseHost\" value=\"1\" " . ($bUseHost? "checked" : "") . "> Show Specific Host (enter host ID # or host name)<BR>
    Host ID: <input id=\"HostID\" name=\"HostID\" value=\"$strHostID\">
    <BR>Host Name: <input id=\"HostName\" name=\"HostName\" value=\"$strHostName\">
<BR><BR>

  <input type=\"checkbox\" id=\"cbUseLat\" name=\"cbUseLat\" value=\"1\" " . ($bUseLat ? "checked" : "") . "> Use Lat/Lon Constraint (+/- 90 Lat, +/- 180 Lon)
<BR>
  Lat Min: <input id=\"LatMin\" name=\"LatMin\" value=\"" . $strLatMin . "\">
  Lat Max: <input id=\"LatMax\" name=\"LatMax\" value=\"" . $strLatMax . "\">
  Lon Min: <input id=\"LonMin\" name=\"LonMin\" value=\"" . $strLonMin . "\">
  Lon Max: <input id=\"LonMax\" name=\"LonMax\" value=\"" . $strLonMax . "\">
<BR><BR>


  <input type=\"checkbox\" id=\"cbUseSensor\" name=\"cbUseSensor\" value=\"1\" " . ($bUseSensor ? "checked" : "") . "> Use Sensor Constraint

<select name=\"type_sensor\" id=\"type_sensor\">

";
echo "<H5>";

  for ($i = 0; $i < sizeof($arrSensor); $i++)  {
     echo "<option value=" . $arrSensor[$i][0];
     if ($type_sensor == $arrSensor[$i][0]) echo " selected";
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

if (!$dateStart) {
  $dateStart = date("Y/m/d", time());  
}
if (!$dateEnd) {
  $dateEnd = date("Y/m/d", time() + (3600*24));  
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

   echo "</select>";


// end the form
echo "<BR><BR>
<input type=\"checkbox\" id=\"cbUseCSV\" name=\"cbUseCSV\" value=\"1\" " . ($bUseCSV? "checked" : "") . "> Create Text/CSV File of Triggers?
<BR><BR>
   <input type=\"submit\" value=\"Submit Constraints\" />
   </form> <H7>";

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
   $whereString .= " AND t.usgs_quakeid>0 AND q.magnitude >= " . $quake_mag_min;
}

if ($bUseLat) {
   $whereString .= " AND t.latitude BETWEEN $strLatMin AND $strLatMax AND t.longitude BETWEEN $strLonMin AND $strLonMax ";
}

if ($bUseSensor) {
   $whereString .= " AND t.type_sensor=$type_sensor ";
}

if ($bUseTime) {
   $whereString .= " AND t.time_received BETWEEN unix_timestamp('" . $dateStart . " " . sprintf("%02d", $timeHourStart) . ":" . sprintf("%02d", $timeMinuteStart) . ":00') " 
        . " AND unix_timestamp('" . $dateEnd . " " . sprintf("%02d", $timeHourEnd) . ":" . sprintf("%02d", $timeMinuteEnd) . ":00') ";
}

/*$queryNew = "select q.id as quakeid, q.time_utc as quake_time, q.magnitude as quake_magnitude, 
q.depth_km as quake_depth, q.latitude as quake_lat,
q.longitude as quake_lon, q.description, q.url, q.guid,
t.id as triggerid, t.hostid, t.ipaddr, t.result_name, t.time_trigger as trigger_time, 
(t.time_received-t.time_trigger) as delay_time, t.time_sync as trigger_sync,
t.sync_offset, t.significance, t.magnitude as trigger_mag, 
t.latitude as trigger_lat, t.longitude as trigger_lon, t.file as trigger_file, t.dt as delta_t,
t.numreset, s.description as sensor_description, t.sw_version, t.usgs_quakeid, t.time_filereq as trigger_timereq, 
t.received_file, t.file_url
FROM
  qcnalpha.qcn_trigger t LEFT OUTER JOIN usgs_quake q ON t.usgs_quakeid = q.id
   LEFT JOIN qcn_sensor s ON t.type_sensor = s.id 
";
*/
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

//$count = 1e6;

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

 
$start_1_offset = $start_at + 1;
if (!$bUseCSV) {
echo "
    <p>$count records match the query.
    Displaying $start_1_offset to $last.<p>
";
}

$url = $q->get_url("trig.php");
if ($detail) {
    $url .= "&detail=$detail";
}
$queryString = "&nresults=$page_entries_to_show"
       . "&cbUseHost=$bUseHost"
       . "&cbUseArchive=$bUseArchive"
       . "&cbUseFile=$bUseFile"
       . "&cbUseCSV=$bUseCSV"
       . "&cbUseQuake=$bUseQuake"
       . "&cbUseLat=$bUseLat"
       . "&cbUseTime=$bUseTime"
       . "&cbUseSensor=$bUseSensor"
       . "&type_sensor=$type_sensor"
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


if ($bUseCSV) {   
   // tmp file name tied to user ID & server time
   $fileTemp = sprintf("data/%ld_u%d.csv", time(), $user->id);
   $ftmp = fopen($fileTemp, "w");
   if ($ftmp) {
      fwrite($ftmp, qcn_trigger_header_csv());
   }
   else {
      $fileTemp = ""; // to check for status later on down
   }
}


$result = mysql_query($main_query);
if ($result) {
    echo "<form name=\"formDetail\" method=\"get\" action=trigreq.php >";
    start_table();
    if (!$bUseCSV && !$ftmp) qcn_trigger_header();
    while ($res = mysql_fetch_object($result)) {
        if ($bUseCSV && $ftmp) {
           fwrite($ftmp, qcn_trigger_detail_csv($res));
        }
        else { 
           qcn_trigger_detail($res);
        }
    }
    end_table();
    mysql_free_result($result);
} else {
    echo "<h2>No results found</h2>";
}

if ($bUseCSV && $ftmp) {
  echo "<BR><BR><A HREF=\"" . $fileTemp . "\">Download CSV/Text File Here (File Size " . sprintf("%7.2f", (filesize($fileTemp) / 1e6)) . " MB)</A> (you may want to right-click to save locally)<BR><BR>";
}
else {
 echo "
  <input type=\"submit\" value=\"Submit Trigger File Requests\" />
  <input type=\"button\" value=\"Check All\" onclick=\"SetAllCheckBoxes('formDetail', 'cb_reqfile[]', true);\" >
  <input type=\"button\" value=\"Uncheck All\" onclick=\"SetAllCheckBoxes('formDetail', 'cb_reqfile[]', false);\" >
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

function qcn_trigger_header_csv() {
   return "TriggerID, HostID, IPAddr, ResultName, TimeTrigger, Delay, TimeSync, SyncOffset, "
    . "Magnitude, Significance, Latitude, Longitude, NumReset, DT, Sensor, Version, Time File Req, "
    . "Received File, File Download, USGS ID, Quake Magnitude, Quake Time, "
    . "Quake Lat, Quake Long, USGS GUID, Quake Desc"
    . "\n";
}

function qcn_trigger_detail_csv($res)
{
    $quakestuff = "";
    if ($res->usgs_quakeid) {
          $quakestuff = $res->usgs_quakeid . "," .
             $res->quake_magnitude . "," . 
             time_str_csv($res->quake_time) . "," .
             $res->quake_lat . "," .
             $res->quake_lon . "," .
             $res->guid . "," .
             $res->description; 
    }
    else {
          $quakestuff = ",,,,,,";
    }

    return $res->triggerid . "," . $res->hostid . "," . $res->ipaddr . "," .
       $res->result_name . "," . time_str_csv($res->trigger_time) . "," . round($res->delay_time, 2) . "," .
        time_str_csv($res->trigger_sync) . "," . $res->sync_offset . "," . $res->trigger_mag . "," . $res->significance . "," .
        round($res->trigger_lat, 8) . "," . round($res->trigger_lon, 8) . "," . ($res->numreset ? $res->numreset : 0) . "," .
        $res->delta_t . "," . $res->sensor_description . "," . $res->sw_version . "," .
        time_str_csv($res->trigger_timereq) . "," . ($res->received_file == 100 ? " Yes " : " No " ) . "," .
        ($res->file_url ? $res->file_url : "N/A") . "," .
        $quakestuff .
        "\n";

}

function qcn_trigger_header() {
    echo "
        <tr>
        <th>Request?</th>
        <th>ID</th>
        <th>HostID</th>
        <th>IP Addr</th>
        <th>Result</th>
        <th>TimeTrig</th>
        <th>Delay(s)</th>
        <th>TimeSync</th>
        <th>SyncOffset(s)</th>
        <th>Magnitude</th>
        <th>Significance</th>
        <th>Latitude</th>
        <th>Longitude</th>
        <th>NumReset</th>
        <th>DT</th>
        <th>Sensor</th>
        <th>Version</th>
        <th>Time File Req</th>
        <th>Received File</th>
        <th>File Download</th>
        <th>USGS ID</th>
        <th>Quake Magnitude</th>
        <th>Quake Time (UTC)</th>
        <th>Quake Latitude</th>
        <th>Quake Longitude</th>
        <th>Quake Description</th>
        <th>USGS GUID</th>
        </tr>
    ";
}


function qcn_trigger_detail($res) 
{
    $sensor_type = $res->sensor_description;
    echo "
        <tr>
        <td><input type=\"checkbox\" name=\"cb_reqfile[]\" id=\"cb_reqfile[]\" value=\"$res->triggerid\"" . 
       ($res->received_file == 100 || $res->trigger_timereq>0 ? " disabled " : " " ) . 
       "></td>
        <td>$res->triggerid</td>
        <td><a href=\"db_action.php?table=host&id=$res->hostid\">" . host_name_by_id($res->hostid) . "</a></td>
        <td>$res->ipaddr</td>
        <td>$res->result_name</td>
        <td>" . time_str($res->trigger_time) . "</td>
        <td>" . round($res->delay_time, 2) . "</td>
        <td>" . time_str($res->trigger_sync) . "</td>
        <td>$res->sync_offset</td>
        <td>$res->trigger_mag</td>
        <td>$res->significance</td>
        <td>" . round($res->trigger_lat,4) . "</td>
        <td>" . round($res->trigger_lon,4) . "</td>
        <td>" . ($res->numreset ? $res->numreset : 0) . "</td>
        <td>$res->delta_t</td>
        <td>$sensor_type</td>
        <td>$res->sw_version</td>";
        
        echo "
        <td>" . time_str($res->trigger_timereq) . "</td>
        <td>" . ($res->received_file == 100 ? " Yes " : " No " ) . "</td>";

        if ($res->file_url) {
          echo "<td><a href=\"" . $res->file_url . "\">Download</a></td>";
        }
        else {
          echo "<td>N/A</td>";
        }

        if ($res->usgs_quakeid) {
           echo "<td><a href=\"db_action.php?table=usgs_quake&id=$res->usgs_quakeid\">$res->usgs_quakeid</a></td>";
           echo "<td>$res->quake_magnitude</td>";
           echo "<td>" . time_str($res->quake_time) . "</td>";
           echo "<td>$res->quake_lat</td>";
           echo "<td>$res->quake_lon</td>";
           echo "<td>$res->description</td>";
           echo "<td>$res->guid</td>";
        }
        else {
           echo "<td>N/A</td>";
           echo "<td>&nbsp</td>";
           echo "<td>&nbsp</td>";
           echo "<td>&nbsp</td>";
           echo "<td>&nbsp</td>";
           echo "<td>&nbsp</td>";
           echo "<td>&nbsp</td>";
        }

    echo "</tr>
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
    if ($x == 0) return "";
    return gmdate('Y/m/d H:i:s', $x); // . " UTC";
}

?>
