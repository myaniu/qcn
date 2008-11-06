<?php

require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

$aryTrig = $_GET["cb_reqfile"];
$numTrig = count($aryTrig);

db_init();

$q = new SqlQueryString();

//admin_page_head("QCN Trigger Listing");
echo "<html><head>
  <title>QCN Trigger File Request</title>
</head><body " . BODY_COLOR . ">\n";
  echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=\"6\"><b><a href=\"index.php\">".PROJECT.":</a>  QCN Trigger File Request </b></font></td></tr></table>\n";

if ($numTrig == 0) {
    echo "<H3>No Triggers Chosen!</H3><BR><BR>";
}
else {
    echo "<H3>Processing request, please wait...</H3><BR><BR>";

    for ($i = 0; $i < $numTrig; $i++) {
       $query = "insert into msg_to_host
(create_time,hostid,variety,handled,xml)
select
unix_timestamp(),
t.hostid,
'filelist',
0,
concat(
'<trickle_down>
<result_name>', r.name, '</result_name>
<filelist>
<sendme>', t.file, '</sendme>
</filelist>
</trickle_down>\n')
from qcn_trigger t, result r
where t.hostid=r.hostid and t.id=$aryTrig[$i]
  and r.sent_time=(select max(rr.sent_time) from result rr where rr.hostid=r.hostid)";

       $result = mysql_query($query);
       if ($result) {
          $result = mysql_query("update qcn_trigger set time_filereq=unix_timestamp() where id=$aryTrig[$i]");
          if ($result) {
             echo "Request successfully sent for Trigger # " . $aryTrig[$i] . "<BR>";
          }
          else {
             echo "Error #2 for Request of Trigger # " . $aryTrig[$i] . "<BR>Try again later.<BR><BR>";
          }
       }
       else {
             echo "Error #1 for Request of Trigger # " . $aryTrig[$i] . "<BR>Try again later.<BR><BR>";
       }
       //mysql_free_result($result);

    }

    echo "<BR><BR>$numTrig requests processed!<BR><BR>";
}

echo "<H3>Hit the 'Back' key on your browser to return to the previous page</H3>";

admin_page_tail();

?>
