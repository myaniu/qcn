<?
$xml = $_POST["xml"];

if ($xml) {
  echo "XML";
}
else {
   header("HTTP/1.0 400 Bad Request");
   echo "Invalid data";
}

?>
