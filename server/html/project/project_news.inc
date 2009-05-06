<?php
$project_news = array(
   array("May 5, 2009",
"A Quake-Catcher Network participant <A HREF=\"img/qcngermanym45.png\">detected a magnitude 4.5 earthquake in SW Germany</A>, near the borders of France and Switzerland using his JoyWarrior USB sensor and QCN/BOINC.  This is a fairly rare event to capture on QCN and we thank him and others running QCN!  The participant's computer was only 10km from the epicentre.<BR>",
 "QCN Participant Detects <A HREF=\"http://earthquake.usgs.gov/eqcenter/recenteqsww/Quakes/us2009ggaf.php\">M4.5 earthquake in Southwest Germany</A>"
  ),
   array("April 24, 2009",
"Two Quake-Catcher Network participants felt and recorded yesterday's magnitude 4.0 earthquake in Yorba Linda, California (southern Cal, southeast of Los Angeles).  <A HREF=\"http://qcn.stanford.edu/sensor/forum_thread.php?id=231\">You can view some of the data at this link.</A>  They are only about 4 km / 2.4 miles from the epicenter, so it was a good test of QCN and the JoyWarrior USB sensors!  Happily everyone is OK with just a few plates shaken up etc.<BR>",
 "QCN Participants Detect M4.0 earthquake in Yorba Linda, California"
  ),
   array("April 22, 2009",
"QCN is going to require a BOINC client upgrade (within the next two weeks) to at least version 6.2.  This will enable QCN to run more reliably tanks to new BOINC features such as higher-priority CPU scheduling for low-CPU tasks such as QCN.  <A HREF=\"http://boinc.berkeley.edu/download.php\">Download the latest version here.</A>  We recommend BOINC version 6.6 or higher, as available on the download link, to ensure that your QCN software and sensor will work at a higher priority level, and cut down on the 'resets' especially when running other BOINC projects alongside QCN.<BR>",
 "Minimum BOINC Version 6.2 for QCN (6.6+ preferred)"
  ),
   array("April 21, 2009",
"A few QCN participants caught the recent M5.1 quake in Hawaii.  Also as of today I am removing the 'alpha test' status of the Quake-Catcher Network, although the URL will remain the same so you do not have to reconnect/reattach.  The credit stats should now be tracked by <A HREF=\"http://boincstats.com\">boincstats.com</A><BR>",
 "Hawaii Quake Detected; QCN Is Now 'Non-Alpha'"
  ),
   array("March 30, 2009",
"<A HREF=\"http://earthquake.usgs.gov/eqcenter/recenteqsww/Quakes/nc40234037.php\">At least four QCN participant's machines detected today's San Jose earthquke.</A>
  Also for the first time the external USB JoyWarrior sensor was used to detect a quake (it's previously just been Mac laptops).  So keep running QCN and thanks for participating!<BR>" ,
 "QCN Participant Machines Detect Today's San Jose M4.2 Quake"
  ),
   array("January 14, 2009",
"<A HREF=\"http://qcn.stanford.edu/learning/requests.php\">QCN's USB sensors are now available for order</a>. 
 The sensors are $5 for K-12 teachers, and $49 for everyone else.  
  With your help, we can grow
the network larger, better, and faster! Thanks for your support!<BR>",
 "External USB Sensors Available"
  ),
   array("November 22, 2008",
"The 'ScienCentral' channel has <A HREF=http://www.youtube.com/watch?v=5TYBAk2FUeI>posted a short (two minute) video-clip regarding the Quake-Catcher Network featuring an interview with Prof. Jesse Lawrence of Stanford.</A><BR>",
 "ScienceCentral video about QCN on YouTube"
    ),
   array("November 7, 2008",
"New version 4.25 is up for all platforms - this version perturbs some of the sensitivity algorithm used to detect triggers.  It will hopefully lead to less 'false positive' triggers/trickles as well as help us 'fine tune' our algorithm.<BR>",
 "New version 4.25, All Platforms, Tests Triggering Algorithm"
    ),
   array("November 4, 2008",
"New version 4.22 is up for all platforms.  This has improved error handling.  If you are using the JoyWarrior 24F8 external USB accelerometer, you will want to upgrade the QCN USB driver software <A HREF=http://qcn.stanford.edu/download/qcnusb-intel.zip>for Intel Mac</A> or <A HREF=http://qcn.stanford.edu/download/qcnusb-ppc.zip>for PowerPC Macs</A>.<BR>",
 "New version 4.22, All Platforms"
    ),
   array("October 30, 2008",
 "In honor of my hometown team the Philadelphia Phillies winning the World Series, I put up a new version (4.00) that will track how much time you've run QCN, and grant BOINC 'cobblestone' credits accordingly.  This new version also has some improved graphics in the screensaver (in my biased opinion :-).<BR>The 'bouncing cube' now shows a plot of your accelerometer on each cube face, the earth view has a nice multitexture of the earth at day and night (if your graphics card supports it), and in screensaver mode it will cycle through these different scenes so you will get some variety.  Also I have made workunits shorter - that way after a 'day' of run-time on QCN, you will get a new workunit and new software versions.  This is an 'optional' upgrade now if you 'Abort' your current workunit or 'Reset Project.'  Once we have a few people running 4.00+ for a few days I will send out an automatic message to upgrade.<BR>",
 "New Version 4.00, With BOINC Credits and Graphics Upgrades"
    ),
   array("October 24, 2008",
"We now have maps to show the recent triggers recorded by participants of the Quake Catcher Network - <A HREF=http://qcn.stanford.edu/sensor/maptrigtime.php>click here to see the triggers recorded for the past day, week, or month</A> - then click again on a region to see further details on Google Maps!<BR>",
 "Trigger Maps for QCN Participants!"
    ),
   array("October 21, 2008",
"The server was down a few hours tonight for some routine maintenance, sorry for the inconvenience!<BR>",
 "Server Maintenance Today"
    ),
   array("October 16, 2008",
"There are new versions (3.70) for Intel & PowerPC Mac OS X, Microsoft Windows, and (yes!) Linux.  The Windows version supports the MotionNode Accel USB accelerometer, and all versions support the JoyWarrior24F8 USB accelerometer (Mac requires the driver for the JoyWarrior - see below).  To get a new version hit the 'Reset Project' or 'Task / Abort' button on your BOINC Manager client.<BR>",
 "New Versions (3.70) for Mac, Windows, and Linux; MotionNode Accel USB support for Windows"
    ),
   array("October 4, 2008",
"There are new versions (3.47) for Intel & PowerPC Mac OS X and Windows that should fix some problems with stability and graphics.  Also, support for the JoyWarrior 24F8 sensor is in place as well.  If you are running BOINC on a Mac, you will need to download a small driver program and install it so that the QCN software running under BOINC can read the JoyWarrior sensor (this is not needed for 'built-in' laptop accelerometers).  <A HREF=http://qcn.stanford.edu/sensor/download/qcnusb-intel.zip>The Intel Mac driver is here</A>  and <A HREF=http://qcn.stanford.edu/sensor/download/qcnusb-ppc.zip>the PowerPC Mac driver is here </A>.  The Intel Mac driver is a hopefully 'easy to follow' installation program; unfortunately the PowerPC Mac driver is a simple shell script you will need to unzip and run in a 'Terminal' window.  JoyWarrior 24F8 support on Windows is also in place, and should not require a separate driver program (at the current time!).  As usual, please report problems/questions in the <A HREF=http://qcn.stanford.edu/sensor/forum_index.php>Message Boards</A>.<BR>",
 "New Versions (3.47) for Mac and Windows, JoyWarrior 24F8 support"
    ),
   array("September 25, 2008",
"This week's issue of 'The Economist' (Sept. 25th '08) has an article on QCN written by Francois Grey of CERN.  You can <A HREF='http://www.economist.com/science/displaystory.cfm?story_id=12295198'>read it online here</A><BR>",
 "QCN article in the 9/25/08 issue of 'The Economist' magazine"
    ),
   array("August 9, 2008",
"Please note if your BOINC running QCN seems to hang or crash.  There is a bug in BOINC for long account files > 64KB -- basically if the historical 
earthquake list gets too large then BOINC will not start and may seem to hang.  Unfortunately the only solution is to quit BOINC, and remove this file 'account_qcn.stanford.edu_sensor.xml' which is located (on Macs) at /Library/Application Support/BOINC Data (or Windows check c:\program files\BOINC).  You should then be able to start BOINC and you will have to reattach/relogin to the QCN project.  Sorry for the inconvenience!<BR>",
 "Note (and fix) for BOINC Crashes"
    ),
    array("July 29, 2008",
"There was a magnitude 5.4 earthquake east of Los Angeles, detected by about 15 machines running QCN.  
<A HREF='http://qcn.stanford.edu/EVENTS/2008_211'>Click here for more details</A>.  Most machines
reported back to the servers with 2-4 seconds of the initial detection trigger.  These results 
(in addition to the previous quake in Reno a few months ago) show great promise for the QCN software!
So thanks and keep running QCN!<BR>",
 "QCN Testers Detect Quake in Los Angeles"
    ),
    array("July 11, 2008",
"New versions (2.81) have been put up for all platforms supported by QCN (Mac Intel, Mac PPC, Windows).
This version fixes some bugs and adds some enhancements to the SAC data produced by the software.  It 
also adds support for the codemercs.com JoyWarrior 24F8 External USB accelerometer on Mac and Windows.
To get the new software, just click 'Reset Project' of the 'Project Tab' on the 'Advanced' menu on BOINC.<BR>",
 "New Version of Software Available (version 2.81)"
    ),
    array("May 30, 2008",
"An alpha version of the QCN software for Windows has been created
which will work on Lenovo Thinkpad laptops.  If you have a Thinkpad 
please try it out and post any comments in the 'Windows' newsgroup.  Thanks!
Also thanks to Ben Suter, Stanford alum, for helping 'crack the code' for
the Thinkpads on QCN!<BR>",
 "QCN BOINC Software for Windows, Lenovo Laptops"
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
    ),
    array("March 27, 2008",
        "There's a <A HREF=\"http://www.nature.com/news/2008/080326/full/452397a.html\">short article about QCN in Nature News Online</a><BR>",
        "Nature News article on QCN"
    ),
    array("February 28, 2008",
        "Thanks to all those taking part in the alpha test!  New versions have been uploaded (version number 2.46) for the Intel and PowerPC Macs, and Windows.  Note the Windows version is still 'graphics only' as the accelerometer work has not begun.  Next stage will be server side analysis to see if triggers match any known seismic events.<BR>",
  "Updated Versions to 2.46"
    ),
    array("February 3, 2008",
        "We are now in alpha testing mode -- currently support Intel and PPC Macs running OS X.  Development under way for Windows (HP laptop accelerometers).",
        "Start of Alpha Test (Intel and PPC Mac OS X).  Please note that global prefs will be set to 1 CPU active, run after idle for 3 minutes - please only attach if you do not mind these default global preferences!"
    ),
    array("December 15, 2007",
        "QCN personnel (Jesse, Elizabeth, and Carl) attended the American Geophysical Society conference this week, and had a poster and demo session 
         showing the current state of the QCN software which will run under BOINC",
        "QCN Demo and Poster at AGU Conference"
    ),
    array("December 5, 2007",
        "Thanks to Dennis Michael of Stanford for procuring and setting up three servers for the project (web, upload, and database).",
        "QCN Has Servers!"
    )
);

?>
