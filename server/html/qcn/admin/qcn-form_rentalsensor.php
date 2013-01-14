<? 
# Generate a random token for a transaction
include'token.php';   
$token = genToken();
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>QCN Purchase Form</title>
<script src="http://www.google.com/jsapi" type="text/javascript"></script>
<script type="text/javascript">
  google.load("jquery", "1.3.2");
</script>
<script src="_javascripts/address.js" type="text/javascript"></script>
<script src="_javascripts/validateRental.js" type="text/javascript"></script>
<link type="text/css" rel="stylesheet" href="_stylesheets/forms.css" media="screen" />
</head>
<body>

<div id="container">
  <h3>QCN Sensor Rental Application &amp; Order Form</h3>
  <form action="rentalthanks.php" method="post" id="qcnrental">
  
      <fieldset>
        <h4>School Information:</h4><br />
  
        <label>School Name</label>
        <input type="text" name="SCHOOLNAME" id="schoolname" tabindex="1" />
        
        <label>School District</label>
        <input type="text" name="DISTRICT" id="district" tabindex="2" />
  
        <label>Title I school</label> 	
        <input type="text" name="TITLEI" id="titlei" tabindex="3" />
              
        <label>Percent free /<br />reduced lunch</label>
        <select name="FREELUNCH" id="englearners"  tabindex="4"> 
          <option value="0">  0 %</option>
          <option value="5">  5 %</option>
          <option value="10"> 10 %</option>
          <option value="20"> 20 %</option>
          <option value="30"> 30 %</option>                
          <option value="40"> 40 %</option>
          <option value="50"> 50 %</option>
          <option value="60"> 60 %</option>                
          <option value="70"> 70 %</option>
          <option value="80"> 80 %</option>
          <option value="90"> 90 %</option>
          <option value="100">100 %</option>
        </select>        

        <div class="clearfix"></div>         
        <label>Percent English <br />language learners</label>
        <select name="ENGLEARNERS" id="englearners" tabindex="5"> 
          <option value="0">  0 %</option>
          <option value="5">  5 %</option>
          <option value="10"> 10 %</option>
          <option value="20"> 20 %</option>
          <option value="30"> 30 %</option>                
          <option value="40"> 40 %</option>
          <option value="50"> 50 %</option>
          <option value="60"> 60 %</option>                
          <option value="70"> 70 %</option>
          <option value="80"> 80 %</option>
          <option value="90"> 90 %</option>
          <option value="100">100 %</option>
        </select>
  
        <label>Grades Taught?</label>
        <input type="text" name="GRADESTAUGHT" id="gradestaught" tabindex="5" />
        <label>Number of <br />Years Taught?</label>
        <input type="text" name="YEARSTAUGHT" id="yearstaught" tabindex="7" />
        
        <label>Percent non-white <br />students?</label>
        <select name="NONWHITE" id="nonwhite" tabindex="7"> 
          <option value="5">  0 %</option>
          <option value="5">  5 %</option>
          <option value="10"> 10 %</option>
          <option value="20"> 20 %</option>
          <option value="30"> 30 %</option>                
          <option value="40"> 40 %</option>
          <option value="50"> 50 %</option>
          <option value="60"> 60 %</option>                
          <option value="70"> 70 %</option>
          <option value="80"> 80 %</option>
          <option value="90"> 90 %</option>
          <option value="100">100 %</option>
        </select>    
      
      </fieldset>
  
      <fieldset>
        <h4>Contact Information:</h4><br />
        <label>Full Name</label>
        <input type="text" name="NAME" id="billname" tabindex="8" />
      
        <label>Phone</label>
        <input type="text" name="PHONE" tabindex="9" />
      
        <label>Email</label>
        <input type="text" name="EMAIL" tabindex="10" />
      
        <label>Address</label>
        <input type="text" name="ADDRESS" id="billaddress" tabindex="10" />
      
        <label>City</label>
        <input type="text" name="CITY" id="billcity" tabindex="11" />
      
        <label>State</label>  
        <select name="STATE" id="billstate" tabindex="12">
          <option value="">Select your state</option>     <option value="AL">AL</option> <option value="AK">AK</option> <option value="AZ">AZ</option>
          <option value="AR">AR</option> <option value="CA">CA</option> <option value="CO">CO</option> <option value="CT">CT</option>
          <option value="DE">DE</option> <option value="DC">DC</option> <option value="FL">FL</option> <option value="GA">GA</option>
          <option value="HI">HI</option> <option value="ID">ID</option> <option value="IL">IL</option> <option value="IN">IN</option>
          <option value="IA">IA</option> <option value="KS">KS</option> <option value="KY">KY</option> <option value="LA">LA</option>
          <option value="ME">ME</option> <option value="MD">MD</option> <option value="MA">MA</option> <option value="MI">MI</option>
          <option value="MN">MN</option> <option value="MS">MS</option> <option value="MO">MO</option> <option value="MT">MT</option>
          <option value="NE">NE</option> <option value="NV">NV</option> <option value="NH">NH</option> <option value="NJ">NJ</option>
          <option value="NM">NM</option> <option value="NY">NY</option> <option value="NC">NC</option> <option value="ND">ND</option>
          <option value="OH">OH</option> <option value="OK">OK</option> <option value="OR">OR</option> <option value="PA">PA</option>
          <option value="RI">RI</option> <option value="SC">SC</option> <option value="SD">SD</option> <option value="TN">TN</option>
          <option value="TX">TX</option> <option value="UT">UT</option> <option value="VT">VT</option> <option value="VA">VA</option>
          <option value="WA">WA</option> <option value="WV">WV</option> <option value="WI">WI</option> <option value="WY">WY</option>
        </select>
      
        <div class="clearfix"></div>      
        <label>Zip</label>
        <input type="text" name="ZIP" class="num" id="billzip" tabindex="13" />
      
        <div style="clear:both; position: relative; padding: 0px; overflow:auto">
        <label>Country</label>
        <select name="COUNTRY" id="billcountry" tabindex="13">
          <option selected="selected" value="USA">United States</option>
        </select>
          <p style="font-size:70%;"><em>*QCN currently only ships inside the US</em></p>
         </div>
      </fieldset>
  
  
  <!-- Quantity information form -->  
      <fieldset>
        <h4>Quantity &amp; Length of Rental</h4>
        <label>Quantity</label>
                   
        <select name="QUANTITY" id="SENSOR_5"> 
          <option value="1">1</option>
          <option value="2">2</option>
          <option value="3">3</option>
          <option value="4">4</option>
          <option value="5">5</option>
          <option value="6">6</option>
          <option value="7">7</option>
          <option value="8">8</option>
          <option value="9">9</option>          
          <option value="10">10</option>
          <option value="11">11</option>
          <option value="12">12</option>
          <option value="13">13</option>
          <option value="14">14</option>          
          <option value="15">15</option>          
        </select>
        <p style="font-size:70%;">&nbsp; (15 Sensor maximum)</p>

        <label>Length of rental</label>
        <select name="RENTWEEKS" id="SENSOR_5"> 
          <option value="1">1 Week</option>
          <option value="2">2 Weeks</option>
          <option value="3">3 Weeks</option>
        </select>             
        
        <div class="clearfix"></div>
        <label>Starting on</label>
        <select name="RENTMONTH" id="rentmonth" class="date"> 
          <option value="01">Jan</option>
          <option value="02">Feb</option>
          <option value="03">March</option>
          <option value="04">April</option>
          <option value="05">May</option>
          <option value="06">June</option>
          <option value="07">July</option>
          <option value="08">August</option>
          <option value="09">September</option>          
          <option value="10">October</option>
          <option value="11">November</option>
          <option value="12">December</option>    
        </select>
        <select name="RENTDAY" id="rentday" class="date"> 
          <option value="01">1</option>           <option value="02">2</option>           <option value="03">3</option>
          <option value="04">4</option>           <option value="05">5</option>           <option value="06">6</option>
          <option value="07">7</option>           <option value="08">8</option>           <option value="09">9</option>          
          <option value="10">10</option>          <option value="11">11</option>          <option value="12">12</option>
          <option value="13">13</option>          <option value="14">14</option>          <option value="15">15</option>          
          <option value="16">16</option>          <option value="17">17</option>          <option value="18">18</option>
          <option value="19">19</option>          <option value="20">20</option>          <option value="21">21</option>
          <option value="22">22</option>          <option value="23">23</option>          <option value="24">24</option>          
          <option value="25">25</option>          <option value="26">26</option>          <option value="27">27</option>
          <option value="28">28</option>          <option value="29">29</option>          <option value="30">30</option>                    
          <option value="31">31</option>                    
        </select>         
        <select name="RENTYEAR" id="rentyear" class="date"> 
          <option selected="selected" value="<?= date('Y') ?>"><?= date('Y') ?></option>
          <option value="<?= date('Y')+1 ?>"><?= date('Y')+1 ?></option>
          <option value="<?= date('Y')+2 ?>"><?= date('Y')+2 ?></option>          
        </select>
      </fieldset>


    <fieldset style="border-color:red; border-style: outset">
        <h4>By borrowing QCN sensors, you agree to the following terms:</h4>       
        <ol style="font-size:12px">
          <li>We will send up to 15 sensors to you, at least one week before you need them at no cost to you.</li>
          <li>We will send the sensors in a reusable,  flat rate box via the US postal service.</li>
          <li>You will return the sensors, using the same flat rate box and the address label that we provide.  You will be responsible for the return postage of $10.35</li>
          <li>You will return the sensors after 3 weeks.</li>
          <li>You will be responsible for the cost of the sensors (at $49 each) if you do not return them in a timely fashion.</li>
        </ol>
      <p><textarea cols="57" rows="10" name="TANDC" align="justify">By clicking submit you agree to these terms:
The sensor is distributed as part of an experimental research project by the Quake Catcher Network, a project of Stanford University and the University of California at Riverside. Two computer programs that can be obtained separately for free are necessary for intended use of the sensor. A computer program called QCNLive is necessary to see the sensors measurements and a computer program called BOINC is necessary to upload data to QCN. In order to install and use these two free computer programs, you must consent to the privacy policies and terms of use governing each, which may change. 
The sensor is sold as is and with all faults. The sensor and the free software are experimental and may not function as expected for the only two intended purposes: education and submitting data to QCN. Without limiting the foregoing, this device will not provide warning of an earthquake nor will this device protect you from an earthquake in any way. The seller disclaims all warranties, including merchantability. Keep the sensor out of reach of small children and pets and do not use it for any purpose in which harm would result if the sensor and/or the software did not work as expected.</textarea></p>
         <input type="hidden" name="TOKEN" value="<?= $token ?>">
          <div class="spacer"></div>
          <input class="box" type="checkbox" id="terms" name="TERMS">
          <label class="for-checkbox" for="terms"><strong>&nbsp; Yes, I have read and agree to the terms of borrowing the QCN sensors</strong></label>
        
        <div class="spacer"></div>
        <input type="button" id="submitForm" class="button" value="Submit Application" style="width: 120px;">
    </fieldset>        
  
  </form>
  <div class="spacer"></div>
</body>
</html>
