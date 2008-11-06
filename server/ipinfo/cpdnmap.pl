#!/usr/bin/perl
# lookup IP addresses

my $DIR = "/home/boinc";

use strict;

# image libraries need explicit includes or env vars
use lib qw(/usr/local/lib/perl5/site_perl/5.8.0/i686-linux/auto/);

use DBI;
use LWP::UserAgent;
use Image::Imlib2;
use Image::WorldMap;
use File::Copy;

my $ua = LWP::UserAgent->new;

my $license_key = qw(rLmbF7nMvSpw);
my $ipaddress;

my $sqlinsert = "insert into iplookup  " .
"(ipaddr, countrycode, region, city, latitude, longitude, active) " . 
"(  " .
"select distinct substr(t.ipaddr, 1, instr(t.ipaddr, '.', 1, 3)-1), " . 
"countrycode, null, null, null, null, 1 " .
"from trickle t  " .
"where not exists " . 
"(select 1  " .
"from iplookup ip " . 
"where  " .
"ip.ipaddr = " . 
"substr(t.ipaddr, 1, instr(t.ipaddr, '.', 1, 3)-1)))";

my $sql = "select distinct ip.ipaddr " . 
  "from iplookup ip where latitude is null " .
  "and ((countrycode != 'A2' and countrycode != 'A1') or countrycode is null)";

my $dbh = 
        DBI->connect(
          "DBI:Oracle:host=climatedbs2.oucs.ox.ac.uk"
        . ";sid=cpdn;port=1521",
                 "cpdn", "cl01dbora",
               { 'Warn' => 1, 'PrintError' => 1, 'RaiseError' => 1,
                      'AutoCommit' => 1 }) 
   or die();

 # inserts new ip's from trickles that may have come in
 my $sth = $dbh->prepare($sqlinsert) or die();
 print("Inserting new trickle record IP addresses...\015\012");
 $sth->execute() or die();
 $sth->finish();

  my $ctr = 0;
 # now run the query to get all the missing ipaddresses

# CMC commented out, we used up our IP addr lookups!
# $sth = $dbh->prepare($sql) or die();
# print("Querying MaxMind.com Web Service for IP addresses...\015\012");
# $sth->execute() or die();
# $sth->bind_columns(undef, \$ipaddress);
#
#  while($sth->fetch())
#  {
#     # just take the left-most 24 bits of an IP address
#     # to save queries and money
#     my $url = "http://maxmind.com:8010/b?l=" . 
#        $license_key . "&i=" . $ipaddress . ".1";  
#
#     my $request = HTTP::Request->new('GET', $url);
#     my $res = $ua->request($request);
#     if ($res->is_success())
#     {
#        my ($country, $region, $city, $lat, $long) = split(",",$res->content);
#        # update oracle table for this IP
#        #if ($country ne "A2")  {
#        my $upd = "UPDATE iplookup SET countrycode=?,"
#                . "region=?, city=?, latitude=?, longitude=? "
#                . "WHERE ipaddr=?";
#        my $sth2 = $dbh->prepare($upd);
#        $sth2->bind_param(1, $country, $DBI::SQL_VARCHAR);
#        $sth2->bind_param(2, $region, $DBI::SQL_VARCHAR);
#        $sth2->bind_param(3, $city, $DBI::SQL_VARCHAR);
#        $sth2->bind_param(4, $lat, $DBI::SQL_DOUBLE);
#        $sth2->bind_param(5, $long, $DBI::SQL_DOUBLE);
#        $sth2->bind_param(6, $ipaddress, $DBI::SQL_VARCHAR);
#        $sth2->execute();
#        $sth2->finish();
#           $ctr++;
#     }
#     else
#     {
#        print("Error on " . $ipaddress . "\015\012");
#     }                
#  }

#$sth->finish();
#
#print("$ctr IP Addresses updated from MaxMind.com\015\012");

# now draw the map!

# note that I am weeding out odd IPs lookups that are at 0,0
# it allows for 0 latitude but not both 0 lat & 0 long
$sql = "select city, latitude, longitude, count(distinct t.machineid) "
. "from iplookup ip, trickle t "
. "where latitude is not null and "
. "(latitude!=0 or (latitude=0 and longitude!=0)) "
. "and ip.ipaddr=substr(t.ipaddr, 1, instr(t.ipaddr, '.', 1, 3)-1) "
. "group by city,latitude,longitude order by longitude,latitude";

 # IP file
 my ($latitude, $longitude, $city, $count);

 $sth = $dbh->prepare($sql) or die();
 $sth->execute() or die();
 $sth->bind_columns(undef, \$city, \$latitude, \$longitude, \$count);

 print("Drawing the map...\015\012");

 #my $map = Image::WorldMap->new($DIR . "/earth-huge.png", "arial/8");
 #my $map = Image::WorldMap->new($DIR . "/earth-big.png");
 my $map = Image::WorldMap->new($DIR . "/earth-big.png", "arial/8");
 #my $map = Image::WorldMap->new($DIR . "/earth-ok.png", "arial/8");
 
  open (FILE,">" . $DIR . "/ip.txt") or die();

 # my $ctr = 0;
  #$sth->fetch();  ## for debugging just do one and comment out while
  while($sth->fetch()) # && $ctr<20)
  {
     #$map->add($longitude, $latitude, $city);  # use city names
     print FILE $latitude . "," . $longitude . "," . $count . "\015\012"; 
     $map->add($longitude, $latitude, "", [0,0,0]);   # no city names
     #$ctr++;
  }

close(FILE);
# gzip and move it
qx(/bin/gzip $DIR/ip.txt);
move($DIR . "/ip.txt.gz", "/var/www/html/insecure/map/ip.txt.gz");
unlink($DIR . "/ip.txt");
unlink($DIR . "/ip.txt.gz");

# manually add download servers (RED)
# climateprediction.net
#$map->add(-0.7000, 52.0333, "Oxford", [255,0,0]);   # no city names
$map->add(-0.7000, 52.0333, "", [255,0,0]);   # no city names

# berkeley.edu (seti)
#$map->add(-122.2536, 37.8668, "Berkeley, CA", [255,0,0]);   # no city names
$map->add(-122.2536, 37.8668, "", [255,0,0]);   # no city names

# MIT
#$map->add(-71.4594, 42.4464, "Boston, MA", [255,0,0]);   # no city names
$map->add(-71.4594, 42.4464, "", [255,0,0]);   # no city names

# New Zealand ftp.niwa.co.nz -- 202.36.29.2
#$map->add(174.7833, -41.3, "Wellington", [255,0,0]); 
$map->add(174.7833, -41.3, "", [255,0,0]); 

# Hamburg, Germany www.mad.zmaw.de -- 136.172.65.22
#$map->add(10.0, 53.5500, "Hamburg", [255,0,0]);   # no city names
$map->add(10.0, 53.5500, "", [255,0,0]);   # no city names

# Reading, UK -- 134.225.1.128
$map->add(-1.000, 51.4333, "", [255,0,0]);   # no city names

# manually add upload servers (BLUE)
$map->add(-1.2500, 51.7500, "", [255,0,0]);   # no city names

$sth->finish();

$map->draw($DIR . "/cpdnmap.png");

qx(/usr/X11R6/bin/convert -scale 1600x800 $DIR/cpdnmap.png $DIR/cpdnmap.jpg);

move($DIR . "/cpdnmap.jpg", "/var/www/html/insecure/map/cpdnmap.jpg");

# generate HTML file to hold the map

 open (FILE,">" . $DIR . "/index.html") or die();

    # write the line AND add a line end

   print FILE printHeader("User Map");

 print FILE "<IMG SRC=\"/map/cpdnmap.jpg\"><BR><BR>";
 print FILE "Map Legend: Black=CPDN Users, "
  . "Red=Download Servers (and Text Labels for Org/City), Blue=Upload Servers";

   print FILE "<BR><BR><A HREF=\""
     . "http://www.climateprediction.net/client/locations/\">"
     . "View Users Clustered Within Region</A>";
   print FILE "<BR><BR><H2>climateprediction.net Users By Country</H2>";

# now print a la the country section

#   print FILE "</BODY></HTML>";

print FILE 
 "<BR><BR><TABLE WIDTH='100%'  BORDER= BORDERCOLOR='silver'"
. " BGCOLOR='#F5F5F5' CELLSPACING=0 CELLPADDING=2>";
print FILE "<TR><TD COLSPAN=2 ALIGN=RIGHT><B>Total # of Registered Users:</B></TD>";
 my $query = "select count(*) from participant";
 $sth = $dbh->prepare($query);
 $sth->execute();
 my $user;
 $sth->bind_columns(undef, \$user);
 $sth->fetch();

  print FILE "<TD>$user</TD></TR>";

 $sth->finish();

 $query = "select ip.countrycode, c.name, "
    . "count(distinct t.machineid) "
    . "from trickle t, country c, iplookup ip "
    . "where ip.ipaddr=substr(t.ipaddr, 1, instr(t.ipaddr, '.', 1, 3)-1) "
    . "and ip.countrycode=c.id "
    . "group by ip.countrycode, c.name "
    . "order by c.name";
  
print FILE "<TR><TD COLSPAN=3><B>Country Counts (Unique Machines via Trickle Info)</B></TD></TR>\n";

print FILE 
 "<TR><B><TD><B>Code</B></TD>"
. "<TD><B>Country</B></TD>"
. "<TD><B>Number</B></TD></TR>";

 $sth = $dbh->prepare($query) or die();
 $sth->execute() or die();
  my ($user, $machine, $cliver);
 $sth->bind_columns(undef, \$user, \$machine, \$cliver);
  while($sth->fetch())
  {
    print FILE "<TR><TD>$user</TD><TD>$machine</TD><TD>$cliver</TD></TR>";
  }
 $sth->finish();
 
print FILE "</TABLE><BR><BR><A HREF=\"region.html\">Go to User Region Page</A><BR><BR>";
print FILE "<A HREF=\"city.html\">Go to User City Page</A><BR><BR>";

 print FILE "</BODY></HTML>";

 #unlock and close the file
 close (FILE);

# regions
doRegion(\$dbh);
doCity(\$dbh);

# crank out launch stats
doHourlyStats(\$dbh);

 $dbh->disconnect();

 move($DIR . "/index.html", "/var/www/html/insecure/map/index.html");

#qx(/usr/bin/perl $DIR/cpdnmap-huge.pl);

sub doHourlyStats
{
  use GD::Graph::linespoints;
  use GD::Graph::points;
  use GD::Graph::bars;
  my $rdbh = shift();

  my ($datehr, $cnt, @x, @y);

  open (FILE,">" . $DIR . "/hourly.html") or die();

    # write the line AND add a line end
   print FILE printHeader("Hourly Stats");

   print FILE "<BR><H2>climateprediction.net Registrations By Date/Hour</H2><BR><BR>";
   print FILE "<IMG SRC=\"chart.jpg\"><BR><BR>";
   print FILE "<IMG SRC=\"chart2.jpg\"><BR><BR>";
   print FILE "<IMG SRC=\"chart3.jpg\"><BR><BR>";

print FILE "<TR><TD COLSPAN=2 ALIGN=RIGHT><B>Total # of Registered Users:</B></TD>";
 my $query = "select count(*) from participant";
 my $sth = $$rdbh->prepare($query);
 $sth->execute();
 $sth->bind_columns(undef, \$datehr);
 $sth->fetch();

  print FILE "<TD>&nbsp;&nbsp;" . $datehr . "</TD></TR>";

 $sth->finish();

# $query = "select to_char(registrationdate, 'DD/MM HH24'), count(*) "
#    . "from participant "
#    . "where registrationdate>=current_date-7 "
#    # to_date('11-Sep-2003 00:00:00', 'DD-Mon-YYYY HH24:MI:SS') "
#    . "and to_char(registrationdate, 'DD/MM HH24')<>to_char(current_date, 'DD/MM HH24') "
#    . "group by to_char(registrationdate, 'DD/MM HH24')";

$query = "select to_char(registrationdate, 'MMDD DD-MON HH24'), count(*) "
    . "from participant "
    . "where registrationdate>=current_date-7 "
    . "and to_char(registrationdate, 'MMDD DD-MON HH24')"
    . "<>to_char(current_date, 'MMDD DD-MON HH24') "
    . "group by to_char(registrationdate, 'MMDD DD-MON HH24') "
    . "order by to_char(registrationdate, 'MMDD DD-MON HH24') ";

print FILE
 "<BR><BR><TABLE WIDTH='100%'  BORDER= BORDERCOLOR='silver'"
. " BGCOLOR='#F5F5F5' CELLSPACING=0 CELLPADDING=2>";

#print FILE
# "<TR><B><TD><B>Date / Hour (24)</B></TD>"
#. "<TD><B>New Registrations</B></TD></TR>";

 $sth = $$rdbh->prepare($query) or die();
 $sth->execute() or die();
 $sth->bind_columns(undef, \$datehr, \$cnt);
  while($sth->fetch())
  {
    #print FILE "<TR><TD>$datehr</TD><TD>$cnt</TD></TR>";
    # print labels every 4 hours
    my $xlabel = substr($datehr, length($datehr)-2, 2);
    if ($xlabel eq "00" 
      or $xlabel eq "04"
      or $xlabel eq "08"
      or $xlabel eq "12"
      or $xlabel eq "16"
      or $xlabel eq "20"
       ) {
       ## ensures a nice even hourly break
       push @x, substr($datehr, 5);  }
           #2, 2) . "/" 
           # .  substr($datehr, 0, 2) . " " . $xlabel; }  
    else {
       push @x, ""; }
        
    push @y, $cnt;
  }
 $sth->finish();

 print FILE "</TABLE><BR><BR>";
 
 print FILE "<A HREF=\"index.html\">Go to CPDN Map Page</A><BR><BR>";
 print FILE "<A HREF=\"index2.html\">Go to Super-Size CPDN Map Page</A><BR><BR>";
 print FILE "<A HREF=\"region.html\">Go to User Region Page</A><BR><BR>";

 print FILE "</BODY></HTML>";

  close (FILE);

  move($DIR . "/hourly.html", "/var/www/html/insecure/map/hourly.html");

 # now finish the chart plot and move jpg over to webdir
  my $chart = GD::Graph::bars->new(1024,768);

  $chart->set(
      x_label => "Date and Hour (24-hr)",
      y_label => "Number of New Users",
      x_ticks => 1,
     # x_long_ticks => 1,
      x_tick_length => 5,
      markers => [6],
      show_values => 1,                              
      x_labels_vertical => 1,
      x_label_position => .5,
      title => "Date/Hour Distribution of New Registrations Over the Past Week",
  );

  # now set the GD graphics object object (from libgd and GD.pm module)

  my $gd = $chart->plot([\@x, \@y]) or die $chart->error;

  open(IMG, ">" . $DIR . "/chart.png") or die $!;
  binmode IMG;
  print IMG $gd->png;
  qx(/usr/X11R6/bin/convert $DIR/chart.png $DIR/chart.jpg);
  move($DIR . "/chart.jpg", "/var/www/html/insecure/map/chart.jpg");

   $query = "select trunc(registrationdate, 'WW'), count(*) "  
    . "from participant "                                             
    . "where registrationdate>=to_date('10-09-2003 00:00:00', 'DD-MM-YYYY HH24:MI:SS') "
    . "group by trunc(registrationdate, 'WW') "             
    . "order by trunc(registrationdate, 'WW') ";            

 @x=(); 
 @y=();                                                                      

 $sth = $$rdbh->prepare($query) or die();                             
 $sth->execute() or die();                                            
 $sth->bind_columns(undef, \$datehr, \$cnt);                          
  while($sth->fetch())                                                
  {                                                                   
    push @x, $datehr;   ## ensures a nice even hourly break       
    push @y, $cnt;                                                    
  }                                                                   
 $sth->finish();                                                      

 # now finish the chart plot and move jpg over to webdir              
  my $chart = GD::Graph::linespoints->new(1024,768);                         
                                                                      
  $chart->set(                                                        
      x_label => "Week Start Date",                             
      y_label => "Number of New Users",                               
      x_ticks => 1,                                                   
     # x_long_ticks => 1,                                             
      x_tick_length => 5,                                             
      markers => [6],
      show_values => 1,                              
      x_labels_vertical => 1,                                         
      x_label_position => .5,                                         
      title => "New Users By Week",    
  );                                                                  
                                                                      
  # now set the GD graphics object object (from libgd and GD.pm module)
  undef $gd;
  my $gd = $chart->plot([\@x, \@y]) or die $chart->error; 
                                                                      
  open(IMG, ">" . $DIR . "/chart2.png") or die $!;                      
  binmode IMG;                                                        
  print IMG $gd->png;                                                 
  qx(/usr/X11R6/bin/convert $DIR/chart2.png $DIR/chart2.jpg);
  move("$DIR/chart2.jpg", "/var/www/html/insecure/map/chart2.jpg");

$query = "select trunc(trickledate, 'IW'), count(distinct machineid) " .
"from trickle " .
#"where trickledate>to_date('10-09-2003', 'DD-MM-YYYY') " .
"where trickledate>=to_date('10-09-2003 00:00:00', 'DD-MM-YYYY HH24:MI:SS') " .
"group by trunc(trickledate,'IW') " . 
"order by trunc(trickledate,'IW')";

 $sth = $$rdbh->prepare($query) or die();                             
 $sth->execute() or die();                                            
 $sth->bind_columns(undef, \$datehr, \$cnt);    
  @x=();
  @y=();
                      
  while($sth->fetch())                                                
  {                                                                   
       push @x, $datehr;
       push @y, $cnt;                                                    
  }                                                                   
 $sth->finish();                                                      
                                                                      
                                                                              
 # now finish the chart plot and move jpg over to webdir              
  my $chart = GD::Graph::linespoints->new(1024,768);
                                                                      
  $chart->set(                                                        
      x_label => "Week Start Date",                             
      y_label => "Number of Active Machines",                               
      x_ticks => 1,                                                   
      markers => [6],
      x_tick_length => 5,                                             
      x_labels_vertical => 1,                                         
      x_label_position => .5,           
      show_values => 1,                              
      title => "Actively Trickling Machines Per Week",    
  );                                                                  
                                                                      
  # now set the GD graphics object object (from libgd and GD.pm module)
  undef $gd;
  my $gd = $chart->plot([\@x, \@y]) or die $chart->error;             
                          
  open(IMG, ">" . $DIR . "/chart3.png") or die $!;                      
  binmode IMG;                                                        
  print IMG $gd->png;                                                 
  qx(/usr/X11R6/bin/convert $DIR/chart3.png $DIR/chart3.jpg);
  move("$DIR/chart3.jpg", "/var/www/html/insecure/map/chart3.jpg");

 $$rdbh->disconnect();                                                
}

sub doRegion
{
 my $rdbh = shift();

 my $query = "select ip.countrycode, c.name, NVL(r.name, '(Unknown)'), "
    . "count(distinct t.machineid) "
    . "from trickle t, country c, iplookup ip, region r "
    . "where ip.ipaddr=substr(t.ipaddr, 1, instr(t.ipaddr, '.', 1, 3)-1) "
    . "and ip.countrycode=c.id "
    . "and (ip.countrycode=r.countrycode (+) "
    . "and ip.region=r.region (+)) "
    . "group by ip.countrycode, c.name, NVL(r.name, '(Unknown)') "
    . "order by c.name, NVL(r.name, '(Unknown)')";

 open (FILE,">" . $DIR . "/region.html") or die();

    # write the line AND add a line end
   print FILE printHeader("Users By Region");

print FILE
 "<BR><TABLE WIDTH='100%'  BORDER= BORDERCOLOR='silver'"
. " BGCOLOR='#F5F5F5' CELLSPACING=0 CELLPADDING=2>";
print FILE "<TR><TD COLSPAN=4><B>Country/Region Counts (Unique Machines via Trickle Info for Known Geographical Regions)</B></TD></TR>\n";

print FILE
 "<TR><B><TD><B>Code</B></TD>"
. "<TD><B>Country</B></TD>"
. "<TD><B>Region</B></TD>"
. "<TD><B>Number</B></TD></TR>";

 my $sth = $$rdbh->prepare($query) or die();
 $sth->execute() or die();
  my ($code, $name, $region, $count);
 $sth->bind_columns(undef, \$code, \$name, \$region, \$count);
  my ($checkct, $checkrg, $checkcode);
  while($sth->fetch())
  {
    if ($code eq $checkct) { $checkct = "&nbsp"; $checkcode = "&nbsp";} 
    else { $checkct = $name; $checkcode = $code; }
    if ($region eq $checkrg) { $checkrg = "&nbsp"; } else { $checkrg = $region; }
    print FILE "<TR><TD>$checkcode</TD><TD>$checkct</TD><TD>$checkrg</TD><TD>$count</TD></TR>";
    $checkct = $code;
  }
 $sth->finish();

print FILE "</TABLE><BR><BR><A HREF=\"city.html\">Go to User City Page</A><BR><BR>";
 print FILE "</BODY></HTML>";

 #unlock and close the file
 close (FILE);

 move($DIR . "/region.html", "/var/www/html/insecure/map/region.html");

}

sub doCity
{
 my $rdbh = shift();

 my $query = "select ip.countrycode, c.name, NVL(r.name, '(Unknown)'), "
    . "NVL(ip.city, '(Unknown)'), "
    . "count(distinct t.machineid) "
    . "from trickle t, country c, iplookup ip, region r "
    . "where ip.ipaddr=substr(t.ipaddr, 1, instr(t.ipaddr, '.', 1, 3)-1) "
    . "and ip.countrycode=c.id "
    . "and (ip.countrycode=r.countrycode (+) and "
    . "ip.region=r.region (+)) "
    . "group by ip.countrycode, c.name, NVL(r.name, '(Unknown)'), "
    . "NVL(ip.city, '(Unknown)') "
    . "order by c.name, NVL(r.name, '(Unknown)'), NVL(ip.city, '(Unknown)')";

 open (FILE,">" . $DIR . "/city.html") or die();

    # write the line AND add a line end
   print FILE printHeader("Users By City");

print FILE
 "<BR><TABLE WIDTH='100%'  BORDER= BORDERCOLOR='silver'"
. " BGCOLOR='#F5F5F5' CELLSPACING=0 CELLPADDING=2>";
print FILE "<TR><TD COLSPAN=4><B>Region/City Counts (Unique Machines via Trickle Info for Known Geographical Regions)</B></TD></TR>\n";

print FILE
 "<TR><B><TD><B>Code</B></TD>"
. "<TD><B>Country</B></TD>"
. "<TD><B>Region</B></TD>"
. "<TD><B>City</B></TD>"
. "<TD><B>Number</B></TD></TR>";

 my $sth = $$rdbh->prepare($query) or die();
 $sth->execute() or die();
  my ($code, $name, $region, $city, $count);
  my ($checkct, $checkrg, $checkcode);
 $sth->bind_columns(undef, \$code, \$name, \$region, \$city, \$count);
  while($sth->fetch())
  {
    if ($code eq $checkct) { $checkct = "&nbsp"; $checkcode = "&nbsp";} 
    else { $checkct = $name; $checkcode = $code; }
    if ($region eq $checkrg) { $checkrg = "&nbsp"; } else { $checkrg = $region; }
    print FILE "<TR><TD>$checkcode</TD><TD>$checkct</TD><TD>$checkrg</TD><TD>$city</TD><TD>$count</TD></TR>";
    $checkct = $code;
    $checkrg = $region;
  }
 $sth->finish();

print FILE "</TABLE><BR><BR><A HREF=\"region.html\">Go to User Region Page</A><BR><BR>";
 print FILE "</BODY></HTML>";

 #unlock and close the file
 close (FILE);

 move($DIR . "/city.html", "/var/www/html/insecure/map/city.html");

}

sub printHeader
{
my ($Second, $Minute, $Hour, $Day, $Month, $Year,
   $WeekDay, $DayOfYear, $IsDST) = localtime(time);

$Month++;
$Year += 1900;

return 
"<html>" .
"<head>" .
"<link REL=\"SHORTCUT ICON\" HREF=\"http://cpdn.comlab.ox.ac.uk/favicon.ico\">" .
"<title>climateprediction.net " . shift() . " Page - Generated On " .
sprintf("%02d/%02d/%4d %02d:%02d:%02s GMT",
       $Day, $Month, $Year, $Hour, $Minute, $Second)
. "</title><BODY BGCOLOR=\"#F5F5F5\">" .
"<meta name=\"keywords\" content=\"climate prediction global warming distributed computing climateprediction.net\">" .
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">" .
"<meta name=\"Author\" content=\"Oxford University / climateprediction.net\">" .
"</head><BODY>" .
"<A HREF=\"http://www.climateprediction.net\">" .
"<IMG SRC=\"cpn_logo.gif\" border=\"0\" alt=\"climateprediction.net\"></A><BR><BR>";

}

