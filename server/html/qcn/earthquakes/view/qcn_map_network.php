<?php
require_once("/var/www/qcn/inc/utils.inc");

//page_top("QCN Regional Arrays");
echo "<html><head><title=\"QCN Regional Maps\"></head><body bgcolor=\"#ffffff\">\n";

echo "<script src=\"http://maps.google.com/maps/api/js?sensor=false\" type=\"text/javascript\"></script>\n";
    $lon  = $_REQUEST["lon"];
    $lat  = $_REQUEST["lat"];
    $ifile = $_REQUEST["ifile"];

echo "  <div id=\"map\" style=\"width: 700px; height: 750px; float: left;\"></div>\n";
    echo "  <script type=\"text/javascript\">\n";
    icon_setup();
    echo "\n    var locations = [\n";
    $contents = file($ifile);
    foreach ($contents as $line_num => $line) {
     list($slon,$slat,$message,$mag,$type1) = split('[;]',$line);
     if (trim($type1,"\n")=="Lenovo Thinkpad (Windows)" || trim($type1,"\n")=="Mac Intel") {
      $type2=2;
     } else if (trim($type1,"\n")=="QCN") {
      $type2=3;
     } else if (trim($type1,"\n")=="USGS") {
      $type2=4;
     } else {
      $type2=1;
     }
     echo "   ['".trim($message,"\n")."' , ".$slat.", ".$slon.", ".$type2.", ".$mag."],\n";
    }
    //unlink($ifile);
    echo "];\n";
    echo " var map = new google.maps.Map(document.getElementById('map'), {
      zoom: 3,
      center: new google.maps.LatLng(35,-95),
      mapTypeId: google.maps.MapTypeId.ROADMAP
    });

    var infowindow = new google.maps.InfoWindow(
        {disableAutoPan: true}
    );";


    echo"
    var marker, i;

    for (i = 0; i < locations.length; i++) {  
     var newsize1 = Math.round(locations[i][4]*3+10);
     var newsize2 = Math.round(newsize1 / 2.);
     if (locations[i][3]==1) 
      {
      marker = new google.maps.Marker({
        position: new google.maps.LatLng(locations[i][1], locations[i][2]),
        map: map,
         icon: new google.maps.MarkerImage(\"http://qcn.stanford.edu/sensor/img/\" + \"qcn_32_usb.png\",new google.maps.Size(32,32),new google.maps.Point(0,0), new google.maps.Point(newsize2,newsize2),new google.maps.Size(newsize1,newsize1)),
      });
        } 
         else if (locations[i][3]==3) 
        {
      marker = new google.maps.Marker({
        position: new google.maps.LatLng(locations[i][1], locations[i][2]),
        map: map,
         icon: new google.maps.MarkerImage(\"http://qcn.stanford.edu/sensor/img/\" + \"qcn_32_quake.png\",new google.maps.Size(32,32),new google.maps.Point(0,0), new google.maps.Point(newsize2,newsize2),new google.maps.Size(newsize1,newsize1)),
      });

        } 
         else if (locations[i][3]==4) 
        {
      marker = new google.maps.Marker({
        position: new google.maps.LatLng(locations[i][1], locations[i][2]),
        map: map,
         icon: new google.maps.MarkerImage(\"http://qcn.stanford.edu/sensor/img/\" + \"qcn_32_quake_yellow.png\",new google.maps.Size(32,32),new google.maps.Point(0,0), new google.maps.Point(newsize2,newsize2),new google.maps.Size(newsize1,newsize1)),
      });

        } 
        else 
        {
      marker = new google.maps.Marker({
        position: new google.maps.LatLng(locations[i][1], locations[i][2]),
        map: map,
         icon: new google.maps.MarkerImage(\"http://qcn.stanford.edu/sensor/img/\" + \"qcn_32_laptop.png\",new google.maps.Size(32,32),new google.maps.Point(0,0), new google.maps.Point(newsize2,newsize2),new google.maps.Size(newsize1,newsize1)),
      });

        }

      google.maps.event.addListener(marker, 'click', (function(marker, i) {
        return function() {
          infowindow.setContent(locations[i][0]);
          infowindow.open(map, marker);
          infowindow.maxWidth(450);
        }
      })(marker, i));
    }
  </script>\n";
    

echo "</body></html>";


//page_end();


function icon_setup(){
   echo "
       var image = new google.maps.MarkerImage('mapIcons/marker_red.png',
          // This marker is 20 pixels wide by 32 pixels tall.
          new google.maps.Size(20, 34),
          // The origin for this image is 0,0.
          new google.maps.Point(0,0),
          // The anchor for this image is the base of the flagpole at 0,32.
          new google.maps.Point(9, 34)
       );
       var shadow = new google.maps.MarkerImage(
          'http://www.google.com/mapfiles/shadow50.png',
          // The shadow image is larger in the horizontal dimension
          // while the position and offset are the same as for the main image.
          new google.maps.Size(37, 34),
          new google.maps.Point(0,0),
          new google.maps.Point(9, 34)
       );

       // Shapes define the clickable region of the icon.
       // The type defines an HTML &lt;area&gt; element 'poly' which
       // traces out a polygon as a series of X,Y points. The final
       // coordinate closes the poly by connecting to the first
       // coordinate.

       var shape = {
          coord: [9,0,6,1,4,2,2,4,0,8,0,12,1,14,2,16,5,19,7,23,8,26,9,30,9,34,11,34,11,30,12,26,13,24,14,21,16,18,18,16,20,12,20,8,18,4,16,2,15,1,13,0],
          type: 'poly'
       };";

}




?>




