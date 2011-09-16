<?php

require_once("../inc/utils.inc");
require_once("inc/update_data.inc");
page_top();


echo "<p><h1>Real-Time USB Shaking (10 second snapshot):</h1></p>\n";
echo "<p align=\"justify\">Is my USB sensor working? Am I connected to the network? If you tap your USB sensor, you should see your position light up within a few seconds (this will only work if the tap creates a stronger shake than the normal shaking within the past minute).  If you don't see the map light up in the next few seconds, check that 1) your USB sensor is <a href=\"manuals/physical/\">connected to the USB port</a>, 2) you have <a href=\"manuals/boinc_install/\">installed BOINC</a> properly, 3) you <a href=\"manuals/qcn_drivers\">installed the latest drivers</a>, and 4) you have set your <a href=\"manuals/configure_qcn/\">location</a> properly.</p>\n";

echo "<p><strong>USB Sensors</strong> (Click <a href=\"http://qcn.stanford.edu/rt_image/index.php\">here for laptop sensors</a>)\n";

echo "<p><img src=\"http://qcn.stanford.edu/rt_image/images/rt_triggers_dtn.jpg\" width=\"680\" name=\"refreshd\">\n";
echo "<SCRIPT language=\"JavaScript\" type=\"text/javascript\">\n
      <!--\n
      var t = 3000 // interval in miliseconds\n
      image = \"images/rt_triggers_dtn.jpg\" //name of the image\n
      function Start() {\n
      tmp = new Date();\n
      tmp = \"?\"+tmp.getTime()\n
      document.images[\"refreshd\"].src = image+tmp\n
      setTimeout(\"Start()\", t)\n
      }\n
      Start();\n
      // -->\n
      </SCRIPT>\n
      <!-- Code End -->\n 
\n";


echo "<p><img src=\"http://qcn.stanford.edu/images/ShakeMap_Scale.png\" width=\"650\">\n";

//getData ();



page_end();

?>
