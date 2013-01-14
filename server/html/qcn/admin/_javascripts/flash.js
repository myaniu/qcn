function hideflash() { $('.flash-alert').slideToggle(); }

$(document).ready(function(){
    //Check if a table exists on this page
    if ( $("table").length > 0 ) {
      setTimeout("hideflash()", 3000);
    }
});

