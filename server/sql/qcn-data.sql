drop table if exists qcn_host_ipaddr;
drop table if exists qcn_trigger;
drop table if exists qcn_geo_ipaddr;
drop table if exists usgs_quake;
drop table if exists qcn_sensor;

create table qcn_sensor (id smallint not null primary key, is_usb boolean not null default 0, description varchar(64));
insert into qcn_sensor values (0, 0, 'Not Found');
insert into qcn_sensor values (1, 0, 'Mac PPC 1');
insert into qcn_sensor values (2, 0, 'Mac PPC 2');
insert into qcn_sensor values (3, 0, 'Mac PPC 3');
insert into qcn_sensor values (4, 0, 'Mac Intel');
insert into qcn_sensor values (5, 0, 'Lenovo Thinkpad (Windows)');
insert into qcn_sensor values (6, 0, 'HP Laptop (Windows)');
insert into qcn_sensor values (7, 1, 'JoyWarrior 24F8 USB');
insert into qcn_sensor values (8, 1, 'MotionNode Accel USB');

create table qcn_host_ipaddr
(
id int(11) not null primary key auto_increment,
hostid int(11) not null,
ipaddr varchar(32) not null default '',
location varchar(32) not null default '',
latitude double,
longitude double,
geoipaddrid int(11) not null default 0
);

create unique index qcn_host_ipaddr_id on qcn_host_ipaddr (hostid,ipaddr,geoipaddrid);

create table qcn_geo_ipaddr
(
id int(11) not null primary key auto_increment,
ipaddr varchar(32) not null,
time_lookup double,
country varchar(32),
region  varchar(32),
city    varchar(32),
latitude double,
longitude double
);

create unique index qcn_geo_ipaddr_ipaddr on qcn_geo_ipaddr (ipaddr);

create table usgs_quake
(
id int(11) not null primary key auto_increment,
time_utc double,
magnitude double,
depth_km double,
latitude double,
longitude double,
description varchar(256),
processed bool,
url varchar(256),
guid varchar(256)
);

create unique index usgs_quake_guid on usgs_quake (guid);

create table qcn_trigger
(
id int(11) not null primary key auto_increment,
hostid int(11) not null,
ipaddr varchar(32) not null,
result_name varchar(64) not null,
time_trigger double,
time_received double,
time_sync double,
sync_offset double,
significance double,
magnitude double,
latitude double,
longitude double,
depth_km double,
file varchar(64),
dt float,
numreset int(6),
type_sensor int(2),
sw_version float,
usgs_quakeid int(11),
time_filereq double,
received_file tinyint(1),
file_url varchar(128),
runtime_clock double,
runtime_cpu double
);

create index qcn_trigger_time on qcn_trigger (time_trigger desc);
create index qcn_trigger_hostid on qcn_trigger(hostid, time_trigger);
create index qcn_trigger_timelatlng on qcn_trigger(time_trigger, latitude, longitude);
create index qcn_trigger_hostid_filereq on qcn_trigger(hostid,time_filereq,received_file);
create index qcn_trigger_quakeid on qcn_trigger (usgs_quakeid);
create index qcn_trigger_file on qcn_trigger (file);
create index qcn_trigger_result_name on qcn_trigger (result_name,id);

/* temp tables for stats */
CREATE TABLE qcn_recalcresult (resultid int(11) NOT NULL PRIMARY KEY, weight double, total_credit double);
ALTER TABLE qcn_recalcresult ADD INDEX recalc_result (resultid);

CREATE TABLE qcn_stats
         (userid integer, hostid integer, teamid integer,
             resultid integer, total_credit double, weight double);

ALTER TABLE qcn_stats ADD INDEX qcn_stats_userid (userid);
ALTER TABLE qcn_stats ADD INDEX qcn_stats_hostid (hostid);
ALTER TABLE qcn_stats ADD INDEX qcn_stats_teamid (teamid);

CREATE TABLE qcn_finalstats
(
    id int(11) auto_increment NOT NULL PRIMARY KEY,
    hostid int(11) NOT NULL,
    resultid int(11) NOT NULL,
    time_received double NOT NULL DEFAULT 0,
    runtime_clock double NOT NULL DEFAULT 0,
    runtime_cpu double NOT NULL DEFAULT 0
);

ALTER TABLE qcn_finalstats ADD UNIQUE INDEX qcn_finalstats_hostid (hostid,resultid);
ALTER TABLE qcn_finalstats ADD INDEX qcn_finalstats_resultid (resultid);

/* Now generate the stored procedures */

SOURCE do_final_trigger.sql;

SOURCE do_stats.sql;


