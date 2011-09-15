<?php

require_once("../project/common.inc");

date_default_timezone_set('UTC');         // Set the default time zone to UTC

function require_once_r2($file) {
$cwd = getcwd();
chdir(BASEDIR . "/boinc/sensor/html/user/");

require_once($file);

chdir($cwd);

}


function page_top($title=null){

$print = $_GET["print"];

require_once(BASEDIR . "/boinc/sensor/html/inc/google_translate.inc");

html_header();

echo " <div id=\"container\">\n";

if (!$print) {
banner($title);
navigation_top();
navigation_bar();
echo "     <div id=\"maincontent\">\n";
} else {
echo "     <div id=\"maincontent\">\n";
banner_print($title);
}        




}


function html_header(){
echo " <!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
echo " <html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n";
echo " <head>\n";
echo " <title>Quake-Catcher Network</title>\n";
echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"" . BASEURL . "/css/base.css\" media=\"all\" />\n";
echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"" . BASEURL . "/css/top_rightsidebar.css\" media=\"screen\" />\n";
echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"" . BASEURL . "/css/print.css\" media=\"print\" />\n";
echo " <script type=\"text/javascript\" src=\"" . BASEURL . "/=scripts/winpopup.js\"></script>\n";
echo " <META name=\"description\" content=\"Quake-Catcher Network, Community Seismic Network, Strong-motion seismic netowrk, earthquakes, laptops, USB sensors, USB accelerometers, Distributed Computing, BOINC, Schools, K-12, Lesson Plans, Classroom Activities, Advanced alert, Early Warning, Rapid Aftrshock Mobilization Program, Desktop Network, Seismology, Earth Science\">\n";
echo " <META name=\"keywords\" content=\"Quake-Catcher Network, Community Seismic Network, Strong-motion seismic netowrk, earthquakes, laptops, USB sensors, USB 
accelerometers, Distributed Computing, BOINC, Schools, K-12, Lesson Plans, Classroom Activities, Advanced alert, Early Warning, Rapid Aftrshock Mobilization Program, Desktop Network, Seismology, Earth Science\">\n"; 

echo "<script src=\"http://www.google.com/jsapi?key=ABQIAAAAi1z8qL921_qvE089UEGaWBR_-dF_tBKZEzXEmb5veXIjh7ji-BQqbf02Ax-8y6LZM1XiWL5fFhcdQg\" type=\"text/javascript\"></script>\n";


echo " </head>\n";
echo " <body bgcolor=\"#999999\">\n";
echo " <div id=\"skipnav\"><a href=\"#maincontent\">Skip navigation</a></div>\n";

}

function banner_print($title=null) {
 echo "       <table width=100% padding=0 spacing=0 border=0><tr>\n<td width=\"15%\">\n";
 google_translate_new();
 echo "      </td><td>\n";
 $url_now = $_SERVER['SCRIPT_NAME'];
 $show_mag = $_GET["show_mag"];
 if ($show_mag) {$url_now=$url_now."?show_mag=y";}
 echo "   <table width=\"140\" align=\"right\"><tr align=\"right\"><td><a href=\"$url_now\"><font color=\"#58789d\">Full Version</font></a></td></tr></table>\n";
if ($title) {
 echo "<h1><center><b>$title</b></center></h1>";
} else {
 echo "   <h1><center><b>The Quake-Catcher Network</b></center></h1>\n";
}
 echo "   </td></tr></table>\n";

}

function banner($title=null) {
// This places the banner at the top of the screen
 echo "       <div id=\"banner\">\n";
 echo "       <table width=100% padding=0 spacing=0 border=0><tr><td width=\"15%\">\n";
 google_translate_new();
 echo "      </td><td>\n";
 $url_now = $_SERVER['SCRIPT_NAME'];
 $show_mag = $_GET["show_mag"];
 if ($show_mag) {$url_print=$url_now."?show_mag=y&print=y";}else{$url_print=$url_now."?print=y";}
// $url_now = curPageURL();
 echo "   <table width=\"140\" align=\"right\"><tr align=\"right\"><td align=\"right\"><a href=\"$url_print\"><font color=\"\">Print Version</font></a><br><br></td></tr><tr><td><img src=\"" . BASEURL . "/images/QCN_Logo.png\" width=\"100\" align=\"right\"></td></tr></table>\n";
if ($title) {
 echo "<h1><center><b>$title</b></center></h1>";
} else {
 echo "   <table align=\"center\" margin=0 padding=0><tr><td><h1><center><b>The Quake-Catcher Network</b></center></h1>\n";
 echo "   \n";
 echo "   <p><h3 valign=\"bottom\" color=\"#ffffff\"><center><i>\"Bringing Seismology to Homes and Schools.\"</i></center></h3></td></tr></table></div>\n";
}
 echo "   </td></tr></table>\n";
 echo "   </div>\n";
}

function navigation_top(){
echo "   <!--Optional Banner End-->\n";
echo "   <div id=\"sidebar\">\n";
echo "     <div id=\"sidecontent\"> \n";

echo "       <h1>Search </h1>\n";
echo "       <form name=\"sensor_request\" method=\"LINK\" action=\"" . BASEURL . "/search.php\">\n";
echo "       <table><td><tr>\n";
echo "         <input name=\"search_text\" type=\"text\" id=\"search_text\" size=\"23\">\n";
echo "         </tr><tr align=\"center\">\n";
echo "         <input type=\"submit\" name=\"Search\" value=\"Search\">\n";
echo "         </tr></td>\n";
echo "         </table>\n";
echo "         </form>\n"; 

echo "       <h1>Download Now </h1>\n";
echo "       <ul>\n";
echo "         <li><a href=\"" . BASEURL . "/downloads/index.php#Network\">QCN Network Software </a></li>\n";
echo " 	<li><dd><a href=\"http://boinc.berkeley.edu/download.php\">(BOINC)</a></p></li></dd>\n";
echo "         <li><a href=\"" . BASEURL . "/downloads/index.php#Interactive\">QCN Interactive Software</a></li>\n";
echo " 	<li><dd><a href=\"" . BASEURL . "/downloads/index.php#Interactive\">(QCNLive)</a></li></dd>\n";
echo "         <li><a href=\"" . BASEURL . "/downloads/index.php#usbdrivers\">USB Sensor Drivers</a></li>\n";
echo "         <li><a href=\"" . BASEURL . "/manuals/index.php\">Manuals/Instructions</a></li>\n";
echo "       </ul><br>\n";

echo "       <h1>Request A Sensor</h1>\n";
echo "       <ul>\n";
echo "         <a href=\"" . BASEURL . "/learning/requests.php#K12\"><li>K-12 Program</li></a>\n";
echo "         <a href=\"" . BASEURL . "/learning/requests.php#Purchase\"><li>Purchase a Sensor</li></a>\n";
echo "         <a href=\"" . BASEURL . "/learning/requests.php#Donate\"><li>Give Sensors to Schools</li></a>\n";
echo "       </ul>\n";
echo "       <br>\n";

echo "       <h1>Recent News</h1>\n";
echo "       <ul>\n";

require_once_r(BASEDIR . "/boinc/sensor/html/inc/some_news.inc");

echo "       <br>\n";

echo "       </ul>\n";


echo " <map name=\"trigger_latest_w\">\n";
echo "         <area shape=\"rect\" coords=\"0,2,225,232\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=38&cy=-120&timeint=W\">\n";

echo "         <area shape=\"rect\" coords=\"2,236,228,511\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=-20&cy=-120&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"227,3,428,234\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=38&cy=-70&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"231,238,442,510\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=-20&cy=-70&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"430,3,605,237\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=50&cy=1&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"445,241,732,510\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=-10&cy=5&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"609,3,803,239\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=38&cy=80&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"735,244,1022,511\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=-20&cy=140&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"806,3,1021,243\" href=\"" . BASEURL . "/sensor/maptrig.php?cx=38&cy=140&timeint=W\">\n";
echo " </map>\n";

echo " <h1 id=\"qcnmap\">QCN Sensor Map</h1>\n";
echo " <p><img src=\"" . BASEURL . "/sensor/img/trigger_latest_w.jpg\" usemap=\"#trigger_latest_w\" border=\"0\" width=\"178\">\n";

echo " <h1 id=\"qcngrowth\">QCN Growth</h1>\n";
echo " <p><a href=\"" . BASEURL . "/sensor/img/weekly.png\"><img src=\"" . BASEURL . "/sensor/img/weekly.png\" border=\"0\" width=\"178\"></a>\n";

echo " <h1 id=\"sponsors\">QCN Sponsors</h1>\n";
echo " <p align=\"center\"><a href=\"http://www.nsf.gov\"><img src=\"" . BASEURL . "/images/nsf_logo.jpg\" width=\"100\"></img></a>\n";
echo " <p align=\"center\"><a href=\"http://www.scec.org\"><img src=\"" . BASEURL . "/images/scec_logo.jpg\" width=\"100\"></img></a>\n";
echo " <p align=\"center\"><a href=\"http://www.iris.edu\"><img src=\"" . BASEURL . "/images/iris_logo.png\" width=\"100\"></img></a>\n";
echo " <p align=\"center\"><a href=\"http://www.ups.com\"><img src=\"" . BASEURL . "/images/ups_logo.jpg\" width=\"75\"></img></a>\n";
echo " <p align=\"center\"><a href=\"http://www.o-navi.com\"><img src=\"" . BASEURL . "/images/O-NAVI.png\" width=\"100\"></img></a>\n";

echo "     </div>\n";


echo "   </div>\n";


echo "   <div id=\"content\">\n";

}


function navigation_bar(){

echo "   <table width=\"100%\"><tr><td bgcolor=\"#58789d\">\n";

echo "   <div id=\"nav\"> \n";
echo "     <ul>\n";
echo "       <li><a href=\"" . BASEURL . "/index.php\">Home</a></li>\n";
echo "       <li><a href=\"" . BASEURL . "/about/index.php\">About QCN </a>\n";
echo "       <ul>\n";
echo "           <li><a href=\"" . BASEURL . "/about/laptop.php\">Laptop Network</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/about/desktop.php\">Desktop Network</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/RAMP/\"            >Aftershock Program</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/about/contacts\">Contact Us</a></li>\n";
echo "       </ul>\n";
echo "       <li><a href=\"" . BASEURL . "/learning/join.php\">Join QCN</a>\n";
echo "       <ul>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/requests.php\">Request a Sensor</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/downloads/\">Download QCN Monitoring</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/downloads/\">Download QCN Interactive</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/manuals/\">Manuals/Instructions</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/RAMP/\">Aftershock Programs</a></li>\n";
echo "       </ul></li>\n";
echo "       <li><a href=\"" . BASEURL . "/press/index.php\">News</a>\n";
echo "         <ul>\n";
echo "           <li><a href=\"" . BASEURL . "/press/news.php\">Recent News</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/press/releases.php\">Press Releases</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/images/\">Images</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/press/videos.php\">Videos</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/press/facts.php\">Facts</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/press/in_news.php\">In The News</a></li>\n";
echo "         </ul>\n";
echo "       </li>\n";

echo "       <li><a href=\"" . BASEURL . "/learning/index.php\">Learning</a>\n";
echo "    <ul>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/requests.php\">Request a Sensor</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/software.php\">QCN Interactive</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/lessons.php\">Lessons & Activities</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/downloads/index.php\">Software Download</a>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/earthquakes.php\">Learn About Earthquakes</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/glossary.php\">Glossary</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/affiliates_ed.php\">Educational Affiliates</a></li>\n";
echo "         </ul>\n";
echo "       <li><a href=\"" . BASEURL . "/research/index.php\">Research</a>\n";
echo "            <ul>\n";
echo "           <li><a href=\"" . BASEURL . "/learning/requests.php\">Request a Sensor</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/research/data.php\">Data Download</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/research/opportunities.php\">Development Opportunities</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/downloads/index.php\">Software Download</a>\n";
echo "           <li><a href=\"" . BASEURL . "/research/affiliates_r.php\">Research Affiliates</a></li>\n";
echo "         </ul>\n";
echo "       <li><a href=\"" . BASEURL . "/sensor/\">My QCN/BOINC</a>\n";
echo "            <ul>\n";
echo "           <li><a href=\"" . BASEURL . "/sensor/home.php\">My Account</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/sensor/edit_host_info_form.php\">My Location</a></li>\n";
echo "            <li><a href=\"" . BASEURL . "/downloads/index.php\">Software Download</a>\n";
echo "           <li><a href=\"" . BASEURL . "/sensor/forum_index.php\">Message Boards</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/sensor/user_search.php\">User Database</a></li>\n";
echo "           <li><a href=\"" . BASEURL . "/sensor/stats.php\">Statistics</a></li>\n";
echo "         </ul>\n";
echo "         </li>";
echo "         </ul>\n";
echo "         </div>\n";
echo "         </td></tr>\n";

echo "   </table>\n";
}

function page_end(){
 $print = $_GET["print"];

 if (!$print) {
  echo "    </div>\n";
  echo "    <br style=\"clear: both;\" />\n";
  echo "  </div>\n";
  echo "  <div id=\"footer\"> \n";
  echo "    <ul>\n";
  echo "      <li><a href=\"" . BASEURL . "/\">QCN Home</a></li>\n";
  echo "      <li><a href=\"" . BASEURL . "/about/contacts\">Contact</a></li>\n
            <li><a href=\"" . BASEURL . "/downloads/\">Download Now </a></li> \n
            <li class=\"last\"><a href=\"" . BASEURL . "/about/legal/\">Legal & Privacy</a></li>\n";
  echo "    </ul>\n";
  echo "  </div>\n";
 }

 echo "</div>\n";
 echo "<div id=\"copyright\" class=\"vcard\"> <!-- InstanceBeginEditable name=\"copyright\" -->&copy; <span class=\"fn org\">Quake-Catcher Network </span>.  All Rights Reserved.<!-- InstanceEndEditable --></div>\n";

 echo "</body>\n";
 echo "</html>\n";
}
?>
