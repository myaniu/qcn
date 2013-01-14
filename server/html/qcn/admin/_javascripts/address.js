// Set global variables
var qty5;
var qty49;

$(document).ready(function() {

  switchFields($('#domestic').val());
 
  // Focus cursor in first field
  $(':text:visible:enabled:first').focus();

  // Copy billing info into shipping info fields on click.
  $('#moreSensors').click(function () { 
    if ( !this.checked ) {
      $('#purchaseAdditionalSensors').toggle();
        qty49 = '0';
        qty5 = $('#SENSOR_5').val();
        $('#SENSOR_49').val(qty49);
        var total = (qty5 * 5) + (qty49 * 49);
        updatePrice(total);
    } else {
      $('#purchaseAdditionalSensors').toggle();
        qty49  = '0';
        qty5 = $('#SENSOR_5').val();
        var total = (qty5 * 5) + (qty49 * 49);
        updatePrice(total);
    }

  });

  // Domestic/International 
  $('#domestic').change(function(){
    switchFields($(this).val());
  });

  // Copy field values
  $('#sameAddress').click(function(){
    $('#shipname').val($('#billname').val());
    $('#shipaddress').val($('#billaddress').val());
    $('#shipcity').val($('#billcity').val());
    $('#shipstate').val($('#billstate').val());
    $('#shipzip').val($('#billzip').val());
    $('#shipcountry').val($('#billcountry').val());
    $('#shipstate2').val($('#billstate2').val());
    $('#shipzip2').val($('#billzip2').val());
    $('#shipcountry2').val($('#billcountry2').val());
  });

  // Update price when quantity changes
  $('#SENSOR_5').change(function(event){
    qty5  = $(this).val();
    qty49 = $('#SENSOR_49').val();
    var total = (qty5 * 5) + (qty49 * 49);
    updatePrice(total);
  });
  $('#SENSOR_49').keyup(function(event){
    var qty49  = $(this).val();
    var qty5 = $('#SENSOR_5').val();
    var total = (qty5 * 5) + (qty49 * 49);
    updatePrice(total);

  });
  $('#SENSOR49').keyup(function(event){
    calc49Price();
  });

  function switchFields(domestic) {
    if (domestic == 'domestic') {
        $('#intl_form').hide();
        $('#intl_form2').hide();
        $('#domestic_form').show();
        $('#domestic_form2').show();
        $('#surcharge').hide();
    } else {
        $('#domestic_form').hide();
        $('#domestic_form2').hide();
        $('#intl_form').show();
        $('#intl_form2').show();
        $('#surcharge').show();
    }
    calc49Price();
  }

  function calc49Price() {
    var qty49 = $('#SENSOR49').val();
    var total = (qty49 * 49);
    if ($('#domestic').val() == 'intl') {
	total += 10;
    }
    updatePrice(total);
  }

  function updatePrice(total) {
      $('#total').html('Total purchase price: $' + total );    
  }

});
