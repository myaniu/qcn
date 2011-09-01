select db,hostid,latitude,longitude, num_24hrs from
(select 'Q' db,hostid,latitude,longitude, count(*) num_24hrs 
from qcnalpha.qcn_trigger 
where time_trigger > unix_timestamp() - (3600*24) group by 'Q', hostid, latitude, longitude
UNION
select 'C' db, hostid,latitude,longitude, count(*) num_24hrs 
from continual.qcn_trigger 
where time_trigger > unix_timestamp() - (3600*24) group by 'C', hostid, latitude, longitude
) qcn_24hrs
order by db, hostid, latitude, longitude;

