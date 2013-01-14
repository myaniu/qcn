<?php

  # Open database connection
  include 'database.php';
  
  #Set page as current in navigation
  $citieslisting = ' class="current"';

  # Only display the flash message when 'msg' is received from city.php stating that 
  # the deletion occured.
  
  if ( isset($_GET['msg']) ) {
  $msg = mysql_real_escape_string($_GET['msg']);
  switch ($msg) {
      case 'newcity':
        $flashalert = '<h1 class="flash-alert">City entry was successfully created.</h1>';  
        break;
      case 'deletecity':
        $flashalert = '<h1 class="flash-alert">City entry was successfully deleted.</h1>';  
        break;
    }
  }

  # Page header including doctype
  include 'page-header.php';

  print '<a class="link-button" href="newcity.php">Add a new city &raquo;</a>';
  print '<div class="spacer"></div><h1>List of cities</h1>';

  $page = "<br /><table>
               <tr style='font-weight:bold;background:#EEE'>
                  <td>City</td>
                  <td>County</td>
                  <td>Tax</td>
               </tr>";  
  
  $result = mysql_query("SELECT * FROM cities_tbl ORDER BY city ASC");
  while( $row = mysql_fetch_array($result) ) {

  $rid = $row['region_id'];
  $taxresult = mysql_query("SELECT * FROM taxregions_tbl WHERE id = $rid");
  $taxrow = mysql_fetch_array($taxresult);
  
  $page .= "<tr>
                  <td><a href='city.php?id=" .$row['id']. "'>"   .$row['city']. "</td>
                  <td>" . $taxrow['region'] . "</td>
                  <td>" . $taxrow['tax'] . " %</td>                  
                  </tr>";

  }
  $page .= "</table>";

  echo $page;
  mysql_close($link);
 
?>



</div>

</body>
</html>


