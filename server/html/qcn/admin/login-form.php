<?
  # Page header including doctype
  include 'page-header.php';
  
  # Display error message to unathorized users
  if ( isset($_GET['msg']) )
    $errormsg = "<p style='color:red;text-align:center'>You must login to access this section!</p>";

?>

<center><h2>QCN Store Admin tool</h2></center>
<form action="index.php" method="post" id="qcnpurchase">
    <fieldset id="login">
      <?= $errormsg ?>
      <label>Login</label>
      <input type="text" name="qcn-username" />
      <label>Password</label>
      <input type="password" name="qcn-password" />
      <div class="spacer"></div>
      <input type="submit" name="submit" class="button" value="Login" style="width: 120px;">
    </fieldset>
</form>
