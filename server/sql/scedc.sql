select 
  concat(m.mydb, ',' , m.id, ',', from_unixtime(m.time_trigger), ',',
    m.magnitude, ',', m.significance, ',', s.description, ',',
    m.latitude, ',', m.longitude, ',', m.file)
from
(
select 'Q' mydb, t.*
from qcnalpha.qcn_trigger t
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00') 
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between 31.5 and 37.5 and longitude between -121 and -114

UNION

select 'C' mydb, tt.*
from continual.qcn_trigger tt
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00') 
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between 31.5 and 37.5 and longitude between -121 and -114
) m, qcnalpha.qcn_sensor s

where m.type_sensor=s.id
order by time_trigger
;
