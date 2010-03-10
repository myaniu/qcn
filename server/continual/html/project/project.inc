<?php

// CMC note -- added Google Maps stuff for QCN

require_once("../inc/util.inc");

$master_url = parse_config(get_config(), "<master_url>");
define("PROJECT", "Quake-Catcher Network Continual Sensor Monitoring");
define("URL_BASE", "http://qcn.stanford.edu/continual/");
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "white.css");
define("COPYRIGHT_HOLDER", "Stanford University");
define("SYS_ADMIN_EMAIL", "carlgt1@yahoo.com");
define("UOTD_ADMIN_EMAIL", "carlgt1@yahoo.com"); # who gets user of the day pool running low e-mails?
define("TITLE_COLOR", "");
define("TITLE_FONT", "");
define("QUAKE_XML", "../../qcn-quake.xml");
define("MAP_BASE", "img/earth_web.jpg");
define("MAP_TRIGGER", "img/trigger_latest.jpg");
define("MAP_TRIGGER_D", "img/trigger_latest_d.jpg");
define("MAP_TRIGGER_W", "img/trigger_latest_w.jpg");
define("MAP_TRIGGER_M", "img/trigger_latest_m.jpg");
define("MAP_TRIGGER_H", "img/trigger_latest_h.jpg");
define("MIN_MAGNITUDE", "2.0");
define("MIN_MAGNITUDE_D", "3.0");
define("MIN_MAGNITUDE_W", "3.5");
define("MIN_MAGNITUDE_M", "4.0");
define("MIN_MAGNITUDE_H", "3.0");
define("MAX_NUM_TRIGGER", "1000");

// CMC have default global preferences
define("GLOBAL_PREFS_DEFAULT", "<global_preferences>\n"
. "<mod_time>" . time() . "</mod_time>\n"
. "<leave_apps_in_memory>0</leave_apps_in_memory>\n"
. "<suspend_while_on_batteries>0</suspend_while_on_batteries>\n"
. "<run_on_batteries>1</run_on_batteries>\n"
. "<run_if_user_active>1</run_if_user_active>\n"
. "<run_gpu_if_user_active>0</run_gpu_if_user_active>\n"
. "<idle_time_to_run>0</idle_time_to_run>\n"
. "<suspend_if_no_recent_input>0</suspend_if_no_recent_input>\n"
. "<cpu_scheduling_period_minutes>60</cpu_scheduling_period_minutes>\n"
. "<work_buf_min_days>0</work_buf_min_days>\n"
. "<work_buf_additional_days>0.25</work_buf_additional_days>\n"
. "<max_cpus>1</max_cpus>\n"
. "<cpu_usage_limit>100</cpu_usage_limit>\n"
. "<disk_interval>60</disk_interval>\n"
. "<disk_max_used_gb>100</disk_max_used_gb>\n"
. "<disk_max_used_pct>50</disk_max_used_pct>\n"
. "<disk_min_free_gb>0.001</disk_min_free_gb>\n"
. "<vm_max_used_pct>75</vm_max_used_pct>\n"
. "<ram_max_used_busy_pct>50</ram_max_used_busy_pct>\n"
. "<ram_max_used_idle_pct>90</ram_max_used_idle_pct>\n"
. "<max_bytes_sec_down>0</max_bytes_sec_down>\n"
. "<max_bytes_sec_up>0</max_bytes_sec_up>\n"
. "<suspend_cpu_usage>101</suspend_cpu_usage>\n"
. "</global_preferences>");

// Email addresses separated by pipe ( | ) that will receive user reported 
// offensive forum posts.
define("POST_REPORT_EMAILS", "moderator1@$master_url|moderator2@$master_url");

define("MAP_ICON_0", "http://www.google.com/intl/en_us/mapfiles/ms/micons/red-dot.png");
define("MAP_ICON_1", "http://www.google.com/intl/en_us/mapfiles/ms/micons/blue-dot.png");
define("MAP_ICON_2", "http://www.google.com/intl/en_us/mapfiles/ms/micons/green-dot.png");
define("MAP_ICON_3", "http://www.google.com/intl/en_us/mapfiles/ms/micons/purple-dot.png");
define("MAP_ICON_4", "http://www.google.com/intl/en_us/mapfiles/ms/micons/yellow-dot.png");

function project_banner($title) {
    // Put your project title and logo here

    echo "<table width=\"100%\">
        <tr>
        <td><h2>$title</h2></td>
        <td align=right>
       </td></tr></table>
    ";
}

function project_footer($show_return, $show_date) {
    echo "<br><hr noshade size=1><center>";
    if ($show_return) {
        echo "<a href=\"".URL_BASE."\">Return to ".PROJECT." main page</a><br>\n";
    }
    echo "<br><br>Copyright &copy; ".date("Y ").COPYRIGHT_HOLDER."</center>\n";
    if ($show_date) {
        echo "<font size=-2 color=bbbbbb>Generated ",time_str(time()),"</font>\n";
    }
}

function show_profile_heading1() {
    return "Your personal background.";
}

function show_profile_question1() {
    return "
        Tell us about yourself.
        You could tell us where you're from, your age, occupation, hobbies,
        or anything else about yourself.
    ";
}

function show_profile_heading2() {
    return "Your opinions about " . PROJECT;
}

function show_profile_question2() {
    return "
        Tell us your thoughts about " . PROJECT . "<ol>
        <li>Why do you run " . PROJECT . "?
        <li>What are your views about the project?
        <li>Any suggestions?
        </ol>
    ";
}

function project_workunit($wu){
    //echo "This is displayed on the workunit page";
}

function project_user_summary($user){
    //echo "Text here will go to the user summary page";
}

function project_user_page_private($user){
    //echo "Whatever is here is added to the private account page";
}

function project_user_credit($user){
    //echo "Whatever is here is added to the private account page";
}

function project_forum_post_rules() {
    // Add <li> entries for additional forum post rules
    return "
    ";
}

function printGoogleMapsHeader()
{ // note the key was registered on http://www.google.com/maps/api for qcn.stanford.edu
//echo "<script src=\"http://maps.google.com/maps?file=api&v=1&key=ABQIAAAA7FMxjQepz_hQElGJ5tGPvhR_-dF_tBKZEzXEmb5veXIjh7ji-BSpO1s_bzlk2XSTyJa-06G_6sf7rw\""; 
  echo "\n<script src=\"http://maps.google.com/maps?file=api&v=2&key=ABQIAAAA7FMxjQepz_hQElGJ5tGPvhR_-dF_tBKZEzXEmb5veXIjh7ji-BSpO1s_bzlk2XSTyJa-06G_6sf7rw\" type=\"text/javascript\"></script>\n";
} 

function printGoogleMapsScript($prefs)
{
if (!$prefs) return;
 
echo "
<script type=\"text/javascript\">
    //<![CDATA[

   var lastOverlay = new Array(5);
   var micon = new Array(5);
   var map;
   var geocoder;
   var iSelect = 0;

   function getElement(aID)
   {
     return (document.getElementById) ? document.getElementById(aID)
                                      : document.all[aID];
   }

   function clickedAddressLookup(addrline) {

       geocoder.getLatLng(
          addrline,
          function(point) {
            if (!point) {
              alert(addrline + \" not found\");
            } else { /*
              map.setCenter(point, 13);
              var marker = new GMarker(point);
              map.addOverlay(marker);
              marker.openInfoWindowHtml(address);
              */
              if (iSelect < 0) { iSelect = 0; }
              getElement(\"lat\" + iSelect).value = point.y;
              getElement(\"lng\" + iSelect).value = point.x;
              getElement(\"lnm\" + iSelect).value = addrline;
              addMarker(map, point);
              clickedRadio(iSelect, 1);
            }
         }
       );
   }

   function clickedRadio(num, isselected) {
      if (isselected) {
         iSelect = num;
      }
      moveMap(getElement(\"lat\" + iSelect).value, getElement(\"lng\" + iSelect).value);
   }

   function clickedButton(num, bClear) {
      if (bClear) {
         getElement(\"ipa\" + num).value = \"\";
         //getElement(\"hipa\" + num).value = \"\";
      }
      else {
         getElement(\"ipa\" + num).value = getElement(\"txthidIP\").value;
         //getElement(\"hipa\" + num).value = getElement(\"txthidIP\").value;
      }
   }

   function moveMap(lat, lng) {
      if (lat != null && lat != \"\" && lng != null && lng != \"\") {
        newpt = new GLatLng(lat, lng);
        map.setCenter(newpt, 15);
      }
   }

   function formSetPosition(pos, ival) {
      getElement(\"lat\" + ival).value = pos.lat();
      getElement(\"lng\" + ival).value = pos.lng();
      //getElement(\"hlat\" + ival).value = pos.lat();
      //getElement(\"hlng\" + ival).value = pos.lng();
   }

   // little function to add a point
   function addMarker(map, point)
   {
          if (lastOverlay[iSelect]) {
             map.removeOverlay(lastOverlay[iSelect]);
             delete lastOverlay[iSelect];
             lastOverlay[iSelect] = null;
          }
          mopts = Object(); //GMarkerOptions();
          if (iSelect>-1) mopts.icon = micon[iSelect];
          //mopts.draggable = true;
          //mopts.bouncy = true;
          lastOverlay[iSelect] = new GMarker(point, mopts);
          //lastOverlay[iSelect].enableDragging();
          lastOverlay[iSelect].title = iSelect;
          map.addOverlay(lastOverlay[iSelect]);
          formSetPosition(point, iSelect);
          /*GEvent.addListener(lastOverlay[iSelect], 'dragstart', function() {
            // set the new position to the one at the end of the dragging
            iSelect = lastOverlay[iSelect].title;
          });
          GEvent.addListener(lastOverlay[iSelect], 'dragend', function() {
            // set the new position to the one at the end of the dragging
            formSetPosition(this.getPoint(), iSelect);
          });*/
   }
  
   function showMap() 
   {
      if (!GBrowserIsCompatible()) return;
      map  = new GMap2(getElement(\"map\"));
      map.addControl(new GLargeMapControl3D);
      map.addControl(new GScaleControl);
      map.addControl(new GMapTypeControl);
      geocoder = new GClientGeocoder();
      for (i = 0 ; i < 5 ; i++) {
        lastOverlay[i] = null;
        micon[i] = new GIcon();
        micon[i].shadow = \"http://labs.google.com/ridefinder/images/mm_20_shadow.png\";
        micon[i].iconSize = new GSize(32, 32);
        micon[i].shadowSize = new GSize(60, 32);
        micon[i].iconAnchor = new GPoint(16, 32);
        micon[i].infoWindowAnchor = new GPoint(9, 2);
        micon[i].infoShadowAnchor = new GPoint(12, 32);
      }
   ";

   for ($i = 0 ; $i < 5 ; $i++)
     echo "   micon[" . $i . "].image = \"" . constant("MAP_ICON_" . $i) . "\";\n";

   for ($i = 4 ; $i >= 0 ; $i--) {
     echo "
      if (getElement(\"lat" . $i . "\").value != \"\" && getElement(\"lng" . $i . "\").value != \"\") {
            iSelect = " . $i . ";
            newpt = new GLatLng(getElement(\"lat" . $i . "\").value, getElement(\"lng" . $i . "\").value);
            map.setCenter(newpt, 15);
            addMarker(map,newpt);
      }
     ";
   }

echo "
      iSelect = 0;
      GEvent.addListener(map, 'click', function(overlay, point) {
        //if (overlay) {
        //   map.removeOverlay(overlay);
        //}
        if (point) {
           addMarker(map, point);
        }
      });
  }

";

echo "

    //]]>

    </script>
  ";
}


// this overrides the util.inc page_head
//function page_head($google=false, $prefs=null)
function page_head($title, $java_onload=null, $title_plain=null, $prefix="", $google=false, $prefs=null, $maptrig=false) 
{
       $metahttp = "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>\n"
         . "<link rel=\"stylesheet\" type=\"text/css\" href=\"" . STYLESHEET . "\">\n"
         . "<link rel=\"shortcut icon\" href=\"http://qcn.stanford.edu/favicon.ico\" type=\"image/vnd.microsoft.icon\">\n"
         . "<link rel=\"icon\" href=\"http://qcn.stanford.edu/favicon.ico\" type=\"image/vnd.microsoft.icon\">\n";

       if ($google) {
         echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">
           <html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:v=\"urn:schemas-microsoft-com:vml\">
             $metahttp 
           <head>\n<title>" . PROJECT . " - " . $title . "</title>\n";

         printGoogleMapsHeader();

         if (!$maptrig)  {
            printGoogleMapsScript($prefs);
            echo "</head>\n<body onload=\"showMap()\" onunload=\"GUnload()\">\n";
         }
       }
       else {
          echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n
           <html>
            <head>\n<title>" . PROJECT . " - " . $title . "</title>\n" . 
            $metahttp . 
           "</head>\n<body>\n";
       }

       // IE likes this first bit
       if ($google && strpos($_SERVER['HTTP_USER_AGENT'], "MSIE"))
       {
        echo "v\\:* {
          behavior:url(#default#VML);
        }
        ";
       }

}

// disguises the curl using fake headers and a fake user agent.
function disguise_curl($url)
{
  $curl = curl_init();

  // Setup headers - I used the same headers from Firefox version 2.0.0.6
  // below was split up because php.net said the line was too long. :/
  $header[0] = "Accept: text/xml,application/xml,application/xhtml+xml,";
  $header[0] .= "text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5";
  $header[] = "Cache-Control: max-age=0";
  $header[] = "Connection: keep-alive";
  $header[] = "Keep-Alive: 300";
  $header[] = "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7";
  $header[] = "Accept-Language: en-us,en;q=0.5";
  $header[] = "Pragma: "; // browsers keep this blank.

  curl_setopt($curl, CURLOPT_URL, $url);
  curl_setopt($curl, CURLOPT_USERAGENT, 'Googlebot/2.1 (+http://www.google.com/bot.html)');
  curl_setopt($curl, CURLOPT_HTTPHEADER, $header);
  curl_setopt($curl, CURLOPT_REFERER, 'http://www.google.com');
  curl_setopt($curl, CURLOPT_ENCODING, 'gzip,deflate');
  curl_setopt($curl, CURLOPT_AUTOREFERER, true);
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($curl, CURLOPT_TIMEOUT, 10);

  $html = curl_exec($curl); // execute the curl command
  curl_close($curl); // close the connection

  return $html; // and finally, return $html
}

function show_trigger($hostid, $heading, $private=true)
{
   if ($hostid<1) return;
   // make a table of triggers for this host
   $tr = null;
   $colorPing = "\"#999999\"";
   $db = BoincDb::get();
   // if $private just show lat/lng rounded to .01 so we're not showing exact addresses of participants!
   if ($private) {
      $sqllatlng = "round(latitude,2) lat, round(longitude,2) lng, ";
   }
   else {
      $sqllatlng = "latitude lat, longitude lng, ";
   }

   // CMC note - DBNAME gets replaced by appropriate database name in the db->do_query()
   $sql = "select t.id,file,time_trigger,time_received,"
       . "time_sync,sync_offset,significance, magnitude, " . $sqllatlng 
       . "dt, numreset, s.description, sw_version,os_type,runtime_cpu,runtime_clock,varietyid,levelvalue,levelid,received_file,file_url,result_name "
       . "from DBNAME.qcn_trigger t, DBNAME.qcn_sensor s where t.type_sensor=s.id and hostid=$hostid order by time_trigger desc limit " . MAX_NUM_TRIGGER;
   $result=$db->do_query($sql);
   echo "<H2>$heading</H2>\n";
   echo "<H3>Last " . MAX_NUM_TRIGGER . " Triggers for this Computer</H3>\n";
   echo "<H5>(NB: <font color=$colorPing>grey text</font> is a 'real' trigger, otherwise it's a 10-minute file)</H5>\n";
   start_table();
   echo "<tr><th>Trigger ID</th>
             <th>Trigger Time (UTC)</th>
             <th>Received Time (UTC)</th>
             <th>Sync Time (UTC)</th>
             <th>Sync Offset</th>
             <th>Significance</th>
             <th>Magnitude</th>
             <th>Latitude</th>
             <th>Longitude</th>
             <th>Elevation</th>
             <th>Delta T</th>
             <th>Num Reset</th>
             <th>Sensor Type</th>
             <th>Version</th>
             <th>OS</th>
             <th>WU CPU Time</th>
             <th>WU Run Time</th>
             <th>File Name</th>
         </tr>";

   if ($result && mysql_num_rows($result)>0) {  // we found some records
      $i = 0;
      while($row=mysql_fetch_array($result)) {
         if ($i++ % 2) {
             $bgcolor = "\"#F4F4F4\"";
         }
         else {
             $bgcolor = "\"#FFFFFF\"";
         }
         if ($row[17]==0) { // it's a real trigger
           $strTableFont = "<font color=$colorPing>";
           $strFontEnd = "</font>";
         } 
         else {
           $strTableFont = "";
           $strFontEnd   = "";
         }

         $elev = $row[18]; // level/elevation data

/*
mysql> select * from qcn_level;
+----+------------------------------------+
| id | description                        |
+----+------------------------------------+
|  0 | N/A                                | 
|  1 | Floor (+/- above/below surface)    | 
|  2 | Meters (above/below surface)       | 
|  3 | Feet (above/below surface)         | 
|  4 | Elevation - meters above sea level | 
|  5 | Elevation - feet above sea level   | 
+----+------------------------------------+
*/
         switch($row[19]) { // level id
            case 0:  $elev=""; break;
            case 1:  $elev.="Fl"; break;
            case 2:  $elev.="m"; break;
            case 3:  $elev.="f"; break;
            case 4:  $elev.="m(sl)"; break;
            case 5:  $elev.="f(sl)"; break;
         }
/*
         if ($row[17]==1) { // it's a real trigger
            $filename = " ping trigger - no file for result " . $row[22];
         } 
         else {
*/
           if ($row[20] == 100) { // received_file: show file download link
              $filename = "<A HREF=\"$row[21]\">$row[1]</A>";
           }
           else {
              $filename = $row[1];
           }
//         }

         echo "    <tr bgcolor=$bgcolor>
                   <td>$strTableFont$row[0]$strFontEnd</td>
                   <td>$strTableFont" . time_str($row[2]) . "$strFontEnd</td>
                   <td>$strTableFont" . time_str($row[3]) . "$strFontEnd</td>
                   <td>$strTableFont". time_str($row[4]) . "$strFontEnd</td>
                   <td>$strTableFont" . round($row[5],3) . "$strFontEnd</td>
                   <td>$strTableFont". round($row[6],3) . "$strFontEnd</td>
                   <td>$strTableFont". round($row[7],3) . "$strFontEnd</td>
                   <td>$strTableFont". round($row[8],3) . "$strFontEnd</td>
                   <td>$strTableFont". round($row[9],3) . "$strFontEnd</td>
                   <td>$strTableFont". $elev . "$strFontEnd</td>
                   <td>$strTableFont" . ($row[10] ? $row[10] : 0) . "$strFontEnd</td>
                   <td>$strTableFont$row[11]$strFontEnd</td>
                   <td>$strTableFont$row[12]$strFontEnd</td>
                   <td>$strTableFont$row[13]$strFontEnd</td>
                   <td>$strTableFont$row[14]$strFontEnd</td>
                   <td>$strTableFont$row[15]$strFontEnd</td>
                   <td>$strTableFont$row[16]$strFontEnd</td>
                   <td>$strTableFont$filename$strFontEnd</td>
              </font></tr>";
      }
   }

   end_table();
}


?>
