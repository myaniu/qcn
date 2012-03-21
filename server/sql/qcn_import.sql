create table qcn_scedc
(db varchar(10), 
triggerid int(11), 
hostid int(11), 
time_utc varchar(16), 
time_us int(7), 
magnitude float, significance float, latitude float, longitude float, 
file varchar(255), 
numreset int(7), sensor varchar(128), alignment varchar(64), 
level_value float, level_type varchar(64), usgs_quake_time varchar(16), 
quake_depth_km float, quake_lat float, quake_lon float, quake_mag float, quake_id int(11)
);


/*

/usr/local/mysql/bin/mysqlimport -h db-private -u root -p --ignore-lines=1 sensor qcn_scedc.csv


I found some extra fields I'm rerunning so the csv/excel for mat will be like this, which can be "mysqlimport"-ed into a mysql table on scedc's end very easily:

db, triggerid, hostid, time_utc, time_us, magnitude, significance, latitude, longitude, file, numreset, sensor, alignment, level_value, level_type, usgs_quake_time, quake_depth_km, quake_lat, quake_lon, quake_mag, quake_id
"C","10846816","484","2012-03-13 19:00:00","180.0","0.0","0.0","34.066640807","-118.441988826","continual_qcnaa_048424_000000_1331665200.zip","0","ONavi A 16-bit USB","North","12","Floor (+/- above/below surface)","","","","","","",
"C","10846810","528","2012-03-13 19:00:00","1057.0","0.0","0.0","34.0666408","-118.441988","continual_qcnaa_048582_000000_1331665200.zip","0","ONavi A 16-bit USB","North","3","Floor (+/- above/below surface)","","","","","","",
"C","10846808","537","2012-03-13 19:00:00","9889.0","0.0","0.0","34.0666408","-118.441988","continual_qcnaa_048611_000000_1331665200.zip","0","ONavi A 16-bit USB","North","-1","Floor (+/- above/below surface)","","","","","","",
"C","10846809","794","2012-03-13 19:00:00","19563.0","0.0","0.0","34.13654088","-118.128274977","continual_qcnaa_048606_000000_1331665200.zip","0","ONavi A 16-bit USB","North","1","Floor (+/- above/below surface)","","","","","","",
"C","10846848","529","2012-03-13 19:00:00","21651.0","0.0","0.0","34.0666408","-118.441988","continual_qcnaa_048200_000000_1331665200.zip","0","ONavi A 16-bit USB","North","6","Floor (+/- above/below surface)","","","","","","",
"C","10846903","537","2012-03-13 19:10:00","2544.0","0.0","0.0","34.0666408","-118.441988","continual_qcnaa_048611_000000_1331665800.zip","0","ONavi A 16-bit USB","North","-1","Floor (+/- above/below surface)","","","","","","",
"C","10846938","529","2012-03-13 19:10:00","3840.0","0.0","0.0","34.0666408","-118.441988","continual_qcnaa_048200_000000_1331665800.zip","0","ONavi A 16-bit USB","North","6","Floor (+/- above/below surface)","","","","","","",
"C","10846944","794","2012-03-13 19:10:00","9207.0","0.0","0.0","34.13654088","-118.128274977","continual_qcnaa_048606_000000_1331665800.zip","0","ONavi A 16-bit USB","North","1","Floor (+/- above/below surface)","","","","","","",
"C","10846914","484","2012-03-13 19:10:00","13791.0","0.0","0.0","34.066640807","-118.441988826","continual_qcnaa_048424_000000_1331665800.zip","0","ONavi A 16-bit USB","North","12","Floor (+/- above/below surface)","","","","","","",

the above is basically a "flat file" of the qcn_trigger table with all the lookup fields "spelled out"  (i.e. instead of alignid=2 I say "North" or levelid=1 I say "Floor (+/- surface) etc)

here's the actual mysql tables:

the main trigger table:


+---------------+-------------+------+-----+---------+----------------+
| Field         | Type        | Null | Key | Default | Extra          |
+---------------+-------------+------+-----+---------+----------------+
| id            | int(11)     | NO   | PRI | NULL    | auto_increment |
| hostid        | int(11)     | NO   | MUL | NULL    |                |
| ipaddr        | varchar(32) | NO   |     | NULL    |                |
| result_name   | varchar(64) | NO   | MUL | NULL    |                |
| time_trigger  | double      | YES  | MUL | NULL    |                |
| time_received | double      | YES  |     | NULL    |                |
| time_sync     | double      | YES  |     | NULL    |                |
| sync_offset   | double      | YES  |     | NULL    |                |
| significance  | double      | YES  |     | NULL    |                |
| magnitude     | double      | YES  |     | NULL    |                |
| latitude      | double      | YES  | MUL | NULL    |                |
| longitude     | double      | YES  | MUL | NULL    |                |
| levelvalue    | float       | YES  |     | NULL    |                |
| levelid       | smallint(6) | YES  |     | NULL    |                |
| alignid       | smallint(6) | YES  |     | NULL    |                |
| file          | varchar(64) | YES  | MUL | NULL    |                |
| dt            | float       | YES  |     | NULL    |                |
| numreset      | int(6)      | YES  |     | NULL    |                |
| qcn_sensorid  | int(3)      | YES  | MUL | NULL    |                |
| sw_version    | varchar(8)  | YES  |     | NULL    |                |
| os_type       | varchar(8)  | YES  |     | NULL    |                |
| qcn_quakeid   | int(11)     | YES  | MUL | NULL    |                |
| time_filereq  | double      | YES  | MUL | NULL    |                |
| received_file | tinyint(1)  | YES  | MUL | NULL    |                |
| runtime_clock | double      | NO   |     | 0       |                |
| runtime_cpu   | double      | NO   |     | 0       |                |
| varietyid     | smallint(6) | NO   | MUL | 0       |                |
| flag          | tinyint(1)  | NO   | MUL | 0       |                |
+---------------+-------------+------+-----+---------+----------------+
--------------

USGS Quakes:

qcn_quake



+-------------+--------------+------+-----+---------+----------------+
| Field       | Type         | Null | Key | Default | Extra          |
+-------------+--------------+------+-----+---------+----------------+
| id          | int(11)      | NO   | PRI | NULL    | auto_increment |
| time_utc    | double       | YES  | MUL | NULL    |                |
| magnitude   | double       | YES  | MUL | NULL    |                |
| depth_km    | double       | YES  |     | NULL    |                |
| latitude    | double       | YES  | MUL | NULL    |                |
| longitude   | double       | YES  | MUL | NULL    |                |
| description | varchar(256) | YES  |     | NULL    |                |
| processed   | tinyint(1)   | YES  |     | NULL    |                |
| url         | varchar(256) | YES  |     | NULL    |                |
| guid        | varchar(256) | YES  | UNI | NULL    |                |
+-------------+--------------+------+-----+---------+----------------+
--------------

BOINC Host (computer) table:

+----------------------------+--------------+------+-----+---------+----------------+
| Field                      | Type         | Null | Key | Default | Extra          |
+----------------------------+--------------+------+-----+---------+----------------+
| id                         | int(11)      | NO   | PRI | NULL    | auto_increment |
| create_time                | int(11)      | NO   |     | NULL    |                |
| userid                     | int(11)      | NO   | MUL | NULL    |                |
| rpc_seqno                  | int(11)      | NO   |     | NULL    |                |
| rpc_time                   | int(11)      | NO   |     | NULL    |                |
| total_credit               | double       | NO   | MUL | NULL    |                |
| expavg_credit              | double       | NO   | MUL | NULL    |                |
| expavg_time                | double       | NO   |     | NULL    |                |
| timezone                   | int(11)      | NO   |     | NULL    |                |
| domain_name                | varchar(254) | YES  |     | NULL    |                |
| serialnum                  | varchar(254) | YES  |     | NULL    |                |
| last_ip_addr               | varchar(254) | YES  |     | NULL    |                |
| nsame_ip_addr              | int(11)      | NO   |     | NULL    |                |
| on_frac                    | double       | NO   |     | NULL    |                |
| connected_frac             | double       | NO   |     | NULL    |                |
| active_frac                | double       | NO   |     | NULL    |                |
| cpu_efficiency             | double       | NO   |     | NULL    |                |
| duration_correction_factor | double       | NO   |     | NULL    |                |
| p_ncpus                    | int(11)      | NO   |     | NULL    |                |
| p_vendor                   | varchar(254) | YES  |     | NULL    |                |
| p_model                    | varchar(254) | YES  |     | NULL    |                |
| p_fpops                    | double       | NO   |     | NULL    |                |
| p_iops                     | double       | NO   |     | NULL    |                |
| p_membw                    | double       | NO   |     | NULL    |                |
| os_name                    | varchar(254) | YES  |     | NULL    |                |
| os_version                 | varchar(254) | YES  |     | NULL    |                |
| m_nbytes                   | double       | NO   |     | NULL    |                |
| m_cache                    | double       | NO   |     | NULL    |                |
| m_swap                     | double       | NO   |     | NULL    |                |
| d_total                    | double       | NO   |     | NULL    |                |
| d_free                     | double       | NO   |     | NULL    |                |
| d_boinc_used_total         | double       | NO   |     | NULL    |                |
| d_boinc_used_project       | double       | NO   |     | NULL    |                |
| d_boinc_max                | double       | NO   |     | NULL    |                |
| n_bwup                     | double       | NO   |     | NULL    |                |
| n_bwdown                   | double       | NO   |     | NULL    |                |
| credit_per_cpu_sec         | double       | NO   |     | NULL    |                |
| venue                      | varchar(254) | NO   |     | NULL    |                |
| nresults_today             | int(11)      | NO   |     | NULL    |                |
| avg_turnaround             | double       | NO   |     | NULL    |                |
| host_cpid                  | varchar(254) | YES  |     | NULL    |                |
| external_ip_addr           | varchar(254) | YES  |     | NULL    |                |
| max_results_day            | int(11)      | NO   |     | NULL    |                |
| error_rate                 | double       | NO   |     | 0       |                |
+----------------------------+--------------+------+-----+---------+----------------+
44 rows in set (0.00 sec)


-------------

"variety" ie type of trigger:

qcn_variety

+----+---------------------+
| id | description         |
+----+---------------------+
| -2 | Final Stats Trigger |
| -1 | Quakelist Trigger   |
|  0 | Normal Trigger      |
|  1 | Ping Trigger        |
|  2 | Continual Trigger   |
+----+---------------------+

----------------

sensor alignment table:

qcn_align

+----+-------------+
| id | description |
+----+-------------+
|  0 | Unaligned   |
|  1 | North       |
|  2 | South       |
|  3 | East        |
|  4 | West        |
|  5 | Wall        |
+----+-------------+

-----------------

level types:

qcn_level

+----+------------------------------------+
| id | description                        |
+----+------------------------------------+
|  0 | N/A                                |
|  1 | Floor (+/- above/below surface)    |
|  2 | Meters (above/below surface)       |
|  3 | Feet (above/below surface)         |
|  4 | Elevation - meters above sea level |
|  5 | Elevation - feet above sea level   |
+----+------------------------------------+


-----------------

sensor types:

qcn_sensor table

qcn_sensor
    -> ;
+-----+--------+---------------------------+
| id  | is_usb | description               |
+-----+--------+---------------------------+
|   0 |      0 | Not Found                 |
|   1 |      0 | Mac PPC 1                 |
|   2 |      0 | Mac PPC 2                 |
|   3 |      0 | Mac PPC 3                 |
|   4 |      0 | Mac Intel                 |
|   5 |      0 | Lenovo Thinkpad (Windows) |
|   6 |      0 | HP Laptop (Windows)       |
| 100 |      1 | JoyWarrior 24F8 USB       |
| 101 |      1 | MotionNode Accel USB      |
| 102 |      1 | O1 USB                    |
| 103 |      1 | JoyWarrior 24F14 USB      |
| 104 |      1 | ONavi A 12-bit USB        |
| 105 |      1 | ONavi A 16-bit USB        |
| 106 |      1 | ONavi A 24-bit USB        |
+-----+--------+---------------------------+

----------------------

user ip addr / lat-lng / geoip info:

qcn_host_ipaddr;
+-------------+-------------+------+-----+---------+----------------+
| Field       | Type        | Null | Key | Default | Extra          |
+-------------+-------------+------+-----+---------+----------------+
| id          | int(11)     | NO   | PRI | NULL    | auto_increment |
| hostid      | int(11)     | NO   | MUL | NULL    |                |
| ipaddr      | varchar(32) | NO   |     |         |                |
| location    | varchar(32) | NO   |     |         |                |
| latitude    | double      | YES  |     | NULL    |                |
| longitude   | double      | YES  |     | NULL    |                |
| levelvalue  | float       | YES  |     | NULL    |                |
| levelid     | smallint(6) | YES  |     | NULL    |                |
| alignid     | smallint(6) | YES  |     | NULL    |                |
| geoipaddrid | int(11)     | NO   |     | 0       |                |
+-------------+-------------+------+-----+---------+----------------+



*/

