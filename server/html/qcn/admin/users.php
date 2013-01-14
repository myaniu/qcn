<?php

  # Authenticate session
  include_once 'session.php';
  
  # Open database connection
  include_once 'database.php';  

  # Check if a post is being made 
  # and perform database update
  
  if ( isset($_GET['uid']) ) {
    $username = mysql_real_escape_string($_GET['uid']);
    
    # Redirect user to transactions page if uid does not equal the session data

    if ( $_SESSION['username'] != $username) { header("location:purchases.php?msg=noauth"); }
    else {
        //Select User from Database
        $result = mysql_query("SELECT * FROM qcnusers_tbl WHERE username = '$username'");
        $row = mysql_fetch_array($result);
        $password = $row['password'];
  
        $count = mysql_num_rows($result); 
        
        if( $count < 1 ) { header("location:purchases.php?msg=noauth"); }
      
      }
      
    } else {
        if ( isset($_POST['updatepwd']) ) {

        # Update the users password

            $username = mysql_real_escape_string($_POST['username']);
            $newpassword = mysql_real_escape_string($_POST['password']);
            $vnewpassword = mysql_real_escape_string($_POST['vpassword']);
                                    
            if ($newpassword != $vnewpassword) { 
              $flashalert = '<h1 class="flash-alert">Your passwords do not match. Please try again.</h1>';  
              //Select User from Database
              $result = mysql_query("SELECT * FROM qcnusers_tbl WHERE username = '$username'");
              $row = mysql_fetch_array($result);
              $password = $row['password'];              
            } else {
            
              #encrypt password
              $newpassword = sha1($newpassword);
              mysql_query("UPDATE qcnusers_tbl SET password = '$newpassword' WHERE username = '$username'");
              $flashalert = '<h1 class="flash-alert">Your password has been updated.</h1>';                
            
            }

        
        } else { header("location:purchases.php?msg=noauth"); }
        
    }


  # Page header including doctype and session data
  # page-header was included after $flashaert was set, so that when a POST occurs
  # an appropriate message will be displayed to user.
  
  include_once 'page-header.php';  
  
  # Print page heading
  print '<div class="spacer"></div>';
  print '<h1>Update password for '.$username.'</h1><br />';
  
  $page = "<form action='users.php' method='post'>
              <fieldset>
                  <table>
                  <tr style='font-weight:bold;background:#EEE'>
                    <td colspan='2'>Password</td>
                  </tr>
                  <tr>
                    <td>New Password</td>
                    <td>
                    <input type='hidden' name='username' value='$username' />
                    <input type='password' name='password' value='' />
                    </td></tr><tr>
                    <td>New Password Again</td>
                    
                    <td><input type='password' name='vpassword' value='' />                    
                    </td>
                  </tr>
                </table>
                <input type='submit' value='Update Password' name='updatepwd' class='button'>
              </fieldset>
            </form>";

  # Print page results
  echo $page;
  # Close db connection
  mysql_close($link);
  
?>



</div>

</body>
</html>