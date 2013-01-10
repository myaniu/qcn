<?php

/**
* Phoogle Maps 2.0 | Uses Google Maps API to create customizable maps
* that can be embedded on your website
*    Copyright (C) 2005  Justin Johnson
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*
*
* Phoogle Maps 2.0
* Uses Google Maps Mapping API to create customizable
* Google Maps that can be embedded on your website
*
* @class        Phoogle Maps 2.0
* @author       Justin Johnson <justinjohnson@system7designs.com>
* @copyright    2005 system7designs
*/

// plate boundary data
require_once("../project/project.inc");
require_once("../project/plate_boundary.inc");

class PhoogleMap{
/**
* validPoints : array
* Holds addresses and HTML Messages for points that are valid (ie: have longitude and latitutde)
*/
    var $validPoints = array();
/**
* invalidPoints : array
* Holds addresses and HTML Messages for points that are invalid (ie: don't have longitude and latitutde)
*/
    var $invalidPoints = array();
/**
* mapWidth
* width of the Google Map, in pixels
*/
    var $mapWidth = 300;
/**
* mapHeight
* height of the Google Map, in pixels
*/
    var $mapHeight = 300;

/**
* apiKey
* Google API Key
*/
    var $apiKey = GOOGLE_MAPS_KEY;

/**
* showControl
* True/False whether to show map controls or not
*/
    var $showControl = true;
	
/**
* showType
* True/False whether to show map type controls or not
*/
    var $showType = true;
/**
* controlType
* string: can be 'small' or 'large'
* display's either the large or the small controls on the map, small by default
*/

    var $controlType = 'large';
    
/**
* zoomIn
* int: 0-17
* set's the initial zoom level of the map
*/

    var $zoomIn = 4;




/**
* @function     addGeoPoint
* @description  Add's an address to be displayed on the Google Map using latitude/longitude
*               early version of this function, considered experimental
*/

function addGeoPoint($lat,$long,$infoHTML,$is_quake,$is_usb){  // CMC modified for is_quake and is_usb info
    $pointer = count($this->validPoints);
        $this->validPoints[$pointer]['lat'] = $lat;
        $this->validPoints[$pointer]['long'] = $long;
        $this->validPoints[$pointer]['htmlMessage'] = $infoHTML;
        $this->validPoints[$pointer]['is_quake'] = $is_quake;
        $this->validPoints[$pointer]['is_usb'] = $is_usb;
    }
    
/**
* @function     centerMap
* @description  center's Google Map on a specific point
*               (thus eliminating the need for two different show methods from version 1.0)
*/

function centerMap($lat,$long){ 
    $this->centerMap = "map.setCenter(" 
       . "new google.maps.LatLng(" . $lat . "," . $long . "));\n          map.setZoom($this->zoomIn);\n"; 
}
  
/* 
function drawPlateBoundary() { // CMC - draws the plate boundaries in purple on google maps
  $global g_aPlateBoundary;  // our global array from 

  $platecnt = 0;
  for ($i = 0 ; $i < sizeof($g_aPlateBoundary; $i+=2)  {
     echo "var polyline[" . $platecnt . "]  = new GPolyline([\n";
     if ($i = 0 || $g_aPlateBoundary[$i] == 0 && $g_aPlateBoundary[$i+1] == 0) { // bump up our plate counter and end this polyline
         if ($i > 0) {  // it's really the end
             $platecnt++;
             echo "], \"#ff00ff\", 2);
                  map.addOverlay(polyline);
             "; 
         }
         if ($i != sizeof($g_aPlateBoundary) - 2) { // if we're not at the end of the array, which would be two less than size of array
         echo "var polyline[" . $platecnt . "]  = new GPolyline([\n";
     } 
  new google.maps.LatLng(37.4419, -122.1419),
  new google.maps.LatLng(37.4519, -122.1519)
  }
} 
*/
    
/**
* @function     addAddress
* @param        $address:string
* @returns      Boolean True:False (True if address has long/lat, false if it doesn't)
* @description  Add's an address to be displayed on the Google Map
*               (thus eliminating the need for two different show methods from version 1.0)
*/
	function addAddress($address,$htmlMessage=null){
	 if (!is_string($address)){
		die("All Addresses must be passed as a string");
	  }
           $apiURL = "\"//maps.googleapis.com/maps/api/js?key=" . GOOGLE_MAPS_KEY
 . "&sensor=false&v=3\"";
		$addressData = file_get_contents($apiURL.urlencode($address));
		
		$results = $this->xml2array(utf8_encode($addressData));
		if (empty($results['kml'][Response]['Placemark']['Point']['coordinates'])){
			$pointer = count($this->invalidPoints);
			$this->invalidPoints[$pointer]['lat']= $results['kml'][Response]['Placemark']['Point']['coordinates'][0];
			$this->invalidPoints[$pointer]['long']= $results['kml'][Response]['Placemark']['Point']['coordinates'][1];
			$this->invalidPoints[$pointer]['passedAddress'] = $address;
			$this->invalidPoints[$pointer]['htmlMessage'] = $htmlMessage;
		  }else{
			$pointer = count($this->validPoints);
			$this->validPoints[$pointer]['lat']= $results['kml'][Response]['Placemark']['Point']['coordinates'];
			$this->validPoints[$pointer]['long']= $results['kml'][Response]['Placemark']['Point']['coordinates'];
			$this->validPoints[$pointer]['passedAddress'] = $address;
			$this->validPoints[$pointer]['htmlMessage'] = $htmlMessage;
		}
	
	
	}
/**
* @function     showValidPoints
* @param        $displayType:string
* @param        $css_id:string
* @returns      nothing
* @description  Displays either a table or a list of the address points that are valid.
*               Mainly used for debugging but could be useful for showing a list of addresses
*               on the map
*/
	function showValidPoints($displayType,$css_id){
    $total = count($this->validPoints);
      if ($displayType == "table"){
        echo "<table id=\"".$css_id."\">\n<tr>\n\t<td>Address</td>\n</tr>\n";
        for ($t=0; $t<$total; $t++){
            echo"<tr>\n\t<td>".$this->validPoints[$t]['passedAddress']."</td>\n</tr>\n";
        }
        echo "</table>\n";
        }
      if ($displayType == "list"){
        echo "<ul id=\"".$css_id."\">\n";
      for ($lst=0; $lst<$total; $lst++){
        echo "<li>".$this->validPoints[$lst]['passedAddress']."</li>\n";
        }
        echo "</ul>\n";
       }
	}
/**
* @function     showInvalidPoints
* @param        $displayType:string
* @param        $css_id:string
* @returns      nothing
* @description  Displays either a table or a list of the address points that are invalid.
*               Mainly used for debugging shows only the points that are NOT on the map
*/
	function showInvalidPoints($displayType,$css_id){
      $total = count($this->invalidPoints);
      if ($displayType == "table"){
        echo "<table id=\"".$css_id."\">\n<tr>\n\t<td>Address</td>\n</tr>\n";
        for ($t=0; $t<$total; $t++){
            echo"<tr>\n\t<td>".$this->invalidPoints[$t]['passedAddress']."</td>\n</tr>\n";
        }
        echo "</table>\n";
        }
      if ($displayType == "list"){
        echo "<ul id=\"".$css_id."\">\n";
      for ($lst=0; $lst<$total; $lst++){
        echo "<li>".$this->invalidPoints[$lst]['passedAddress']."</li>\n";
        }
        echo "</ul>\n";
       }
	}
/**
* @function     setWidth
* @param        $width:int
* @returns      nothing
* @description  Sets the width of the map to be displayed
*/
	function setWidth($width){
		$this->mapWidth = $width;
	}
/**
* @function     setHeight
* @param        $height:int
* @returns      nothing
* @description  Sets the height of the map to be displayed
*/
	function setHeight($height){
		$this->mapHeight = $height;
	}
/**
* @function     setAPIkey
* @param        $key:string
* @returns      nothing
* @description  Stores the API Key acquired from Google
*/
	function setAPIkey($key){
		$this->apiKey = $key;
	}
/**
* @function     printGoogleJS
* @returns      nothing
* @description  Adds the necessary Javascript for the Google Map to function
*               should be called in between the html <head></head> tags
*/
	function printGoogleJS(){
             printGoogleMapsHeader();
	}
/**
* @function     showMap
* @description  Displays the Google Map on the page
*/
	function showMap(){

            if ($this->mapWidth == 1 || $this->mapHeight == 1) { //set defaults and autosize
                $this->mapWidth = 800;
                $this->mapHeight = 600;
		echo "\n<div id=\"map_canvas\" style=\"width:1000px; height:800px\">\n</div>\n";
		echo "    <script type=\"text/javascript\">
    //<![CDATA[
    var winwidth = 800;
    var winheight = 600;
    var gIW = null;

    //Opera Netscape 6 Netscape 4x Mozilla 
    if (window.innerWidth || window.innerHeight){ 
      winwidth = window.innerWidth; 
      winheight = window.innerHeight; 
    } 
    //IE Mozilla 
    if (document.body.clientWidth || document.body.clientHeight){ 
       winwidth = document.body.clientWidth; 
       winheight = document.body.clientHeight; 
    }
    if (self.innerWidth || self.innerHeight) {
       winwidth = self.innerWidth;
       winheight = self.innerHeight;
    }

  function infoWindowHandler(myIW, myMap, myMarker) {
    if (gIW) gIW.close();
    gIW = myIW;
    gIW.open(myMap, myMarker);
  }

    function showmap(){
     		   var map = new google.maps.Map(
                      document.getElementById('map_canvas'), 
                      { size:new google.maps.Size(winwidth-100, winheight-200),
                             mapTypeId: google.maps.MapTypeId.ROADMAP
                      }
                    );
                 ";
           }
            else { // we've sent in explicit sizes
		echo "<div id=\"map_canvas\" style=\"width: ".$this->mapWidth."px; height: ".$this->mapHeight."px\">\n</div>
		    <script type=\"text/javascript\">
                    //<![CDATA[
                    function showmap(){
      		        var map = new google.maps.Map(document.getElementById('map_canvas'),
                                   {         mapTypeId: google.maps.MapTypeId.ROADMAP
                                   }
                              );
                      ";
                 }

      		if (empty($this->centerMap)){
                   echo "map.setCenter(
                      new google.maps.LatLng(" . $this->validPoints[0]['lat'] 
                       . ", " . $this->validPoints[0]['long'] . "));\n";
                   echo "          map.setZoom($this->zoomIn);\n";
                }else{
                    echo $this->centerMap;
                }   

                   // CMC modified to use  station or quake icons
		    echo "
                   	var iconQuake = \"img/qcn_32_quake.png\";
                   	var iconLaptop = \"img/qcn_32_laptop.png\";
                   	var iconUSB = \"img/qcn_32_usb.png\";
                      ";

    /*
		if ($this->showControl){
          if ($this->controlType == 'small'){echo "map.addControl(new google.maps.SmallMapControl());\n";}
          if ($this->controlType == 'large'){echo "map.addControl(new google.maps.LargeMapControl3D());\n";}
          echo "map.addControl(new google.maps.ScaleControl());\n";
			}
		if ($this->showType){
		echo "map.addControl(new google.maps.MapTypeControl());\n";
		} 
      */

    $numPoints = count($this->validPoints);
    for ($g = 0; $g<$numPoints; $g++){
        echo "var ltlg".$g." = new google.maps.LatLng(" 
           . $this->validPoints[$g]['lat'].",".$this->validPoints[$g]['long'].");\n";
       
          // CMC here pick the right icon
           if ($this->validPoints[$g]['is_quake']) {
              echo "var marker".$g." = new google.maps.Marker({ 
                    position: ltlg".$g.", 
                    map: map,
                    icon: iconQuake 
                  });\n";
           }
           else {
              if ($this->validPoints[$g]['is_usb']) {
              echo "var marker".$g." = new google.maps.Marker({ 
                    position: ltlg".$g.", 
                    map: map,
                    icon: iconUSB
                  });\n";
              }
              else {
                echo "var marker".$g." = new google.maps.Marker({ 
                    position: ltlg".$g.", 
                    map: map,
                    icon: iconLaptop
                  });\n";
              }
           }

              if ($this->validPoints[$g]['htmlMessage']!=null) {
                 echo "    var iw$g= new google.maps.InfoWindow({
                              content:  \"" . $this->validPoints[$g]['htmlMessage'] . "\"
                           });
                           google.maps.event.addListener(marker$g, \"click\", function() { infoWindowHandler(iw$g, map, marker$g); });\n";
              }else{
                 echo "    var iw$g= new google.maps.InfoWindow({
                              content:  \"" . $this->validPoints[$g]['passedAddress']  . "\"
                           });
                           google.maps.event.addListener(marker$g, \"click\", function() { infoWindowHandler(iw$g, map, marker$g); });\n";
              }
	}

		echo "    
     }

               google.maps.event.addDomListener(window, 'load', showmap);
            //]]>
    	</script>\n";
		}
 ///////////THIS BLOCK OF CODE IS FROM Roger Veciana's CLASS (assoc_array2xml) OBTAINED FROM PHPCLASSES.ORG//////////////
   	function xml2array($xml){
		$this->depth=-1;
		$this->xml_parser = xml_parser_create();
		xml_set_object($this->xml_parser, $this);
		xml_parser_set_option ($this->xml_parser,XML_OPTION_CASE_FOLDING,0);//Don't put tags uppercase
		xml_set_element_handler($this->xml_parser, "startElement", "endElement");
		xml_set_character_data_handler($this->xml_parser,"characterData");
		xml_parse($this->xml_parser,$xml,true);
		xml_parser_free($this->xml_parser);
		return $this->arrays[0];

    }
    function startElement($parser, $name, $attrs){
		   $this->keys[]=$name; 
		   $this->node_flag=1;
		   $this->depth++;
     }
    function characterData($parser,$data){
       $key=end($this->keys);
       $this->arrays[$this->depth][$key]=$data;
       $this->node_flag=0; 
     }
    function endElement($parser, $name)
     {
       $key=array_pop($this->keys);
       if($this->node_flag==1){
         $this->arrays[$this->depth][$key]=$this->arrays[$this->depth+1];
         unset($this->arrays[$this->depth+1]);
       }
       $this->node_flag=1;
       $this->depth--;
     }
//////////////////END CODE FROM Roger Veciana's CLASS (assoc_array2xml) /////////////////////////////////


}//End Of Class


?>
