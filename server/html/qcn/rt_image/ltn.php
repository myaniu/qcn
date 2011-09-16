<?php

require_once("../earthquakes/inc/common.inc");
require_once("../inc/utils.inc");
require_once("inc/update_data.inc");
page_top();


echo "<p><h1>Real-Time Trigger Map (10 second snapshot):</h1></p>\n";
echo "<p align=\"center\">(3-5 second delay)\n";

echo "<p><h2>Laptop Sensors:</h2>";

echo "<p><img src=\"" . BASEURL . "/rt_image/images/rt_triggers_ltn.jpg\" width=\"500\" name=\"refresh\">";
echo "<SCRIPT language=\"JavaScript\" type=\"text/javascript\">\n
      <!--\n
      var t = 120 // interval in seconds\n
      image = \"images/rt_triggers_ltn.jpg\" //name of the image\n
      function Start() {\n
      tmp = new Date();\n
      tmp = \"?\"+tmp.getTime()\n
      document.images[\"refresh\"].src = image+tmp\n
      setTimeout(\"Start()\", t*3)\n
      }\n
      Start();\n
      // -->\n
      </SCRIPT>\n
      <!-- Code End -->\n 
\n";

//echo "<p><h2>USB Sensors</h2>";

/*echo "<p><img src=\"" . BASEURL . "/rt_image/images/rt_triggers_dtn.jpg\" width=\"500\" name=\"refreshd\">";
echo "<SCRIPT language=\"JavaScript\" type=\"text/javascript\">\n
      <!--\n
      var t = 120 // interval in seconds\n
      image = \"images/rt_triggers_dtn.jpg\" //name of the image\n
      function Start() {\n
      tmp = new Date();\n
      tmp = \"?\"+tmp.getTime()\n
      document.images[\"refreshd\"].src = image+tmp\n
      setTimeout(\"Start()\", t*3)\n
      }\n
      Start();\n
      // -->\n
      </SCRIPT>\n
      <!-- Code End -->\n 
\n";

*/

echo "<p><img src=\"" . BASEURL . "/images/ShakeMap_Scale.png\" width=\"650\">\n";

//getData ();



page_end();

?>
