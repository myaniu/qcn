"make clean && make" will build user-defined functions for QCN for the 
mysql database installation on qcn-db (a shared object udf_qcn.so in 
the /usr/local/mysql/lib/plugin directory)

After building, you will want to run the following in a mysql login screen:

 DROP FUNCTION quake_hit_test;
 DROP FUNCTION lat_lon_distance_m;

 CREATE FUNCTION lat_lon_distance_m RETURNS REAL SONAME "udf_qcn.so";
 CREATE FUNCTION quake_hit_test RETURNS INTEGER SONAME "udf_qcn.so";

