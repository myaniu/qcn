<?php
# Export all transactions from database into an excel file

# Open database connection
include 'database.php';

# Excel export class
require_once "xls.php";
  
  $xls = new ExcelExport();

  # Create headings
  $xls->addRow(Array("Name","Phone","Email","Address", "City", "State", "Zip", "Country", "School Name", "District", "Title I", "Percent Free Lunch", "Percent Eng Learners", "Percent Non White", "Years Taught", "Grades Taught", "Quantity",	"Request Date", "Start Date", "End Date",	"Approved", "Shipped"));

  # Read from transactions table
  $result = mysql_query("SELECT * FROM rentals_tbl Order By requestdate DESC");                

  while( $row = mysql_fetch_array($result) ) {

      # Populate spreadsheet cells with database row data      
      $xls->addRow(Array($row['name'], $row['phone'], $row['email'], $row['address'], $row['city'], $row['state'], $row['zip'], $row['country'], $row['schoolname'],  $row['district'],  $row['titlei'], $row['freelunch'],  $row['englearners'],  $row['nonwhite'],  $row['yearstaught'],  $row['gradestaught'], $row['quantity'],  $row['requestdate'], $row['startdate'], $row['enddate'], $row['rental_approved'], $row['shipstatus']));
  
  }
  
  # Create excel file for download
  $xls->download("qcn-BorrowedSensors.xls");


  # Close database connection
  mysql_close($link);
?>