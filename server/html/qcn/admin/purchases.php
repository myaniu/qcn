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
  $transactionlisting = ' class="current"';

  # Page header including doctype
  include_once 'page-header.php';

  # Print export button
  print '<a class="link-button" href="export.php">Export Purchase Transactions &raquo;</a>';    
  
  # Print Page Title
  print '<div class="spacer"></div><h1>QCN Purchase Transactions Log <span style="font-size:80%;font-weight:normal"></h1>';

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
                  <td>Quantity. <br />@ $5.00</td>
                  <td>Quantity <br />@ $49.00</td>
                  <td>Date of Purchase</td>                  
                  <td>Transaction Id</td>                                    
                  <td>Region<br /> Tax</td>
                  <td>Tax <br />Deduction</td>
                  <td>Purchase <br />Price</td>
                  <td>Adjusted <br />Total</td>   
                  <td class='divider'>$5 Sensor <br />Approved <br /></td>       
                  <td>Shipped</td>                         
                  <td>Confirmation <br />Sent</td>                                    
               </tr>";  
  

  $result = mysql_query("SELECT * FROM transactions_tbl Order By purchasedate DESC");                
  while( $row = mysql_fetch_array($result) ) {
  
      # Get associated region id
      $rid = $row['region_id'];

      # Calculate total amount owed by client
    
      $purchasePrice = ($row['qtySensor5'] * 5.00) + ($row['qtySensor49'] * 49.00);
      $country = $row['country'];
      if ($country != 'USA' && $country != 'CAN' && strtotime($row['purchasedate']) > strtotime('03/12/2012')) {
          $purchasePrice += 10;
      }
      $purchasePrice = $purchasePrice . '.00';
    
      # Calculate tax amount to be deducted from total owed
      # Sensors are sold at a flat rate, and tax is deducted from that price and paid
      # for by QCN.
      
      # Fetch the taxable ambout for each city based on county
      
      $taxresult = mysql_query("SELECT * FROM taxregions_tbl WHERE id = '$rid'");

    
          # Calculate what was charged to the buyer
          if ($row['qtySensor5'] != 0 && $row['qtySensor49'] != 0) { 
            $billedAmount =  (($row['qtySensor5'] * 5.00) + ($row['qtySensor49'] * 49.00));
          }
          if ($row['qtySensor5'] != 0 && $row['qtySensor49'] == 0 ) { 
            $billedAmount =  ($row['qtySensor5'] * 5.00);
          }      
          if ($row['qtySensor5'] == 0 && $row['qtySensor49'] != 0 ) { 
            $billedAmount =  ($row['qtySensor49'] * 49.00);
          }      
          if ($country != 'USA' && $country != 'CAN' && strtotime($row['purchasedate']) > strtotime('03/12/2012')) {
              $billedAmount += 10;
          }
          $billedAmount = $billedAmount . '.00';
    
          # Set tax
          $taxregion = $row['region_id'];
          if ($taxregion != '4') { 
            $taxresult = mysql_query("SELECT * FROM taxregions_tbl WHERE id = '$taxregion'"); 
            $taxrow = mysql_fetch_array($taxresult);
            $tax = $taxrow['tax'];
	    if (strtotime($row['purchasedate']) < strtotime('07/01/2011')) {
		if ($taxregion == '1' || $taxregion == '3') {
		    $tax += 0.75;
		} else if ($taxregion == '2') {
		    $tax += 0.625;
		}
	    } else if (strtotime($row['purchasedate']) < strtotime('01/01/2013')) {
                if ($taxregion == '1' || $taxregion == '3') {
                    $tax -= 0.25;
                } else if ($taxregion == '2') {
                    $tax -= 0.375;
                }
	    }
          } else { $tax = '0'; }
    
          if ($tax != '0') {
            $taxAmount = $billedAmount * ($tax * .01);
            $taxAmount = round($taxAmount, 2);  // $1.96            
            $total = $billedAmount - $taxAmount;
          } else {
            $taxAmount = '0';
            $total = $billedAmount;
          }
          
      $total = $purchasePrice - $taxAmount;
      $total = round($total, 2);  // $1.96
      
      if ( $row['qtySensor5'] != '0' ) { $approved = $row['sensor5_approved']; } else { $approved = 'n/a'; }
      
      $page .= "<tr>
                      <td><a href='purchase.php?id=" . $row['id'] . "'>" .$row['name']. "</a></td>  
                      <td>" .$row['phone']. "</td>  
                      <td>" .$row['email']. "</td>  
                      <td>" .$row['address']. "</td>  
                      <td>" .$row['city']. "</td>  
                      <td>" .$row['state']. "</td>  
                      <td>" .$row['zip']. "</td>  
                      <td>" .$row['country']. "</td>  
                      <td>" .$row['qtySensor5']. "</td>  
                      <td>" .$row['qtySensor49']. "</td>  
                      <td>" .$row['purchasedate']. "</td>                    
                      <td>" .$row['transactionid']. "</td>        
                      <td>" .$tax. " %</td>
                      <td style='text-align:right;color:red'>$" .$taxAmount. "</td>                      
                      <td style='text-align:right'>$" .$billedAmount. "</td>
                      <td style='text-align:right;color:green'>$" .$total. "</td>                      
                      <td align='center' class='divider'><strong>" . $row['sensor5_approved'] . "</strong></td>  
                      <td align='center'><strong>" .$row['shipstatus']. "</strong></td>  
                      <td align='center'><strong>" .$row['sensor5_econfirmed']. "</strong></td>                                              
                  </tr>";
  }
  
  $page .= "</table>";
  echo $page;
  mysql_close($link);
 
?>

</div>
</body>
</html>
