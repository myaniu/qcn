<?php

  # Ensure secure session (Authentication)
  include 'session.php'; 

  if ( isset($_POST['updatecity']) ) {
  
  $deletecity = mysql_real_escape_string($_POST['delete']);
   # Check status of "delete entry" checkbox
   
    if ( $deletecity != 'on') {
  
    # Update cities_tbl with new values
  
      $cityid = mysql_real_escape_string($_POST['id']);    
      $city = mysql_real_escape_string($_POST['city']);    
      $region_id = mysql_real_escape_string($_POST['region_id']);    

      $sql = "UPDATE cities_tbl SET region_id = '$region_id', city = '$city' WHERE id = $cityid";
      mysql_query($sql);
      
  } else {
  
      # Delete city row
      $cityid = mysql_real_escape_string($_POST['id']);    
      $sql = "DELETE FROM cities_tbl WHERE id = $cityid";
      mysql_query($sql);
      # Send user to all cities listing
      header("location:cities.php?msg=deletecity");
  }
     
    $flashalert = '<h1 class="flash-alert">City information was submitted.</h1>';  

  }

  //Aquire ID via GET if the variable is being passed via URL, otherwise it's in $_POST
  if ( isset($_GET['id']) )
    $cityid = mysql_real_escape_string($_GET['id']);

  //Retrieve results from table based on $cityid
  $result = mysql_query("SELECT * FROM cities_tbl WHERE id = $cityid");
  $row = mysql_fetch_array($result);
  $rid = $row['region_id'];

    $page = "<form action='city.php' method='post' id='city'>
                <input type='hidden' name='id' value='".$row['id']."' />
                <fieldset>
                  <table style='width:600px;margin:auto'>
                    <tr style='background-color:#EEE'><td colspan='2'><h3>" . $row['city'] . "</h3></td></tr>
                    <tr><td>City</td><td><input type='text' name='city' value='" . $row['city'] . "' /></td></tr> 
                    <tr><td>County</td>
                    <td>
                      <select name='region_id'>";

    $regionresult = mysql_query("SELECT * FROM taxregions_tbl ORDER BY id ASC");
    while( $regionrow = mysql_fetch_array($regionresult) ) {                    
      if( $rid == $regionrow['id'] ) { $selected = 'selected'; } else { $selected = ''; }
      
    $page   .= "<option value='". $regionrow['id']. "' $selected>" . $regionrow['region'] . " -- (" . $regionrow['tax'] ."%)</option>";
    }    
                    
    $page .= " </select>
                </td></tr>                     
                <tr><td><label for='delete'><strong style='color:red'>Permanently delete this entry?</strong></label></td><td><input type='checkbox' name='delete' id='delete' class='box' /></td></tr>
                </table>
                <div class='spacer'></div>
                <input type='submit' name='updatecity' value='Update city entry' class='button' />
                </fieldset>
                </form>";    

  mysql_close($link);

  # Page header including doctype
  include 'page-header.php';
?>
  <div class="spacer"></div>
  <h1>Edit City</h1><br />
  <?= $page; //print transaction data ?>
  <br /><center><a href="cities.php">&laquo; Return to list of cities</a></center>  
</body>
</html>







