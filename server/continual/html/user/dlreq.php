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

$DB = "continual_download";
$URL_UPL_BASE = "http://qcn-upl.stanford.edu/trigger/continual/job/u";


$aryTrig = $_POST["cb_reqfile"];
$numTrig = count($aryTrig);

$q = new SqlQueryString();

//page_head("QCN Trigger Listing");
echo "<html><head>
  <title>QCN Trigger File Request</title>
</head><body " . BODY_COLOR . ">\n";
  echo TABLE . "<tr " . TITLE_COLOR . "><td>" . TITLE_FONT . "<font size=\"6\"><b><a href=\"index.php\">".PROJECT.":</a>  QCN Trigger File Request </b></font></td></tr></table>\n";

   $insertid = 0;
if ($numTrig == 0) {
    echo "<H3>No Triggers Chosen!</H3><BR><BR>";
}
else {
    $triglist = "(";
    for ($i = 0; $i < $numTrig; $i++) {
      $triglist .= $aryTrig[$i];
      if ($i < $numTrig-1) $triglist .= ",\n";
    }
    $triglist .= ")";

    $query = "INSERT INTO " . $DB . ".job (userid,create_time,list_triggerid) VALUES ("
       . "$user->id, unix_timestamp(), '$triglist')";

    $loopctr = 0; 
    $result = mysql_query($query);
    $insertid = mysql_insert_id();
}

  if ($insertid) {
    $myurl = $URL_UPL_BASE . $user->id . "_j" . $insertid . ".zip";
    echo "<BR><BR><H3>$numTrig trigger file requests processed!</H3><BR><BR>";
    echo "<BR><BR>An email will be sent to $user->email_addr when this job is processed with the download link/URL:<BR><BR>"
       . "<A HREF=\"" . $myurl . "\">" . $myurl . "</A><BR><BR>";
  }

echo "<H3>Hit the 'Back' key on your browser to return to the previous page</H3>";

page_tail();

?>
