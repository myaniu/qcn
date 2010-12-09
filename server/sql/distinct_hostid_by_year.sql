select distinct(db_hostid), latitude, longitude from 
(
select distinct(concat('Q', hostid)) db_hostid , latitude,longitude from qcnalpha.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
union
select distinct(concat('Q', hostid)) db_hostid, latitude,longitude from qcnarchive.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
union
select distinct(concat('C', hostid)) db_hostid, latitude,longitude from continual.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
union
select distinct(concat('C', hostid)) db_hostid, latitude,longitude from contarchive.qcn_trigger where time_trigger > unix_timestamp('2010-01-01')
) b
;

