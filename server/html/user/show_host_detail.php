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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/host.inc");

$hostid = get_int("hostid", false);
if ( ! $hostid) {
  exit();
}

$host = BoincHost::lookup_id($hostid);
if (!$host) {
    echo "Couldn't find computer (please hit \"back\")";
    exit();
}

$user = get_logged_in_user(true);

$owner = false; // default to true ie privacy on

// Check if the user is on the administrative list:
//if (!$owner) $owner = qcn_admin_user_check($user);

// If the user is the owner of the host, then provide private info
if (!$owner && $host->userid == $user->id) $owner = true;

page_head("Computer $hostid");

show_host($host, $user, $owner);
show_trigger($host->id,$heading, $owner);

page_tail();

?>
