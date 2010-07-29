<?php



function page_top() {
require_once("../inc/google_translate.inc");
echo " <!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n";
echo " <html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n";
echo " <head>\n";
echo " <title>Quake-Catcher Network</title>\n";
echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"http://qcn.stanford.edu/css/base.css\" media=\"all\" />\n";
echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"http://qcn.stanford.edu/css/top_rightsidebar.css\" media=\"screen\" />\n";
echo " <link rel=\"stylesheet\" type=\"text/css\" href=\"http://qcn.stanford.edu/css/print.css\" media=\"print\" />\n";
echo " <script type=\"text/javascript\" src=\"http://qcn.stanford.edu/=scripts/winpopup.js\"></script>\n";
echo " <META name=\"description\" content=\"Quake-Catcher Network, Community Seismic Network, Strong-motion seismic netowrk, earthquakes, laptops, USB sensors, USB accelerometers, Distributed Computing, BOINC, Schools, K-12, Lesson Plans, Classroom Activities, Advanced alert, Early Warning, Rapid Aftrshock Mobilization Program, Desktop Network, Seismology, Earth Science\">\n";
echo " <META name=\"keywords\" content=\"Quake-Catcher Network, Community Seismic Network, Strong-motion seismic netowrk, earthquakes, laptops, USB sensors, USB 
accelerometers, Distributed Computing, BOINC, Schools, K-12, Lesson Plans, Classroom Activities, Advanced alert, Early Warning, Rapid Aftrshock Mobilization Program, Desktop Network, Seismology, Earth Science\">\n"; 
echo " </head>\n";
echo " <body bgcolor=\"#999999\">\n";
echo " <div id=\"skipnav\"><a href=\"#maincontent\">Skip navigation</a></div>\n";


echo " <div id=\"container\">\n";
#echo "   <div id=\"banner\"><img src=\"http://qcn.stanford.edu/images/banner.gif\" alt=\"Quake-Catcher Network\" title=\"Quake-Catcher Network\" width=\"100%\" /></div>\n";
echo "       <div id=\"banner\">\n";
echo "       <table width=100% padding=0 spacing=0 border=0><tr><td width=\"15%\">\n";
google_translate_new();
echo "      </td><td>\n";
echo "   <h1><center><b>The Quake-Catcher Network</b></center></h1>\n";
echo "   <br><br><br><br><br><br>\n";
echo "   <p><h3 valign=\"bottom\" color=\"#ffffff\"><center><i>\"Bringing Seismology to Homes and Schools.\"</i></center></h3></div>\n";
echo "   </td><td width=15%></td></tr></table>\n";
echo "   </div>\n";
echo "   <table width=\"100%\"><tr><td>\n";

echo "   <div id=\"nav\"> \n";
echo "     <ul>\n";
echo "       <li><a href=\"http://qcn.stanford.edu/index.php\">Home</a></li>\n";
echo "       <li><a href=\"http://qcn.stanford.edu/about/index.php\">About QCN </a>\n";
echo " 	     <ul>\n";

echo "           <li><a href=\"http://qcn.stanford.edu/about/laptop.php\">Laptop Network</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/about/desktop.php\">Desktop Network</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/RAMP/\"            >Aftershock Program</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/about/contact.php\">Contact Us</a></li>\n";
echo "       </ul>\n";
echo "       <li><a href=\"http://qcn.stanford.edu/learning/join.php\">Join Us</a>\n";
echo "       <ul>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/learning/requests.php\">Request a Sensor</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/downloads/\">Download QCN Monitoring</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/downloads/\">Download QCN Interactive</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/manuals/\">Manuals/Instructions</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/RAMP/\">Aftershock Programs</a></li>\n";
echo "       </ul></li>\n";
echo "       <li><a href=\"http://qcn.stanford.edu/press/news.php\">News & Press</a>\n";
echo "         <ul>\n";

echo "           <li><a href=\"http://qcn.stanford.edu/press/news.php\">Recent News</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/press/releases.php\">Press Releases</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/images/\">Images</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/press/videos.php\">Videos</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/press/facts.php\">Facts</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/press/in_news.php\">In The News</a></li>\n";
echo "         </ul>\n";
echo "       </li>\n";
        
echo "       <li><a href=\"http://qcn.stanford.edu/learning/index.php\">Learning Center </a>\n";
echo " 	  <ul>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/learning/software.php\">QCN Interactive</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/learning/lessons.php\">Lessons & Activities</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/downloads/index.php\">Software Download</a>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/learning/earthquakes.php\">Learn About Earthquakes</a></li>\n";
echo " 	  <li><a href=\"http://qcn.stanford.edu/learning/requests.php\">Sensor Requests</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/learning/glossary.php\">Glossary</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/learning/affiliates_ed.php\">Educational Affiliates</a></li>\n";
echo "         </ul>\n";
echo "       <li><a href=\"http://qcn.stanford.edu/research/index.php\">Research Center</a>\n";
echo "  	  <ul>\n";

echo "           <li><a href=\"http://qcn.stanford.edu/research/data.php\">Data Download</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/research/opportunities.php\">Development Opportunities</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/downloads/index.php\">Software Download</a>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/research/affiliates_r.php\">Research Affiliates</a></li>\n";

echo "         </ul>\n";
echo "       <li><a href=\"http://qcn.stanford.edu/sensor/\">My QCN/BOINC</a>\n";
echo "  	  <ul>\n";

echo "           <li><a href=\"http://qcn.stanford.edu/sensor/home.php\">My Account</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/sensor/edit_host_info_form.php\">My Location</a></li>\n";
echo "            <li><a href=\"http://qcn.stanford.edu/downloads/index.php\">Software Download</a>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/sensor/forum_index.php\">Message Boards</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/sensor/user_search.php\">User Database</a></li>\n";
echo "           <li><a href=\"http://qcn.stanford.edu/sensor/stats.php\">Statistics</a></li>\n";

echo "         </ul>\n";
    
echo "<li>";
echo " </ul>\n";
echo "   </div>\n";
echo "   </td></tr>\n";
echo "   <tr><td bgcolor=\"#333333\"><p> </p></td></tr>\n";
echo "   <tr><td bgcolor=\"#AA0000\">\n";
echo "   <br><p> </p>\n";
echo "   <br><p> </p>\n";
echo "   </td></tr></table>\n";
echo "   <!--Optional Banner Start-->\n";
  
echo "   <!--Optional Banner End-->\n";
echo "   <div id=\"sidebar\">\n";
echo "     <div id=\"sidecontent\"> \n";

echo "       <h1>Search </h1>\n";
echo "       <form name=\"sensor_request\" method=\"LINK\" action=\"http://qcn.stanford.edu/search.php\">\n";
echo "       <table><td><tr>\n";
echo "         <input name=\"search_text\" type=\"text\" id=\"search_text\" size=\"20\">\n";
echo "         </tr><tr align=\"center\">\n";
echo "         <input type=\"submit\" name=\"Search\" value=\"Search\">\n";
echo "         </tr></td>\n";
echo "         </table>\n";
echo "         </form>\n"; 

echo "       <h1>Download Now </h1>\n";
echo "       <ul>\n";
echo "         <li><a href=\"http://qcn.stanford.edu/downloads/index.php#Network\">QCN Network Software </a></li>\n";
echo " 	<li><dd><a href=\"http://boinc.berkeley.edu/download.php\">(BOINC)</a></p></li></dd>\n";
echo "         <li><a href=\"http://qcn.stanford.edu/downloads/index.php#Interactive\">QCN Interactive Software</a></li>\n";
echo " 	<li><dd><a href=\"http://qcn.stanford.edu/downloads/index.php#Interactive\">(QCNLive)</a></li></dd>\n";
echo "         <li><a href=\"http://qcn.stanford.edu/downloads/index.php#USB_Drivers\">USB Sensor Drivers</a></li>\n";
echo "         <li><a href=\"http://qcn.stanford.edu/manuals/index.php\">Manuals/Instructions</a></li>\n";
echo "       </ul><br>\n";

echo "       <h1>Request A Sensor</h1>\n";
echo "       <ul>\n";
echo "         <a href=\"http://qcn.stanford.edu/learning/requests.php#K12\"><li>K-12 Program</li></a>\n";
echo "         <a href=\"http://qcn.stanford.edu/learning/requests.php#Purchase\"><li>Purchase a Sensor</li></a>\n";
echo "         <a href=\"http://qcn.stanford.edu/learning/requests.php#Donate\"><li>Give Sensors to Schools</li></a>\n";
echo "       </ul>\n";
echo "       <br>\n";

echo "       <h1>Recent News</h1>\n";
echo "       <ul>\n";

include("/var/www/boinc/sensor/html/inc/some_news.inc");
echo "       <br>\n";

echo "       </ul>\n";


echo " <map name=\"trigger_latest_w\">\n";
echo "         <area shape=\"rect\" coords=\"0,2,225,232\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=38&cy=-120&timeint=W\">\n";

echo "         <area shape=\"rect\" coords=\"2,236,228,511\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-20&cy=-120&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"227,3,428,234\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=38&cy=-70&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"231,238,442,510\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-20&cy=-70&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"430,3,605,237\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=50&cy=1&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"445,241,732,510\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-10&cy=5&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"609,3,803,239\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=38&cy=80&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"735,244,1022,511\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-20&cy=140&timeint=W\">\n";
echo "         <area shape=\"rect\" coords=\"806,3,1021,243\" href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=38&cy=140&timeint=W\">\n";
echo " </map>\n";

echo " <h1 id=\"qcnmap\">QCN Sensor Map</h1>\n";
echo " <p><img src=\"http://qcn.stanford.edu/sensor/img/trigger_latest_w.jpg\" usemap=\"#trigger_latest_w\" border=\"0\" width=\"140\">\n";

echo " <h1 id=\"qcngrowth\">QCN Growth</h1>\n";
echo " <p><a href=\"http://qcn.stanford.edu/sensor/img/weekly.png\"><img src=\"http://qcn.stanford.edu/sensor/img/weekly.png\" border=\"0\" width=\"140\"></a>\n";

echo " <h1 id=\"sponsors\">QCN Sponsors</h1>\n";
echo " <p align=\"center\"><a href=\"http://www.nsf.gov\"><img src=\"http://qcn.stanford.edu/images/nsf_logo.jpg\" width=\"100\"></img></a>\n";
echo " <p align=\"center\"><a href=\"http://www.scec.org\"><img src=\"http://qcn.stanford.edu/images/scec_logo.jpg\" width=\"100\"></img></a>\n";
echo " <p align=\"center\"><a href=\"http://www.iris.edu\"><img src=\"http://qcn.stanford.edu/images/iris_logo.png\" width=\"100\"></img></a>\n";
echo " <p align=\"center\"><a href=\"http://www.ups.com\"><img src=\"http://qcn.stanford.edu/images/ups_logo.jpg\" width=\"75\"></img></a>\n";


echo "     </div>\n";


echo "   </div>\n";


echo "   <div id=\"content\">\n";
echo "     <!--Optional Breadcrumb Start-->\n";
   
echo "     <!--Optional Breadcrumb End-->\n";

  
echo "     <div id=\"maincontent\">\n";

}

function page_end(){
echo "    </div>\n";
echo "    <br style=\"clear: both;\" />\n";
echo "  </div>\n";
echo "  <div id=\"footer\"> \n";
echo "    <ul>\n";
echo "      <li><a href=\"http://qcn.stanford.edu/\">QCN Home</a></li>\n";
echo "      <li><a href=\"http://qcn.stanford.edu/about/contact.php\">Contact</a></li><li class=\"last\"><a href=\"http://qcn.stanford.edu/downloads/\">Download Now </a></li> \n";

echo "    </ul>\n";
echo "  </div>\n";
echo "</div>\n";
echo "<div id=\"copyright\" class=\"vcard\"> <!-- InstanceBeginEditable name=\"copyright\" -->&copy; <span class=\"fn org\">Quake-Catcher Network </span>.  All Rights Reserved.<!-- InstanceEndEditable --></div>\n";

echo "</body>\n";
echo "</html>\n";
}
?>
