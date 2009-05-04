select count(distinct hostid) ctr, yearweek(from_unixtime(time_received)) yw
from qcn_trigger
where yearweek(from_unixtime(time_received))
  between 200826 and yearweek(now())-1
group by yearweek(from_unixtime(time_received)) 
order by yearweek(from_unixtime(time_received)) desc;
