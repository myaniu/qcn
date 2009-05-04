select yearweek(from_unixtime(time_received)) yw, count(distinct hostid) ctr
from qcn_trigger
where yearweek(from_unixtime(time_received))
  between 
      date(date_sub(date_sub(now(), interval 180 day), 
         interval dayofweek(date(date_sub(date_sub(now(), interval 180 day), 
             interval dayofweek(date_sub(now(), interval 180 day))-1 day)))-1 day))
    and 
      date(date_sub(now(), interval dayofweek(now())-1 day))
group by yearweek(from_unixtime(time_received)) 
order by yearweek(from_unixtime(time_received))
;
