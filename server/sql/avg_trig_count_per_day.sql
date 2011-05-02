select hostid, 
   s.description sensor, 
   concat(year(from_unixtime(time_trigger)), dayofyear(time_trigger))  year_day,
   count(*) trig_per_day
from qcn_trigger t, qcn_sensor s
   where t.type_sensor = s.id and varietyid=0
 group by hostid, sensor, year_day;


