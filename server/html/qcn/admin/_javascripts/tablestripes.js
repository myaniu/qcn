$(function() { 
  $("table tr:nth-child(odd)").addClass("striped");  
});


$(document).ready(function(){
  var prevbg;
  
  var trOver = $("tr").mouseover(function(){
    prevbg = $(this).css('background-color');
    $(this).css("background-color","#BBB");     

  });

  var trOut = $("tr").mouseout(function(){
    $(this).css("background-color", prevbg);    
  });    

//    var trClick = $("tr").mouseout(function(){

});
