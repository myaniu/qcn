<?php

  # Ensure secure session (Authentication)
  include_once 'session.php'; 

  # Open mysql connection and select database
  include_once 'database.php';  


  # Check status of "delete entry" checkbox
  if ( isset($_POST['updatepurchase']) &&  $_POST['delete'] != '' ) {

      # Delete transaction row
      $purchaseid = mysql_real_escape_string($_POST['id']);    

      $sql = "DELETE FROM transactions_tbl WHERE id = $purchaseid";
      mysql_query($sql);

      # Send user to all transactions listing and set display message
      header("location:purchases.php?msg=deleted");
    
  # Else build transaction detail page and retrieve data
  } else {
      
      # Update shipping status
      if (isset($_POST['updatepurchase']) && $_POST['delete'] == '' ) {      
 
          $order_id = mysql_real_escape_string($_POST['id']);
          $sensor5_approved = mysql_real_escape_string($_POST['sensor5_approved']);          
          $sensor5_econfirmed = mysql_real_escape_string($_POST['sensor5_econfirmed']); 
          $shipstatus = mysql_real_escape_string($_POST['shipstatus']); 
          $lastupdatedby = mysql_real_escape_string($_POST['lastupdatedby']); 
          $lastupdatedon = mysql_real_escape_string($_POST['lastupdatedon']);           
          
          #Handle the disabled drop downs
          if ($sensor5_approved == '') { $sensor5_approved = 'no'; }
          if ($sensor5_econfirmed == '') { $sensor5_econfirmed = 'no'; }      
          if ($shipstatus == '') { $shipstatus = 'no'; }      
                
          $query = mysql_query("UPDATE transactions_tbl SET 
                    sensor5_approved = '$sensor5_approved',
                    sensor5_econfirmed = '$sensor5_econfirmed',
                    shipstatus = '$shipstatus',
                    lastupdatedby = '$lastupdatedby',
                    lastupdatedon = '$lastupdatedon'
                    WHERE id = '$order_id'");        
  
          $flashalert = '<h1 class="flash-alert">Transaction status updated.</h1>';

      }
 
      #include page-header and session data
      include 'page-header.php';

      #Aquire ID via GET
      if (isset($_GET['id'])) { $order_id = mysql_real_escape_string($_GET['id']); }
      else { $order_id = mysql_real_escape_string($_POST['id']); }
    
      //Retrieve results from table based on $ORDER_ID
      $result = mysql_query("SELECT * FROM transactions_tbl WHERE id = $order_id");
      $row = mysql_fetch_array($result);

      # If the ship status has not been updated, check status from db.
      if ( !isset($_POST['updatepurchase']) ) { 
          $sensor5_approved = $row['sensor5_approved']; 
          $sensor5_econfirmed = $row['sensor5_econfirmed']; 
          $shipstatus = $row['shipstatus']; 
          $lastupdatedby = $row['lastupdatedby']; 
          $lastupdatedon = $row['lastupdatedon']; 
      }
      
      //Calculate total price of sensors
      $totalDue =  (($row['qtySensor5'] * 5.00) + ($row['qtySensor49'] * 49.00));
      $country = $row['country'];
      if ($country != 'USA' && $country != 'CAN' && strtotime($row['purchasedate']) > strtotime('03/12/2012')) {
          $totalDue += 10;
      }
      $totalDue = $totalDue . '.00';
              
      $page = "   <h1>Detailed transaction information</h1><br />
                  <div class='spacer'></div>
                  <table style='width:600px;margin:auto'>
                    <tr style='background-color:#EEE'><td colspan='2'><strong>Billing Info</strong></td></tr>
                    <tr><td>Transaction id</td><td>" . $row['transactionid'] . "</td></tr>
                    <tr><td>Purchase Date</td><td>" . $row['purchasedate'] . "</td></tr> 
                    <tr><td>Name</td><td>" . $row['name'] . "</td></tr> 
                    <tr><td>Phone</td><td>" . $row['phone'] . "</td></tr> 
                    <tr><td>Email</td><td>" . $row['email'] . "</td></tr> 
                    <tr><td>Address</td><td>" . $row['address'] . "</td></tr> 
                    <tr><td>City</td><td>" . $row['city'] . "</td></tr> 
                    <tr><td>State</td><td>" . $row['state'] . "</td></tr> 
                    <tr><td>Zip</td><td>" . $row['zip'] . "</td></tr> 
                    <tr><td>Country</td><td>" . $row['country'] . "</td></tr> 
                    <tr><td>Qty. @ $5 <span style='font-size:80%;font-style:italic'>(Maximum 3)</span></td><td>" . $row['qtySensor5'] . "</td></tr> 
                    <tr><td>Qty. @ $49</td><td>" . $row['qtySensor49'] . "</td></tr> 
                    <tr><td><strong>Total Purchase Amount</strong></td><td>\$$totalDue</td></tr>
                  </table>
                  <div class='spacer'></div>
    
                  <table style='width:600px;margin:auto'>
                    <tr style='background-color:#EEE'><td colspan='2'><strong>Shipping Info</strong></td></tr>
                    <tr><td>Name</td><td>" . $row['shipname'] . "</td></tr> 
                    <tr><td>Address</td><td>" . $row['shipaddress'] . "</td></tr> 
                    <tr><td>City</td><td>" . $row['shipcity'] . "</td></tr> 
                    <tr><td>State</td><td>" . $row['shipstate'] . "</td></tr> 
                    <tr><td>Zip</td><td>" . $row['shipzip'] . "</td></tr> 
                    <tr><td>Country</td><td>" . $row['shipcountry'] . "</td></tr> 
                  </table>";
    
      if ( $row['schoolname'] != '' ) {
                  
        $page .= "              
                    <div class='spacer'></div>              
                    <table style='width:600px;margin:auto'>
                      <tr style='background-color:#EEE'><td colspan='2'><strong>School Info</strong></td></tr>
                      <tr><td>School Name</td><td>" . $row['schoolname'] . "</td></tr> 
                      <tr><td>School District</td><td>" . $row['district'] . "</td></tr> 
                      <tr><td>Title I</td><td>" . $row['titlei'] . "</td></tr> 
                      <tr><td>Percent free / reduced lunch</td><td>" . $row['freelunch'] . "</td></tr> 
                      <tr><td>Percent English language learners</td><td>" . $row['englearners'] . "</td></tr> 
                      <tr><td>Grades Taught</td><td>" . $row['gradestaught'] . "</td></tr> 
                      <tr><td>Years Taught</td><td>" . $row['yearstaught'] . "</td></tr>                 
                      <tr><td>Percent non-white students?</td><td>" . $row['nonwhite'] . "</td></tr>                 
                    </table>";              
        }
        
        $page .= "
                    <div class='spacer'></div>
                    <form action='purchase.php' method='post'>
                      <table style='width:600px;margin:auto'>";
                    if ( $row['qtySensor5'] > '0') {
              
                      $page .= "
                      <tr style='background-color:#EEE'><td colspan='2'><strong>Transaction Status</strong></td></tr>
                      <tr><td><label for='delete'><strong style='color:green'>$5 Sensor approved? <cite>(By Jenny)</cite></strong></label></td><td>";
                      
                        $page .= "<select name='sensor5_approved'>";

                        if ( $sensor5_approved != 'yes' && $row['qtySensor5'] > '0') {
                              $page .= "<option value='no'>No</option>";
                              $page .= "<option value='yes'>Yes</option>";
                        } else {
                              $page .= "<option value='yes'>Yes</option>";                    
                              $page .= "<option value='no'>No</option>";
                        }
                    

                              $page .= " </select></td></tr>";                      
                    }

        $page .= "<tr><td><label for='shipstatus' disabled='disabled'><strong style='color:green'>Sensor Shipped</strong></label></td>";
        
                    $page .= "<td><select name='shipstatus'>";

                    if ( $shipstatus != 'yes') {
                          $page .= "<option value='no'>No</option>";
                          $page .= "<option value='yes'>Yes</option>";
                    } else {
                          $page .= "<option value='yes'>Yes</option>";                    
                          $page .= "<option value='no'>No</option>";
                    }
        $page .= " </select></td></tr>";                  

        $page .= "<tr><td><label for='delete'><strong style='color:green'>Email Confirmation Sent</strong></label></td>
                  <td>";

                    $page .= "<select name='sensor5_econfirmed'>";
                    
                    if ( $sensor5_econfirmed != 'yes') {
                          $page .= "<option value='no'>No</option>";
                          $page .= "<option value='yes'>Yes</option>";
                    } else {
                          $page .= "<option value='yes'>Yes</option>";                    
                          $page .= "<option value='no'>No</option>";
                    }
                  
        $page .= " </select></td></tr>
                   <tr><td>
                       <input type='hidden' name='lastupdatedby' value='".$_SESSION['username']."' />
                       <input type='hidden' name='lastupdatedon' value='". date("F j, Y, g:i a") ."' />
                       <cite>Record last updated by:</cite></td><td><cite>". $row['lastupdatedby'] ."&nbsp;&nbsp; <strong>" . $row['lastupdatedon'] ."</strong></cite></td></tr>
                   </table>";

        $page .= "  <table style='width:600px;margin:20px auto; background:#CCC;'>
                      <tr><td><input type='hidden' name='id' value='$order_id' /><label for='delete'><strong style='color:red'>Permanently delete this entry?</strong></label></td>
                      <td><input type='checkbox' name='delete' id='delete' class='box' />&nbsp;<cite>(Warning! This cannot be undone)</cite></td></tr>
                    </table>
                    <table style='width:600px;margin:auto'>
                        <tr><td colspan='2' align='right'><input type='submit' value='Update Transaction' name='updatepurchase' /></td></tr>
                    </table>
                    </form>";
   
      mysql_close($link);
      #print out detailed transaction data 
      print $page; 
  
    } # close else statement.
    
?>
  <br /><center><a href="purchases.php">&laquo; Return to the Transactions list</a></center>
  <div class="spacer"></div>
  </body>
  </html>
  
