select db_hostid, latitude, longitude from 
(
select concat('Q', hostid) db_hostid , latitude,longitude 
from qcnalpha.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid, latitude, longitude
union 
select concat('Q', hostid) db_hostid, latitude,longitude 
from qcnarchive.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid, latitude, longitude
union
select concat('C', hostid) db_hostid, latitude,longitude 
from continual.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid, latitude, longitude
union
select concat('C', hostid) db_hostid, latitude,longitude 
from contarchive.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
group by db_hostid, latitude, longitude
) b
group by db_hostid, latitude, longitude;
