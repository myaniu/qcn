<?php

  #
  # Last modified on: September 16, 2009
  #
  # Martin Krakowski (mkrakowski@stanford.edu)
  # Transaction Admin interface for QCN Sensor store
  #
  # FILES :
  #
  # index.php         --> Process login information
  # login-form.php    --> Login form include file
  # logout.php        --> Session destroy, and point to login screen (index.php)
  # session.php       --> Check for login (authenticates all pages)
  # purchases.php  --> Display list of all transactions
  # purchase.php   --> Display details of a single transaction
  # taxes.php         --> Display and modify tax data for California as well as San Mateo, and Santa Clara counties
  # cities.php        --> Display list of cities within Santa Clara and San Mateo counties along with associated tax
  # city.php          --> Display and modify (& delete entry) details of a single city
  # newcity.php       --> Add a new city entry
  # insert_order.php  --> Post transaction details before and after gateway authentication to db
  #
  #***********************************************************************************************************************

  //Check if a POST Exists
  if ( isset($_POST['submit']) ) {
    
    include 'database.php';    
    
    ob_start();
    
    $username = $_POST['qcn-username'];
    $password = $_POST['qcn-password'];
  
    //Prevent SQL Injection
    $username = stripslashes($username);
    $password = stripslashes($password);
    $username = mysql_real_escape_string($username);
    $password = mysql_real_escape_string($password);
    $password = sha1($password);
   
    //Select User & Password from Database
    $sql = "SELECT * FROM qcnusers_tbl WHERE username='$username' and password='$password'";
    $result = mysql_query($sql);

    // Mysql_num_row is counting table row
    $count = mysql_num_rows($result);
    // If result matched $username and $password, table row must be 1 row
    if( $count == 1 ) {
      //session_register("username");
      //session_register("password");
      session_start();
      $_SESSION['started'] = true;
      $_SESSION['username'] = $username;
      $_SESSION['password'] = $password;
      header("location:purchases.php");
   }
    else {
   
    //Display Error Message
    $errormsg = "<p style='color:red;text-align:center'>Wrong User name or password.</p>";
    include 'login-form.php';
    }
    
    ob_end_flush();
  
  } 
  
  //If $_POST is not set display login prompt
  else {
    $errormsg = "<p style='color:red;text-align:center'>Please login to access the admin.</p>";
  include 'login-form.php';  
  } 


?>
