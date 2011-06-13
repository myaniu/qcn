<?php
/*
// note the optional final field for a URL - this so when it's processed by
// facebook the appropriate URL/link will show up
// if you put a news item that doesn't need a URL, just put a blank at the end of the array
// the array format is:
     array("Month Day, Year HH:MM",
           "
                long description/news item - with two BRs to separate nicely:
                <BR><BR>  
           ",
           "News Heading",
           "optional URL"
       ),
*/


$project_news = 
 array(
   array("June 9, 2011 11:55",
"Carl from QCN will be presenting at the <A HREF=\"http://okcon.org/2011/programme\">Open Knowledge Conference in Berlin</A> on June 29th (workshop on 'open science') and June 30th (panel discussion). 
<BR><BR>
",
 "Open Knowledge Conference in Berlin, Germany (June 28th-July 1st)",
 "http://okcon.org/2011/programme"
 ),
   array("April 22, 2011 19:22",
"Thanks to BOINC & QCN participant 'dudumomo' - videos of the recent 'Asia@home Hackfest' are <A HREF='http://www.freetorrent.fr/catalog.php?search=Asia%40home'>now available via torrent download</A>.  Each download is a few hundred MB and features talks by David Anderson of SETI@home and BOINC, and Carl Christensen and Elizabeth Cochran of QCN.  The talks were made in March at Academia Sinica in Taipei, Taiwan.
<BR><BR>
",
 "Asia@home Hackfest Videos Available",
 "http://www.freetorrent.fr/catalog.php?search=Asia%40home"
 ),
   array("March 14, 2011 19:56",
"Carl from QCN is presenting at the <A HREF=\"http://mozilla.com.cn/event/23-drumbeat-en/\">2011 China Drumbeat Hackfest</A> in Beijing from March 23-25th, 2011. The goal is to encourage volunteer computing and 'citizen science' application development.  Programmers are welcome!<BR><BR>
",
 "2011 China Drumbeat Hackfest March 23-25, 2011",
 "http://mozilla.com.cn/event/23-drumbeat-en/"
 ),
   array("February 23, 2011 21:14",
"QCN is participating in the <A HREF=\"http://event.twgrid.org/isgc2011/asiaathome.html\">Asia@home Hackfest</A>.  The goal of the event is bring more seismology applications to run under BOINC, and possibly deploy QCN in Taiwan. 
<BR><BR>
",
 "Asia@home Hackfest March 19-23, 2011",
 "http://event.twgrid.org/isgc2011/asiaathome.html"
 ),
   array("February 22, 2011 19:20",
"On February 21, 2011 at 23:51 (UTC) a M6.3 earthquake occurred just outside of Christchurch, New Zealand.  <A HREF=\"http://qcn.stanford.edu/sensor/download/nz-chch-m63.jpg\">Several QCN stations installed in and around Christchurch recorded this earthquake</A>. Due to the shallow depth of this event and proximity to Christchurch the shaking was quite severe in the city. The QCN stations recorded accelerations up to about 1g (9.8m/s/s), equivalent to the force of gravity. The QCN stations in the region continue to monitor the ongoing aftershock sequence.
<BR><BR>
",
 "February 21, 2011 M6.3 Earthquake in Christchurch Recorded on QCN Stations",
 "http://qcn.stanford.edu/sensor/download/nz-chch-m63.jpg"
 ),
  array("December 13, 2010 22:13",
"We have unfortunately had a problem with the JoyWarrior 24F14 sensor setup, in that some were shipped out without the required 2g settings for QCN.  Also the sensors cannot be setup within the QCN software unlike the 24F8 USB sensors.  So if your sensor does not seem to be working correctly, <A HREF=\"http://qcn.stanford.edu/sensor/download/jw24f14.zip\">please download this small program (for Mac and Windows)</A>, and follow the steps in the README.txt file.  We apologize for any inconvenience.
<BR><BR>
",
 "JoyWarrior 24F14 Sensor Setup for QCN",
 "http://qcn.stanford.edu/sensor/download/jw24f14.zip"
  ),
   array("December 9, 2010 22:13",
"If you are going to the AGU Fall Conference in San Francisco next week, please look us up at our various exhibits/posters.
<BR><BR>
",
 "QCN at AGU Conference",
 ""
  ),
   array("November 4, 2010 23:23",
"Due to construction in a new server room at Stanford, we expect intermittent outages between November 22nd and 24th.  No action is required on your part as BOINC will just reconnect when the servers are back online.<BR><BR>",
 "Server Outages November 22nd - 24th, 2010",
 ""
  ),
   array("October 30, 2010 05:34",
"New software is on the server which should fix some bugs, false/too many triggers, and enhanced processing of the seismic data files which reduces their size.  You should automatically get this new version via BOINC within a day.  Also a new feature is in place whereby we can get followup information after a quake (basically a trickle to our server with more information 4-5 seconds after the original trigger).  Please report any problems to the 'Message Boards' (and hopefully this won't be the scariest thing you'll see this Halloween)!<BR><BR>",
 "Upgraded QCN Software (version 5.83)",
 ""
  ),
   array("September 17, 2010 17:40",
"CNN has released a <A HREF=\"http://edition.cnn.com/2010/TECH/innovation/09/17/ibm.earthquakes/index.html\">news story</A> about QCN and some of our rapid response efforts in Chile and New Zealand.<BR><BR>",
 "CNN News Story On QCN and Our 'Rapid Aftershock Mobilization Program'",
 "http://edition.cnn.com/2010/TECH/innovation/09/17/ibm.earthquakes/index.html"
 ),
   array("August 18, 2010 10:58",
"Prof. Cochran will do a presentation on the Quake-Catcher Network at the London Citizen Cyberscience Summit on September 3rd.  If you are in the area you may wish to see this and the other interesting presentations on volunteer computing, 'citizen science', etc.  Tickets and a full programme are <A HREF=\"http://www.citizencyberscience.net/summit/\">available at this link</A><BR><BR>",
 "London Citizen Cyberscience Summit - 2-3 Sept 2010",
 "http://www.citizencyberscience.net/summit/"
  ),
   array("June 23, 2010 17:16",
"
A QCN participant has detected the rare magnitude 5.5 earthquake in Quebec, Canada today.<BR>
<A HREF=\"http://qcn.stanford.edu/sensor/img/quebec_june_23_2010.png\">QCN Participant Plot of M5.5 Quebec</A><BR><BR>",
 "QCN Participant Detects M5.5 Quebec Earthquake Today",
 "http://qcn.stanford.edu/sensor/img/quebec_june_23_2010.png"
  ),
   array("June 18, 2010 15:45",
"
The QCN Servers will be down for maintenance and upgrades on Tuesday June 22nd from approximately
3PM PST (6PM EST, 11PM BST).  You will not be able to send triggers or access the websites
for about an hour from this time.
<BR><BR>",
 "QCN Server Maintenance June 22nd 3PM PST",
 ""
  ),
   array("June 15, 2010 15:45",
"
We have received 500 of <A HREF=\"http://qcn.stanford.edu/images/jw24F14.png\">the new JoyWarrior 24F14 'Tomcat' USB sensors</A> from codemercs.com, with more on the way.  It's in a new stylish red case.  So if you
placed an order you should be getting it soon, as they are being shipped out by our student
helpers Tom and Jordan.  We apologize again for the delay
but it was out of our control due to production and shipping delays (the recent volcano in Iceland didn't help either!).  
The new sensor is fairy high sensitive, 14-bit resolution, and appears to be 16 times better than the 
previous sensors.  We are still testing it on a shake table but preliminary results are very promising.  
<A HREF=\"http://qcn.stanford.edu/learning/requests.php\">Click here for the order form (still $49).</A>
<BR><BR>",
 "New JoyWarrior 24F14 Sensors Have Arrived",
 "http://qcn.stanford.edu/learning/requests.php"
  ),
   array("April 24, 2010 12:00",
"
QCN now has an 'official' group on Facebook.  We hope to have news items, photos, and events listed here.  If you're interested and use Facebook, 
<A HREF=\"http://www.facebook.com/group.php?gid=118103081550123\">click on this link to see the page</A>, and perhaps become a 'Fan' for regular updates on your Wall.
<BR><BR>",
 "QCN on Facebook",
 "http://www.facebook.com/group.php?gid=118103081550123"
  ),
   array("April 20, 2010 12:00",
"
QCN project scientists were interviewed by National Public Radio (NPR) show 'All Things Considered' 
<A HREF=\"http://www.npr.org/templates/story/story.php?storyId=126073353\">(listen to clip here)</A>.
<BR><BR>",
 "QCN on National Public Radio",
  "http://www.npr.org/templates/story/story.php?storyId=126073353"
  ),
   array("April 12, 2010 12:00",
"
The BBC have done a story on the Quake-Catcher Network featuring Prof. Cochran.  
<A HREF=\"http://news.bbc.co.uk/2/hi/technology/8615558.stm\">It is available for viewing here.</A>
<BR><BR>",
 "BBC News Item on QCN", "http://news.bbc.co.uk/2/hi/technology/8615558.stm"
  ),
   array("April 4, 2010 12:00",
"
The M7.2 earthquake that occurred this afternoon along the Laguna Salada fault in Mexico was recorded by several QCN participants in Southern California.  <A HREF=\"http://qcn.stanford.edu/sensor/img/M7.2_Mexico.png\">Attached is a plot of waveforms from stations</A> that have reported back already; we will update this plot as more information comes in. We have a number of stations also reporting in with data for the numerous aftershocks that followed the large mainshock.
<BR><BR>",
 "M7.2 Earthquake in Mexico", "http://qcn.stanford.edu/sensor/img/M7.2_Mexico.png"
  ),
   array("March 16, 2010 12:00",
"This morning there was a <A HREF=\"http://earthquake.usgs.gov/earthquakes/eqinthenews/2010/ci14601172/\">magnitude 4.4 earthquake in the Los Angeles area.</A>
  Many QCN <A HREF=\"http://qcn.stanford.edu/sensor/img/20100316_M4.4_LosAngeles.png\">participants detected this earthquake, as shown here.</A><BR><BR>",
 "Los Angeles area M4.4 Earthquake", "http://qcn.stanford.edu/sensor/img/20100316_M4.4_LosAngeles.png"
  ),
   array("March 12, 2010 12:00",
"Today there is a <A HREF=\"http://www.latimes.com/news/local/la-me-quakecatchers12-2010mar12,0,440482.story\">good article on QCN in the Los Angeles Times (click here)</A>, featuring Elizabeth Cochran and some of her 'schools outreach' work and an interview with a QCN participant.<BR><BR>",
 "Los Angeles Times article on QCN", "http://www.latimes.com/news/local/la-me-quakecatchers12-2010mar12,0,440482.story"
  ),
   array("March 8, 2010 12:00",
"On March 9, two graduate students flew to Chile to begin installing 100 QCN sensors in the region affected by the recent M8.8 earthquake. Our partners at the University of Concepcion are coordinating the installation of sensors. Volunteers from the University of Concepcion and University of Santiago will assist with installing sensors in cities throughout the region. We hope to record many of the aftershocks that are still occurring in the area to better understand the distribution of aftershocks and to study the properties of earthquakes. We will include updates on their progress and the data that are recorded in the coming weeks. If you are in Chile and would like to participate in the Quake-Catcher Network Rapid Aftershock Mobilization Project (QCN-RAMP) <A HREF=\"http://qcn.stanford.edu/RAMP/RAMP_Chile_2010.php\">please fill out the form on this link.</A><BR><BR>",
 "QCN responds to the M8.8 earthquake in Chile", "http://qcn.stanford.edu/RAMP/RAMP_Chile_2010.php"
  ),
   array("February 27, 2010 12:00",
"On February 27, 2010 a M8.8 earthquake struck Chile just northwest of the major city of Concepcion. A QCN partner at the University of Concepcion was testing a JoyWarrior USB sensor with the goal of introducing QCN to the region in the future. The sensor caught the M8.8 mainshock and a number of aftershocks that followed. The computer was attached to a backup power supply that kept the computer running for one hour after the earthquake knocked out power to most of the city.  Unfortunately, at the time of the earthquake the sensor was not connected to the floor, but most of the earthquake record looks surprisingly good. The very high accelerations approximately 15 seconds into the record may be the sensor falling off of the box it was sitting on.  <A HREF=\"http://qcn.stanford.edu/sensor/download/M8.8_first_trigger.png\">Click here to view the QCN output.</A><BR><BR>",
 "QCN participant records the M8.8 earthquake in Chile", "http://qcn.stanford.edu/sensor/download/M8.8_first_trigger.png"
  ),
   array("February 10, 2010 12:00",
"A rare midwestern US earthquake was detected by a QCN participant in Illinois.  The quake was of <A HREF=\"http://earthquake.usgs.gov/earthquakes/recenteqsus/Quakes/us2010snay.php\">magnitude 3.8 and was 45 miles northwest of Chicago, Illinois</A>.  The quake was only one of three detected in the upper Midwest in the past 21 years.  The QCN participant was using an external JoyWarrior USB sensor - you can see the <A HREF=\"http://qcn-web.stanford.edu/download/illinois.png\">plot of the QCN-detected quake activity here.</A><BR><BR>",
 "QCN Participant Detects Illinois M3.8 Quake", "http://qcn-web.stanford.edu/download/illinois.png"
  ),
   array("January 27, 2010",
"Due to maintenance work in the server room at Stanford, QCN will be down the morning (Pacific Time) of the 27th.<BR><BR>",
 "Planned Server Outage Weds 01/27/2010",""
  ),
   array("November 9, 2009",
"New versions of QCNLive for Mac and Windows have been developed which allow you to record the sensor output in a 'csv' (comma-separated values) format for import into spreadsheet software such as Excel.  Also web links to the USGS site have been updated to reflect the new USGS website layout.<BR>",
 "New QCNLive versions (5.34)",""
  ),
   array("May 5, 2009",
"A Quake-Catcher Network participant <A HREF=\"http://qcn.stanford.edu/sensor/img/qcngermanym45.png\">detected a magnitude 4.5 earthquake in SW Germany</A>, near the borders of France and Switzerland using his JoyWarrior USB sensor and QCN/BOINC.  This is a fairly rare event to capture on QCN and we thank him and others running QCN!  The participant's computer was only 10km from the epicentre.<BR>",
 "QCN Participant Detects M4.5 earthquake in Southwest Germany", "http://qcn.stanford.edu/sensor/img/qcngermanym45.png"
  ),
   array("April 24, 2009",
"Two Quake-Catcher Network participants felt and recorded yesterday's magnitude 4.0 earthquake in Yorba Linda, California (southern Cal, southeast of Los Angeles).  <A HREF=\"http://qcn.stanford.edu/sensor/forum_thread.php?id=231\">You can view some of the data at this link.</A>  They are only about 4 km / 2.4 miles from the epicenter, so it was a good test of QCN and the JoyWarrior USB sensors!  Happily everyone is OK with just a few plates shaken up etc.<BR>",
 "QCN Participants Detect M4.0 earthquake in Yorba Linda, California", ""
  ),
   array("April 22, 2009",
"QCN is going to require a BOINC client upgrade (within the next two weeks) to at least version 6.2.  This will enable QCN to run more reliably tanks to new BOINC features such as higher-priority CPU scheduling for low-CPU tasks such as QCN.  <A HREF=\"http://boinc.berkeley.edu/download.php\">Download the latest version here.</A>  We recommend BOINC version 6.6 or higher, as available on the download link, to ensure that your QCN software and sensor will work at a higher priority level, and cut down on the 'resets' especially when running other BOINC projects alongside QCN.<BR>",
 "Minimum BOINC Version 6.2 for QCN (6.6+ preferred)",  ""
  ),
   array("April 21, 2009",
"A few QCN participants caught the recent M5.1 quake in Hawaii.  Also as of today I am removing the 'alpha test' status of the Quake-Catcher Network, although the URL will remain the same so you do not have to reconnect/reattach.  The credit stats should now be tracked by <A HREF=\"http://boincstats.com\">boincstats.com</A><BR>",
 "Hawaii Quake Detected; QCN Is Now 'Non-Alpha'", ""
  ),
   array("March 30, 2009",
"<A HREF=\"http://earthquake.usgs.gov/eqcenter/recenteqsww/Quakes/nc40234037.php\">At least four QCN participant's machines detected today's San Jose earthquke.</A>
  Also for the first time the external USB JoyWarrior sensor was used to detect a quake (it's previously just been Mac laptops).  So keep running QCN and thanks for participating!<BR>" ,
 "QCN Participant Machines Detect Today's San Jose M4.2 Quake", ""
  ),
   array("January 14, 2009",
"<A HREF=\"http://qcn.stanford.edu/learning/requests.php\">QCN's USB sensors are now available for order</a>. 
 The sensors are $5 for K-12 teachers, and $49 for everyone else.  
  With your help, we can grow
the network larger, better, and faster! Thanks for your support!<BR>",
 "External USB Sensors Available", "http://qcn.stanford.edu/learning/requests.php"
  ),
   array("November 22, 2008",
"The 'ScienCentral' channel has <A HREF=http://www.youtube.com/watch?v=5TYBAk2FUeI>posted a short (two minute) video-clip regarding the Quake-Catcher Network featuring an interview with Prof. Jesse Lawrence of Stanford.</A><BR>",
 "ScienceCentral video about QCN on YouTube"
  ,""
  ),
   array("November 7, 2008",
"New version 4.25 is up for all platforms - this version perturbs some of the sensitivity algorithm used to detect triggers.  It will hopefully lead to less 'false positive' triggers/trickles as well as help us 'fine tune' our algorithm.<BR>",
 "New version 4.25, All Platforms, Tests Triggering Algorithm"
  ,""
    ),
   array("November 4, 2008",
"New version 4.22 is up for all platforms.  This has improved error handling.  If you are using the JoyWarrior 24F8 external USB accelerometer, you will want to upgrade the QCN USB driver software <A HREF=http://qcn.stanford.edu/download/qcnusb-intel.zip>for Intel Mac</A> or <A HREF=http://qcn.stanford.edu/download/qcnusb-ppc.zip>for PowerPC Macs</A>.<BR>",
 "New version 4.22, All Platforms"
  ,""
    ),
   array("October 30, 2008",
 "In honor of my hometown team the Philadelphia Phillies winning the World Series, I put up a new version (4.00) that will track how much time you've run QCN, and grant BOINC 'cobblestone' credits accordingly.  This new version also has some improved graphics in the screensaver (in my biased opinion :-).<BR>The 'bouncing cube' now shows a plot of your accelerometer on each cube face, the earth view has a nice multitexture of the earth at day and night (if your graphics card supports it), and in screensaver mode it will cycle through these different scenes so you will get some variety.  Also I have made workunits shorter - that way after a 'day' of run-time on QCN, you will get a new workunit and new software versions.  This is an 'optional' upgrade now if you 'Abort' your current workunit or 'Reset Project.'  Once we have a few people running 4.00+ for a few days I will send out an automatic message to upgrade.<BR>",
 "New Version 4.00, With BOINC Credits and Graphics Upgrades"
  ,""
    ),
   array("October 24, 2008",
"We now have maps to show the recent triggers recorded by participants of the Quake Catcher Network - <A HREF=http://qcn.stanford.edu/sensor/maptrigtime.php>click here to see the triggers recorded for the past day, week, or month</A> - then click again on a region to see further details on Google Maps!<BR>",
 "Trigger Maps for QCN Participants!"
  ,""
    ),
   array("October 21, 2008",
"The server was down a few hours tonight for some routine maintenance, sorry for the inconvenience!<BR>",
 "Server Maintenance Today"
  ,""
    ),
   array("October 16, 2008",
"There are new versions (3.70) for Intel & PowerPC Mac OS X, Microsoft Windows, and (yes!) Linux.  The Windows version supports the MotionNode Accel USB accelerometer, and all versions support the JoyWarrior24F8 USB accelerometer (Mac requires the driver for the JoyWarrior - see below).  To get a new version hit the 'Reset Project' or 'Task / Abort' button on your BOINC Manager client.<BR>",
 "New Versions (3.70) for Mac, Windows, and Linux; MotionNode Accel USB support for Windows"
  ,""
    ),
   array("October 4, 2008",
"There are new versions (3.47) for Intel & PowerPC Mac OS X and Windows that should fix some problems with stability and graphics.  Also, support for the JoyWarrior 24F8 sensor is in place as well.  If you are running BOINC on a Mac, you will need to download a small driver program and install it so that the QCN software running under BOINC can read the JoyWarrior sensor (this is not needed for 'built-in' laptop accelerometers).  <A HREF=http://qcn.stanford.edu/sensor/download/qcnusb-intel.zip>The Intel Mac driver is here</A>  and <A HREF=http://qcn.stanford.edu/sensor/download/qcnusb-ppc.zip>the PowerPC Mac driver is here </A>.  The Intel Mac driver is a hopefully 'easy to follow' installation program; unfortunately the PowerPC Mac driver is a simple shell script you will need to unzip and run in a 'Terminal' window.  JoyWarrior 24F8 support on Windows is also in place, and should not require a separate driver program (at the current time!).  As usual, please report problems/questions in the <A HREF=http://qcn.stanford.edu/sensor/forum_index.php>Message Boards</A>.<BR>",
 "New Versions (3.47) for Mac and Windows, JoyWarrior 24F8 support"
  ,""
    ),
   array("September 25, 2008",
"This week's issue of 'The Economist' (Sept. 25th '08) has an article on QCN written by Francois Grey of CERN.  You can <A HREF='http://www.economist.com/science/displaystory.cfm?story_id=12295198'>read it online here</A><BR>",
 "QCN article in the 9/25/08 issue of 'The Economist' magazine"
  ,""
    ),
   array("August 9, 2008",
"Please note if your BOINC running QCN seems to hang or crash.  There is a bug in BOINC for long account files > 64KB -- basically if the historical 
earthquake list gets too large then BOINC will not start and may seem to hang.  Unfortunately the only solution is to quit BOINC, and remove this file 'account_qcn.stanford.edu_sensor.xml' which is located (on Macs) at /Library/Application Support/BOINC Data (or Windows check c:\program files\BOINC).  You should then be able to start BOINC and you will have to reattach/relogin to the QCN project.  Sorry for the inconvenience!<BR>",
 "Note (and fix) for BOINC Crashes"
  ,""
    ),
    array("July 29, 2008",
"There was a magnitude 5.4 earthquake east of Los Angeles, detected by about 15 machines running QCN.  
<A HREF='http://qcn.stanford.edu/EVENTS/2008_211'>Click here for more details</A>.  Most machines
reported back to the servers with 2-4 seconds of the initial detection trigger.  These results 
(in addition to the previous quake in Reno a few months ago) show great promise for the QCN software!
So thanks and keep running QCN!<BR>",
 "QCN Testers Detect Quake in Los Angeles"
  ,""
    ),
    array("July 11, 2008",
"New versions (2.81) have been put up for all platforms supported by QCN (Mac Intel, Mac PPC, Windows).
This version fixes some bugs and adds some enhancements to the SAC data produced by the software.  It 
also adds support for the codemercs.com JoyWarrior 24F8 External USB accelerometer on Mac and Windows.
To get the new software, just click 'Reset Project' of the 'Project Tab' on the 'Advanced' menu on BOINC.<BR>",
 "New Version of Software Available (version 2.81)"
  ,""
    ),
    array("May 30, 2008",
"An alpha version of the QCN software for Windows has been created
which will work on Lenovo Thinkpad laptops.  If you have a Thinkpad 
please try it out and post any comments in the 'Windows' newsgroup.  Thanks!
Also thanks to Ben Suter, Stanford alum, for helping 'crack the code' for
the Thinkpads on QCN!<BR>",
 "QCN BOINC Software for Windows, Lenovo Laptops"
  ,""
    ),
    array("May 14, 2008",
"We are happy to report that two QCN alpha test participants who live in and
near Reno Nevada detected vibrations from the recent Magnitude 5.1
earthquake in Reno using QCN software. We're very excited and are currently
preparing a paper to show these preliminary results, i.e. that the QCN
software seems to detect real seismic events and report to our servers
within seconds. Hopefully this is the start of a real system that can be
used for advanced warning someday! We're also happy to report that both
participants and their laptops are fine! :-)
<BR>",
        "QCN Records Reno Nevada Earthquake of April 26, 2008"
  ,""
    ),
    array("March 27, 2008",
        "There's a <A HREF=\"http://www.nature.com/news/2008/080326/full/452397a.html\">short article about QCN in Nature News Online</a><BR>",
        "Nature News article on QCN"
  ,""
    ),
    array("February 28, 2008",
        "Thanks to all those taking part in the alpha test!  New versions have been uploaded (version number 2.46) for the Intel and PowerPC Macs, and Windows.  Note the Windows version is still 'graphics only' as the accelerometer work has not begun.  Next stage will be server side analysis to see if triggers match any known seismic events.<BR>",
  "Updated Versions to 2.46"
  ,""
    ),
    array("February 3, 2008",
        "We are now in alpha testing mode -- currently support Intel and PPC Macs running OS X.  Development under way for Windows (HP laptop accelerometers).",
        "Start of Alpha Test (Intel and PPC Mac OS X).  Please note that global prefs will be set to 1 CPU active, run after idle for 3 minutes - please only attach if you do not mind these default global preferences!"
  ,""
    ),
    array("December 15, 2007",
        "QCN personnel (Jesse, Elizabeth, and Carl) attended the American Geophysical Society conference this week, and had a poster and demo session 
         showing the current state of the QCN software which will run under BOINC",
        "QCN Demo and Poster at AGU Conference"
  ,""
    ),
    array("December 5, 2007",
        "Thanks to Dennis Michael of Stanford for procuring and setting up three servers for the project (web, upload, and database).",
        "QCN Has Servers!"
  ,""
    )
);

?>
