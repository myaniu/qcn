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

$hostid = $_GET["hostid"];
if ( ! $hostid) {
  $user = get_logged_in_user(true);
  $hostid = $user->id;
  $ipprivate = true;
} else {
  check_get_args(array("hostid", "ipprivate"));
// Requested host id
  $hostid = get_int("hostid");
  $ipprivate = false;
}

BoincDb::get(true);

if ($user->id==$hostid) $auth = true;

if (!$ipprivate) $ipprivate = get_str("ipprivate", true);
$host = BoincHost::lookup_id($hostid);
if (!$host) {
    echo "Couldn't find computer (please hit \"back\")";
    exit();
}

$user = get_logged_in_user(true);


// Check if the user is on the administrative list:
if (!$ipprivate) $ipprivate = qcn_admin_user_check($user);
// If the user is the owner of the host, then provide private info
if ($user->hostid == $hostid && !$ipprivate) {
   $ipprivate = true;
}

if ($user->id != $host->userid) {
    $user = null;
}

page_head("Computer $hostid");



show_host($host, $user, $ipprivate);
show_trigger($host->id,$heading,$ipprivate);

page_tail();

?>
