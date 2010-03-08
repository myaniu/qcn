drop table if exists qcn_host_ipaddr;
drop table if exists qcn_trigger;
drop table if exists qcn_geo_ipaddr;
drop table if exists usgs_quake;
drop table if exists qcn_sensor;
drop table if exists qcn_level;
drop table if exists qcn_align;
drop table if exists qcn_variety;

create table qcn_sensor (id smallint not null primary key, is_usb boolean not null default 0, description varchar(64));
insert into qcn_sensor values (0, 0, 'Not Found');
insert into qcn_sensor values (1, 0, 'Mac PPC 1');
insert into qcn_sensor values (2, 0, 'Mac PPC 2');
insert into qcn_sensor values (3, 0, 'Mac PPC 3');
insert into qcn_sensor values (4, 0, 'Mac Intel');
insert into qcn_sensor values (5, 0, 'Lenovo Thinkpad (Windows)');
insert into qcn_sensor values (6, 0, 'HP Laptop (Windows)');
insert into qcn_sensor values (100, 1, 'JoyWarrior 24F8 USB');
insert into qcn_sensor values (101, 1, 'MotionNode Accel USB');
insert into qcn_sensor values (102, 1, 'O1 USB');

create table qcn_level (id smallint not null primary key, description varchar(64));
insert into qcn_level values (0, 'N/A');
insert into qcn_level values (1, 'Floor (+/- above/below surface)');
insert into qcn_level values (2, 'Meters (above/below surface)');
insert into qcn_level values (3, 'Feet (above/below surface)');
insert into qcn_level values (4, 'Elevation - meters above sea level');
insert into qcn_level values (5, 'Elevation - feet above sea level');

create table qcn_align (id smallint not null primary key default 0, description varchar(64));
insert into qcn_align values (0, 'Unaligned');
insert into qcn_align values (1, 'North');
insert into qcn_align values (2, 'South');
insert into qcn_align values (3, 'East');
insert into qcn_align values (4, 'West');
insert into qcn_align values (5, 'Wall');

create table qcn_variety (id smallint not null primary key default 0, description varchar(64));
insert into qcn_variety values (0, 'Normal Trigger');
insert into qcn_variety values (1, 'Ping Trigger');
insert into qcn_variety values (2, 'Continual Trigger');


create table qcn_host_ipaddr
(
id int(11) not null primary key auto_increment,
hostid int(11) not null,
ipaddr varchar(32) not null default '',
location varchar(32) not null default '',
latitude double,
longitude double,
levelvalue float,
levelid smallint,
alignid smallint,
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
alter table usgs_quake add index usgs_quake_magnitude (magnitude);
alter table usgs_quake add index usgs_time_utc (time_utc);
alter table usgs_quake add index usgs_latitude (latitude);
alter table usgs_quake add index usgs_longitude (longitude);

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
levelvalue float,
levelid smallint,
alignid smallint,
file varchar(64),
dt float,
numreset int(6),
type_sensor int(3),
sw_version varchar(8),
os_type varchar(8),
usgs_quakeid int(11),
time_filereq double,
received_file tinyint(1),
file_url varchar(128),
runtime_clock double,
runtime_cpu double,
varietyid smallint not null default 0,
flag boolean not null default 0
);

create index qcn_trigger_time on qcn_trigger (time_trigger desc, ping asc);
create index qcn_trigger_hostid on qcn_trigger(hostid, time_trigger, ping);
create index qcn_trigger_timelatlng on qcn_trigger(time_trigger, latitude, longitude, ping);
create index qcn_trigger_hostid_filereq on qcn_trigger(hostid,time_filereq,received_file);
create index qcn_trigger_quakeid on qcn_trigger (usgs_quakeid);
create index qcn_trigger_file on qcn_trigger (file);
create index qcn_trigger_result_name on qcn_trigger (result_name,id,ping);
create index qcn_trigger_type_sensor on qcn_trigger (type_sensor);
create index qcn_trigger_usgs_quakeid on qcn_trigger (usgs_quakeid);
create index qcn_trigger_flag on qcn_trigger (flag);

/* temp tables for stats */
CREATE TABLE qcn_recalcresult (resultid int(11) NOT NULL PRIMARY KEY, weight double, total_credit double, time_received double);
ALTER TABLE qcn_recalcresult ADD INDEX recalc_result (resultid);

CREATE TABLE qcn_stats
         (userid integer, hostid integer, teamid integer,
             resultid integer, total_credit double, weight double, expavg_time double);

ALTER TABLE qcn_stats ADD INDEX qcn_stats_userid (userid);
ALTER TABLE qcn_stats ADD INDEX qcn_stats_hostid (hostid);
ALTER TABLE qcn_stats ADD INDEX qcn_stats_teamid (teamid);

CREATE TABLE qcn_finalstats
(
    id int(11) auto_increment NOT NULL PRIMARY KEY,
    resultid int(11) NOT NULL,
    time_received double NOT NULL DEFAULT 0,
    runtime_clock double NOT NULL DEFAULT 0,
    runtime_cpu double NOT NULL DEFAULT 0
);

ALTER TABLE qcn_finalstats ADD UNIQUE INDEX qcn_finalstats_resultid (resultid);

/* make the index for result(random) */
ALTER TABLE result ADD INDEX result_random (random);

create table qcn_showhostlocation (hostid int(11) primary key not null);

/* Now generate the stored procedures */

SOURCE do_final_trigger.sql;

SOURCE do_stats.sql;


