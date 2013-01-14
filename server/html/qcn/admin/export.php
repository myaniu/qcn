<?php
# Export all transactions from database into an excel file

# Open database connection
include 'database.php';

# Excel export class
require_once "xls.php";
  
  $xls = new ExcelExport();

  # Create headings
  $xls->addRow(Array("Name","Phone","Email","Address", "City", "State", "Zip", "Country", "Quantity. @ $5.00", "Quantity @ $49.00",	"Date of Purchase",	"Transaction Id", "Amount Billed", "Tax (Percent)", "Tax Amount", "Grand Total", "$5 Sensor Approved", "Ship Status", "Email Confirmation Sent"));

  # Read from transactions table
  $result = mysql_query("SELECT * FROM transactions_tbl Order By purchasedate DESC");                

  while( $row = mysql_fetch_array($result) ) {
    
      # Calculate what was charged to the buyer
      if ($row['qtySensor5'] != 0 && $row['qtySensor49'] != 0) { 
        $billedAmount =  (($row['qtySensor5'] * 5.00) + ($row['qtySensor49'] * 49.00)) . '.00';
      }
      if ($row['qtySensor5'] != 0 && $row['qtySensor49'] == 0 ) { 
        $billedAmount =  ($row['qtySensor5'] * 5.00) . '.00';
      }      
      if ($row['qtySensor5'] == 0 && $row['qtySensor49'] != 0 ) { 
        $billedAmount =  ($row['qtySensor49'] * 49.00) . '.00';
      }      

      
      # Set tax
      $taxregion = $row['region_id'];
      if ($taxregion != '4') { 
        $taxresult = mysql_query("SELECT * FROM taxregions_tbl WHERE id = '$taxregion'"); 
        $taxrow = mysql_fetch_array($taxresult);
        $tax = $taxrow['tax'];
        if (strtotime($row['purchasedate']) < strtotime('07/01/2011')) {
            $tax += 1;
        }
      } else { $tax = '0'; }

      if ($tax != '0') {
        $taxAmount = $billedAmount * ($tax * .01);
        $total = $billedAmount - $taxAmount;
      } else {
        $taxAmount = '0';
        $total = $billedAmount;
      }
      

      # Populate spreadsheet cells with database row data      
      $xls->addRow(Array($row['name'], $row['phone'], $row['email'], $row['address'], $row['city'], $row['state'], $row['zip'], $row['country'], $row['qtySensor5'], $row['qtySensor49'], $row['purchasedate'], $row['transactionid'], $billedAmount, $tax, $taxAmount, $total, $row['sensor5_approved'], $row['shipstatus'], $row['sensor5_econfirmed']));
  
  }
  
  # Create excel file for download
  $xls->download("qcn-transactions.xls");


  # Close database connection
  mysql_close($link);
?>
