drop table if exists qcn_host_ipaddr;
drop table if exists qcn_trigger;
drop table if exists qcn_trigger_followup;
drop table if exists trigmem.qcn_trigger_memory;
drop table if exists trigmem.qcn_trigger_followup;
drop database trigmem;
drop table if exists qcn_geo_ipaddr;
drop table if exists qcn_quake;
drop table if exists qcn_sensor;
drop table if exists qcn_level;
drop table if exists qcn_align;
drop table if exists qcn_variety;
drop table if exists qcn_dluser;
drop table if exists qcn_post;
drop table if exists qcn_ramp_participant;
drop table if exists qcn_ramp_coordinator;
drop table if exists qcn_constant;
drop table if exists qcn_kml_region;

create table qcn_constant (
    id int not null primary key auto_increment,
    description varchar(64) not null,
    value_int int,
    value_text varchar(64),
    value_float float
);
create index qcn_constant_description on qcn_constant(description);
insert into qcn_constant (description, value_int) values ('ArchiveTime', unix_timestamp());

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
insert into qcn_sensor values (102, 1, 'ONavi 1 USB');
insert into qcn_sensor values (103, 1, 'JoyWarrior 24F14 USB');
insert into qcn_sensor values (104, 1, 'ONavi A 12-bit USB');
insert into qcn_sensor values (105, 1, 'ONavi A 16-bit USB');
insert into qcn_sensor values (106, 1, 'ONavi A 24-bit USB');

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
insert into qcn_variety values (-2, 'Final Stats Trigger');
insert into qcn_variety values (-1, 'Quakelist Trigger');
insert into qcn_variety values (0, 'Normal Trigger');
insert into qcn_variety values (1, 'Ping Trigger');
insert into qcn_variety values (2, 'Continual Trigger');

create table qcn_dluser (userid int not null primary key);

create table qcn_ramp_coordinator (
    id int not null primary key,
    userid int not null,
    receive_distribute boolean,
    help_troubleshoot boolean,
    enlist_volunteers boolean,
    how_many int,
    active boolean not null default 1,
    comments varchar(255),
    time_edit int 
);
create unique index qcn_ramp_coordinator_userid on qcn_ramp_coordinator(userid);

create table qcn_ramp_participant (
    id int not null primary key auto_increment,
    userid int not null,
    qcn_ramp_coordinator_id int,
    fname varchar(64),
    lname varchar(64),
    email_addr varchar(100),
    addr1 varchar(64),
    addr2 varchar(64),
    city varchar(64),
    region varchar(64),
    postcode varchar(20),
    country varchar(64),
    latitude double,
    longitude double,
    gmap_placename varchar(64),
    gmap_view_level int,
    gmap_view_type int,
    phone varchar(64),
    fax varchar(64),
    bshare_coord boolean,
    bshare_map boolean,
    bshare_ups boolean,
    cpu_type varchar(20),
    cpu_os varchar(20),
    cpu_age int,
    cpu_floor int,
    cpu_admin boolean,
    cpu_permission boolean,
    cpu_firewall boolean, 
    cpu_proxy boolean,
    cpu_internet boolean,
    cpu_unint_power boolean,
    sensor_distribute boolean,
    comments blob,
    loc_home boolean not null default 0,
    loc_business boolean not null default 0,
    loc_affix_perm boolean not null default 0,
    loc_self_install boolean not null default 0,
    loc_weekend_install boolean not null default 0,
    loc_time_hour install smallint,
    loc_time_minute install smallint,
    loc_years_host smallint,
    regional boolean not null default 0,
    active boolean not null default 1,
    time_edit int,
    kml_regionid int not null default 0
);
create unique index qcn_ramp_participant_userid on qcn_ramp_participant(userid);
create index qcn_ramp_participant_kml_regionid on qcn_ramp_participant(kml_regionid);

create table qcn_kml_region
(
  id int primary key auto_increment,
  name varchar(64),
  filename varchar(255),
  data blob
);

insert into qcn_kml_region values (1, 'Anchorage Alaska RAMP', 'Anchorage_RAMP.kml', null);
insert into qcn_kml_region values (2, 'California Metro RAMP', 'CA_Metro_RAMP.kml', null);
insert into qcn_kml_region values (3, 'Hayward RAMP', 'Hayward_RAMP.kml', null);
insert into qcn_kml_region values (4, 'Istanbul RAMP', 'Istanbul_RAMP.kml', null);
insert into qcn_kml_region values (5, 'New Madrid Fault RAMP (MO,TN,AR,KY)', 'NM_RAMP.kml', null);
insert into qcn_kml_region values (6, 'Oregon Metro RAMP', 'OR_Metro_RAMP.kml', null);
insert into qcn_kml_region values (7, 'Pacific Northwest Coastal RAMP', 'PNW_Coast_RAMP.kml', null);
insert into qcn_kml_region values (8, 'San Andreas Fault North RAMP', 'SAF_N_RAMP.kml', null);
insert into qcn_kml_region values (9, 'San Francisco RAMP', 'SAF_RAMP.kml', null);
insert into qcn_kml_region values (10, 'San Andreas Fault South RAMP', 'SAF_S_RAMP.kml', null);
insert into qcn_kml_region values (11, 'Washington State Metro RAMP', 'WA_Metro_RAMP.kml', null);
insert into qcn_kml_region values (12, 'Wasatch (SLC Utah) RAMP', 'Wasatch_RAMP.kml', null);

create table qcn_reds (
    id int not null primary key auto_increment,
    userid int not null,
    name varchar(64),
    email_addr varchar(100),
    magnitude_min_global double,
    magnitude_min_local double,
    active boolean not null default 1,
    time_edit int
);
create unique index qcn_reds_userid on qcn_reds(userid);

create table qcn_post (
    id int not null primary key,
    where_clause varchar(255) not null,
    url varchar(255) not null,
    active boolean not null default 1,
    contact_name varchar(255),
    contact_email varchar(255),
    contact_address varchar(255)
);

insert into qcn_post values (1, 
  'latitude BETWEEN -15 AND -13 AND longitude BETWEEN -173 AND -169', 
  'http://qcn-upl.stanford.edu/carlc/test-post.php',
  1,
  'Mihai Tarabuta',
  'mihai.tarabuta@mobilis.com',
  ''
);

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

create table qcn_quake
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

create unique index qcn_quake_guid on qcn_quake (guid);
alter table qcn_quake add index qcn_quake_magnitude (magnitude);
alter table qcn_quake add index qcn_time_utc (time_utc);
alter table qcn_quake add index qcn_latitude (latitude);
alter table qcn_quake add index qcn_longitude (longitude);

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
qcn_quakeid int(11),
time_filereq double,
received_file tinyint(1),
runtime_clock double,
runtime_cpu double,
varietyid smallint not null default 0,
flag boolean not null default 0
);

create index qcn_trigger_time on qcn_trigger (time_trigger desc, varietyid asc);
create index qcn_trigger_hostid on qcn_trigger(hostid, time_trigger, varietyid);
create index qcn_trigger_timelatlng on qcn_trigger(time_trigger, latitude, longitude, varietyid);
create index qcn_trigger_result_name on qcn_trigger (result_name,id,varietyid);

create index qcn_trigger_hostid_filereq on qcn_trigger(hostid,time_filereq,received_file);
create index qcn_trigger_quakeid on qcn_trigger (qcn_quakeid);
create index qcn_trigger_file on qcn_trigger (file);
create index qcn_trigger_type_sensor on qcn_trigger (type_sensor);
create index qcn_trigger_qcn_quakeid on qcn_trigger (qcn_quakeid);
create index qcn_trigger_flag on qcn_trigger (flag);

create table qcn_trigger_followup
(
id int(11) not null primary key,
mxy1p float null,
mz1p float null,
mxy1a float null,
mz1a float null,
mxy2a float null,
mz2a float null,
mxy4a float null,
mz4a float null
);

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

create database trigmem;
create table trigmem.qcn_trigger_memory 
(
db_name varchar(16) not null,
triggerid int(11) not null,
hostid int(11) not null,
ipaddr varchar(32) not null,
result_name varchar(64) not null,
time_trigger double,
time_received double,
time_sync double,
sync_offset double,
significance double,
magnitude double,
mxy1p float null,
mz1p float null,
mxy1a float null,
mz1a float null,
mxy2a float null,
mz2a float null,
mxy4a float null,
mz4a float null,
latitude double,
longitude double,
levelvalue float,
levelid smallint,
alignid smallint,
file varchar(64),
dt float,
numreset int(6),
type_sensor int(3),
varietyid smallint not null default 0,
qcn_quakeid int not null default 0,
posted boolean not null default false
) ENGINE = MEMORY;

alter table trigmem.qcn_trigger_memory ADD PRIMARY KEY (db_name, triggerid);
create index qcn_trigger_memory_time on trigmem.qcn_trigger_memory (time_trigger desc, varietyid asc);
create index qcn_trigger_memory_hostid on trigmem.qcn_trigger_memory(hostid, time_trigger, qcn_quakeid, varietyid, posted);
create index qcn_trigger_memory_latlng on trigmem.qcn_trigger_memory(latitude, longitude, qcn_quakeid, varietyid, posted);
create index qcn_trigger_memory_file on trigmem.qcn_trigger_memory(file);

/*

insert into trigmem.qcn_trigger_memory
   select 'qcnalpha',id,hostid,ipaddr,result_name,time_trigger,time_received,time_sync,sync_offset,
      significance,magnitude,latitude,longitude,
      levelvalue, levelid, alignid, dt, numreset, type_sensor, varietyid
         from qcn_trigger where time_sync>1e6 order by time_trigger desc limit 10000;


 select round(time_trigger,0) tt, round(latitude,1) lat, round(longitude,1) lng, count(*) 
    from trigmem.qcn_trigger_memory  group by tt, lat,lng;

select from_Unixtime(min(time_trigger)) min_time, from_Unixtime(max(time_trigger)) max_time
   from trigmem.qcn_trigger_memory;

*/

SOURCE qcn_country_latlng.sql

/* Now generate the stored procedures */

SOURCE do_final_trigger.sql;

SOURCE do_stats.sql;


