<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

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
    echo "<div id=\"mainnav\">
        <h2>About ".PROJECT."</h2>
        " . PROJECT. " is a research project that uses Internet-connected
        computers to do research in seismology.
        You can participate by downloading and running a free program
        on your computer.
        <p>
        This project is based at Stanford University, in collaboration with the University of California at Riverside.
        <ul>
        <li> <A HREF='http://qcn.stanford.edu'>QCN Stanford Web Page</A>
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
        </ul>

        <h2>Returning participants</h2>
        <ul>
        <li><a href=\"home.php\">Your account</a> - view stats, modify preferences
        <li><a href=\"team.php\">Teams</a> - create or join a team
        <li><a href=\"cert1.php\">Certificate</a>
        <li> <a href=\"apps.php\">".tra("Applications")."</a>

        </ul>
        <h2>".tra("Community")."</h2>
        <ul>
        <li><a href=\"profile_menu.php\">".tra("Profiles")."</a>
        <li><a href=\"user_search.php\">User search</a>
        <li><a href=\"forum_index.php\">".tra("Message boards")."</a>
        <li><a href=\"forum_help_desk.php\">".tra("Questions and Answers")."</a>
        <li><a href=\"stats.php\">Statistics</a>
        <li><a href=language_select.php>Languages</a>
        </ul>";

if ($user->donated) {
  echo "
        <h2>".tra("Extra Links")."</h2>
        <ul>
        <li><a href=\"trig.php\">".tra("Search Triggers")."</a>
        <li><a href=\"trig.php?cbUseLat=1&cbUseTime=1&LatMin=-39&LatMax=-30&LonMin=-76&LonMax=-69\">".tra("Search Triggers (Chile RAMP Area)")."</a>
        <li><a href=\"dl.php\">".tra("Download Trigger Data")."</a>
        <li><a href=\"dl.php?cbUseLat=1&cbUseTime=1&LatMin=-39&LatMax=-30&LonMin=-76&LonMax=-69\">".tra("Download Trigger Data (Chile RAMP Area)")."</a>
        ";

     // database backup
     $backupfile = "data/qcn-backup-continual.sql.gz";
     $fsizebackup = filesize($backupfile);
     if ($fsizebackup) {
         echo "   <li><a href=\"$backupfile\">" .
              sprintf("Full Database Backup Dated %s UTC  (Size %9.2f MB)", date ("F d Y H:i:s", filemtime($backupfile)), 
                $fsizebackup / 1.048576e6)  . "</a>";
     }

     echo "   </ul>";
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

$charset = tra("CHARSET");

if ($charset != "CHARSET") {
    header("Content-type: text/html; charset=$charset");
}

echo "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">";

echo "<html>
    <head>
    <title>".PROJECT."</title>
	<link rel=\"stylesheet\" type=\"text/css\" href=\"main.css\" media=\"all\" />
    <link rel=\"stylesheet\" type=\"text/css\" href=\"".STYLESHEET."\">
    <link rel=\"alternate\" type=\"application/rss+xml\" title=\"".$rssname."\" href=\"".$rsslink."\">
";
include 'schedulers.txt';
echo "
    </head><body>
    <span class=page_title>".PROJECT."</span>
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
    <a href=\"http://boinc.berkeley.edu/\"><img align=\"middle\" border=\"0\" src=\"img/pb_boinc.gif\" alt=\"Powered by BOINC\"></a>
    </p>
    </td>
";

if (!$stopped) {
    $profile = get_current_uotd();
    if ($profile) {
        echo "
            <td id=\"uotd\">
            <h2>".tra("User of the day")."</h2>
        ";
        show_uotd($profile);
        echo "</td></tr>\n";
    }
}

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
    <a href=\"rss_main.php\">RSS feed</a> <img src=\"img/rss_icon.gif\" alt=\"RSS\">.</p>
    </td>
    </tr></table>
";


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
        <area shape=\"rect\" coords=\"231,238,442,510\" href=\"maptrig.php?cx=-37&cy=-70\">
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



if ($caching) {
    page_tail_main(true);
    end_cache(INDEX_PAGE_TTL);
} else {
    page_tail_main();
}

?>
