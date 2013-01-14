<?php
  include_once 'session.php';

  # Only display the flash message when 'msg' is received from purchase.php stating that 
  # the deletion occured.
  
  if ( isset($_GET['msg']) ) {
    $msg = mysql_real_escape_string($_GET['msg']);
    if ($msg == 'deleted' ) { $flashalert = '<h1 class="flash-alert">Transaction entry was successfully deleted.</h1>';  }
    else if ($msg == 'noauth' ) { $flashalert = '<h1 class="flash-alert">Sorry you do not have permission to do that.</h1>';  }
  } 


  # Open database connection
  include_once 'database.php';

  # Set this page as current in navigation
  $rentallisting = ' class="current"';

  # Page header including doctype
  include_once 'page-header.php';

  # Print export button
  print '<a class="link-button" href="exportrentals.php">Export Borrowed Transactions &raquo;</a>';    
  
  # Print Page Title
  print '<div class="spacer"></div><h1>QCN Borrowed Sensor Log <span style="font-size:80%;font-weight:normal"></h1>';

  $page = "<br /><table>
               <tr style='font-weight:bold;background:#EEE'>
                  <td>Full Name</td>
                  <td>Phone</td>
                  <td>Email</td>
                  <td>Street Address</td>  
                  <td>City</td>
                  <td>State</td>
                  <td>Zip</td>
                  <td>Country</td>  
                  <td>Requested on</td>
                  <td>Rental Start Date</td>
                  <td>Rental Start Date</td>
                  <td class='divider'>Rental <br />Approved <br /></td>       
                  <td>Shipped</td>                                                
               </tr>";  
  

  $result = mysql_query("SELECT * FROM rentals_tbl Order By requestdate DESC");                
  while( $row = mysql_fetch_array($result) ) {
  
    
    $page .= "<tr>
                    <td><a href='rental.php?id=" . $row['id'] . "'>" .$row['name']. "</a></td>  
                    <td>" .$row['phone']. "</td>  
                    <td>" .$row['email']. "</td>  
                    <td>" .$row['address']. "</td>  
                    <td>" .$row['city']. "</td>  
                    <td>" .$row['state']. "</td>  
                    <td>" .$row['zip']. "</td>  
                    <td>" .$row['country']. "</td>  
                    <td>" .$row['requestdate']. "</td>
                    <td>" .$row['startdate']. "</td>
                    <td>" .$row['enddate']. "</td> 
                    <td>" .$row['rental_approved']. "</td>                       
                    <td>" .$row['shipstatus']. "</td> 
                </tr>";
  }
      
  $page .= "</table>";
  echo $page;
  mysql_close($link);
 
?>

</div>
</body>
</html>