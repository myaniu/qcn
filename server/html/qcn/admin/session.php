<?php
  # State session
  date_default_timezone_set('America/Los_Angeles');  
  # Check if user is logged in otherwise redirect to login screen
  if (!($_SESSION['started'])) session_start();
  if( !isset($_SESSION["username"]) ) {
  //if(!session_is_registered("username")) {
    header("location:index.php");
  } else { include_once 'database.php'; }
?>
