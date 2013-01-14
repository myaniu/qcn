<?php

  #Set page as current in navigation
  $taxlisting = ' class="current"';

  # Authenticate session
  include_once 'session.php';
  
  # Open database connection
  include_once 'database.php';  

  # Check if a post is being made 
  # and perform database update
  
  if ( isset($_POST['updatetax']) ) {
   
      foreach ($_POST as $key => $value) {
        $newtax = mysql_real_escape_string(trim($value));
        $rowid = substr($key, 10, 1);
      
        # Update tax with new values
        mysql_query("UPDATE taxregions_tbl SET tax = $newtax WHERE id = $rowid ");    
      }

      # Set updated record message
      $flashalert = '<h1 class="flash-alert">Tax information was submitted.</h1>';  
    
  }
  
  # Page header including doctype and session data
  # page-header was included after $flashaert was set, so that when a POST occurs
  # an appropriate message will be displayed to user.
  
  include_once 'page-header.php';  
  
  # Print page heading
  print '<div class="spacer"></div>';
  print '<h1>Regional sales tax adjustment</h1><br />';

  
  $page = "<form action='taxes.php' method='post'>
                <fieldset>
                <table>
                <tr style='font-weight:bold;background:#EEE'>
                  <td>Region</td>
                  <td>Tax</td>
                </tr>";  
  

  $result = mysql_query("SELECT * FROM taxregions_tbl ORDER BY id ASC");
  while( $row = mysql_fetch_array($result) ) {
  
  
  $page .= "<tr>
                  <td>" .$row['region']. "</td>
                  <td>
                  <input type='text' name='region-id_" .$row['id']. "' class='num' value='" .$row['tax']. "'> <p>%</p>
                  </td>
                </tr>";
  
  }
  $page .=  "</table>
                <input type='submit' value='Update Taxes' name='updatetax' class='button'>
                </fieldset>
                </form>";

  # Print page results
  echo $page;

  # Close db connection
  mysql_close($link);
  
?>



</div>

</body>
</html>