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

$DB = "continual";

$query_base = "select 
t.id as triggerid, t.hostid, h.domain_name, t.ipaddr, t.result_name, t.time_trigger as trigger_time, 
(t.time_received-t.time_trigger) as delay_time, t.time_sync as trigger_sync,
t.sync_offset, t.significance, t.magnitude as trigger_mag, 
t.latitude as trigger_lat, t.longitude as trigger_lon, t.levelvalue, t.levelid, l.description as leveldesc, 
t.file as trigger_file, t.dt as delta_t,
t.numreset, s.description as sensor_description, t.sw_version, t.usgs_quakeid, t.time_filereq as trigger_timereq, 
t.received_file, t.file_url,t.varietyid
FROM
  " . $DB . ".qcn_trigger t
   LEFT JOIN " . $DB . ".host h ON t.hostid = h.id 
   LEFT JOIN " . $DB . ".qcn_sensor s ON t.type_sensor = s.id 
   LEFT OUTER JOIN " . $DB . ".qcn_level l ON t.levelid = l.id 
";


$user = get_logged_in_user(true);

/* sample query string (html query string)

http://qcn.stanford.edu/sensor_ops/trig.php?quake_mag_min=3.0&LatMin=&LatMax=&LonMin=&LonMax=&type_sensor=0&cbUseTime=1&date_start=2009-08-20&time_hour_start=0&time_minute_start=0&date_end=2009-08-21&time_hour_end=0&time_minute_end=0&rb_sort=ttd

*/

// first off get the sensor types
$sqlsensor = "SELECT id,description FROM " . $DB . ".qcn_sensor order by id";
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

$nresults = get_int("nresults", true);
$last_pos = get_int("last_pos", true);

$bUseArchive = get_int("cbUseArchive", true);
$bUseFile  = get_int("cbUseFile", true);
//$bUseQuake = get_int("cbUseQuake", true);
$bUseLat   = get_int("cbUseLat", true);
$bUseSensor = get_int("cbUseSensor", true);
$bUseTime  = get_int("cbUseTime", true);

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

$strHostID = get_int("HostID", true);
$strHostName = get_str("HostName", true);

// end of gets

if (!$nresult) $nresult = 1000;

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

if (!$sortOrder) $sortOrder = "ttd";  // trigger time desc is default sort order

if ($nresult) {
    $entries_to_show = $nresult;
} else {
    $entries_to_show = 1000;
}
$page_entries_to_show = $entries_to_show;

if ($last_pos) {
    $start_at = $last_pos;
} else {
    $start_at = 0;
}

//page_head("Trigger Download");
echo "<html><head>
<script type=\"text/javascript\" src=\"calendarDateInput.js\">

/***********************************************
* Jason's Date Input Calendar- By Jason Moon http://calendar.moonscript.com/dateinput.cfm
* Script featured on and available at http://www.dynamicdrive.com
* Keep this notice intact for use.
***********************************************/

</script>
  <title>Trigger Download</title>
</head><body " . BODY_COLOR . ">\n";
 echo "<h5>";
  echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=\"6\"><b><a href=\"dl.php\">".PROJECT.":</a>  Trigger Download</b></font></td></tr></table>\n";


// if no constraints then at least use time within past day
if ((!$bUseHost || !$strHostID || !$strHostName) && !$bUseFile && !$bUseLat && !$bUseTime && !$bUseSensor) {
   $bUseTime= 1;
   $tsNow = time();
   // date_start=2009-08-20
   $dateStart = date("Y-m-d", $tsNow);
   $dateEnd = date("Y-m-d", $tsNow + (3600*24));
   $whereString .= " AND t.time_trigger BETWEEN "
      . "unix_timestamp('" . $dateStart . " 00:00:00')" 
      . " AND unix_timestamp('" . $dateEnd . " 00:00:00')";
}

$sortString = "t.time_trigger DESC";

echo "
<form name='formSelect' method=\"get\" action=dl.php >
<HR>
Constraints:<br><br>
  <input type=\"checkbox\" id=\"cbUseFile\" name=\"cbUseFile\" value=\"1\" " . ($bUseFile ? "checked" : "") . "> Only Show If Files Received
<BR>
<BR>
  <input type=\"checkbox\" id=\"cbUseHost\" name=\"cbUseHost\" value=\"1\" " . ($bUseHost? "checked" : "") . "> Show Specific Host (enter host ID # or host name)<BR>
    Host ID: <input id=\"HostID\" name=\"HostID\" value=\"$strHostID\">
    <BR>Host Name: <input id=\"HostName\" name=\"HostName\" value=\"$strHostName\">
<BR><BR>
  <input type=\"checkbox\" id=\"cbUseLat\" name=\"cbUseLat\" value=\"1\" " . ($bUseLat ? "checked" : "") . "> Use Lat/Lon Constraint (+/- 90 Lat, +/- 180 Lon)
<BR>
  Lat Min: <input id=\"LatMin\" name=\"LatMin\" value=\"$strLatMin\">
  Lat Max: <input id=\"LatMax\" name=\"LatMax\" value=\"$strLatMax\">
  Lon Min: <input id=\"LonMin\" name=\"LonMin\" value=\"$strLonMin\">
  Lon Max: <input id=\"LonMax\" name=\"LonMax\" value=\"$strLonMax\">
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
  <input type=\"checkbox\" id=\"cbUseTime\" name=\"cbUseTime\" value=\"1\" " . ($bUseTime ? "checked" : "") 
     . "> Use Time Constraint
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

if ($dateStart) {
  echo "<script>DateInput('date_start', true, 'YYYY-MM-DD', '$dateStart')</script>";
}
else {
  echo "<script>DateInput('date_start', true, 'YYYY-MM-DD')</script>";
}

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

for ($i = 0; $i < 60; $i+=10) {
   echo "<option value=$i ";
   if ($i == $timeMinuteStart) echo "selected";
   echo ">" . sprintf("%02d", $i);
}

echo "
</select>

</td><td>

End Time (UTC):";

if ($dateEnd) {
  echo "<script>DateInput('date_end', true, 'YYYY-MM-DD', '$dateEnd')</script>";
}
else {
  echo "<script>DateInput('date_end', true, 'YYYY-MM-DD')</script>";
}

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

for ($i = 0; $i < 60; $i+=10) {
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

echo "<BR><BR>
  Max Triggers Per Page:  <input id=\"nresult\" name=\"nresult\" value=\"$nresult\">
<BR>";

// end the form
echo "<BR><BR>
   <input type=\"submit\" value=\"Submit Constraints\" />
   </form> <H7>";

$whereString = "t.varietyid>=0";

if ($bUseFile) {
   $whereString .= " AND t.received_file = 100 ";
}

/*
if ($bUseQuake) {
   $whereString .= " AND t.usgs_quakeid>0 AND q.magnitude >= " . $quake_mag_min;
}
*/

if ($bUseHost) {
  if ($strHostID) {
     $whereString .= " AND t.hostid = " . $strHostID;
  }
  else if ($strHostName) {
     $whereString .= " AND h.domain_name = '" . $strHostName . "'";
  }
}

if ($bUseLat) {
   $whereString .= " AND t.latitude BETWEEN $strLatMin AND $strLatMax AND t.longitude BETWEEN $strLonMin AND $strLonMax ";
}

if ($bUseSensor) {
   $whereString .= " AND t.type_sensor=$type_sensor ";
}

if ($bUseTime) {
   $whereString .= " AND t.time_trigger BETWEEN unix_timestamp('" . $dateStart . " " . sprintf("%02d", $timeHourStart) . ":" . sprintf("%02d", $timeMinuteStart) . ":00') " 
        . " AND unix_timestamp('" . $dateEnd . " " . sprintf("%02d", $timeHourEnd) . ":" . sprintf("%02d", $timeMinuteEnd+1) . ":00') ";
}

$sortString = "t.time_trigger DESC";

switch($sortOrder)
{
   case "maga":
      $sortString = "q.magnitude ASC, t.time_trigger DESC";
      break;
   case "magd":
      $sortString = "q.magnitude DESC, t.time_trigger DESC";
      break;
   case "tta":
      $sortString = "t.time_trigger ASC";
      break;
   case "ttd":
      $sortString = "t.time_trigger DESC";
      break;
   case "lata":
      $sortString = "t.latitude ASC, t.longitude ASC";
      break;
   case "latd":
      $sortString = "t.latitude DESC, t.longitude DESC";
      break;
   case "lona":
      $sortString = "t.longitude ASC, t.latitude ASC";
      break;
   case "lond":
      $sortString = "t.longitude DESC, t.latitude DESC";
      break;
   case "hosta":
      $sortString = "t.hostid ASC";
      break;
   case "hostd":
      $sortString = "t.hostid DESC";
      break;
}

$query = $query_base . " WHERE " . $whereString . " ORDER BY " . $sortString;

//print "<BR><BR>$query<BR><BR>";

//$main_query = $q->get_select_query($entries_to_show, $start_at);
        if ($entries_to_show) {
            if ($start_at) {
                $main_query = $query . " limit $start_at,$entries_to_show";
            } else {
                $main_query = $query . " limit $entries_to_show";
            }
        } else {
                $main_query = $query;
        }

//$count = 1e6;

$count = query_count($query);

if ($count < $start_at + $entries_to_show) {
    $entries_to_show = $count - $start_at;
}

$last = $start_at + $entries_to_show;

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

   $colorPing = "\"#999999\"";
   echo "<H5>(NB: <font color=$colorPing>grey text</font> is a 'real' trigger, otherwise it's a 10-minute file)</H5>\n";

/*
echo "<BR><BR>
$bUseTime<BR>
$timeHourStart<BR>
$timeMinuteStart<BR>
$timeHourEnd<BR>
$timeMinuteEnd<BR>
<BR><BR>";
*/
 
$start_1_offset = $start_at + 1;

echo "
    <p>$count records match the query.
    Displaying $start_1_offset to $last.<p>
";

$url = $q->get_url("dl.php");
if ($detail) {
    $url .= "&detail=$detail";
}

$queryString = "&nresult=$page_entries_to_show"
       . "&cbUseFile=$bUseFile"
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

$result = mysql_query($main_query);
if ($result) {
    echo "<form name=\"formDetail\" method=\"post\" action=dlreq.php >";
    start_table();
    qcn_trigger_header();
    $i = 0;
    $bgcolor = "";
    $strTableFont = "";
    $strFontEnd = "";
    while ($res = mysql_fetch_object($result)) {
         if ($i++ % 2) {
             $bgcolor = "\"#F4F4F4\"";
         }
         else {
             $bgcolor = "\"#FFFFFF\"";
         }
         if ($res->varietyid==0) { // it's a real trigger
           $strTableFont = "<font color=$colorPing>";
           $strFontEnd = "</font>";
         }
         else {
           $strTableFont = "";
           $strFontEnd   = "";
         }
        qcn_trigger_detail($res, $bgcolor, $strTableFont, $strFontEnd);
    }
    end_table();
    mysql_free_result($result);
} else {
    echo "<h2>No results found</h2>";
}

echo "
  <input type=\"submit\" value=\"Submit Download Requests\" />
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

page_tail();


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
        <th>Elev</th>
        <th>Elev Units</th>
        <th>NumReset</th>
        <th>DT</th>
        <th>Sensor</th>
        <th>Version</th>
        <th>Received File</th>
        <th>File Download</th>
        </tr>
    ";
}


function qcn_trigger_detail($res, $bgcolor, $strTableFont, $strFontEnd) 
{
    $sensor_type = $res->sensor_description;
    echo "
        <tr bgcolor=$bgcolor>
        <td>$strTableFont<input type=\"checkbox\" name=\"cb_reqfile[]\" id=\"cb_reqfile[]\" value=\"$res->triggerid\"" . 
           ($res->received_file == 100 ? " " : " disabled ") . ">$strFontEnd</td>
        <td>$strTableFont$res->triggerid$strFontEnd</td>
        <td>$strTableFont<a href=\"show_host_detail.php?hostid=$res->hostid\">" . host_name_by_id($res->hostid) . "</a>$strFontEnd</td>
        <td>$strTableFont$res->ipaddr$strFontEnd</td>
        <td>$strTableFont$res->result_name$strFontEnd</td>
        <td>$strTableFont" . time_str($res->trigger_time) . "$strFontEnd</td>
        <td>$strTableFont" . round($res->delay_time, 2) . "$strFontEnd</td>
        <td>$strTableFont" . time_str($res->trigger_sync) . "$strFontEnd</td>
        <td>$strTableFont$res->sync_offset$strFontEnd</td>
        <td>$strTableFont$res->trigger_mag$strFontEnd</td>
        <td>$strTableFont$res->significance$strFontEnd</td>
        <td>$strTableFont" . round($res->trigger_lat,4) . "$strFontEnd</td>
        <td>$strTableFont" . round($res->trigger_lon,4) . "$strFontEnd</td>
        <td>$strTableFont" . round($res->levelvalue,4) . "$strFontEnd</td>
        <td>$strTableFont" . $res->leveldesc . "$strFontEnd</td>
        <td>$strTableFont" . ($res->numreset ? $res->numreset : 0) . "$strFontEnd</td>
        <td>$strTableFont$res->delta_t$strFontEnd</td>
        <td>$strTableFont$sensor_type$strFontEnd</td>
        <td>$strTableFont$res->sw_version$strFontEnd</td>";
        
        echo "
        <td>$strTableFont" . ($res->received_file == 100 ? " Yes " : " No " ) . "$strFontEnd</td>";

        if ($res->file_url) {
          echo "<td>$strTableFont<a href=\"" . $res->file_url . "\">Download</a>$strFontEnd</td>";
        }
        else {
          echo "<td>$strTableFont" . "N/A$strFontEnd</td>";
        }
/*
        if ($res->usgs_quakeid) {
           echo "<td>$strTableFont<a href=\"db_action.php?table=usgs_quake&id=$res->usgs_quakeid\">$res->usgs_quakeid</a>$strFontEnd</td>";
           echo "<td>$strTableFont$res->quake_magnitude$strFontEnd</td>";
           echo "<td>$strTableFont" . time_str($res->quake_time) . "$strFontEnd</td>";
           echo "<td>$strTableFont$res->quake_lat$strFontEnd</td>";
           echo "<td>$strTableFont$res->quake_lon$strFontEnd</td>";
           echo "<td>$strTableFont$res->description$strFontEnd</td>";
           echo "<td>$strTableFont$res->guid$strFontEnd</td>";
        }
        else {
           echo "<td>$strTableFontN/A$strFontEnd</td>";
           echo "<td>$strTableFont&nbsp$strFontEnd</td>";
           echo "<td>$strTableFont&nbsp$strFontEnd</td>";
           echo "<td>$strTableFont&nbsp$strFontEnd</td>";
           echo "<td>$strTableFont&nbsp$strFontEnd</td>";
           echo "<td>$strTableFont&nbsp$strFontEnd</td>";
           echo "<td>$strTableFont&nbsp$strFontEnd</td>";
        }
*/
    echo "</font></tr>
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


?>
