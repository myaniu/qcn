<?php

require_once('../inc/boinc_db.inc');
require_once('../inc/util.inc');
require_once('../project/project_specific_prefs.inc');

db_init();

qcn_generate_world_map($data, 'H');
qcn_generate_world_map($data, 'D');
qcn_generate_world_map($data, 'W');
qcn_generate_world_map($data, 'M');

?>
