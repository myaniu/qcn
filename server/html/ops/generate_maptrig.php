<?php

require_once('../inc/boinc_db.inc');
require_once('../inc/util.inc');
require_once('../project/project_specific_prefs.inc');

db_init();

qcn_generate_world_map($data, 'H');
qcn_generate_world_map($data, 'D');
qcn_generate_world_map($data, 'W');
qcn_generate_world_map($data, 'M');

// NEW -- clean out the qcn_trigger_memory table - latest 10 minutes of triggers should be plenty
  $query = "delete from trigmem.qcn_trigger_memory where time_trigger>(unix_timestamp() - (10*60))";
  $result = mysql_query($query);
 
  // free memory
  $query = "ALTER TABLE trigmem.qcn_trigger_memory ENGINE=MEMORY";
  $result = mysql_query($query);
  

?>
