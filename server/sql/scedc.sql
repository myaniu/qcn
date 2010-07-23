select 'Q' db,t.* from qcnalpha.qcn_trigger t
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00') 
and varietyid in (0,2)
and latitude between 31.5 and 37.5 and longitude between -121 and -114
UNION
select 'C' db,tt.* from continual.qcn_trigger tt
where time_trigger between unix_timestamp('2010-04-04 00:00:00') and unix_timestamp('2010-04-07 00:00:00') 
and varietyid in (0,2)
and latitude between 31.5 and 37.5 and longitude between -121 and -114
;
