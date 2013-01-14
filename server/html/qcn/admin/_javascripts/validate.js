//Simple form validation

$(document).ready(function() {

  $('#submitForm').click(function() {
  var errors = 0;
  var intl = false;
    if ($('#domestic').val() == 'intl') {
	intl = true;
    } 
    $('input:text').each(function(i) {
        var inputname = $(this).attr("name");
        var inputval = $(this).val();
        if (intl) {
            if (inputname == 'ZIP' || inputname == 'ZIPTOSHIP') {
                inputval = 'x';
	    }
        } else {
	    if (inputname == 'ZIP2' || inputname == 'STATE2' || inputname == 'COUNTRY2' ||
                inputname == 'ZIPTOSHIP2' || inputname == 'STATETOSHIP2' || inputname == 'COUNTRYTOSHIP2') {
		inputval = 'x';
	    }
        }
        if ( inputval != '' ) { 
          $(this).removeClass("form-error"); 
        } 
        else { 
          $(this).addClass("form-error"); 
          errors++;
        }
    });
    if (!intl) {
      if ($('#billstate').val() == '') { errors++; }
      if ($('#shipstate').val() == '') { errors++; }
    }
    if ( errors > '0') { alert("There are "+errors+" errors in your submission. Please modify the highlighted field(s)."); }
    else {
      $('#qcnpurchase').submit();
      return false;        
    } 

  });

});
