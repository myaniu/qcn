/*

CMC -- this is included with the boinc/sched/handle_request.C -- basically it will override the "message from host" insert (mfh.insert) to send triggers
mfh.variety == "trigger" to a separate table for speed & ease of lookups

*/

#ifndef _QCN_TRIGGER_H
#define _QCN_TRIGGER_H

#include "config.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
using namespace std;

#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "boinc_db.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "server_types.h"
#include "sched_util.h"
#include "sched_msgs.h"

// the curl executable to use for Maxmind GeoIP queries (would linking to curl lib be better?  but have to tell BOINC)
//#define EXEC_CURL     "/usr/local/bin/curl"
//#define FORMAT_GEOIP  "-f -s 'http://geoip1.maxmind.com/b?l=0q9qp6z4BS40&i=%s.1'"
#define FORMAT_GEOIP  "http://geoip1.maxmind.com/b?l=0q9qp6z4BS40&i=%s.1"

#define BYTESIZE_URL   64
#define BYTESIZE_CURL 512

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

extern DB_CONN boinc_db;

extern int handle_qcn_trigger(const DB_MSG_FROM_HOST* pmfh, bool bPing);

// structures matching the mySQL tables for QCN defined by qcn/server/qcn-data.sql

struct QCN_HOST_IPADDR 
{
    int id;
    int hostid;
    char ipaddr[32];
    char location[32];
    double latitude;
    double longitude;
    int geoipaddrid;
};

struct QCN_GEO_IPADDR 
{
    int id;
    char ipaddr[32];
    double time_lookup;
    char country[32];
    char region[32];
    char city[32];
    double latitude;
    double longitude;
};

struct QCN_TRIGGER 
{
    int id;
    int hostid;
    char ipaddr[32];
    char result_name[64];
    double time_trigger;
    double time_received;
    double time_sync;
    double sync_offset;
    double significance;
    double magnitude;
    double latitude;
    double longitude;
    double depth_km;
    char file[64];
    double dt;
    int numreset;
    int type_sensor;
    double sw_version;
    int usgs_quakeid;
    int received_file;
    char file_url[64];
    double runtime_clock;
    double runtime_cpu;
    int ping;
};

class DB_QCN_HOST_IPADDR : public DB_BASE, public QCN_HOST_IPADDR 
{
public:
    DB_QCN_HOST_IPADDR(DB_CONN* dc=0) :
          DB_BASE("qcn_host_ipaddr", dc ? dc : &boinc_db)  { }

    void clear() {memset(this, 0x00, sizeof(DB_QCN_HOST_IPADDR));}

    int get_id() {return id;}

    void db_print(char* buf)
    {
      sprintf(buf,
        "hostid=%d,"
        "ipaddr='%s',"
        "location='%s',"
        "latitude=%f,"
        "longitude=%f,"
        "geoipaddrid=%d",
        hostid, ipaddr, location, latitude, longitude, geoipaddrid
      );
    }

    void db_parse(MYSQL_ROW& r)
    {
      int i=0;
      clear();
      id = safe_atoi(r[i++]);
      hostid = safe_atoi(r[i++]);
      strcpy2(ipaddr, r[i++]);
      strcpy2(location, r[i++]);
      latitude = safe_atof(r[i++]);
      longitude = safe_atof(r[i++]);
      geoipaddrid = safe_atoi(r[i++]);
    }
};

class DB_QCN_GEO_IPADDR : public DB_BASE, public QCN_GEO_IPADDR 
{
public:
    DB_QCN_GEO_IPADDR(DB_CONN* dc=0) :
          DB_BASE("qcn_geo_ipaddr", dc ? dc : &boinc_db)  { }

    void clear() {memset(this, 0x00, sizeof(DB_QCN_GEO_IPADDR));}

    int get_id() {return id;}

    void db_print(char* buf)
    {
      sprintf(buf,
        "ipaddr='%s',"
        "time_lookup=%f,"
        "country='%s',"
        "region='%s',"
        "city='%s',"
        "latitude=%f,"
        "longitude=%f",
        ipaddr, time_lookup, country, region, city, latitude, longitude
      );
    }

    void db_parse(MYSQL_ROW& r)
    {
      int i=0;
      clear();
      id = safe_atoi(r[i++]);
      strcpy2(ipaddr, r[i++]);
      time_lookup = safe_atof(r[i++]);
      strcpy2(country, r[i++]);
      strcpy2(region, r[i++]);
      strcpy2(city, r[i++]);
      latitude = safe_atof(r[i++]);
      longitude = safe_atof(r[i++]);
    }
};

class DB_QCN_TRIGGER : public DB_BASE, public QCN_TRIGGER 
{
public:
    DB_QCN_TRIGGER(DB_CONN* dc=0) :
          DB_BASE("qcn_trigger", dc ? dc : &boinc_db)  { }

    void clear() {memset(this, 0x00, sizeof(DB_QCN_TRIGGER));}

    int get_id() {return id;}

    void db_print(char* buf)
    {
      sprintf(buf,
        "hostid=%d,"
        "ipaddr='%s',"
        "result_name='%s',"
        "time_trigger=%f,"
        "time_received=unix_timestamp(),"
        "time_sync=%f,"
        "sync_offset=%f,"
        "significance=%f,"
        "magnitude=%f,"
        "latitude=%f,"
        "longitude=%f,"
        "depth_km=%f,"
        "file='%s',"
        "dt=%f,"
        "numreset=%d,"
        "type_sensor=%d,"
        "sw_version=%.2f,"
        "usgs_quakeid=%d,"
        "received_file=%d,"
        "file_url='%s',"
        "runtime_clock=%f,"
        "runtime_cpu=%f,",
        "ping=%d",
        hostid, ipaddr, result_name, time_trigger, time_sync, sync_offset,
        significance, magnitude, latitude, longitude, depth_km, file, dt, numreset, type_sensor, sw_version,
        usgs_quakeid, received_file, file_url, runtime_clock, runtime_cpu, ping
      );
    }

    void db_parse(MYSQL_ROW& r)
    {
      int i=0;
      clear();
      id = safe_atoi(r[i++]);
      hostid = safe_atoi(r[i++]);
      strcpy2(ipaddr, r[i++]);
      strcpy2(result_name, r[i++]);
      time_trigger = safe_atof(r[i++]);
      time_received = safe_atof(r[i++]);
      time_sync = safe_atof(r[i++]);
      sync_offset = safe_atof(r[i++]);
      significance = safe_atof(r[i++]);
      magnitude = safe_atof(r[i++]);
      latitude = safe_atof(r[i++]);
      longitude = safe_atof(r[i++]);
      depth_km = safe_atof(r[i++]);
      strcpy2(file, r[i++]);
      dt = safe_atof(r[i++]);
      numreset = safe_atoi(r[i++]);
      type_sensor = safe_atoi(r[i++]);
      sw_version = safe_atof(r[i++]);
      usgs_quakeid = safe_atoi(r[i++]);
      received_file = safe_atoi(r[i++]);
      strcpy2(file_url, r[i++]);
      runtime_clock = safe_atof(r[i++]);
      runtime_cpu = safe_atof(r[i++]);
      ping = safe_atoi(r[i++]);
    }
};


#endif  // ifndef _QCN_TRIGGER_H

