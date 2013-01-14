<?php

  # Open mysql connection and select database
  include_once 'database.php';
  # Get referring page name
  $page_name = $_SERVER['HTTP_REFERER'];
  $slash_idx = strrpos($page_name, "/");
  $page_name = substr($page_name, $slash_idx + 1);
  
  if ($page_name != 'qcn-form_rentalsensor.php' ) { print '<h2>There was an error with your submission'; }
  else {
    //Collect and Sanitize POST data
    $name = mysql_real_escape_string($_POST['NAME']);
    $phone = mysql_real_escape_string($_POST['PHONE']);
    $email = mysql_real_escape_string($_POST['EMAIL']);
    $address = mysql_real_escape_string($_POST['ADDRESS']);
    $city = mysql_real_escape_string($_POST['CITY']);
    $state = mysql_real_escape_string($_POST['STATE']);
    $zip = mysql_real_escape_string($_POST['ZIP']);
    $country = mysql_real_escape_string($_POST['COUNTRY']);
    $quantity = mysql_real_escape_string($_POST['QUANTITY']);    
    $rentweeks = mysql_real_escape_string($_POST['RENTWEEKS']);    
    $startyear = mysql_real_escape_string($_POST['RENTYEAR']);    
    $startmonth = mysql_real_escape_string($_POST['RENTMONTH']);    
    $startday = mysql_real_escape_string($_POST['RENTDAY']);    
    $terms = mysql_real_escape_string($_POST['TERMS']);     
        
    #Set rental dates
    $requestdate = date('Y-m-d');
    $startdate = $startyear . "-" . $startmonth . "-" . $startday;
    $enddate = date('Y-m-d', mktime(0, 0, 0, $startmonth, $startday + ( 7 * $rentweeks), $startyear));

    # School information fields
    $token = mysql_real_escape_string($_POST['TOKEN']);
    $schoolname = mysql_real_escape_string($_POST['SCHOOLNAME']);
    $district = mysql_real_escape_string($_POST['DISTRICT']);
    $titlei = mysql_real_escape_string($_POST['TITLEI']);
    $freelunch = mysql_real_escape_string($_POST['FREELUNCH']);
    $englearners = mysql_real_escape_string($_POST['ENGLEARNERS']);  
    $gradestaught = mysql_real_escape_string($_POST['GRADESTAUGHT']);  
    $yearstaught = mysql_real_escape_string($_POST['YEARSTAUGHT']);  
    $nonwhite = mysql_real_escape_string($_POST['NONWHITE']);  

    # Set Transaction Status information
    $approved = 'no';
    $shipstatus = 'no';
    $lastupdatedby = '';
    $lastupdatedon = '';
    
  
    # Check to see if token already exists in rentals_tbl
    # Existing token means user has updated form fields
    
    $tokenResult = mysql_query("SELECT * FROM rentals_tbl WHERE token='$token'");  
    $tokenCount = mysql_num_rows($tokenResult);
  
      if( $tokenCount < 1 ) {
      
        # INSERT OR UPDATE gathered data into table 

        $query = mysql_query("INSERT INTO rentals_tbl VALUES (
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
                '".$requestdate."',          
                '".$startdate."',                          
                '".$enddate."',                          
                '".$name."', 
                '".$phone."', 
                '".$email."', 
                '".$address."', 
                '".$city."', 
                '".$state."', 
                '".$zip."', 
                '".$country."', 
                '".$quantity."', 
                '".$approved."',                
                '".$shipstatus."',
                '".$lastupdatedby."',
                '".$lastupdatedon."'
                )");



        $last_insert = mysql_insert_id();
    } else {
        # Since a token already exists UPDATE the existing entry
  
        $query = mysql_query("UPDATE rentals_tbl SET 
                  schoolname = '$schoolname',
                  district = '$district',
                  titlei = '$titlei',
                  freelunch = '$freelunch',
                  englearners = '$englearners',
                  gradestaught = '$gradestaught',
                  yearstaught = '$yearstaught',
                  nonwhite = '$nonwhite',                    
                  requestdate = '$requestdate',
                  startdate = '$startdate',                  
                  enddate = '$enddate',                                    
                  name = '$name', 
                  phone = '$phone', 
                  email = '$email', 
                  address = '$address', 
                  city = '$city', 
                  state = '$state', 
                  zip = '$zip', 
                  country = '$country', 
                  quantity = '$quantity', 
                  WHERE token = '$token'");    
      } 
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
    <br />
    <fieldset>
      <h4>Request sucessfully submitted!</h4>
      <p>Thank you for your request to borrow a classroom set of sensors. </p>
      <p>Regards,<br />QCN Team</p>
    </fieldset>
    

  <?php if ( $schoolname != '' ) { ?>    
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
  <?php //} ?>
  
    <table style='width:550px;margin:auto'>
      <tr style='background-color:#CCC'><td colspan='2'><h4>Contact Info</h4></td></tr>
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
      <tr style='background-color:#CCC'><td colspan='2'><h4>Quantity</h4></td></tr>
      <tr><th>Quantity &amp; Date range</th><td><?= $quantity ?></td></tr> 
      <tr><th>Rental Start date</th><td><?= $startdate ?></td></tr>       
      <tr><th>Rental End date</th><td><?= $enddate ?></td></tr>       
    </table> 
    <div class="spacer"></div>
    
<?php }   ?>


  </body>
  </html>  
