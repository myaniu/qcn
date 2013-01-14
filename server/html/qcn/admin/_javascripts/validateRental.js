//Simple form validation

$(document).ready(function() {

  $('#submitForm').click(function() {
  var errors = 0;
  
    $('input:text').each(function(i) {
        var inputval = $(this).val();
        
        if ( inputval != '' ) { 
          $(this).removeClass("form-error"); 
        } 
        else { 
          $(this).addClass("form-error"); 
          errors++;
        }
    });
      if ($('#billstate').val() == '') { errors++; }
      if ($('#shipstate').val() == '') { errors++; }

    if ( errors > '0') { alert("There are "+errors+" errors in your submission. Please modify the highlighted field(s)."); }
    else {
          
          if ($('#terms').is(':checked') ) {
                $('#qcnrental').submit();
                return false;                            
          } else { alert("You must agree to the terms of the rental"); }

    }
  });

});