<?php

  # Grab the current page that user is on
  $page_name = $_SERVER['REQUEST_URI'];
  $slash_idx = strrpos($page_name, "/");
  $page_name = substr($page_name, $slash_idx + 1);

  # (AUTHENTICATE) Ensure secure session
  # Do no include session data for index page

  if ($page_name != 'index.php') { include_once 'session.php'; }
  
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
<head>
  <link type="text/css" rel="stylesheet" href="_stylesheets/default.css" media="screen" />
  <script src="http://www.google.com/jsapi"></script>
  <script type="text/javascript">
    google.load("jquery", "1.3.2");
  </script>
  <script src="_javascripts/flash.js"></script>
  <script src="_javascripts/tablestripes.js"></script>
</head>
<body>
<?= $flashalert ?>

<?php if ($page_name != 'index.php') {  ?>

    <div id="container">
      <p id="userid">You are logged in as <em><a href="users.php?uid=<?=$_SESSION['username'] ?>"><?=$_SESSION['username'] ?></a></em></p>
    <div id="navbar">
      <ul>
        <li><a href="purchases.php"<?= $transactionlisting ?>>Purchase Transactions</a></li>
        <li><a href="rentals.php"<?= $rentallisting ?>>Borrowed Sensors</a></li>        
        <li><a href="taxes.php"<?= $taxlisting ?>>Taxes</a></li>        
        <li><a href="cities.php"<?= $citieslisting ?>>Cities</a></li>        
        <li><a href="logout.php"<?= $logoutlisting ?>>Logout</a></li>            
      </ul>
    </div>

<?php  }  ?>