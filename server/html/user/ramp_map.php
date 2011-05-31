<?php
require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/db_ops.inc");
require_once('../inc/translation.inc');
require_once('../inc/phoogle.inc');
require_once('../project/project_specific_prefs.inc');

db_init();

set_time_limit(600);

$user = get_logged_in_user(true);
// authenticate admin-level user
qcn_admin_user_auth($user, true);

$query = "select id, fname, lname, email_addr, addr1, addr2, city, region, postcode, country, latitude, longitude, phone, fax, bshare_coord, bshare_map, bshare_ups, cpu_type, cpu_os, cpu_age, cpu_floor, cpu_admin, cpu_permission, cpu_firewall, cpu_proxy, cpu_internet, cpu_unint_power, sensor_distribute, comments from qcn_ramp_participant WHERE active=1";
$order = "order by country, lname, fname";

$detail = null;
$show_aggregate = false;

$q = new SqlQueryString();

// start $_GET

$nresults = get_int("nresults", true);
$last_pos = get_int("last_pos", true);

$bUseCSV = get_int("cbUseCSV", true);
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

echo "<form name=\"formFilter\" method=\"get\" action=\"ramp_map.php\" >";

if (!$strCountry || $strCountry == "None") $strCountry = "International";
echo "Filter by Country:
<select name=db_country id=db_country>";
     print_country_select($strCountry);
echo "
     </select></td></tr>
<BR><BR>
<input type=\"checkbox\" id=\"cbUseCSV\" name=\"cbUseCSV\" value=\"1\" " . ($bUseCSV? "checked" : "") . "> Create Text/CSV File of Triggers?
<BR><BR>
   <input type=\"submit\" value=\"Submit Constraints\" />
   </form> <H7>";

if ($strCountry != "International" && $strCountry != "None") $query .= " AND country='$strCountry' ";

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
   $whereString .= " AND t.type_sensor=$type_sensor ";
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
  qcnalpha.qcn_trigger t LEFT OUTER JOIN qcn_quake q ON t.qcn_quakeid = q.id
   LEFT JOIN qcn_sensor s ON t.type_sensor = s.id 
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

$url = $q->get_url("ramp_ramp.php");
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


if ($bUseCSV) {   
   // tmp file name tied to user ID & server time
   $fileTemp = sprintf("data/%ld_u%d.csv", time(), $user->id);
   $ftmp = fopen($fileTemp, "w");
   if ($ftmp) {
//      fwrite($ftmp, qcn_ramp_header_csv());
   }
   else {
      $fileTemp = ""; // to check for status later on down
   }
}

// Google Map stuff
$pm = new PhoogleMap();

if ($zoom) {
   $pm->zoomLevel = $zoom;
}
else {
   $pm->zoomLevel = 5;
}

if ($mapwidth) {
   $pm->setWidth($mapwidth);
}
else {
   $pm->setWidth(1);
}

if ($mapheight) {
   $pm->setHeight($mapheight);
}
else {
   $pm->setHeight(1);
}

if ($cx && $cy) {
   $pm->centerMap($cx, $cy);
}
else {  // default to northern California
   $pm->centerMap(38, -115);
}




echo "<script src=\"http://maps.google.com/maps/api/js?sensor=false\" type=\"text/javascript\"></script>\n";

echo "  <div id=\"map_container\" style=\"width: 727px; height: 651px; margin: 0 auto 0 auto;\">";
echo "  <div id=\"map\" style=\"width: 500px; height: 650px; float: left;\"></div>\n";
    qcn_ramp_map_key();
    qcn_ramp_check_boxes();
$result = mysql_query($main_query);
if ($result) {
    echo "<form name=\"formDelete\" method=\"get\" action=\"ramp_map.php\" >";
//    start_table();
//    if (!$bUseCSV && !$ftmp) qcn_ramp_header();

    echo "  <script type=\"text/javascript\">\n";
    icon_setup();
    echo "\n    var locations = [\n";
    while ($res = mysql_fetch_object($result)) {
        if ($bUseCSV && $ftmp) {
           fwrite($ftmp, qcn_ramp_detail_csv($res));
        }
        else { 
        $descript = "DING";
//Name: $res->fname $res->lname \nEmail: $res->email_addr \nAddress: $res->addr1.\nAddress: $res->addr2 \nCity: $res->city \nRegion: $res->region\nPhone: $res->phone\n"; 
        $is_quake = "0";
        $is_usb   = "1";
//           qcn_ramp_detail($res);

        $comment = "<b>Name:</b> $res->fname $res->lname <br><b>Email:</b> $res->email_addr <br><b>Address:</b><ul> $res->addr1 <br>$res->addr2 <br>$res->city, $res->region, $res->postcode<br>$res->country</ul><b>Lat:</b> ".number_format($res->latitude,2).", <b>Lon:</b> ".number_format($res->longitude,2)."<br><b>Phone:</b> $res->phone<br><b>Fax:</b> $res->fax<br><b>Share:</b> $res->bshare_coord<br><b>Share Map:</b> ".yn10($res->bshare_map)." <br><b>CPU Type:</b> ".yn10($res->cpu_type)." <br><b>Operating System:</b> $res->cpu_os <br><b>CPU Usage:</b> ".yn10($res->cpu_age)." <br><b>Floor:</b> $res->cpu_floor <br><b>CPU Admin:</b> ".yn10($res->cpu_admin)." <br><b>Permissions:</b> ".yn10($res->cpu_permission)." <br><b>Firewall</b>: ".yn10($res->cpu_firewall)." <br><b>Proxy:</b> ".yn10($res->cpu_proxy)." <br><b>Internet:</b> ".yn10($res->cpu_internet)." <br><b>Power:</b> ".yn10($res->cpu_unint_power)." <br><b>Distribute:</b> ".yn10($res->sensor_distribute);";// <br>Comments: $res->comments";
        $comment=str_replace("'","",$comment);
        $comment=str_replace("!","",$comment);
        $comment=str_replace("?","",$comment);
        $comment=str_replace("+","",$comment);
//        $comment=str_replace("(","",$comment);
//        $comment=str_replace(")","",$comment);
        if ($res->bshare_coord=1) {
           echo "   ['$comment', $res->latitude, $res->longitude, $res->cpu_internet, ";
           rywg_icon($res);
           echo " ],\n";
        }
         }
    }
    mysql_free_result($result);

    echo "];\n";
    echo "
    myCenter = new google.maps.LatLng(35,-110.);
    var myOptions = {
      zoom: 2,
      center: myCenter,
      mapTypeId: google.maps.MapTypeId.ROADMAP
    }\n";
    echo " var map = new google.maps.Map(document.getElementById('map'), myOptions);\n

    var infowindow = new google.maps.InfoWindow();";

    $region1  = $_REQUEST["region"];
    if ($region1 == null) {$region1 = "all";};


    if ( ($region1 == "PNWall")||($region1 == "ORMetro")||($region1 == "all") ) {
      echo "var ctaLayerOR = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/OR_Metro_RAMP.kml');\n
      ctaLayerOR.setMap(map);\n";
    }
    if ( ($region1 == "PNWall")||($region1 == "WAMetro")||($region1 == "all") ) {
      echo "var ctaLayerWA = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/WA_Metro_RAMP.kml');\n
      ctaLayerWA.setMap(map);\n";
    }
    if ( ($region1 == "PNWall")||($region1 == "PNWCoast")||($region1 == "all") ) {
      echo "var ctaLayerP = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/PNW_Coast_RAMP.kml');\n
      ctaLayerP.setMap(map);\n";
    }
    if ( ($region1 == "Anchorage")||($region1 == "all") ) {
      echo "var ctaLayerA = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/Anchorage_RAMP.kml');\n
      ctaLayerA.setMap(map);\n";
    }
    if ( ($region1 == "Wasatch")||($region1 == "all") ) {
      echo "var ctaLayerW = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/Wasatch_RAMP.kml');\n
      ctaLayerW.setMap(map);\n";
    }
    if ( ($region1 == "NAF")||($region1 == "all") ) {
      echo "var ctaLayerI = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/Istanbul_RAMP.kml');\n
      ctaLayerI.setMap(map);\n";
    }
    if ( ($region1 == "NM")||($region1 == "all") ) {
      echo "var ctaLayerNM = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/NM_RAMP.kml');\n
      ctaLayerNM.setMap(map);\n";
    }
    if ( ($region1 == "CAall")||($region1 == "Hayward")||($region1 == "all") ) {
      echo "var ctaLayerH = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/Hayward_RAMP.kml');\n
      ctaLayerH.setMap(map);\n";
    }
    if ( ($region1 == "CAall")||($region1 == "SAFN")||($region1 == "all") ) {
      echo "var ctaLayerSN = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/SAF_N_RAMP.kml');\n
      ctaLayerSN.setMap(map);\n";
    }
    if ( ($region1 == "CAall")||($region1 == "SAFS")||($region1 == "all") ) {
      echo "var ctaLayerSS = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/SAF_S_RAMP.kml');\n
      ctaLayerSS.setMap(map);\n";
    }
    if ( ($region1 == "CAall")||($region1 == "CAMetro")||($region1 == "all") ) {
      echo "var ctaLayer2 = new google.maps.KmlLayer('http://qcn.stanford.edu/RAMP/KML/CA_Metro_RAMP.kml');\n
      ctaLayer2.setMap(map);\n";
    }

    echo "
    var marker, i;

    for (i = 0; i < locations.length; i++) {  
      marker = new google.maps.Marker({
        position: new google.maps.LatLng(locations[i][1], locations[i][2]),
        map: map,
        icon: new google.maps.MarkerImage(\"http://labs.google.com/ridefinder/images/mm_20_\" + locations[i][4] + \".png\"),
//        shape: shape
          
      });

      google.maps.event.addListener(marker, 'click', (function(marker, i) {
        return function() {
          infowindow.setContent(locations[i][0]);
          infowindow.open(map, marker);
          infowindow.maxWidth(200);
        }
      })(marker, i));
    }
    setCenter(myCenter);
  </script>\n";
    
  echo "</div>\n";


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
}
    if ($bUseCSV && $ftmp) {
      fclose($ftmp);
    }


page_tail();


function qcn_ramp_header_csv() {
   return "ID, FirstName, LastName, Email, Address1, Address2, City, Region, "
     . "PostCode, Country, Latitude, Longitude, Phone, Fax, ShareCoord, ShareMap, ShareUPS, "
     . "CPUType, OpSys, CPUAgeYrs, CPUFloor#, AdminRts, Permission, Firewall, Proxy, Internet, UnintPower, DistribSensor"
     . "\n";
}

function qcn_ramp_detail_csv($res)
{
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
       "\"$res->fax\"" . "," .
       ($res->bshare_coord ? "\"Y\"" : "\"N\"") . "," .
       ($res->bshare_map ? "\"Y\"" : "\"N\"") . "," .
       ($res->bshare_ups ? "\"Y\"" : "\"N\"") . "," .
       "\"$res->cpu_type\"" . "," .
       "\"$res->cpu_os\"" . "," .
       "$res->cpu_age" . "," .
       "$res->cpu_floor" . "," .
       ($res->cpu_admin ? "\"Y\"" : "\"N\"") . "," .
       ($res->cpu_permission ? "\"Y\"" : "\"N\"") . "," .
       ($res->cpu_firewall ? "\"Y\"" : "\"N\"") . "," .
       ($res->cpu_proxy ? "\"Y\"" : "\"N\"") . "," .
       ($res->cpu_internet ? "\"Y\"" : "\"N\"") . "," .
       ($res->cpu_unint_power ? "\"Y\"" : "\"N\"") . "," .
       ($res->sensor_distribute ? "\"Y\"" : "\"N\"") . "," .
        "\n";

       //"\"$res->comments\"" . 
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
       <td>$res->sensor_distribute</td>
       <td>$res->comments</td>
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



function yn10($num10) {
   if ($num10==0) {
      return $yn="no";
   } else {
      return $yn="yes";
   }
}

function icon_setup(){
   echo "
       var image = new google.maps.MarkerImage('mapIcons/marker_red.png',
          // This marker is 20 pixels wide by 32 pixels tall.
          new google.maps.Size(20, 34),
          // The origin for this image is 0,0.
          new google.maps.Point(0,0),
          // The anchor for this image is the base of the flagpole at 0,32.
          new google.maps.Point(9, 34)
       );
       var shadow = new google.maps.MarkerImage(
          'http://www.google.com/mapfiles/shadow50.png',
          // The shadow image is larger in the horizontal dimension
          // while the position and offset are the same as for the main image.
          new google.maps.Size(37, 34),
          new google.maps.Point(0,0),
          new google.maps.Point(9, 34)
       );

       // Shapes define the clickable region of the icon.
       // The type defines an HTML &lt;area&gt; element 'poly' which
       // traces out a polygon as a series of X,Y points. The final
       // coordinate closes the poly by connecting to the first
       // coordinate.

       var shape = {
          coord: [9,0,6,1,4,2,2,4,0,8,0,12,1,14,2,16,5,19,7,23,8,26,9,30,9,34,11,34,11,30,12,26,13,24,14,21,16,18,18,16,20,12,20,8,18,4,16,2,15,1,13,0],
          type: 'poly'
       };";

}


function qcn_ramp_map_key(){
 echo "<div id=\"key\" style=\"width: 224px; height: 200px; float: right;\">
       <table align=\"left\">
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_red.png\"></td>
           <td>No Info Sharing</td></tr>\n
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_orange.png\"></td>
           <td>Proxy or Firewall</td></tr>\n
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_yellow.png\"></td>
           <td>CPU on Upper floor</td></tr>\n
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_black.png\"></td>
           <td>CPU > 5 years</td></tr>\n
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_gray.png\"></td>
           <td>Firewall</td></tr>\n
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_white.png\"></td>
           <td>No Admin Privledges</td></tr>\n
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_purple.png\"></td>
           <td>Not usually on Internet</td></tr>\n
       <tr><td><img src=\"http://labs.google.com/ridefinder/images/mm_20_green.png\"></td>
           <td>Perfect</td></tr>\n
       </table>\n
       </div>";
}

function rywg_icon($res){
       if ($res->bshare_coord==0){
        echo "\"red\"";
        return;
       }
       if ($res->bshare_map==0){
        echo "\"red\"";
        return;
       }
       if ($res->cpu_age>5){
        echo "\"black\"";
        return;
       }
       if ($res->cpu_internet==0){
        echo "\"purple\"";
        return;
       }
       if ($res->cpu_proxy==1){
        echo "\"orange\"";
        return;
       }
       if ($res->cpu_firewall==1){
        echo "\"gray\"";
        return;
       }
       if ($res->cpu_floor>3){
        echo "\"yellow\"";
        return;
       }
       if ($res->cpu_admin==0){
        echo "\"white\"";
        return;
       }
       if ($res->cpu_permission==0){
        echo "\"blue\"";
        return;
       }
       if ($res->cpu_unint_power==0){
        echo "\"green\"";
        return;
       }
       echo "\"green\"";
       
}


function qcn_ramp_check_boxes() {
   
   echo "<div id=\"ramp_box\" style=\"width: 224px; height: 400px; float: right;\">

       <p><b>United States:</b></p>\n
       <ul><li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=CAall\">California</a></li>\n
           <ul><li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=SAFN\">San Andreas Fault (North)</a></li>\n
               <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=SAFS\">San Andreas Fault (South)</a></li>\n
               <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=Hayward\">Hayward/Calaveras Fault</a></li>\n
               <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=CAMetro\">SF Bay Area</a></li>\n
               <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=CAMetro\">Greater Los Angeles Basin</a></li>\n
           </ul>\n
           <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=PNWall\">Pacific NorthWest</a></li>\n
           <ul><li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=ORMetro\">Oregon Metropolitan Areas</a></li>\n
               <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=WAMetro\">Washington Metropolitan Areas</a></li>\n
               <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=PNWCoast\">Coastal Regions</a></li>\n
           </ul>\n
           <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=Wasatch\">Wasatch Fault, (Salt Lake Utah)</a></li>\n
           <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=NM\">New Madrid (Tenn, Missouri, Arkansas, Kentucky)</a></li>\n
           <li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=Anchorage\">Anchorage Alaska</a></li>\n
       </ul>
       <p><b>International:</b></p>\n
       <ul><li><a href=\"http://qcn.stanford.edu/sensor/ramp_map.php?region=NAF\">Northern Anatolian Fault (Istanbul, Turkey)</a></li></ul>\n
       </div>\n\n";

}


?>




