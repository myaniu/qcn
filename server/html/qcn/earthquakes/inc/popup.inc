<?php
require_once("common.inc");
function popup_page(){
echo "<html><head>

</head><body>
<a onclick=\"window.open(BASEURL, 'windowname', 'width=300,height=300,left=400,top=500,location=0,scrollbars=0,resizable=0,status=0,rtoolbar=0,menubar=0');\"><font color=\"blue\" style=\"cursor: hand\"><b><u>Click Me!!</u></b></font></a>
</body></html>\n";
}
//<a href=\"JavaScript:newPopup(BASEURL);\">Open a popup window</a>
//<!-- Codes by Quackit.com -->
//<script type=\"text/javascript\">
//// Popup window code
//function newPopup(url) {
//	popupWindow = window.open(
//		url,'popUpWindow','height=300,width=300,left=100,top=100,resizable=no,scrollbars=no,toolbar=no,menubar=no,directories=no,status=yes')
//}
//</script>

?>
