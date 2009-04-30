select count(distinct hostid), yearweek(from_unixtime(time_received)) 
from qcn_trigger group by yearweek(from_unixtime(time_received)) 
order by yearweek(from_unixtime(time_received)) desc;
