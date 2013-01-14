<?php

# Simulate a form POST 
# for submission to Gateway after sql insertion

$url = parse_url($serverURL);
$requestString = "POST ".$url['path']." HTTP/1.1\r\nConnection: close\r\nHost: ".$url['host']."\r\nContent-type: application/x-www.form-urlencoded\r\nContent-length: ".strlen($requestData)."\r\n\r\n".$requestData;
$fp = fsockopen($url['host'], 80, $err_num, $err_msg, 5);

if ($fp) {
  fputs($fp, $requestString);
  $response='';

  while (!feof($fp)) {
    $response .= fread($fp, 1024);
  }

fclose($fp);
}


?>