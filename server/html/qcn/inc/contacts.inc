<?php

class contact {
   var $name,$role,$dept,$univ,$add1,$add2,$add3,$city,$state,$country,$zip,$email,$phone,$fax;
}

function show_contact($short) {
   
  $cfile = BASEPATH . "/qcn/about/contacts/".$short."/contact.info";
  $contents = file($cfile);

  foreach ($contents as $line_num => $line) {
    list($tname,$value) = split('[:]',$line);

    if     ($tname=="name") {$name=$value;}
    elseif ($tname=="role") {$role=$value;}
    elseif ($tname=="dept") {$dept=$value;}
    elseif ($tname=="univ") {$univ=$value;}
    elseif ($tname=="add1") {$add1=$value;}
    elseif ($tname=="add2") {$add2=$value;}
    elseif ($tname=="add3") {$add3=$value;}
    elseif ($tname=="city") {$city=$value;}
    elseif ($tname=="state"){$state=$value;}
    elseif ($tname=="zip")  {$zip  =$value;}
    elseif ($tname=="email"){$email=$value;}
    elseif ($tname=="phone"){$phone=$value;}
    elseif ($tname=="fax")  {$fax  =$value;}

  }
  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.png";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.png";
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" align=\"left\" height=\"115\" width=\"160\">\n";}
  
  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.jpg";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.jpg";  
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" align=\"left\" height=\"115\" width=\"160\">\n";}
  
  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.gif";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.gif";
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" align=\"left\" height=\"115\" width=\"160\">\n";}

  if ($name)  {echo "<strong>$name</strong><br/>\n";}
  if ($role)  {echo "<i>$role</i><br/>\n";}
  if ($dept)  {echo "$dept<br/>\n";}
  if ($univ)  {echo "$univ<br/>\n";}
  if ($add1)  {echo "$add1<br/>\n";}
  if ($add2)  {echo "$add2<br/>\n";}
  if ($add3)  {echo "$add3<br/>\n";}
  if ($city)  {echo "$city, $state $zip<br/>\n";}
  if ($email) {echo "$email<br/>\n";}
  if ($phon)  {echo "$phone<br/>\n";}
  if ($fax)   {echo "$fax<br/>\n";}

}


function show_contact_short($short,$tr=null,$P_or_L=null) {
/* Show contact information in abreviated format:

   $short  - is the short name such as initials (e.g., JFL or ESC or CMC)
   $tr     - table row (S) start new, (M) middle (end last & start new), (S) end last
   $P_or_L - (P) Portrait or (L) Landscape 

*/

  $cfile = BASEPATH . "/qcn/about/contacts/".$short."/contact.info";
  $contents = file($cfile);

  foreach ($contents as $line_num => $line) {
    list($tname,$value) = split('[:]',$line);

    if     ($tname=="name") {$name=$value;}
    elseif ($tname=="role") {$role=$value;}
    elseif ($tname=="dept") {$dept=$value;}
    elseif ($tname=="univ") {$univ=$value;}
    elseif ($tname=="add1") {$add1=$value;}
    elseif ($tname=="add2") {$add2=$value;}
    elseif ($tname=="add3") {$add3=$value;}
    elseif ($tname=="city") {$city=$value;}
    elseif ($tname=="state"){$state=$value;}
    elseif ($tname=="zip")  {$zip  =$value;}
    elseif ($tname=="email"){$email=$value;}
    elseif ($tname=="phone"){$phone=$value;}
    elseif ($tname=="fax")  {$fax  =$value;}

  }

  if ( ($tr=="S")||($tr=="M") ) {echo "<tr>";}    // Start new table row
  if ($P_or_L=="P") {
    $height=150;
    $width =115;
  } else if ($P_or_L=="L") {
    $height=115;
    $width =160;
  }
 

  echo "<td width=\"50%\"><a href=\" . BASEURL . "/about/contacts/contacts.php?dir=".$short."\">\n";

  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.png";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.png";
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" align=\"left\" height=\"".$height."\" width=\"".$width."\" hspace=\"6\">\n";}
  
  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.jpg";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.jpg";  
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" align=\"left\" height=\"".$height."\" width=\"".$width."\" hspace=\"6\">\n";}
  
  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.gif";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.gif";
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" align=\"left\" height=\"".$height."\" width=\"".$width."\" hspace=\"6\">\n";}

  echo "</a>\n";
  echo "";
  if ($name)  {echo "<a href=\" . BASEURL . "/about/contacts/".$short."/\">\n<h3>$name</h3>\n</a>";}
  if ($role)  {echo "<i>$role</i><br/>\n";}
  if ($univ)  {echo "$univ<br/>\n";}
  if ($email) {echo "<div style=\"alink:#900;vlink:#900;link:#900;\"><a href=\"mailto:$email\">$email</a><br/></div>\n";}
  if ($phone)  {echo "$phone<br/>\n";}
  echo "</td>\n";
  if ( ($tr=="M")||($tr=="E") ) {echo "</tr>\n";} // End last table row
}

function show_name($short) {
  $cfile = BASEPATH . "/qcn/about/contacts/".$short."/contact.info";

  if (file_exists($cfile)) {
  $contents = file($cfile);
  } else {
     echo "<p>Contact not found. Please try again.\n";
     return;
  }

  foreach ($contents as $line_num => $line) {
    list($tname,$value) = split('[:]',$line);

    if ($tname=="name") {
      $name=$value;
      echo "<h1><center>".$name."</center></h1>\n";
      return $name;
    }
  }  
}



function show_info($short) {


  $cfile = BASEPATH . "/qcn/about/contacts/".$short."/contact.info";

  if (file_exists($cfile)) {
  $contents = file($cfile);
  } else {
     echo "<p>Contact not found. Please try again.\n";
     return;
  }

  foreach ($contents as $line_num => $line) {
    list($tname,$value) = split('[:]',$line);

    if     ($tname=="name") {$name=$value;}
    elseif ($tname=="role") {$role=$value;}
    elseif ($tname=="dept") {$dept=$value;}
    elseif ($tname=="univ") {$univ=$value;}
    elseif ($tname=="add1") {$add1=$value;}
    elseif ($tname=="add2") {$add2=$value;}
    elseif ($tname=="add3") {$add3=$value;}
    elseif ($tname=="city") {$city=$value;}
    elseif ($tname=="state"){$state=$value;}
    elseif ($tname=="zip")  {$zip  =$value;}
    elseif ($tname=="email"){$email=$value;}
    elseif ($tname=="phone"){$phone=$value;}
    elseif ($tname=="fax")  {$fax  =$value;}
    elseif ($tname=="web")  {$web  =$value;}

  }

  echo "<td width=\"120\"><strong>\n";
   
   if ($role)  {echo "<p>Title:</p>\n";}
   if ($dept)  {echo "<p>Department:</p>\n";}
   if ($univ)  {echo "<p>Insitution:</p>\n";}
   if ($add1)  {echo "<p>Address:<br/>\n";}
   if ($add2)  {echo "&#32;<br/>\n";}
   if ($add3)  {echo "&#32;<br/>\n";}
   if ($city)  {echo "&#32;<br/>\n";}
   if ($email) {echo "<p>Email:</p>\n";}
   if ($phone) {echo "<p>Phone:</p>\n";}
   if ($fax)   {echo "<p>Fax:</p>\n";}
   if ($web)   {echo "<p>Web:</p>\n";}

  echo "</strong></td><td width=\"300\">\n";

   if ($role)  {echo "<p><i>$role</i></p>\n";}
   if ($dept)  {echo "<p>$dept</p>\n";}
   if ($univ)  {echo "<p>$univ</p>\n";}
   if ($add1)  {echo "<p>$add1<br/>\n";}
   if ($add2)  {echo "   $add2<br/>\n";}
   if ($add3)  {echo "   $add3<br/>\n";}
   if ($city)  {echo "   $city, $state $zip<br/>\n";}
   if ($email) {echo "<p><a href=\"mailto:$email\">$email</a></p>\n";}
   if ($phone) {echo "<p>$phone</p>\n";}
   if ($fax)   {echo "<p>$fax</p>\n";}
   if ($web)   {echo "<p><a href=\"http://$web\">$web</a></p>\n";}

  echo "</td>\n";

}

function show_picture_main($short){

  $height=300;
  $width =230;

  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.png";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.png";
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" height=\"".$height."\" width=\"".$width."\">\n";return;}
  
  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.jpg";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.jpg";  
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" height=\"".$height."\" width=\"".$width."\">\n";return;}
  
  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_main.gif";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_main.gif";
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" height=\"".$height."\" width=\"".$width."\">\n";}

}

function show_picture_other($short,$num,$width,$height,$align){

  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_".$num.".png";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_".$num.".png";

  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" height=\"".$height."\" width=\"".$width."\" align=\"".$align."\">\n"; return;}

  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_".$num.".jpg";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_$num.jpg";
  if (file_exists($ifile)) {echo "<img src=\"$ifile2\" height=\"$height\" width=\"$width\" align=\"$align\" hspace=\"6\">\n"; return;}

  $ifile = BASEPATH . "/qcn/about/contacts/".$short."/picture_".$num.".gif";
  $ifile2= BASEURL . "/about/contacts/".$short."/picture_".$num.".gif";
  if (file_exists($ifile)) {echo "<img src=\"".$ifile2."\" height=\"".$height."\" width=\"".$width."\" align=\"".$align."\">\n";}

}


function show_contact_page($short,$edit=null) {
/* Show contact information in abreviated format:

   $short  - is the short name such as initials (e.g., JFL or ESC or CMC)
   $tr     - table row (S) start new, (M) middle (end last & start new), (S) end last
   $P_or_L - (P) Portrait or (L) Landscape 

*/
  write_onclick();
  $name=show_name($short);

  echo "<table><tr><td width=\"250\">\n";
  show_picture_main($short);
  echo "</td>\n";
  if ($edit) {
   edit_file($short,$file_edit="contact.info",$ncol="50");
  } else {
   show_info($short);
  }
  echo "</tr></table>";

  echo "<hr/>\n";
  if ($edit) {
   edit_file($short,$file_edit="contact.about");
  } else {
   show_blurb($short,$name);
  }
  echo "<hr/>\n";
  if ($edit) {
   edit_file($short,$file_edit="contact.q_and_a");
  } else {
   show_q_and_a($short,$name);
  };
  echo "<hr/>\n";
}

function write_onclick(){
echo "
<script type=\"text/javascript\">
   
function update_form(FormName, FieldName, FormValue)
{
	if(!document.forms[FormName])
		return;
	document.forms[FormName].elements[FieldName] = FormValue;
	if(!objCheckBoxes)
}
</script>\n";
}

function edit_file($short,$filen,$ncol=null){

  $cfile = BASEPATH . "/qcn/about/contacts/".$short."/".$filen;

  if (file_exists($cfile)) {
   $content = file($cfile);
   $contents2 = implode ( $content );
  } else {
   $contents2="";
  }
  if ($ncol) {}else{$ncol=83;}
  echo "<td>\n";
  echo "<form name=\"".$filen."\" method=\"post\" action=\" . BASEURL . "/about/contacts/edit_contacts.php\">\n";
  echo "<input type=\"hidden\" name=\"dir\" value=\"".$short."\">\n";
  echo "<input type=\"hidden\" name=\"file_name\" value=\"".$filen."\">\n";
  echo "<p><textarea name=\"content\" cols=\"".$ncol."\" rows=\"25\">\n";
  echo $contents2;
  echo "</textarea>\n";
  echo "<p><input type=\"button\" value=\"Update\" onclick=\"update_form('".$filen."','content','".$contents2."')\">\n";
  echo "<input type=\"submit\" name=\"Save File\" value=\"Send\"></td>\n";
  echo "</form>\n";
  echo "</td>\n";
}


function show_blurb($short,$name){

  $cfile = BASEPATH . "/qcn/about/contacts/".$short."/contact.about";
  if (file_exists($cfile)) {
   $contents = file($cfile);
   list($first_name,$middle_name,$last_name) = split('[ ]',$name);
   echo "<h2><center>About ".$first_name.":</center></h2>\n";
   echo "<center><table width=\"500\"><tr><td>";
   show_picture_other($short,$num="01",$width="300",$height="200",$align="right");
   foreach ($contents as $line_num => $line) {
     echo "<p align=\"justify\">$line\n";
   }
   echo "</td></tr></table></center>";
   echo "<br/>\n";
  } else {
     echo "<p><center>Profile Coming Soon.</center>\n";
     return;
  }

}


function show_q_and_a($short,$name){

  $cfile = BASEPATH . "/qcn/about/contacts/".$short."/contact.q_and_a";
  if (file_exists($cfile)) {


   list($first_name,$middle_name,$last_name) = split('[ ]',$name);
   echo "<h2><center>Quakes &amp; Aftershocks with $first_name:<br/> (Q&A)</center></h2>\n";

   $contents = file($cfile);
   echo "<center><table width=\"500\"><tr><td>\n";
   show_picture_other($short,$num="02",$widht=300,$height=200,$align="right");
   foreach ($contents as $line_num => $line) {
     list($tname,$value) = split('[:]',$line);

     if ($tname=="q") {
       echo "<p align=\"justify\"><strong><i>$value</i></strong><br/>\n";
     } else if ($tname=="a") {
       echo "<dd><p align=\"justify\">$value</p></dd><br/>\n";
     }
   }
   echo "</td></tr></table></center>\n";


  } else {
     echo "<p><center>Q&amp;A coming soon.</center>\n";
     return;
  }


}



?>



