<?php


function show_main_notice() {

if ($message=="ramp") {
echo "<h3><blink><a href=\"http://qcn.stanford.edu/RAMP/\">A QCN Rapid Aftershock Mobilization Program (RAMP) has been activated for ____.  If you wish to learn more or contribute to our efforts, please click here</a></blink><h3>\n";
}


}



function show_sensor_notice() {

if ($message=="out") {
echo "<p><h2><blink>We are currently out of sensors.  Please return in the near future to request a sensor.</blink></h2></p>\n";
} else if ($message=="low") {
echo
echo "<p><h2><blink>We are currently low on sensors.  Please return in the near future to request a sensor unless you need it immediately.</blink></h2></p>\n";
}


?>
