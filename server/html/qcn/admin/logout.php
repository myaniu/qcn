<?php
  session_start();
  session_destroy(); 
  // CMC changed to use newer $_SESSION style for PHP
  $_SESSION["username"] = "";
  $_SESSION["password"] = "";

  header("location:session.php");
?>
