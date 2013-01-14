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
<script src="_javascripts/validate.js" type="text/javascript"></script>
<link type="text/css" rel="stylesheet" href="_stylesheets/forms.css" media="screen" />
</head>
<body>

<div id="container">

  <form action="confirm.php" method="post" id="qcnpurchase">

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
        <h4>Billing Information:</h4><br />
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
          <option value="--">Select your state</option> <option value="--">Not Applicable</option> <option value="AL">AL</option> <option value="AK">AK</option> <option value="AZ">AZ</option>
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
        <label>Zip / Postal Code</label>
        <input type="text" name="ZIP" class="num" id="billzip" tabindex="13" />
      
        <div style="clear:both; position: relative; padding: 0px; overflow:auto">
        <label>Country</label>
        <select name="COUNTRY" id="billcountry" tabindex="13">
          <option selected="selected" value="USA">United States</option> </select>
          <p style="font-size:70%;"><em>*QCN currently only ships inside the US</em></p>
         </div>
      </fieldset>
  
  
  <!-- Shipping information form -->
  
  
      <fieldset>
        
        <h4 style="float:left">Shipping Information:</h4>
          <div id="copy-checkbox">        
          <input style="float:left" type="checkbox" class="box" id="sameAddress" name="sameAddress">
          <label class="for-checkbox" for="sameAddress">Same as Billing Address</label>
        </div>
        <div class="spacer"></div>
        
        
        <label>Full Name</label>
        <input type="text" name="NAMETOSHIP" id="shipname" />
      
        <label>Address</label>
        <input type="text" name="ADDRESSTOSHIP" id="shipaddress" />
      
        <label>City</label>
        <input type="text" name="CITYTOSHIP" id="shipcity" />
      
        <label>State</label>  
        <select name="STATETOSHIP" id="shipstate">
          <option value="--">Select your state</option> <option value="--">Not Applicable</option> <option value="AL">AL</option> <option value="AK">AK</option> <option value="AZ">AZ</option>
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
        <label>Zip / Postal Code</label>
        <input type="text" name="ZIPTOSHIP" class="num" id="shipzip" />
      
        <div style="clear:both; position: relative; padding: 0px; overflow:auto">
        <label>Country</label>
        <select name="COUNTRYTOSHIP" id="shipcountry">
          <option selected="selected" value="USA">United States</option> </select>
          <p style="font-size:70%;"><em>*QCN currently only ships inside the US</em></p>
         </div>
      </fieldset>

    <fieldset>
      <h4 style="float:left">Terms & Conditions:</h4>
      <p><textarea cols="57" rows="10" name="TANDC" align="justify">By clicking submit you agree to these terms:
The sensor is distributed as part of an experimental research project by the Quake Catcher Network, a project of Stanford University and the University of California at Riverside. Two computer programs that can be obtained separately for free are necessary for intended use of the sensor. A computer program called QCNLive is necessary to see the sensors measurements and a computer program called BOINC is necessary to upload data to QCN. In order to install and use these two free computer programs, you must consent to the privacy policies and terms of use governing each, which may change. 
The sensor is sold as is and with all faults. The sensor and the free software are experimental and may not function as expected for the only two intended purposes: education and submitting data to QCN. Without limiting the foregoing, this device will not provide warning of an earthquake nor will this device protect you from an earthquake in any way. The seller disclaims all warranties, including merchantability. Keep the sensor out of reach of small children and pets and do not use it for any purpose in which harm would result if the sensor and/or the software did not work as expected.</textarea></p>
      <p><input type="checkbox" name="accept_terms" checked="checked" disabled="disabled">I agree to QCN's Terms and Conditions.</p>
    </fieldset>

  <!-- Quantity information form -->  
      <fieldset>
        <div class="spacer"></div>
        <h4>Quantity:</h4><br />  	
               
        <label>Quantity</label>
        <input type="hidden" name="UNIQUE_ID" value="SENSOR_5"> 
        <input type="hidden" name="TYPE" value="S">
        <input type="hidden" name="AMT" value="5.00">      
                    
        <select name="QUANTITY" id="SENSOR_5"> 
          <option value="1">1</option>
          <option value="2">2</option>
          <option value="3">3</option>
        </select>
        <p style="font-size:70%;">&nbsp;@ $5.00 / sensor (3 Maximum)</p>
             
        <label></label>  
        <input type="checkbox" class="box" id="moreSensors" name="MORESENSORS">
        <label class="for-checkbox" for="moreSensors">Purchase additional sensors @ $49/sensor</label>
        <div class="spacer"></div>
        <div id="purchaseAdditionalSensors" class="quantity" style="display:none">
          <input type="hidden" name="UNIQUE_ID" value="SENSOR">
          <input type="hidden" name="TYPE" value="S">
          <input type="hidden" name="AMT" value="49.00">        
          <label>Quantity </label>
          <input type="text" name="QUANTITY49" class="num" id="SENSOR_49" value="0" />  
          <p style="font-size:70%;">&nbsp;@ $49/ sensor</p>
        </div>
        <div class="spacer"></div>
        <p id="total">Total purchase price: $5</p>

        <input type="hidden" name="TOKEN" value="<?= $token ?>">
        <input type="hidden" name="LOGIN" value="SUquake01Test">        

        
        <input type="button" id="submitForm" class="button" value="Submit Order" style="width: 120px;">
      </fieldset>
  
  </form>

</body>
</html>
