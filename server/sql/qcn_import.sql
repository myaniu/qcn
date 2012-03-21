create table qcn_import
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


