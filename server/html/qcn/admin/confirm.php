<?php

  # Open mysql connection and select database
  	include_once 'database.php';

  # Get referring page name
  $page_name = $_SERVER['HTTP_REFERER'];
  $slash_idx = strrpos($page_name, "/");
  $page_name = substr($page_name, $slash_idx + 1);
  
  if ($page_name != 'qcn-form_sensor5.php' && $page_name != 'qcn-form_sensor49.php' ) : 
    print '<h2>There was an error with your submission';
  else :

    //Collect and Sanitize POST data
    $name = mysql_real_escape_string($_POST['NAME']);
    $phone = mysql_real_escape_string($_POST['PHONE']);
    $email = mysql_real_escape_string($_POST['EMAIL']);
    $address = mysql_real_escape_string($_POST['ADDRESS']);
    $city = mysql_real_escape_string($_POST['CITY']);
    $shipname = mysql_real_escape_string($_POST['NAMETOSHIP']);
    $shipaddress = mysql_real_escape_string($_POST['ADDRESSTOSHIP']);
    $shipcity = mysql_real_escape_string($_POST['CITYTOSHIP']);
    if (!empty($_POST['domestic']) && $_POST['domestic'] == 'intl') {
	$state = mysql_real_escape_string($_POST['STATE2']);
        $zip = mysql_real_escape_string($_POST['ZIP2']);
        $country = mysql_real_escape_string($_POST['COUNTRY2']);
        $shipstate = mysql_real_escape_string($_POST['STATETOSHIP2']);
        $shipzip = mysql_real_escape_string($_POST['ZIPTOSHIP2']);
        $shipcountry = mysql_real_escape_string($_POST['COUNTRYTOSHIP2']);
        $surcharge = 10;
    } else {
        $state = mysql_real_escape_string($_POST['STATE']);
        $zip = mysql_real_escape_string($_POST['ZIP']);
        $country = mysql_real_escape_string($_POST['COUNTRY']);
        $shipstate = mysql_real_escape_string($_POST['STATETOSHIP']);
        $shipzip = mysql_real_escape_string($_POST['ZIPTOSHIP']);
        $shipcountry = mysql_real_escape_string($_POST['COUNTRYTOSHIP']);
	$surcharge = 0;
    }

    # Handle the $49 sensor form differently since there is only 1 product
    if ( $page_name != 'qcn-form_sensor49.php') {
      $qtySensor5 = mysql_real_escape_string($_POST['QUANTITY']);
      $qtySensor49 = mysql_real_escape_string($_POST['QUANTITY49']);  
    } else {
      $qtySensor49 = mysql_real_escape_string($_POST['QUANTITY']);    
    }
    
    $purchasedate = date("Y-m-d"); 
    $transactionid = 'Not Authorized';
    
    # School information fields for rentals only
    $token = mysql_real_escape_string($_POST['TOKEN']);
    $schoolname = mysql_real_escape_string($_POST['SCHOOLNAME']);
    $district = mysql_real_escape_string($_POST['DISTRICT']);
    $titlei = mysql_real_escape_string($_POST['TITLEI']);
    $freelunch = mysql_real_escape_string($_POST['FREELUNCH']);
    $englearners = mysql_real_escape_string($_POST['ENGLEARNERS']);  
    $gradestaught = mysql_real_escape_string($_POST['GRADESTAUGHT']);  
    $yearstaught = mysql_real_escape_string($_POST['YEARSTAUGHT']);  
    $nonwhite = mysql_real_escape_string($_POST['NONWHITE']);  

    # Terms and conditions
    $tandc = mysql_real_escape_string($_POST['TANDC']);

    # Set Transaction Status information
    $sensor5_approved = 'no';
    $sensor5_econfirmed = 'no';
    $shipstatus = 'no';
    $lastupdatedby = '';
    $lastupdatedon = '';

     
    # Check to see if the token already exists. to prevent multiple db writes incase user reloads the browser  
    #
    # Check if a California city is within either
    # San Mateo, or Santa Clara counties for tax purposes
    if ($country != 'USA' ) { $region_id = '4'; } # No tax for out of country purchases
    else {
          if ( $state != 'CA' ) {
              $region_id = '4';  
          } else {
            $result = mysql_query("SELECT * FROM cities_tbl WHERE city = '".$city."'");    
           
            # Mysql_num_row is counting table row
            $count = mysql_num_rows($result);
            if ( $count > 0 ) {           
               $row = mysql_fetch_array($result);
               $region_id = $row['region_id']; 
            } else {
                # Entry is in California but not in either San Mateo or Santa Clara counties
                $region_id = '1';
            }
          }
      }
  
    # Check to see if token already exists in transactions_tbl
    # Existing token means user has updated form fields
    
    $tokenResult = mysql_query("SELECT * FROM transactions_tbl WHERE token='$token'");
  
    $tokenCount = mysql_num_rows($tokenResult);
  
      if( $tokenCount < 1 ) {
      
        # INSERT OR UPDATE gathered data into table 

        $query = mysql_query("INSERT INTO transactions_tbl VALUES (
                'NULL',
                '".$token."',
                '".$schoolname."',
                '".$district."',
                '".$titlei."',
                '".$freelunch."',
                '".$englearners."',
                '".$gradestaught."',
                '".$yearstaught."',
                '".$nonwhite."',                    
                '".$region_id."',
                '".$transactionid."',          
                '".$purchasedate."',          
                '".$name."', 
                '".$phone."', 
                '".$email."', 
                '".$address."', 
                '".$city."', 
                '".$state."', 
                '".$zip."', 
                '".$country."', 
                '".$qtySensor5."', 
                '".$qtySensor49."', 
                '".$shipname."', 
                '".$shipaddress."', 
                '".$shipcity."', 
                '".$shipstate."', 
                '".$shipzip."', 
                '".$shipcountry."',
                '".$sensor5_approved."',                
                '".$sensor5_econfirmed."',
                '".$shipstatus."',
                '".$lastupdatedby."',
                '".$lastupdatedon."'
                )");
        $last_insert = mysql_insert_id();
    } else {
        # Since a token already exists UPDATE the existing entry
  
        $query = mysql_query("UPDATE transactions_tbl SET 
                  schoolname = '$schoolname',
                  district = '$district',
                  titlei = '$titlei',
                  freelunch = '$freelunch',
                  englearners = '$englearners',
                  gradestaught = '$gradestaught',
                  yearstaught = '$yearstaught',
                  nonwhite = '$nonwhite',                    
                  region_id = '$region_id',
                  transactionid = '$transactionid',          
                  purchasedate = '$purchasedate',
                  name = '$name', 
                  phone = '$phone', 
                  email = '$email', 
                  address = '$address', 
                  city = '$city', 
                  state = '$state', 
                  zip = '$zip', 
                  country = '$country', 
                  qtySensor5 = '$qtySensor5', 
                  qtySensor49 = '$qtySensor49', 
                  shipname = '$shipname', 
                  shipaddress = '$shipaddress', 
                  shipcity = '$shipcity', 
                  shipstate = '$shipstate', 
                  shipzip = '$shipzip', 
                  shipcountry = '$shipcountry'
                  WHERE token = '$token'");    
    }
    
      
    # Close DB connection
    mysql_close($link);
    
  
  ?>
  
  
  <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
  <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
  <head>
  <title>QCN Purchase Form</title>
  <script src="http://www.google.com/jsapi" type="text/javascript"></script>
  <script type="text/javascript">
    google.load("jquery", "1.3.2");
  </script>
  <script type="text/javascript">
    $(function() { 
      //Add stripes to table
      $("table tr:nth-child(odd)").addClass("striped");  
      //Handle Edit button
      $('#edit-info').click(function() {
        
        history.back();
      });
    });
  </script>
  <link type="text/css" rel="stylesheet" href="_stylesheets/forms.css" media="screen" />
  </head>
  <body>
    <h2>Order confirmation</h2>
  <?php if ( $schoolname != '' ) { /* For sensor rentals only */ ?>    
    <div class='spacer'></div>              
    <table style='width:550px;margin:auto'>
      <tr style='background-color:#CCC'><td colspan='2'><h4>School Info</h4></td></tr>
      <tr><th>School Name</th><td><?= $schoolname ?></td></tr> 
      <tr><th>School District</th><td><?= $district ?></td></tr> 
      <tr><th>Title I</th><td><?= $titlei ?></td></tr> 
      <tr><th>Percent free / reduced lunch</th><td><?= $freelunch ?></td></tr> 
      <tr><th>Percent English language learners</th><td><?= $englearners ?></td></tr> 
      <tr><th>Grades Taught</th><td><?= $gradestaught ?></td></tr> 
      <tr><th>Years Taught</th><td><?= $yearstaught ?></td></tr>                 
      <tr><th>Percent non-white students?</th><td><?= $nonwhite ?></td></tr>                 
    </table> 
    <div class='spacer'></div>              
  <?php } ?>
  
    <table style='width:550px;margin:auto'>
      <tr style='background-color:#CCC'><td colspan='2'><h4>Billing / Contact Info</h4></td></tr>
      <tr><th>Full Name</th><td><?= $name ?></td></tr> 
      <tr><th>Phone</th><td><?= $phone ?></td></tr> 
      <tr><th>Email</th><td><?= $email ?></td></tr> 
      <tr><th>Address</th><td><?= $address ?></td></tr> 
      <tr><th>City</th><td><?= $city ?></td></tr> 
      <tr><th>State</th><td><?= $state ?></td></tr> 
      <tr><th>Zip</th><td><?= $zip ?></td></tr>
      <tr><th>Country</th><td><?= $country ?></td></tr>                 
    </table> 
    <div class='spacer'></div>
    
    <table style='width:550px;margin:auto'>
      <tr style='background-color:#CCC'><td colspan='2'><h4>Shipping Info</h4></td></tr>
      <tr><th>Full Name</th><td><?= $shipname ?></td></tr> 
      <tr><th>Address</th><td><?= $shipaddress ?></td></tr> 
      <tr><th>City</th><td><?= $shipcity ?></td></tr> 
      <tr><th>State</th><td><?= $shipstate ?></td></tr> 
      <tr><th>Zip</th><td><?= $shipzip ?></td></tr>                 
      <tr><th>Country</th><td><?= $shipcountry ?></td></tr>                 
    </table> 
    <div class='spacer'></div>
    
    <table style='width:550px;margin:auto'>
      <tr style='background-color:#CCC'><td colspan='2'><h4>Sensor Quantities</h4></td></tr>
      <? if ( $page_name != 'qcn-form_sensor49.php') { ?>
      <tr><th>$5 Sensor</th><td><?= $qtySensor5 ?></td></tr> 
      <? } ?>
      <tr><th>$49 Sensor</th><td><?= $qtySensor49 ?></td></tr> 
      <? if ( $surcharge > 0) { ?>
	<tr><th>Surcharge</th><td>$<?= $surcharge ?></td></tr>
      <? } ?>
      <tr><th class="total" style="font-size:16px">Total Price</th><td class="total"  style="font-size:16px">$<?= (($qtySensor5 * 5) + ($qtySensor49 * 49)+$surcharge) ?>.00</td></tr> 
    </table> 
    <div class="spacer"></div>

    <? /* Split first name / last name for ecomm gateway POST  */ ?>    
    <? $name = explode(' ', $name); ?>
    
<?  // <form action="https://orderpagetest.ic3.com/hop/orderform.jsp" method="post" id="qcnpurchase"> ?>
	<form action="https://orderpage.ic3.com/hop/orderform.jsp" method="post" id="qcnpurchase" target="_top">
    <input type="hidden" name="billTo_firstName" value="<?= $name[0] ?>" />  
        <input type="hidden" name="billTo_lastName" value="<?= $name[1] ?>" />  
        <input type="hidden" name="billTo_phoneNumber" value="<?= $phone ?>" />  
        <input type="hidden" name="billTo_email" value="<?= $email ?>" />  
        <input type="hidden" name="billTo_street1" value="<?= $address ?>" />  
        <input type="hidden" name="billTo_city" value="<?= $city ?>" />  
        <input type="hidden" name="billTo_state" value="<?= $state ?>" />      
        <input type="hidden" name="billTo_postalCode" value="<?= $zip ?>" />  
        <input type="hidden" name="billTo_country" value="<?= $country ?>" />  
        <input type="hidden" name="shipTo_firstName" value="<?= $shipname[0] ?>" />  
        <input type="hidden" name="shipTo_lastName" value="<?= $shipname[1] ?>" />  
        <input type="hidden" name="shipTo_street1" value="<?= $shipaddress ?>" />  
        <input type="hidden" name="shipTo_city" value="<?= $shipcity ?>" />  
        <input type="hidden" name="shipTo_state" value="<?= $shipstate ?>" />      
        <input type="hidden" name="shipTo_postalCode" value="<?= $shipzip ?>" />  
        <input type="hidden" name="shipTo_country" value="<?= $shipcountry ?>" />  
   

      <? if (!isset($qtySensor49) && isset($qtySensor5)) : ?>
        <input type="hidden" name="item_5_productName" value="SENSOR5">
        <input type="hidden" name="item_5_unitPrice" value="5.00">        
        <input type="hidden" name="item_5_quantity" value="<?= $qtySensor5 ?>" />  
        <input type="hidden" name="orderNumber" value="<?= $token ?>" />

		<input type="hidden" name="merchantDefinedData1" value="SENSOR49_QTY_<?= $qtySensor5 ?>" id="merchantDefinedData2">
		<input type="hidden" name="merchantDefinedData2" value="<?= $token ?>" id="merchantDefinedData3">


      <? elseif (isset($qtySensor49) && !isset($qtySensor5)) : ?>
        <input type="hidden" name="item_49_productName" value="SENSOR49">
        <input type="hidden" name="item_49_unitPrice" value="49.00">        
        <input type="hidden" name="item_49_quantity" value="<?= $qtySensor49 ?>" />  
        <input type="hidden" name="orderNumber" value="<?= $token ?>" />  

		<input type="hidden" name="merchantDefinedData1" value="SENSOR49_QTY_<?= $qtySensor49 ?>" id="merchantDefinedData2">
		<input type="hidden" name="merchantDefinedData2" value="<?= $token ?>" id="merchantDefinedData3">
		
	  <? else : //Both sensor types were ordered ?>
		<input type="hidden" name="item_5_productName" value="SENSOR5">
		<input type="hidden" name="item_5_unitPrice" value="5.00">        
		<input type="hidden" name="item_5_quantity" value="<?= $qtySensor5 ?>" />  
		<input type="hidden" name="item_49_productName" value="SENSOR49">
		<input type="hidden" name="item_49_unitPrice" value="49.00">        
		<input type="hidden" name="item_49_quantity" value="<?= $qtySensor49 ?>" />  
		<input type="hidden" name="orderNumber" value="<?= $token ?>" />

		<input type="hidden" name="merchantDefinedData1" value="SENSOR5_QTY_<?= $qtySensor5 ?>" id="merchantDefinedData2">
		<input type="hidden" name="merchantDefinedData2" value="SENSOR49_QTY_<?= $qtySensor49 ?>" id="merchantDefinedData2">
		<input type="hidden" name="merchantDefinedData3" value="<?= $token ?>" id="merchantDefinedData3">

      <? endif; ?>

	<? include_once 'HOP.php'; ?>
	<? $totalAmount = (($qtySensor5 * 5) + ($qtySensor49 * 49) + $surcharge); ?>
	<? InsertSignature3($totalAmount,"usd", "sale"); ?>    

      <fieldset id="confirm" style="width:540px">
        <input type="button" value="&laquo; Edit Personal Information" class="button" id="edit-info" />
        <input type="submit" value="Submit your order &raquo;" class="button" />
      </fieldset>
    </form>
  
  </body>
  </html>  
<? endif; ?>
