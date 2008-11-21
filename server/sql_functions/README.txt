"make clean && make" will build user-defined functions for QCN for the 
mysql database installation on qcn-db (a shared object udf_qcn.so in 
the /usr/local/mysql/lib/plugin directory)

After building, you will need to restart the mysqld instance, i.e.

sudo /etc/init.d/mysqld restart


Then in a mysql client window, you will want to run the following:

 DROP FUNCTION quake_hit_test;
 DROP FUNCTION lat_lon_distance_m;

 CREATE FUNCTION lat_lon_distance_m RETURNS REAL SONAME "udf_qcn.so";
 CREATE FUNCTION quake_hit_test RETURNS INTEGER SONAME "udf_qcn.so";

