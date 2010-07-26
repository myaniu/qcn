<?php

require_once("/var/www/qcn/inc/utils.inc");

page_top();

echo "      


<h1>Rapid Aftershock Mobilization Program (RAMP) in Chile</h1>
<a href=\"http://qcn.stanford.edu/sensor/maptrig.php?cx=-38&cy=-70&timeint=W\">
<img src=\"http://qcn.stanford.edu/images/QCN-USB-Aftershocks_Cut.png\" align=\"right\" width=\"260\" height=\"320\" margin=\"6\"></a>


<h2>Project Goals:</h2>


<p align=\"justify\">This project aims to increase our understanding of earthquakes and the associated seismic hazard using dense seismic networks. By recording many earthquakes within a dense network we can better understand how earthquakes rupture from initiation to termination. The Quake-Catcher Network hopes to attach small seismic sensors to internet-connected computers throughout the region around the recent M8.8 earthquake. 

<h2>Question & Answer:</h2>  

<p><strong>Q: How can I help?</strong>
<ul><p align=\"justify\">A: You can volunteer CPU time to the Quake-Catcher Network on your desktop by filling the form out below to request a free sensor. The Quake-Catcher Network is a seismic network built by connecting USB sensors to desktops and using a small percentage of the CPU (1-5%) to record ground shaking.
</ul>

<p><strong>Q: Is my computer useful?</strong>

<ul><p>A: Your computer can become part of the RAMP network if: 
<ol><li>You have power and internet</li>
    <li>You felt the M8.8 earthquake or aftershocks</li>
    <li>Your computer is less than 5 years old </li>
    <li>Your computer run Windows or Mac OSX </li>
    <li>Your computer has an unused USB port</li> 
</ol>
</ul>

<p><strong>Q: Is my computer in a useful location?</strong>
<ul><p align=\"justify\">A: If you felt the M8.8 earthquake and aftershocks, you are in a useful location.</ul> 

<p><strong>Q: How do I sign up?</strong>
<ul><p>A: You can volunteer your computer by emailing QCN with this form:</p>
         <form name=\"sensor_request\" method=\"post\" action=\"form.php\">
         <input type=hidden name=\"Recipient1\" value=\"QCN\">
         <input type=hidden name=\"subject\"   value=\"QCN_Chile\">
         <input type=hidden name=\"realname\"  value=\"QCN_Chile\">

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
 
      
      

