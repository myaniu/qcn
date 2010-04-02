
create temporary table tmp_setip 
   (select id from continual.qcn_host_ipaddr where (geoipaddrid is null or geoipaddrid=0) and ipaddr is not null and ipaddr!=''
        and hostid not in (select hostid from continual.qcn_host_ipaddr where ipaddr is null or ipaddr='')
        and hostid not in (select hostid from continual.qcn_host_ipaddr where (geoipaddrid is null or geoipaddrid=0) group by hostid having count(*)>1)
);

update continual.qcn_host_ipaddr set ipaddr='' where id in (select id from tmp_setip);


