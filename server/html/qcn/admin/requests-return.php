<?
# Open mysql connection and select database
# Include Cybersource lib
include_once 'database.php';
include_once 'HOP.php';

//  
// Verify sale based on posted decision
// ACCEPT, REJECT, ERROR, REVIEW
//

if (VerifyTransactionSignature($_POST)) {

	$sale_decision = mysql_real_escape_string($_POST['decision']);
	switch($sale_decision) {
		case 'ACCEPT' :
			#Retrieve and sanitize gateway silent POST data
			$token = addslashes($_POST['orderNumber']); 
			$requestId = addslashes($_POST['requestID']);

			# Update transactions_tbl were token exists
			$query = mysql_query("UPDATE transactions_tbl SET 
			         transactionid = '$requestId'
			         WHERE token = '$token'");    

			# Close DB connection
			mysql_close($link);
			
			$message = '<p>Thank you for purchasing a USB sensor and joining the Quake-Catcher Network.  We appreciate your effort and are happy to have you as a part of the seismic network.  By utilizing the low-cost sensors in and attached to internet-connected computers, the Quake-Catcher Network will provide better understanding of earthquakes and may ultimately be used to give early warnings to schools, emergency response systems, and others. 
						  You will soon receive a USB Kit that includes all the information you will need to install your new USB sensor and participate in the Quake-Catcher Network. QCN also provides free educational software designed to help teach about earthquakes and earthquake hazards.  We hope you find this free software useful.  
						  By joining the network you will be able to share information with others in the network, edit your profile, gain access to recent news, post to the message boards, learn about software updates, and more.
						  Once again, thank you for your participation.  The Quake-Catcher Network can only grow with the support of interested individuals like you.</p>';
			break;
		
			case 'REJECT' :
				$message = '<p>Your transaction has been rejected by the payment processor. The reason code is ' . $_POST['reasonCode'] . ' . Your request ID is ' .  $_POST['requestID'] . '. 
							Please contact your credit card company regarding this transaction.  If you have questions about your purchase, please contact: <a href="mailto:cbaroni@stanford.edu">Claudia Baroni</a>';
			break;
		
			case 'ERROR' :
				$message = '<p>An error has occurred in the payment system.  Please try your transaction again later.</p>';
			break;
		
			case 'REVIEW' :
				$message = '<p>Your transaction is waiting to be reviewed.  The reason code is ' . $_POST['reasonCode'] . ' . 
								Your request ID is ' . $_POST['requestID'] . ' . Please contact us to have your transaction reviewed: <a href="mailto:cbaroni@stanford.edu">Claudia Baroni</a></p>';
			break;		
			default:
				$message = '<p>Failed to verify transaction signature!  Please report the problem to: <a href="mailto:cbaroni@stanford.edu">Claudia Baroni</a>'; 	
	}
}

?>


<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>QCN : Thank You!</title>
<link type="text/css" rel="stylesheet" href="/admin/_stylesheets/forms.css" media="screen" />
</head>
<body>
  <h1>Learning Center: Request a Sensor </h1>
  
  <?= $message ?>
  
  <div id="indent">
    <p>Sincerely,<br />
    <strong>-The Quake Catcher Team</strong><br />
    (Jesse F. Lawrence,<br />
    Elizabeth S. Cochran,<br />
    Carl Christensen,<br />
    Jennifer Saltzman &amp; more)<br /></p>
  </div>


<br /><br />

</body>
</html>  
