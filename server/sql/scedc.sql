select 
  concat('Q',t.id, ',' , from_unixtime(time_trigger), ',',
    magnitude, ',', significance, ',', s.description, ',',
    latitude, ',', longitude, ',', file)
 from qcnalpha.qcn_trigger t, qcnalpha.qcn_sensor s
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00') 
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between 31.5 and 37.5 and longitude between -121 and -114
UNION
select
  concat('C',tt.id, ',' , from_unixtime(time_trigger), ',',
    magnitude, ',', significance, ',', s.description, ',',
    latitude, ',', longitude, ',', file)from continual.qcn_trigger tt, continual.qcn_sensor s
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00') 
and time_sync>0
and varietyid in (0,2)
and received_file=100
and latitude between 31.5 and 37.5 and longitude between -121 and -114
;

