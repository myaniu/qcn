select db_hostid, max_trigid, latitude, longitude from 
(
select concat('Q', hostid) db_hostid , max(id) max_trigid, latitude,longitude 
from qcnalpha.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid
union 
select concat('Q', hostid) db_hostid, max(id) max_trigid, latitude,longitude 
from qcnarchive.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid
union
select concat('C', hostid) db_hostid, max(id) max_trigid, latitude,longitude 
from continual.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid
union
select concat('C', hostid) db_hostid, max(id) max_trigid, latitude,longitude 
from contarchive.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid
) b
group by db_hostid;
