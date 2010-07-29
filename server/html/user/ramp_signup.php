<?php

require_once("../project/project.inc");
require_once("../project/project_specific_prefs.inc");
require_once("../inc/utils.inc");
require_once("../inc/db.inc");
require_once("../inc/countries.inc");
require_once("../inc/translation.inc");

db_init();

$user = get_logged_in_user(true, true);
$psprefs = project_specific_prefs_parse($user->project_prefs);

//$db = BoincDb::get();
$row = array();
$action = $_POST["submit"];

if ($action) { // we're updating ramp info
}
else { // we're coming into the page to add or edit ramp info
  // get the row for this user
  $sqlRAMP = "SELECT * FROM qcn_ramp WHERE userid=$user->id";
  $result = mysql_query($sqlRAMP);
  /*if (!$result) {
     echo "<BR><BR><font color=\"red\">Database Error - Try Again Later!</font><BR><BR>";
     page_tail();
     exit();
  }*/
  if ($result && mysql_num_rows($result)==1) {
      $row = mysql_fetch_assoc($result); // assoc array i.e. $row["userid"]
      mysql_free_result($result);
  }
}

// note this has google stuff  
page_head("QCN RAMP Information", null, null, "", true, $psprefs, false, 1);

echo "
<script type=\"text/javascript\">
   function clickedAddressCopy() {
       getElement(\"addrlookup\").value = getElement(\"address1\").value + \", \" + getElement(\"address2\").value + \", \" + getElement(\"city\").value + \", \" + getElement(\"region\").value + \", \" + getElement(\"country\").value;
   }
</script>

<h1>Rapid Aftershock Mobilization Program (RAMP)</h1>
<a href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-38&cy=-70&timeint=W\">
<!-- 
   <img src=\"http://qcn.stanford.edu/images/QCN-USB-Aftershocks_Cut.png\" align=\"right\" width=\"260\" height=\"320\" margin=\"6\">
-->
</a>

<h2>Welcome Back " . $user->name . "</h2>
<BR>
<ul><p align=\"justify\">You can add yourself to QCN RAMP by submitting the following information,
    or edit a previous submission:</p>
         <form name=\"ramp_form\" method=\"post\" action=\"ramp_submit.php\">

         <input type=hidden name=\"Recipient1\" value=\"QCN\">
         <input type=hidden name=\"subject\"   value=\"QCN_RAMP\">
         <input type=hidden name=\"realname\"  value=\"QCN_RAMP\">

<table>";
     row_heading_array(array("Enter Your Postal Address (for UPS Delivery)"));
     row2("Name (First, Last)" , 
       "<input name=\"db_fname\" type=\"text\" id=\"db_fname\" size=\"30\" value=\"" . $row["fname"] . "\">"
       . "&nbsp;&nbsp;<input name=\"db_lname\" type=\"text\" id=\"db_lname\" size=\"60\" value=\"" 
           . $row["lname"] . "\">");
     row2("Street Address (Line 1)",
              "<input name=\"db_addr1\" type=\"text\" id=\"db_addr1\" size=\"60\" value=\"" . $row["addr1"] . "\">");
     row2("Street Address (Line 2)",
              "<input name=\"db_addr2\" type=\"text\" id=\"db_addr2\" size=\"60\" value=\"" . $row["addr2"] . "\">");
     row2("City",
              "<input name=\"db_city\" type=\"text\" id=\"db_city\" size=\"60\" value=\"" . $row["city"] . "\">");
     row2("Region/State/Province",
              "<input name=\"db_region\" type=\"text\" id=\"db_region\" size=\"60\">");

     row2_init("Country", "<select name=country>");
     print_country_select($row["country"]);
     echo "</select></td></tr>";

     row2("Post Code", "<input name=\"db_region\" type=\"text\" id=\"db_region\" size=\"60\">");

echo "
            <tr>
               <td>&nbsp;</td>
               <td>Phone:</td>
               <td><input name=\"phone\" type=\"text\" id=\"phone\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>E-mail:</td>
               <td><input name=\"email\" type=\"text\" id=\"email\" size=\"45\" value=\"" 
                   . $user->email_addr . "\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Latitude:</td>
               <td><input name=\"lat0\" type=\"text\" id=\"lat0\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Longitude:</td>
               <td><input name=\"lng0\" type=\"text\" id=\"lng0\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>placename:</td>
               <td><input name=\"lnm0\" type=\"text\" id=\"lnm0\" size=\"45\"></td>
            </tr>

<tr><td colspan=2>Enter/Select your Location (latitude/longitude)</td></tr>
<tr><td colspan=2><div name=\"map\" id=\"map\" style=\"width: 640px; height: 480px\"></div></td></tr>
<tr><td colspan=2>Use the following box to lookup an address (i.e. 360 Panama Mall, Stanford, CA)</td></tr>
<tr><td colspan=2 width=\"50\"><input type=\"text\" name=\"addrlookup\" id=\"addrlookup\" size=50 value=\"\"> 
    <input type=\"button\" name=\"btncopyaddr\" id=\"btncopyaddr\" onclick=\"clickedAddressCopy()\"
   value=\"Copy Address From Above\" size=20>
    <input type=\"button\" name=\"btnaddress\" id=\"btnaddress\" onclick=\"clickedAddressLookup(addrlookup.value)\" value=\"Lookup Address\" size=20>
</td></tr>

              <tr><td colspan=2><hr></tr>
            <tr>
               <td>&nbsp;</td>
               <td>Computer Age:<br>(Years)</td>
               <td><input name=\"cage\" type=\"text\" id=\"cage\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Operating System:</td>
               <td>OS X:<input name=\"OS\" type=\"radio\" id=\"OS\" value=\"OSX\"><br>
                   Windows XP:<input name=\"OS\" type=\"radio\" id=\"OS\" value=\"WindowsXP\"><br></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Regional Coordinator:</td>
               <td>Are you willing to help distribute sensors to participants in your area?<br>
               
               <dd>Yes:<input name=\"RC\" type=\"radio\" id=\"RC\" value=\"YES\"><br>
               <dd>No: <input name=\"RC\" type=\"radio\" id=\"RC\" value=\"NO\"><br></td>
            </tr>


            <tr>
               <td>&nbsp;</td>
               <td colspan=\"2\"><p>If you have any comments or concerns, please let us know.</p><textarea name=\"essay\" id=\"essay\" rows=\"15\" cols=\"64\"></textarea></td>

            </tr>
            <tr>   <td colspan=3><hr></td>  </tr>
            <tr>
               <td>&nbsp;</td>
               <td>&nbsp;</td>

               <td><input type=\"submit\" name=\"Submit\" value=\"Send\"></td>
            </tr>

</table>
</ul>
";

page_tail();

/*

mysql> desc qcn_ramp_participant;
+-------------------------+--------------+------+-----+---------+-------+
| Field                   | Type         | Null | Key | Default | Extra |
+-------------------------+--------------+------+-----+---------+-------+
| id                      | int(11)      | NO   | PRI | NULL    |       | 
| userid                  | int(11)      | NO   | UNI | NULL    |       | 
| qcn_ramp_coordinator_id | int(11)      | YES  |     | NULL    |       | 
| fname                   | varchar(64)  | YES  |     | NULL    |       | 
| lname                   | varchar(64)  | YES  |     | NULL    |       | 
| email_addr              | varchar(100) | YES  |     | NULL    |       | 
| addr1                   | varchar(64)  | YES  |     | NULL    |       | 
| addr2                   | varchar(64)  | YES  |     | NULL    |       | 
| city                    | varchar(64)  | YES  |     | NULL    |       | 
| region                  | varchar(64)  | YES  |     | NULL    |       | 
| postcode                | varchar(20)  | YES  |     | NULL    |       | 
| country                 | varchar(64)  | YES  |     | NULL    |       | 
| latitude                | double       | YES  |     | NULL    |       | 
| longitude               | double       | YES  |     | NULL    |       | 
| gmap_placename          | varchar(64)  | YES  |     | NULL    |       | 
| gmap_view_level         | int(11)      | YES  |     | NULL    |       | 
| gmap_view_type          | int(11)      | YES  |     | NULL    |       | 
| phone                   | varchar(64)  | YES  |     | NULL    |       | 
| fax                     | varchar(64)  | YES  |     | NULL    |       | 
| bshare_coord            | tinyint(1)   | YES  |     | NULL    |       | 
| bshare_map              | tinyint(1)   | YES  |     | NULL    |       | 
| bshare_ups              | tinyint(1)   | YES  |     | NULL    |       | 
| cpu_type                | varchar(20)  | YES  |     | NULL    |       | 
| cpu_os                  | varchar(20)  | YES  |     | NULL    |       | 
| cpu_age                 | varchar(5)   | YES  |     | NULL    |       | 
| cpu_admin               | varchar(5)   | YES  |     | NULL    |       | 
| cpu_firewall            | varchar(20)  | YES  |     | NULL    |       | 
| cpu_floor               | int(11)      | YES  |     | NULL    |       | 
| internet_access         | varchar(20)  | YES  |     | NULL    |       | 
| unint_power             | varchar(20)  | YES  |     | NULL    |       | 
| active                  | tinyint(1)   | NO   |     | 1       |       | 
| comments                | varchar(255) | YES  |     | NULL    |       | 
+-------------------------+--------------+------+-----+---------+-------+


mysql> desc qcn_ramp_coordinator;
+--------------------+--------------+------+-----+---------+-------+
| Field              | Type         | Null | Key | Default | Extra |
+--------------------+--------------+------+-----+---------+-------+
| id                 | int(11)      | NO   | PRI | NULL    |       |
| userid             | int(11)      | NO   | UNI | NULL    |       |
| receive_distribute | tinyint(1)   | YES  |     | NULL    |       |
| help_troubleshoot  | tinyint(1)   | YES  |     | NULL    |       |
| enlist_volunteers  | tinyint(1)   | YES  |     | NULL    |       |
| how_many           | int(11)      | YES  |     | NULL    |       |
| active             | tinyint(1)   | NO   |     | 1       |       |
| comments           | varchar(255) | YES  |     | NULL    |       |
+--------------------+--------------+------+-----+---------+-------+
8 rows in set (0.00 sec)


*/

?>
