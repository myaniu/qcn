<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/news.inc");
require_once("../inc/cache.inc");
require_once("../inc/uotd.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/translation.inc");
require_once("../inc/text_transform.inc");
require_once("../project/project.inc");
require_once("../project/project_news.inc");


function show_nav() {
    $config = get_config();
    $master_url = parse_config($config, "<master_url>");
    $user = get_logged_in_user(false);
    if (substr($master_url, -1, 1) == "/") {
       $master_url = substr($master_url, 0, strlen($master_url)-1);
    }

    echo "<div id=\"mainnav\">
        <h2>About ".PROJECT."</h2>
        The Quake Catcher Network (QCN) is a research project that uses Internet-connected
        computers to do research, education, and outreach in seismology.
        You can participate by downloading and running a free program
        on your computer.  Currently only certain Mac (OS X) PPC and Intel laptops are supported --
        recent ones which have a built-in accelerometer.
        <p>
        QCN is a joint project between Stanford University and University of California at Riverside.
        <ul>
        <li> <A HREF=\"http://qcn-web.stanford.edu/index.html\">Quake Catcher Network Home Page</A>
        <li> <A HREF=\"http://qcn.stanford.edu/about/contact.html\">Project Personnel</A>
        </ul>
        <h2>Join ".PROJECT."</h2>
        <ul>
        <li><a href=\"info.php\">".tra("Read our rules and policies")."</a>
        <li> This project uses BOINC.
            If you're already running BOINC, select Attach to Project.
            If not, <a target=\"_new\" href=\"http://boinc.berkeley.edu/download.php\">download BOINC</a>.
        <li> When prompted, enter <br><b>".$master_url."</b>
        <li> If you're running a command-line or pre-5.0 version of BOINC,
            <a href=\"create_account_form.php\">create an account</a> first.
        <li> If you have any problems,
            <a target=\"_new\" href=\"http://boinc.berkeley.edu/help.php\">get help here</a>.
        <li> <a href=\"server_status.php\">Server Status Page</a>.
        </ul>

        <h2>Returning participants</h2>
        <ul>
        <li><a href=\"home.php\">Your account</a> - view stats, modify preferences
        <li><a href=\"team.php\">Teams</a> - create or join a team
        <li><a href=\"cert1.php\">Certificate</a>
        <li> <a href=\"apps.php\">".tra("Applications")."</a>
        </ul>
        <h2>Community</h2>
        <ul>
        <li><a href=\"profile_menu.php\">Profiles</a>
        <li><a href=\"user_search.php\">User search</a>
        <li><a href=\"forum_index.php\">Message boards</a>
        <li><a href=\"stats.php\">Statistics</a>
        </ul>";

// CMC changed to forum prefs 4th bit
  if (qcn_admin_user_auth($user)) {  // defined in project/project.inc
       echo "
        <h2>".tra("Extra Links")."</h2>
        <ul>
        <li><a href=\"trig.php\">".tra("Search Triggers")."</a>
        <li><a href=\"dl.php\">".tra("Download Trigger Data")."</a>
        <li><a href=\"ramp.php\">".tra("View RAMP Signups")."</a>
        <li><a href=\"http://qcn.stanford.edu/sensor_ops/todo\">".tra("To-Do List")."</a>";

        //if ($user->id == 15) {
        // check for db replication timestamp
        $kewfile = "/var/www/boinc/sensor/html/user/max.txt";
        if (file_exists($kewfile) && ($handle = fopen($kewfile, 'r'))) {
              $output = fgets($handle); // skip first line
              $output = fgets($handle);              fclose($handle);
              echo "        <li>Kew Sync Diff (seconds): " . $output . "<BR>(should be a small number else server is down)
<BR>";
        }        else {
              echo "        <li>No Replication Sync File on Kew - Better Check!";
    }

  echo "</ul>";
}

echo "
        </div>
    ";
}

$caching = false;

if ($caching) {
    start_cache(INDEX_PAGE_TTL);
}

$stopped = web_stopped();
$rssname = PROJECT . " RSS 2.0" ;
$rsslink = URL_BASE . "rss_main.php";

if (defined("CHARSET")) {
    header("Content-type: text/html; charset=".tr(CHARSET));
}

echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/1999/REC-html401-19991224/loose.dtd\">";
echo "<html>
    <head>
    <title>".PROJECT."</title>
    <link rel=\"stylesheet\" type=\"text/css\" href=\"qcn.css\">
    <link rel=\"alternate\" type=\"application/rss+xml\" title=\"".$rssname."\" href=\"".$rsslink."\">
    </head><body>
    <h1>".PROJECT."</h1>
    <table cellpadding=\"8\" cellspacing=\"4\">
    <tr><td rowspan=\"2\" valign=\"top\" width=\"40%\">
";

if ($stopped) {
    echo "
        <b>".PROJECT." is temporarily shut down for maintenance.
        Please try again later</b>.
    ";
} else {
    db_init();
    show_nav();
}

echo "
    <p>
    <a href=\"http://boinc.berkeley.edu/\"><img align=\"middle\" border=\"0\" src=\"img/pb_boinc.gif\" alt=\"BOINC Logo\"></a>
    </p>
    </td>
";

/*
if (!$stopped) {
    $profile = get_current_uotd();
    if ($profile) {
        echo "
            <td id=\"uotd\">
            <h2>User of the day</h2>
        ";
        show_uotd($profile);
        echo "</td></tr>\n";
    }
}
*/

echo "
    <tr><td id=\"news\">
    <h2>News</h2>
    <p>
";
show_news($project_news, 5);
if (count($project_news) > 5) {
    echo "<a href=\"old_news.php\">...more</a>";
}

echo "
    <p class=\"smalltext\">
    News is available as an
    <a href=\"rss_main.php\">RSS feed</a> <img src=\"img/xml.gif\" alt=\"XML\">.</p>
    </td>
    </tr></table>";

// begin map stuff

echo "<table>
    <tr><p><td>Latest Triggers Recorded (Past 4 Hours) - Generated on " . date("F d Y H:i:s", filectime(MAP_TRIGGER)) . " UTC</td></tr>";

echo "
    <tr><p><td><A HREF=maptrig.php>Click here or on a region of the map for an interactive Google map</A></td></tr>
    <tr><p><td>Legend: Blue triangle = QCN participant trigger, Red circle = Earthquake of minimum magnitude " . MIN_MAGNITUDE . "</td></tr>
    <tr><p><td><i>Note: locations changed at the kilometer-level to protect privacy, unless participant authorized exact location be used</i></td</tr>
     ";

echo "
<tr><td><IMG SRC=\"" . MAP_TRIGGER . "\" usemap=\"#" . MAP_TRIGGER . "\" border=\"0\"></td></tr>
<map name=\"" . MAP_TRIGGER . "\">
        <area shape=\"rect\" coords=\"0,2,225,232\" href=\"maptrig.php?cx=38&cy=-120\">
        <area shape=\"rect\" coords=\"2,236,228,511\" href=\"maptrig.php?cx=-20&cy=-120\">
        <area shape=\"rect\" coords=\"227,3,428,234\" href=\"maptrig.php?cx=38&cy=-70\">
        <area shape=\"rect\" coords=\"231,238,442,510\" href=\"maptrig.php?cx=-20&cy=-70\">
        <area shape=\"rect\" coords=\"430,3,605,237\" href=\"maptrig.php?cx=50&cy=1\">
        <area shape=\"rect\" coords=\"445,241,732,510\" href=\"maptrig.php?cx=-10&cy=5\">
        <area shape=\"rect\" coords=\"609,3,803,239\" href=\"maptrig.php?cx=38&cy=80\">
        <area shape=\"rect\" coords=\"735,244,1022,511\" href=\"maptrig.php?cx=-20&cy=140\">
        <area shape=\"rect\" coords=\"806,3,1021,243\" href=\"maptrig.php?cx=38&cy=140\">
</map>
    ";

echo "
    <tr><p><td><A HREF=maptrigtime.php>Click here for trigger maps for the past day / week / month</A></td></tr>";

// end map stuff

/*
echo "
    <tr><p><td><BR></td></p></tr>
    <tr><p><td>Click on an image below for a full screen picture!</td></p></tr>
    <tr><p><td><A HREF=\"img/qcn-sensor-3d.jpg\"><IMG SRC=img/qcn-sensor-3d-sm.jpg></A></p></td></tr>
    <tr><p><td><A HREF=\"img/qcn-sensor-2d.jpg\"><IMG SRC=img/qcn-sensor-2d-sm.jpg></A></p></td></tr>
    <tr><p><td><A HREF=\"img/qcn-earth-night.jpg\"><IMG SRC=img/qcn-earth-night-sm.jpg></A></p></td></tr>
    <tr><p><td><A HREF=\"img/qcn-earth-day-nz-quake.jpg\"><IMG SRC=img/qcn-earth-day-nz-quake-sm.jpg></A></p></td></tr>
    </table>
";
*/

echo "<tr><td>
    <p>
    <img src=\"img/weekly.png\" alt=\"Weekly QCN Participant Machines\">
    </p>
    </td></tr>";

echo "</table>\n";

include 'schedulers.txt';

if ($caching) {
    page_tail_main(true);
    end_cache(INDEX_PAGE_TTL);
} else {
    page_tail_main();
}

?>
