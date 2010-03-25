<?php
require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/db_ops.inc");

db_init();

$user = get_logged_in_user(true);
// user->donated means they can do download stuff (donated is a SETI@home field reused here)
if (!$user->id || !$user->donated) {
   echo "You are not authorized to use this page.  Please contact a QCN staff member.";
   exit();
}



function SendTriggerFileRequest($strSend, $testhost, $listTrigger)
{
   if (!$listTrigger || strlen($listTrigger)<2) {
          echo "Error - Invalid trigger list for host " . $testhost . "<BR><BR>";
          return;
   }

   $lenlist = strlen($listTrigger);
   $proctriglist = substr($listTrigger, 0, $lenlist-1); // remove the final ,

       $query = "insert into msg_to_host 
(create_time,hostid,variety,handled,xml) 
select 
unix_timestamp(), " . $testhost . ",
'filelist', 
0, 
concat(
'<trickle_down>
<result_name>', r.name, '</result_name>
<filelist>
" . $strSend . "</filelist>
</trickle_down>\n') 
from result r 
where  
r.hostid=" . $testhost . "
and r.sent_time=(select max(rr.sent_time) from result rr where rr.hostid=" . $testhost . ")";

       $result = mysql_query($query);
       if ($result) {
          $result = mysql_query("update qcn_trigger set time_filereq=unix_timestamp() where hostid=" 
               . $testhost . " and id in (" . $proctriglist . ")");
          if ($result) {
             echo "Request successfully sent for Host " . $testhost . " - Triggers " . $proctriglist . "<BR>";
          }
          else {
             echo "Error #2 (trig filerq time) for Host " . $testhost . " - Triggers " . $proctriglist . "<BR>Try again later.<BR><BR>";
          }
       }
       else {
             echo "Error #1 (mult insert) for Host " . $testhost . " - Triggers " . $proctriglist . "<BR>Try again later.<BR><BR>";
       }

}
// end function

require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

$aryTrig = $_GET["cb_reqfile"];
$numTrig = count($aryTrig);

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

    $triglist = "(";
    for ($i = 0; $i < $numTrig; $i++) {
      $triglist .= $aryTrig[$i];
      if ($i < $numTrig-1) $triglist .= ",\n";
    }
    $triglist .= ")";

    $query = "select id,hostid,result_name,file from qcn_trigger "
           . "where id in " . $triglist . " order by hostid,id";
   
//echo $query . "<BR><BR>";

    $loopctr = 0; 
    $result = mysql_query($query);
    $testhost = -1;
    $strSend = "";
    $listTrigger = "";
    while ($res = mysql_fetch_object($result)) {
//echo "Loop # " . $loopctr++ . "<BR>";
 
       if ($testhost == $res->hostid) {
           if ($res->file) {
               $strSend .= "<sendme>" . $res->file . "</sendme>\n";
               $listTrigger .= ($res->id . ",");
           }
       } else {
           // send out the last request, if any
           if ($strSend) {
               SendTriggerFileRequest($strSend, $testhost, $listTrigger);
           }

           // we've switched host id's
           $testhost = $res->hostid;
           if ($res->file) {
              $strSend = "<sendme>" . $res->file . "</sendme>\n";
              $listTrigger = ($res->id . ",");
           }
           else {
              $strSend = "";
              $listTrigger = "";
           }
       }
    }
    // note we need to do one last send as we won't get the final hostid (end of recordset)
    if ($strSend) {
       SendTriggerFileRequest($strSend, $testhost, $listTrigger);
    }
}

    echo "<BR><BR>$numTrig requests processed!<BR><BR>";

echo "<H3>Hit the 'Back' key on your browser to return to the previous page</H3>";

admin_page_tail();

?>
