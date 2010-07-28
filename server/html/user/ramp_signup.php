<?php

require_once("../inc/utils.inc");
require_once("../inc/db.inc");
require_once("../inc/countries.inc");
require_once("../inc/translation.inc");

$user = get_logged_in_user(true, true);

page_top();

echo "      

<h1>Rapid Aftershock Mobilization Program (RAMP)</h1>
<a href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-38&cy=-70&timeint=W\">
<!-- 
   <img src=\"http://qcn.stanford.edu/images/QCN-USB-Aftershocks_Cut.png\" align=\"right\" width=\"260\" height=\"320\" margin=\"6\">
-->
</a>

<h2>Welcome Back " . $user->name . "</h2>


<p align=\"justify\">This project aims to increase our understanding of earthquakes and the associated seismic hazard using dense seismic networks. By recording many earthquakes within a dense network we can better understand how earthquakes rupture from initiation to termination. The Quake-Catcher Network hopes to attach small seismic sensors to internet-connected computers throughout your earthquake-pron region.

";

echo "
<ul><p>You can volunteer your computer by submitting this form to QCN:</p>
         <form name=\"ramp_form\" method=\"post\" action=\"ramp_submit.php\">

         <input type=hidden name=\"Recipient1\" value=\"QCN\">
         <input type=hidden name=\"subject\"   value=\"QCN_RAMP\">
         <input type=hidden name=\"realname\"  value=\"QCN_RAMP\">

<table>
            <tr>
               <td colspan=\"3\"><b>Important Information:</b></td>
            </tr>
            <tr>
               <td width=\"3%\">&nbsp;</td>
               <td width=\"17%\">Name:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Last:</td>
               <td width=\"80%\"><input name=\"name_last\" type=\"text\" id=\"name_last\" size=\"18\">
               &nbsp;&nbsp;First:<input name=\"name_first\" type=\"text\" id=\"name_first\" size=\"18\">
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Phone:</td>
               <td><input name=\"phone\" type=\"text\" id=\"phone\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>E-mail:</td>
               <td><input name=\"email\" type=\"text\" id=\"email\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Address 1:</td>
               <td><input name=\"address1\" type=\"text\" id=\"building\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Address 2:</td>
               <td><input name=\"address2\" type=\"text\" id=\"building\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>City:</td>
               <td><input name=\"city\" type=\"text\" id=\"city\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Province/State:</td>
               <td><input name=\"state\" type=\"text\" id=\"state\" size=\"45\"></td>
            </tr>
            <tr>
               <td>&nbsp;</td>
               <td>Post Code:</td>
               <td><input name=\"post_code\" type=\"text\" id=\"post_code\" size=\"45\"></td>
            </tr>

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
            <tr>
               <td>&nbsp;</td>
               <td>&nbsp;</td>

               <td><input type=\"submit\" name=\"Submit\" value=\"Send\"></td>
            </tr>

</table>
</ul>
";

page_end();

?>
