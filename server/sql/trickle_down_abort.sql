insert into qcnalpha.msg_to_host
(create_time,hostid,variety,handled,xml)
select unix_timestamp(), hostid, 'abort', 0,
concat('<trickle_down>\n<result_name>',
  result_name,
   '</result_name>\n<abort></abort>\n</trickle_down>\n')
  from qcn_trigger where time_trigger>=unix_timestamp()-(86400*7)
    group by hostid,result_name
;
