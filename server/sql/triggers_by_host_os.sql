select h.os_name, h.os_version, count(distinct hostid) 
from qcnarchive.qcn_trigger t, host h 
where t.hostid=h.id 
 group by os_name,os_version
;
