<?php

// Forum index
// shows the categories and the forums in each category

require_once('../inc/forum.inc');
require_once('../inc/pm.inc');
require_once('../inc/time.inc');
require_once('../inc/utils.inc');


$user = get_logged_in_user(false);

// Process request to mark all posts as read
//
if ((get_int("read", true) == 1)) {
    if ($user) {
        check_tokens($user->authenticator);
        BoincForumPrefs::lookup($user);
        $now = time();
        $user->prefs->update("mark_as_read_timestamp=$now");
        Header("Location: ".get_str("return", true));
    }
}

function show_forum_summary($forum) {
    switch ($forum->parent_type) {
    case 0:
        $t = $forum->title;
        $d = $forum->description;
        break;
    case 1:
        $team = BoincTeam::lookup_id($forum->category);
        $t = $forum->title;
        if (!strlen($t)) $t = $team->name;
        $d = $forum->description;
        if (!strlen($d)) $d = "Discussion among members of $team->name";
        break;
    }
    echo "
        <tr class=\"row1\">
        <td width=40%>
            <em>
            <a href=\"forum_forum.php?id=$forum->id\">$t
            </a></em>
            <br><span class=\"smalltext\">$d</span>
        </td>
        <td width=20%>$forum->threads</td>
        <td width=20%>$forum->posts</td>
        <td width=20%>".time_diff_str($forum->timestamp, time())."</td>
    </tr>";
}

page_head(tra("Message boards"));
show_forum_header($user);

echo "<p>
    Talk live via Skype with a volunteer, in any of several languages.
    Go to
    <a href=\"http://boinc.berkeley.edu/help.php\">BOINC Online Help</a>.</p>
";

echo "<H2>Help Desk and Message Boards</H2>\n";

$categories = BoincCategory::enum("is_helpdesk=1 order by orderID");
$first = true;
foreach ($categories as $category) {
    if ($first) {
        $first = false;
        show_forum_title($category, null, null);
        echo "<p>";
        start_forum_table(
            array(tra("Topic"), tra("Questions"), tra("Replies"), tra("Last post"))
        );
        //start_forum_table(array("Topic", "# Questions", "Last post"));
    }
    if (strlen($category->name)) {
        echo "
            <tr class=\"subtitle\">
            <td class=\"category\" colspan=\"4\">", $category->name, "</td>
            </tr>
        ";
    }

    $forums = BoincForum::enum("parent_type=0 and category=$category->id order by orderID");
    foreach ($forums as $forum) {
        show_forum_summary($forum);
        /* 
        echo "
        <tr class=\"row1\">
        <td>
            <b><a href=\"forum_forum.php?id=$forum->id\">$forum->title</a></b>
            <br>", $forum->description, "
        </td>
        <td>", $forum->threads, "</td>
        <td>", $forum->posts, "</td>
        <td>", time_diff_str($forum->timestamp, time()), "</td>
    </tr>
        "; */
    }
}

end_table();
echo "<BR><BR>\n";

$categories = BoincCategory::enum("is_helpdesk=0 order by orderID");
$first = true;
foreach ($categories as $category) {
    if ($first) {
        $first = false;
        show_forum_title($category, NULL, NULL);
        if ($user) {
            $return = urlencode(current_url());
            $tokens = url_tokens($user->authenticator);
            $url = "forum_index.php?read=1$tokens&return=$return";
            show_button($url, "Mark all threads as read", "Mark all threads in all message boards as 'read'.");
        }

        echo "<p>";
        start_forum_table(
            array(tra("Topic"), tra("Threads"), tra("Posts"), tra("Last post"))
        );
    }
    if (strlen($category->name)) {
        echo '
            <tr class="subtitle">
            <td class="category" colspan="4">'.$category->name.'</td>
            </tr>
        ';
    }
    $forums = BoincForum::enum("parent_type=0 and category=$category->id order by orderID");
    foreach ($forums as $forum) {
        show_forum_summary($forum);
    }
}

if ($user && $user->teamid) {
    $forum = BoincForum::lookup("parent_type=1 and category=$user->teamid");
    if ($forum) {
        show_forum_summary($forum);
    }
}
end_table();

if ($user) {
    $subs = BoincSubscription::enum("userid=$user->id");
    if (count($subs)) {
        echo "<h3>Subscribed threads</h2>";
        show_thread_and_context_header();
        $i = 0;
        foreach ($subs as $sub) {
            $thread = BoincThread::lookup_id($sub->threadid);
            if ($thread->hidden) continue;
            show_thread_and_context($thread, $user, $i);
            $i++;
        }
        end_table();
    }
}

//page_tail();
page_bot();
flush();
BoincForumLogging::cleanup();

$cvs_version_tracker[]="\$Id: forum_index.php 16075 2008-09-27 08:19:30Z jbk $";  //Generated automatically - do not edit
?>
