<?php
  date_default_timezone_set('America/Los_Angeles');  

  # Open mysql connection and select QCN database
  $link = mysql_connect('db-private', 'ecommerce', 'qcnEcom!');
  if (!$link)
      die('Could not connect: ' . mysql_error());

  # Select the db
  mysql_select_db('qcn_ecommerce', $link);  

?>