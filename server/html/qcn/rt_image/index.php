<?php
require_once("../earthquakes/inc/common.inc");
require_once("../inc/utils.inc");
require_once("inc/update_data.inc");

page_top();

echo "<p><h1>Real-Time Laptop Shaking (10 second snapshot):</h1></p>\n";
echo "<p align=\"justify\">Is my laptop sensor working? Am I connected to the network? If you shake your laptop, you should see your position on the map light up within a few seconds (this will only work if the tap creates a stronger shaking than the normal shaking within the past minute).  If you do <b>not</b> see the map light up in the next few seconds, check that 1) your <a href=\"" . BASEURL . "manuals/qcn_drivers\">laptop's make and model is supported</a>, 2) you have  <a href=\"" . BASEURL . "manuals/boinc_install/\">installed BOINC</a> properly, and 3) you have <a href=\"" . BASEURL . "manuals/configure_qcn/\">set your location</a> proerly.</p>\n";


echo "<p><strong>Laptop Sensors</strong> (Click <a href=\"" . BASEURL . "/rt_image/usb_sensors.php\">here for USB sensors</a>)\n";

echo "<p><img src=\"" . BASEURL . "/rt_image/images/rt_triggers_ltn.jpg\" width=\"680\" name=\"refresh\">\n";
echo "<SCRIPT language=\"JavaScript\" type=\"text/javascript\">\n
      <!--\n
      var t = 3000                     // interval in milliseconds\n
      image = \"" . BASEURL . "/rt_image/images/rt_triggers_ltn.jpg\" //name of the image\n
      function Start() {\n
      tmp = new Date();\n
      tmp = \"?\"+tmp.getTime()\n
      document.images[\"refresh\"].src = image+tmp\n
      setTimeout(\"Start()\", t)\n
      }\n
      Start();\n
      // -->\n
      </SCRIPT>\n
      <!-- Code End -->\n 
\n";


echo "<p><img src=\"" . BASEURL . "/images/ShakeMap_Scale.png\" width=\"650\">\n";

//getData ();

page_end();

?>
