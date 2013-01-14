<html><body>
<center><h2>QCN Store Admin tool</h2></center>
<form action="hash.php" method="post" id="hash">
    <fieldset id="login">
      <?= $errormsg ?>
      <label>Password</label>
      <input type="password" name="qcn-password" />
      <div class="spacer"></div>
      <input type="submit" name="submit" class="button" value="Get Hash" style="width: 120px;">
    </fieldset>
</form>

<?
    $password = $_POST['qcn-password'];
    $password = stripslashes($password);
    $password = mysql_real_escape_string($password);


    if (strlen($password) < 3 ) {
      echo "Password empty or too short! <BR><BR>";
    }
    else {
      //Prevent SQL Injection
      $password = sha1($password);
      echo "Hashed Password = " . $password . "<BR><BR>";
    }
    echo "</body></html>";
?>
