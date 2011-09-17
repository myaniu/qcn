<?php
require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/sqlquerystring.inc");
require_once("../project/common.inc");

db_init();

set_time_limit(600);

// make sure they're logged in
$user = get_logged_in_user(true);


// if no querystring then we are coming in "fresh" so just show a small subset ie usgs-detected quakes
$bNoQuery = ($_SERVER["QUERY_STRING"] == null);

// authenticate admin-level user
qcn_admin_user_auth($user, true);

// archive cutoff time is two months prior to the first of the current month
//$queryArchiveTime = "SELECT unix_timestamp( concat(year(now()), '/', month(now()), '/01 00:00:00') ) - (60 * 24 * 3600) archive_time";
$try_replica = false;
$unixtimeArchive = mktime(0, 0, 0, date("n"), 1, date("Y")) - (60*24*3600); 

    $config = get_config();
        $user = parse_config($config, "<db_user>");
        $pass = parse_config($config, "<db_passwd>");
    $host = null;
    if ($try_replica == true) {
          $host = parse_config($config, "<replica_db_host>");
    }
    if ($host == null) {
          $host = parse_config($config, "<db_host>");
    }
    if ($host == null) {
        $host = "localhost";
    }
    $link = mysql_pconnect($host, $user, $pass);
    if (!$link) {
        return 1;
    }
$db_name = parse_config($config, "<db_name>");
$db_archive = "";

if ($db_name == "qcnalpha") {
  $db_archive = "qcnarchive";
}
else if ($db_name== "continual") {
  $db_archive = "contarchive";
}

$queryBase = "select q.id as quakeid, q.time_utc as quake_time, q.magnitude as quake_magnitude, 
q.depth_km as quake_depth, q.latitude as quake_lat, 
q.longitude as quake_lon, q.description, q.url, q.guid,
round(lat_lon_distance_m(q.latitude, q.longitude, t.latitude, t.longitude) / 1000.0, 3) as quake_distance_km,
t.id as triggerid, t.hostid, t.ipaddr, t.result_name, t.time_trigger as trigger_time, 
(t.time_received-t.time_trigger) as delay_time, t.time_sync as trigger_sync,
t.sync_offset, t.significance, t.magnitude as trigger_mag, 
t.latitude as trigger_lat, t.longitude as trigger_lon, t.file as trigger_file, t.dt as delta_t,
t.numreset, s.description as sensor_description, t.sw_version, t.qcn_quakeid, t.time_filereq as trigger_timereq, 
t.received_file, REPLACE_ARCHIVE is_archive, t.varietyid, q.url quake_url
FROM REPLACE_DB.qcn_trigger t LEFT OUTER JOIN qcnalpha.qcn_quake q ON t.qcn_quakeid = q.id
   LEFT JOIN qcnalpha.qcn_sensor s ON t.qcn_sensorid = s.id 
";

// full querystring
// /continual_dl/trdl.php?cbCSV=1&cbUseLat=1&LatMin=-39&LatMax=-30&LonMin=-76&LonMax=-69&cbUseSensor=1&qcn_sensorid=100&cbUseTime=1&date_start=2010-03-24&time_hour_start=0&time_minute_start=0&date_end=2010-03-25&time_hour_end=0&time_minute_end=0&rb_sort=ttd

// sort order options: tta/d  hosta/d  maga/d lata/d lona/d
// get the archive time
$queryTime = "SELECT value_int+1 as archive_time FROM qcnalpha.qcn_constant WHERE description='ArchiveTime'";
$result = mysql_query($queryTime);
if ($result) {
  $row = mysql_fetch_row($result);
  $unixtimeArchive = $row[0];
  mysql_free_result($result);
}

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
$bUseQCNQuake = get_int("cbUseQCNQuake", true);
$bUseLat   = get_int("cbUseLat", true);
$bUseSensor = get_int("cbUseSensor", true);
$bUseTime  = get_int("cbUseTime", true);
$bUseHost = get_int("cbUseHost", true);
$strHostID = get_int("HostID", true);
$strHostName = get_str("HostName", true);

$quake_mag_min = get_str("quake_mag_min", true);

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

$sortOrder = get_str("rb_sort", true);

$bVarietyNormal = get_str("cbVarietyNormal", true);
$bVarietyPing = get_str("cbVarietyPing", true);
$bVarietyContinual = get_str("cbVarietyContinual", true);

if ($bVarietyNormal == "" && $bVarietyPing == "" && $bVarietyContinual == "") {
   $bVarietyNormal = "0"; // at least show normal trigs
   if ($db_name == "continual") {
     $bVarietyContinual = "2"; // also show continual trigs
   }
}

/*if ($strLatMin =="") {$strLatMin =  "-90.0";}
if ($strLatMax =="") {$strLatMax =  " 90.0";}
if ($strLonMin =="") {$strLonMin = "-180.0";}
if ($strLonMax =="") {$strLonMax =  "180.0";}*/


// end $_GET

//if (!$quake_mag_min && (!$timeHourStart || !$dateStart)) $quake_mag_min = "3.0";  // set minimum quake mag cutoff
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

if (!$nresults) $nresults = 200;
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
// echo "<font size=\"1\">";
//  echo "<table><tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=\"6\"><b><a href=\"trdl.php\">".PROJECT.":</a>  QCN Trigger Listing </b></font></td></tr></table>\n";


// if no constraints then at least use quakes as otherwise we'll have too many i.e. a million triggers
//if (!$bUseFile && !$bUseQuake && !$bUseQCNQuake && !$bUseLat && !$bUseTime && !$bUseSensor) $bUseQuake = 1;
// actually use time constraint, if quake page use the
//if (!$bUseFile && !$bUseQuake && !$bUseQCNQuake && !$bUseLat && !$bUseTime && !$bUseSensor) $bUseTime = 1;
if ($bNoQuery) {
  $bUseFile = 1;
  $bUseQuake = 1;
  $bUseQCNQuake = 1;
  $quake_mag_min = 3.0;
}

echo "<form name='formSelect' method=\"get\" action=trdl.php >";
//echo "<HR>Constraints:<br><br>";


echo "<table align=\"center\" border=\"1\" bgcolor=\"#ddd\"><tr><td>";
echo "<h1><center>Search QCN Data:</center></h1>";
echo "
<table bgcolor=\"#fff\"><tr><td width=\"300\">
  <p><input type=\"checkbox\" id=\"cbUseLat\" name=\"cbUseLat\" value=\"1\" " . ($bUseLat ? "checked" : "") . ">Location Search: 
  <BR>
  <table>
    <tr>
      <td></td>
      <td bgcolor=\"#DDD\">North Lat:<br> <input id=\"LatMax\" name=\"LatMax\" value=\"" . $strLatMax . "\" size=\"8\"></td>
      <td></td>
    </tr>
    <tr>
      <td bgcolor=\"#DDD\">West Lon:<br> <input id=\"LonMin\" name=\"LonMin\" value=\"" . $strLonMin . "\" size=\"8\"></td>
      <td align=\"center\">+/- 90 Lat <br>+/-180 Lon</td>
      <td bgcolor=\"#DDD\">East Lon:<br> <input id=\"LonMax\" name=\"LonMax\" value=\"" . $strLonMax . "\" size=\"8\"></td>
    </tr>
    <tr>
      <td></td>
      <td bgcolor=\"#DDD\">South Lat:<br> <input id=\"LatMin\" name=\"LatMin\" value=\"" . $strLatMin . "\" size=\"8\"></td>
      <td></td>
    </tr>
  </table><br>";


echo "  <input type=\"checkbox\" id=\"cbUseFile\" name=\"cbUseFile\" value=\"0\" " . ($bUseFile ? "checked" : "") . "> Only Show If Files Received
  <p><input type=\"checkbox\" id=\"cbUseQuake\" name=\"cbUseQuake\" value=\"1\" " . ($bUseQuake ? "checked" : "") . "> Match USGS Quakes:&nbsp;
  Mag >= &nbsp;<input id=\"quake_mag_min\" name=\"quake_mag_min\" value=\"$quake_mag_min\" size=\"4\">

  <p><input type=\"checkbox\" id=\"cbUseQCNQuake\" name=\"cbUseQCNQuake\" value=\"1\" " . ($bUseQCNQuake ? "checked" : "") . "> Show QCN-Detected 'Quakes'";

/* Max number of Triggers per Page:*/
echo "<p><input type=\"checkbox\" disabled=\"disabled\" checked=\"checked\"> Max Data Per Page: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input id=\"nresults\" name=\"nresults\" value=\"$nresults\" size=\"4\">\n";

/* Sort data in this order: */
echo "<p><input type=\"checkbox\" disabled=\"disabled\" checked=\"checked\">Sort Order:";
echo "<ul><h7><select name=\"rb_sort\" id=\"rb_sort\">
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

   echo "</select></ul>\n";


echo "</td>\n";


//Show specific host
echo "
<td>
  <input type=\"checkbox\" id=\"cbUseHost\" name=\"cbUseHost\" value=\"1\" " . ($bUseHost? "checked" : "") . "> Show Specific Host (enter host ID # or host name)
<ul>
    <table><tr><td>Host ID:  </td><td><input id=\"HostID\"   name=\"HostID\"   value=\"$strHostID\"  ></td></tr>
           <tr><td>Host Name:</td><td><input id=\"HostName\" name=\"HostName\" value=\"$strHostName\"></td></tr></table>
</ul>";



/* Show time constraint */
echo "
  <input type=\"checkbox\" id=\"cbUseTime\" name=\"cbUseTime\" value=\"1\" " . ($bUseTime ? "checked" : "") . "> Use Time Constraint
<BR>
";
/*
echo "<BR><font color=red><B>Please note that triggers older than two months are temporarily not available.</b></font><BR><BR>";
*/
echo "<ul><table><tr><td>Start Time:</td><td>";

// set last four hours for start, current time + 1 for end
$timeStart = gmdate("U", time() - (3600*4));
$timeEnd = gmdate("U", time() + 3600);
if (!$dateStart) {
  $dateStart = date("Y-m-d", $timeStart);
  // now set the times based on timeStart & timeEnd
  $timeHourStart = date("H", $timeStart);
  $timeMinuteStart = "00"; //date("i", $timeStart);
}
if (!$dateEnd) {
  $dateEnd = date("Y-m-d", $timeEnd);
  $timeHourEnd   = date("H", $timeEnd);
  $timeMinuteEnd = "00";
}

echo "<script>DateInput('date_start', true, 'YYYY-MM-DD', '$dateStart')</script></td><td>";

echo "<select name=\"time_hour_start\" id=\"time_hour_start\">";
for ($i = 0; $i < 24; $i++) {
   echo "<option value=$i ";
   if ($i == $timeHourStart) echo "selected";
   echo ">" . sprintf("%02d", $i);
}
echo "</select>";

echo ":<select name=\"time_minute_start\" id=\"time_minute_start\">";
for ($i = 0; $i < 60; $i++) {
   echo "<option value=$i ";
   if ($i == $timeMinuteStart) echo "selected";
   echo ">" . sprintf("%02d", $i);
}
echo "</select> (UTC)";

echo "</td><tr>
<tr><td>End Time</td><td>";

echo "<script>DateInput('date_end', true, 'YYYY-MM-DD', '$dateEnd')</script></td><td>";

echo "<select name=\"time_hour_end\">";
for ($i = 0; $i < 24; $i++) {
   echo "<option value=$i ";
   if ($i == $timeHourEnd) echo "selected";
   echo ">" . sprintf("%02d", $i);
}
echo "</select>";

echo ":<select name=\"time_minute_end\" id=\"time_minute_end\">";
for ($i = 0; $i < 60; $i++) {
   echo "<option value=$i ";
   if ($i == $timeMinuteEnd) echo "selected";
   echo ">" . sprintf("%02d", $i);
}
echo "</select> (UTC)\n </td></tr></table></UL> \n ";


/* Select Sensor Type: */

echo "<p><input type=\"checkbox\" id=\"cbUseSensor\" name=\"cbUseSensor\" value=\"1\" " . ($bUseSensor ? "checked" : "") . "> Use Sensor Type:\n";
echo "<ul>\n";
echo "<select name=\"qcn_sensorid\" id=\"qcn_sensorid\">";
echo "<font size=\"1\">";
  for ($i = 0; $i < sizeof($arrSensor); $i++)  {
     echo "<option value=" . $arrSensor[$i][0];
     if ($qcn_sensorid == $arrSensor[$i][0]) echo " selected";
     echo ">" . $arrSensor[$i][1] . "\n";
  }
echo "</select>\n </ul>\n";

/* Data Type:*/
echo "<p><input type=\"checkbox\" id=\"cbVarietyNormal\" name=\"cbVarietyNormal\" value=\"0\" "
   . ($bVarietyNormal == "0" ? "checked" : "") . "> Show Triggered Data \n";

echo "<p><input type=\"checkbox\" id=\"cbVarietyPing\" name=\"cbVarietyPing\" value=\"1\" "
   . ($bVarietyPing == "1" ? "checked" : "") . "> Show State of Health Data \n";
if ($db_name == "continual") { // only show on continual page
 echo "<p><input type=\"checkbox\" id=\"cbVarietyContinual\" name=\"cbVarietyContinual\" value=\"2\" "
    . ($bVarietyContinual == "2" ? "checked" : "") . "> Show Continual Data \n";
}

echo "</ul>\n";

echo "<p><input type=\"checkbox\" id=\"cbUseCSV\" name=\"cbUseCSV\" value=\"1\" " . ($bUseCSV? "checked" : "") . "> Create Text/CSV File of Triggers?";

echo "</td></tr></table>";  // End of inner table



// end the form
echo "<center><input type=\"submit\" value=\"Submit Constraints\" name=\"btnConstraints\" id=\"btnConstraints\" /></center>\n";
echo "</form>";
if ($db_name == "continual") {
echo "<center><a href=\"" . BASEURL . "/continual/trdl.php\">Start Over</a></center>\n";
} else {
echo "<center><a href=\"" . BASEURL . "/sensor/trdl.php\">Start Over</a></center>\n";
}


echo "</td></tr></table>";  // End of outer table



echo " <H7>";

$varStr = "";
if ($bVarietyNormal != "") $varStr = $bVarietyNormal;
if ($bVarietyPing != "") if ($varStr == "") $varStr = $bVarietyPing; else $varStr .= ",$bVarietyPing";
if ($bVarietyContinual != "") if ($varStr == "") $varStr = $bVarietyContinual; else $varStr .= ",$bVarietyContinual";
if ($varStr == "") $varStr = "0";
// alwayus at least have variety = 1
$whereString = "t.varietyid in ($varStr)";


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

if ($bUseQuake || $bUseQCNQuake) {
  if ($bUseQuake && !$bUseQCNQuake) {
     $whereString .= " AND t.qcn_quakeid>0 AND q.magnitude >= " . $quake_mag_min;
  }
  else if (!$bUseQCNQuake && $bUseQCNQuake) {
   $whereString .= " AND t.qcn_quakeid>0 AND q.guid like 'QCN_%' ";
  }
  else { // both
    $whereString .= " AND t.qcn_quakeid > 0 ";
  }
}

if ($bUseLat) {
   $whereString .= " AND t.latitude BETWEEN $strLatMin AND $strLatMax AND t.longitude BETWEEN $strLonMin AND $strLonMax ";
}

if ($bUseSensor) {
   $whereString .= " AND t.qcn_sensorid=$qcn_sensorid ";
}

$unixtimeStart = 0;
$unixtimeEnd = 0;

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

//YYYY-MM-DD
$unixtimeStart = mktime($timeHourStart, $timeMinuteStart, 0, substr($dateStart, 5, 2), substr($dateStart, 8, 2), substr($dateStart, 0, 4));
$unixtimeEnd   = mktime($timeHourEnd,   $timeMinuteEnd,   0, substr($dateEnd  , 5, 2), substr($dateEnd  , 8, 2), substr($dateEnd,   0, 4));
    
$bUseArchive = "";
if ($bUseTime && $unixtimeStart && $unixtimeEnd) {
   $bUseArchive = ($unixtimeStart < $unixtimeArchive || $unixtimeEnd < $unixtimeArchive);
   $whereString .= " AND t.time_trigger BETWEEN $unixtimeStart AND $unixtimeEnd ";
    //echo "<BR>HAHA<BR>" . $dateStart . "<BR>" . $unixtimeStart . "<BR><BR>" . $dateEnd . "<BR>" . $unixtimeEnd . "<BR><BR>";
}

//print $unixtimeStart . "<BR>" . $unixtimeEnd . "<BR>" . $unixtimeArchive;

echo "<input type=\"hidden\" id=\"cbUseArchive\" name=\"cbUseArchive\" value=\"" . ($bUseArchive ? "1" : "") . "\"> ";
echo "<input type=\"hidden\" id=\"db_name\" name=\"db_name\" value=\"" . $db_name . "\"> ";

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
$query = str_replace("REPLACE_DB", $db_name, $queryBase);
$query = str_replace("REPLACE_ARCHIVE", "0", $query);
if ($bUseArchive) {
   $query .= " WHERE " . $whereString . " UNION " . str_replace("REPLACE_DB", $db_archive, $queryBase) . " WHERE " . $whereString . " ORDER BY " . $sortString;
   $query = str_replace("REPLACE_ARCHIVE", "1", $query);
}
else {
  $query .= 
     " WHERE " . $whereString
       . " ORDER BY " . $sortString
     ;
}

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
//print "<BR><BR>$query<BR><BR>";

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

$url = $q->get_url("trdl.php");
if ($detail) {
    $url .= "&detail=$detail";
}
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


$bResultShow = false;
$result = mysql_query($main_query);
if ($result) {
   $bResultShow = true;
    echo "(<font=small>'Request Files' = send msg to host to upload files to QCN,   'Batch Download' = request download existing file) <BR><BR>";
    echo "<form name=\"formDetail\" method=\"post\" action=trdlreq.php >";
    start_table();
    if (!$bUseCSV && !$ftmp) qcn_trigger_header();
    $iii = 0;
    while ($res = mysql_fetch_object($result)) {
        
        if ($bUseCSV && $ftmp) {
           fwrite($ftmp, qcn_trigger_detail_csv($res));
        }
        else { 
           if ($iii == 0) {
            ++$iii;
            $bg_color="#ffffff";
           } else {
            $iii = 0;
            $bg_color="#dddddd";
           }
           qcn_trigger_detail($res,$bg_color);
        }
    }
    end_table();
    mysql_free_result($result);
} else {
    echo "<h2>No results found - try different query settings</h2>";
}

if ($bUseCSV && $ftmp) {
  echo "<BR><BR><A HREF=\"" . $fileTemp . "\">Download CSV/Text File Here (File Size " . sprintf("%7.2f", (filesize($fileTemp) / 1e6)) . " MB)</A> (you may want to right-click to save locally)<BR><BR>";
}
else if ($bResultShow) {
 echo "<input type=\"hidden\" id=\"cbUseArchive\" name=\"cbUseArchive\" value=\"" . ($bUseArchive ? "1" : "") . "\"> ";
 echo "<input type=\"hidden\" id=\"db_name\" name=\"db_name\" value=\"" . $db_name . "\"> ";
 echo "
  <input type=\"button\" value=\"Check All File Requests\" onclick=\"SetAllCheckBoxes('formDetail', 'cb_a_reqfile[]', true); SetAllCheckBoxes('formDetail', 'cb_r_reqfile[]', true);\" >
  <input type=\"button\" value=\"Uncheck All File Requests\" onclick=\"SetAllCheckBoxes('formDetail', 'cb_a_reqfile[]', false); SetAllCheckBoxes('formDetail', 'cb_r_reqfile[]', false);\" >
  <BR><BR>
  <input type=\"button\" value=\"Check All Download Requests\" onclick=\"SetAllCheckBoxes('formDetail', 'cb_a_dlfile[]', true); SetAllCheckBoxes('formDetail', 'cb_r_dlfile[]', true);\" >
  <input type=\"button\" value=\"Uncheck All Download Requests\" onclick=\"SetAllCheckBoxes('formDetail', 'cb_a_dlfile[]', false); SetAllCheckBoxes('formDetail', 'cb_r_dlfile[]', false);\" >
  <BR><BR>
  <input type=\"submit\" value=\"Submit All Requests\" />
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
    . "Received File, File Download, View, USGS ID, Quake Dist (km), Quake Magnitude, Quake Time, "
    . "Quake Lat, Quake Long, USGS GUID, Quake Desc, Is Archive?"
    . "\n";
}

function qcn_trigger_detail_csv($res)
{
    $quakestuff = "";
    if ($res->qcn_quakeid) {
          $quakestuff = $res->qcn_quakeid . "," .
             $res->quake_distance_km . "," .
             $res->quake_magnitude . "," . 
             time_str_csv($res->quake_time) . "," .
             $res->quake_lat . "," .
             $res->quake_lon . "," .
             $res->guid . "," .
             $res->description . ","  .
             $res->is_quake ? 1 : 0 . "," . $res->quake_url; 
    }
    else {
          $quakestuff = ",,,,,,,,,";
    }

   $file_url = get_file_url($res);

    return $res->triggerid . "," . $res->hostid . "," . $res->ipaddr . "," .
       $res->result_name . "," . time_str_csv($res->trigger_time) . "," . round($res->delay_time, 2) . "," .
        time_str_csv($res->trigger_sync) . "," . $res->sync_offset . "," . $res->trigger_mag . "," . $res->significance . "," .
        round($res->trigger_lat, 8) . "," . round($res->trigger_lon, 8) . "," . ($res->numreset ? $res->numreset : 0) . "," .
        $res->delta_t . "," . $res->sensor_description . "," . $res->sw_version . "," .
        time_str_csv($res->trigger_time) . "," . ($res->received_file == 100 ? " Yes " : " No " ) . "," .
        $file_url . "," .
        $quakestuff .
        "\n";

}

function qcn_trigger_header() {
    echo "
        <tr>
        <th><font size=\"1\">Request<BR>Files?</font size></th>
        <th><font size=\"1\">Batch<BR>Download?</font size></th>
        <th><font size=\"1\">ID</font size></th>
        <th><font size=\"1\">HostID</font size></th>
        <th><font size=\"1\">IP Addr</font size></th>
        <th><font size=\"1\">Result</font size></th>
        <th><font size=\"1\">TimeTrig</font size></th>
        <th><font size=\"1\">Delay(s)</font size></th>
        <th><font size=\"1\">TimeSync</font size></th>
        <th><font size=\"1\">SyncOffset(s)</font size></th>
        <th><font size=\"1\">Magnitude</font size></th>
        <th><font size=\"1\">Significance</font size></th>
        <th><font size=\"1\">Latitude</font size></th>
        <th><font size=\"1\">Longitude</font size></th>
        <th><font size=\"1\">NumReset</font size></th>
        <th><font size=\"1\">DT</font size></th>
        <th><font size=\"1\">Sensor</font size></th>
        <th><font size=\"1\">Version</font size></th>
        <th><font size=\"1\">Time File Req</font size></th>
        <th><font size=\"1\">Received File</font size></th>
        <th><font size=\"1\">File Download</font size></th>
        <th><font size=\"1\">View</font size></th>
        <th><font size=\"1\">Quake ID</font size></th>
        <th><font size=\"1\">Quake Dist (km)</font size></th>
        <th><font size=\"1\">Quake Magnitude</font size></th>
        <th><font size=\"1\">Quake Time (UTC)</font size></th>
        <th><font size=\"1\">Quake Latitude</font size></th>
        <th><font size=\"1\">Quake Longitude</font size></th>
        <th><font size=\"1\">Quake Description</font size></th>
        <th><font size=\"1\">USGS GUID</font size></th>
        <th><font size=\"1\">Is Archived?</font size></th>
        </tr>
    ";
}


function qcn_trigger_detail($res,$bg_color) 
{
global $unixtimeArchive;

  // CMC took out hostnamebyid below
    $sensor_type = $res->sensor_description;
    $archpre = $res->is_archive ? "a" : "r"; // prefix to signify if it's an archive record or not
    echo "
        <tr bgcolor=\"".$bg_color."\">
        <td><font size=\"1\"><input type=\"checkbox\" name=\"cb_" . $archpre . "_reqfile[]\" id=\"cb_" . $archpre . "_reqfile[]\" value=\"$res->triggerid\"" . 
       ($res->varietyid!=0 || $res->received_file == 100 || $res->trigger_timereq>0 || $res->trigger_time < $unixtimeArchive ? " disabled " : " " ) . 
       "></font size></td>
        <td><font size=\"1\"><input type=\"checkbox\" name=\"cb_" . $archpre . "_dlfile[]\" id=\"cb_" . $archpre . "_dlfile[]\" value=\"$res->triggerid\"" . 
       ($res->received_file != 100 ? " disabled " : " " ) . 
       "></font size></td>
        <td><font size=\"1\">$res->triggerid</font size></td>
        <td><font size=\"1\"><a href=\"show_host_detail.php?hostid=$res->hostid\">" . $res->hostid . "</a></font size></td>
        <td><font size=\"1\">$res->ipaddr</font size></td>
        <td><font size=\"1\">$res->result_name</font size></td>
        <td><font size=\"1\">" . time_str($res->trigger_time) . "</font size></td>
        <td><font size=\"1\">" . round($res->delay_time, 2) . "</font size></td>
        <td><font size=\"1\">" . time_str($res->trigger_sync) . "</font size></td>
        <td><font size=\"1\">$res->sync_offset</font size></td>
        <td><font size=\"1\">$res->trigger_mag</font size></td>
        <td><font size=\"1\">$res->significance</font size></td>
        <td><font size=\"1\">" . round($res->trigger_lat,4) . "</font size></td>
        <td><font size=\"1\">" . round($res->trigger_lon,4) . "</font size></td>
        <td><font size=\"1\">" . ($res->numreset ? $res->numreset : 0) . "</font size></td>
        <td><font size=\"1\">$res->delta_t</font size></td>
        <td><font size=\"1\">$sensor_type</font size></td>
        <td><font size=\"1\">$res->sw_version</font size></td>";
        
        echo "
        <td><font size=\"1\">" . time_str($res->trigger_timereq) . "</font size></td>
        <td><font size=\"1\">" . ($res->received_file == 100 ? " Yes " : " No " ) . "</font size></td>";
 
        $file_url = get_file_url($res);
        if ($file_url != "N/A") {
          echo "<td><font size=\"1\"><a href=\"" . $file_url . "\">Download</a></font size></td>";
          echo "<td><font size=\"1\"><a href=\"javascript:void(0)\"onclick=\"window.open('" . BASEURL . "/earthquakes/view/view_data.php?dat=".basename($file_url)."','linkname','height=500,width=400,scrollbars=no')\">View</a></font size></td>";
        }
        else {
          echo "<td><font size=\"1\">N/A</font size></td>";
          echo "<td><font size=\"1\">N/A</font size></td>";
        }

        if ($res->qcn_quakeid) {
           echo "<td><font size=\"1\"><A HREF=\"$res->quake_url\">$res->qcn_quakeid</A></font size></td>";
           echo "<td><font size=\"1\">$res->quake_distance_km</font size></td>";
           echo "<td><font size=\"1\">$res->quake_magnitude</font size></td>";
           echo "<td><font size=\"1\">" . time_str($res->quake_time) . "</font size></td>";
           echo "<td><font size=\"1\">$res->quake_lat</font size></td>";
           echo "<td><font size=\"1\">$res->quake_lon</font size></td>";
           echo "<td><font size=\"1\">$res->description</font size></td>";
           echo "<td><font size=\"1\">$res->guid</font size></td>";
           echo "<td><font size=\"1\">" . ($res->is_archive ? "Y" : "N") . "</font size></td>";
        }
        else {
           echo "<td><font size=\"1\">N/A</font size></td>";
           echo "<td><font size=\"1\">&nbsp</font size></td>";
           echo "<td><font size=\"1\">&nbsp</font size></td>";
           echo "<td><font size=\"1\">&nbsp</font size></td>";
           echo "<td><font size=\"1\">&nbsp</font size></td>";
           echo "<td><font size=\"1\">&nbsp</font size></td>";
           echo "<td><font size=\"1\">&nbsp</font size></td>";
           echo "<td><font size=\"1\">&nbsp</font size></td>";
           echo "<td><font size=\"1\">" . ($res->is_archive ? "Y" : "N") . "</font size></td>";
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
    if (!$x) return "";
    return gmdate('Y/m/d H:i:s', $x); // . " UTC";
}

function get_file_url($res)
{
global $db_name;
$fileurl = "N/A";
if ($res->received_file == 100) {
   if ($res->is_archive) {
     $fileurl = UPLOADURL . "/trigger/archive/";
   }
   else {
     $fileurl = UPLOADURL . "/trigger/";
   }
   if ($db_name == "continual") {
      $fileurl .= "continual/";
   }
   if ($res->is_archive) { // add the fanout dir
     $fandir = floor($res->trigger_time/10000);
     $fileurl .= $fandir . "/";
   }
   $fileurl .= $res->trigger_file;
}
return $fileurl;
}

?>
