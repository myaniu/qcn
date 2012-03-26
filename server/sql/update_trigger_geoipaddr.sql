update qcn_trigger t
set hostipaddrid=
IFNULL((select id from qcn_host_ipaddr i 
   where i.hostid=t.hostid
     and i.latitude=t.latitude and i.longitude=t.longitude LIMIT 1), 0)
, geoipaddrid=
IFNULL((select geoipaddrid from qcn_host_ipaddr i 
   where i.hostid=t.hostid
     and i.latitude=t.latitude and i.longitude=t.longitude LIMIT 1), 0)
where t.hostipaddrid = 0;


