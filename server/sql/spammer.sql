create temporary table spammer (select userid from profile where response1 like '%zoloft%');
delete from profile where userid in (select userid from spammer);
delete from user where id in (select userid from spammer);
drop table spammer;

