<?php
  include_once 'session.php';
  include_once 'database.php';
    
  if ( isset($_POST['newcity']) ) {
  
    # Retrieve $_POST data
    $city = mysql_real_escape_string($_POST['city']);    
    $region_id = mysql_real_escape_string($_POST['region_id']);    

    # INSERT new city into cities_tbl
    $query = mysql_query("INSERT INTO cities_tbl VALUES (
            'NULL',
            '".$region_id."',
            '".$city."'
            )");
   
    header("location:cities.php?msg=newcity");    
  }

    # Page header including doctype
    include 'page-header.php';

    $page = "<form action='newcity.php' method='post' id='city'>
                <input type='hidden' name='id' value='".$row['id']."' />
                <fieldset>
                  <table style='width:600px;margin:auto'>
                    <tr style='background-color:#EEE'><td colspan='2'><h3>New city entry</h3></td></tr>
                    <tr><td>City</td><td><input type='text' name='city' /></td></tr> 
                    <tr><td>County</td>
                    <td>
                      <select name='region_id'>";

    $regionresult = mysql_query("SELECT * FROM taxregions_tbl WHERE id > 1 ORDER BY id ASC LIMIT 2");
    while( $regionrow = mysql_fetch_array($regionresult) ) {                    
      if( $rid == $regionrow['id'] ) { $selected = 'selected'; } else { $selected = ''; }
      
    $page   .= "<option value='". $regionrow['id']. "' $selected>" . $regionrow['region'] . " -- (" . $regionrow['tax'] ."%)</option>";
    }    
                    
    $page .= " </select>
                </td></tr>                     
                </table>
                <div class='spacer'></div>
                <input type='submit' name='newcity' value='Create city entry' class='button' />
                </fieldset>
                </form>";    

  #Close DB connection
  mysql_close($link);

?>
  <div class="spacer"></div>
  <h1>Add new City</h1><br />
  <?= $page; //print transaction data ?>
  <br /><center><a href="cities.php">&laquo; Return to list of cities</a></center>  
</body>
</html>







