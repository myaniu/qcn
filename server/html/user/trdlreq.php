<?php

require_once("../inc/util.inc");
require_once("../inc/sqlquerystring.inc");
//require_once("../inc/db_ops.inc");
require_once("../project/common.inc");

// _a_ == archive record  _r_ == regular trigger record
$aryDLTrigA = $_POST["cb_a_dlfile"];
$aryDLTrigR = $_POST["cb_r_dlfile"];
$numDLTrigA = count($aryDLTrigA);
$numDLTrigR = count($aryDLTrigR);

$aryReqTrigA = $_POST["cb_a_reqfile"];
$aryReqTrigR = $_POST["cb_r_reqfile"];
$numReqTrigA = count($aryReqTrigA);
$numReqTrigR = count($aryReqTrigR);
$db_name = $_POST["db_name"];
$bUseArchive = $_POST["cbUseArchive"];

/*
echo "triga:";
print_r($aryDLTrigA);
echo "<BR><BR>trigr:";
print_r($aryDLTrigR);
*/

if ($db_name == "sensor") {
  $DB = "sensor_download";
  $URL_UPL_BASE = UPLOADURL . "/trigger/job/u";
}
else if ($db_name == "continual") {
  $DB = "continual_download";
  $URL_UPL_BASE = UPLOADURL . "/trigger/continual/job/u";
}
else {
   print "Error - invalid database - cannot continue - please go back and try your query again";
}


db_init();

$user = get_logged_in_user(true);
// authenticate admin-level user
qcn_admin_user_auth($user, true);

$q = new SqlQueryString();

//page_head("QCN Trigger Listing");
echo "<html><head>
  <title>QCN Trigger Download & Upload Request</title>
</head><body " . BODY_COLOR . ">\n";
  echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=\"6\"><b><a href=\"index.php\">".PROJECT.":</a>  QCN Trigger File Request </b></font></td></tr></table>\n";

procBatchDownloadRequest();
procTriggerUploadRequest();

echo "<H3>Hit the 'Back' key on your browser to return to the previous page</H3>";

page_tail();

// finished



function procBatchDownloadRequest()
{
  global $db_name, $user, $aryDLTrigA, $aryDLTrigR, $numDLTrigA, $numDLTrigR, $DB, $URL_UPL_BASE;

  $insertid = 0;
  if (($numDLTrigA == 0 && $numDLTrigR == 0) || (!$aryDLTrigA && !$aryDLTrigR) || (!is_array($aryDLTrigA) && !is_array($aryDLTrigR))) {
    echo "<H3>No Triggers Chosen For Downloading!</H3><BR><BR>";
    return;
  }

  // archive triggers
    $triglist = "(";
    for ($i = 0; $i < $numDLTrigA; $i++) {
      $triglist .= $aryDLTrigA[$i];
      if ($i < $numDLTrigA-1) $triglist .= ",\n";
    }

    // regular triggers
    for ($i = 0; $i < $numDLTrigR; $i++) {
      $triglist .= $aryDLTrigR[$i];
      if ($i < $numDLTrigR-1) $triglist .= ",\n";
    }
    $triglist .= ")";

    // archive trigger download
    // not yet implemented

    // regular trigger download
    $query = "INSERT INTO " . $DB . ".job (userid,create_time,list_triggerid) VALUES ("
       . $user->id . ", unix_timestamp(), '" . $triglist . "')";

    $loopctr = 0;
    $result = mysql_query($query);

    if (!$result) {
       echo "Database error - try again later -- errmsg=" . mysql_error(); 
       return;
    }

    $insertid = mysql_insert_id();

    if ($insertid) {
      $tottrig = $numDLTrigA + $numDLTrigR;
      $myurl = $URL_UPL_BASE . $user->id . "_j" . $insertid . ".zip";
      echo "<BR><BR><H3>" . $tottrig . " trigger download file requests processed for database '" . $db_name . "'!</H3><BR><BR>";
      echo "<BR><BR>An email will be sent to $user->email_addr when this job is processed with the download link/URL:<BR><BR>"
         . "<A HREF=\"" . $myurl . "\">" . $myurl . "</A><BR><BR>";
     }
     else {
       echo "Database error on insert_id - try again later -- errmsg=" . mysql_error(); 
       return;
     }
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
             echo "Upload request successfully sent for Host " . $testhost . " - Triggers " . $proctriglist . "<BR>";
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

function procTriggerUploadRequest() 
{
   global $db_name, $user, $aryReqTrigA, $aryReqTrigR, $numReqTrigA, $numReqTrigR, $DB, $URL_UPLOAD_BASE;

   $q = new SqlQueryString();

   if ($numReqTrigR == 0 || !$aryReqTrigR || !is_array($aryReqTrigR)) {
      echo "<H3>No Triggers Chosen For Upload Requests!</H3><BR><BR>";
      return;
    }
    echo "<H3>Processing request, please wait...</H3><BR><BR>";

    // trigger id # is unique across live & archive database, so just send the list to both
    $triglist = "(";
    for ($i = 0; $i < $numReqTrigR; $i++) {
      $triglist .= $aryReqTrigR[$i];
      if ($i < $numReqTrigR - 1) $triglist .= ",\n";
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
    echo "<BR><BR><H3>" . $numReqTrigR . " requests processed!</H3><BR><BR>";
}

?>
