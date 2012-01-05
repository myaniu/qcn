<?php
if (file_exists("../inc/common.inc"))
   require_once("../inc/common.inc");
elseif (file_exists("inc/common.inc"))
   require_once("inc/common.inc");
elseif (file_exists("common.inc"))
   require_once("common.inc");
elseif (file_exists("../earthquakes/inc/common.inc"))
   require_once("../earthquakes/inc/common.inc");


require_once(BASEPATH . "/qcn/inc/inc/utils.inc");
require_once(BASEPATH . "/qcn/rt_image/inc/update_data.inc");


page_top();

$img_size = $_REQUEST["sz"];
if ($img_size == null) {
  $img_size = 600;
}
echo "<p><img src=\"" . BASEURL . "/rt_image/images/rt_triggers_dtn.jpg\" width=\"". $img_size ."\" name=\"refreshd\">\n";
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


echo "<p><img src=\"" . BASEURL . "/images/ShakeMap_Scale.png\" width=\"".$img_size."\">\n";

//getData ();



page_end();

?>
