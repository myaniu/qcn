#ifndef _QCN_TRIGMON_H_
#define _QCN_TRIGMON_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
using std::string;
using std::vector;

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"
// re-use the data structures from the qcn_trigger scheduler stuff
#include "../trigger/qcn_trigger.h"

#define DB_TRIGMEM "trigmem"

// important constants for the main query
#define TRIGGER_TIME_INTERVAL 10
#define TRIGGER_COUNT 10
#define TRIGGER_SLEEP_INTERVAL 3.0
#define TRIGGER_DELETE_INTERVAL 900

#define ENUM_FIRST_PASS     0
#define ENUM_SECOND_PASS    1
#define ENUM_OVER           2

#define Vs 3.4                              // S wave velocity (km/s)
#define Vp 6.4                              // P wave velocity (km/s)
#define T_max 90.                           // Maximum time between triggers
#define D_max 200.                          // Maximum distance between triggers
#define n_long 1000                         // Length of trigger buffer ring
#define n_short 100                         // Max # of correlated triggers
#define C_CNT_MIN 1                         // Min # of correlated triggers for event detect
#define EVENT_MASK 0755

char EVENT_PATH[]= "/var/www/qcn/earthquakes/";

struct trigger {
/*  Data structure for input trigger data to be used with QCN MySQL output & location
    program. This structure written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu                                                  */
   int    hid;                   // Host ID (Sensor number) 
   int    tid;                   // Trigger ID

   char   db[64];               // Database
   float  slon, slat;            // Sensor location
   double trig, rec;             // Time of trigger & Time received
   float  sig, mag;              // Significance and magnitude (sig/noise)
   int    c_cnt;                 // Count of correlated triggers
   int    c_ind[n_short];        // Correlated trigger IDs
   int    c_hid[n_short];        // Correlated host IDs
   float  dis;
};

struct event {
/*  Data structure for events. To be used with QCN location
    program. This structure written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu                                                  */
   
   int    eid;                   // Event ID
   float  elon,elat,edep;        // Event Longitude, Latitude, & Depth
   double  e_time;               // Event Origin Time
   int    e_t_now;               // Event ID Time
   float  e_r2;                  // r-squared correlation
   float  e_mag;                 // Event magnitude
   int    e_cnt;
   double e_t_detect;            // Time detected
   float  e_dt_detect;           // Time from event origin time to detection 
};

struct qcn_host {
   int    hid;                   // Host ID (Sensor number) 
   float  slon, slat;            // Sensor location
   double t_last;                // Last detection by host
};


struct bounds { 
   float x_min;                   // Min longitude
   float x_max;                   // Max longitude
   float y_min;                   // Min latitude 
   float y_max;                   // Max latitude
   float z_min;                   // Min depth 
   float z_max;                   // Max depth
   float xw;                      // Longitudinal grid width
   float yw;                      // Latitudinal grid width
   float zw;                      // Depth grid range
   int   nx;                      // Number of longitudinal grid steps
   int   ny;                      // Number of latitudinal grid steps
   int   nz;                      // Number of depth grid steps
   float dx;                      // Longitudinal step size of grid
   float dy;                      // Latitudinal step size of grid
   float dz;                      // depth step size of grid
   float lon_factor;              // Longitude/latitude factor as approach poles
};




// struct to keep in a vector of the most recent QCN generated quake events
// so followup triggers can be matched to this if we are scanning the trigmem.qcn_trigger_memory 
// table very frequently (i.e. <5 seconds)

struct QCN_QUAKE_EVENT
{
    double dTime;
    int qcn_quakeid;
    double latitude;
    double longitude;
    int count;

    void clear() { memset(this, 0x00, sizeof(QCN_QUAKE_EVENT)); }
    QCN_QUAKE_EVENT() { clear(); };
};

// returns quakeid if event found/created (0 if not)
int getQCNQuakeID(
    const double& dLat, 
    const double& dLng, 
    const int& iCtr, 
    const double& dTimeMin, 
    const double& dTimeMax);
void close_db();
void do_delete_trigmem();
void setQueries();
int do_trigmon(struct trigger t[]);
float ang_dist_km(float lon1, float lat1, float lon2, float lat2);
void vel_calc(float dep, float v[]);
void set_grid3D( struct bounds g, float elon, float elat, float edep, float width, float dx, float zrange, float dz);
void qcn_event_locate(struct trigger t[], int i, struct event e[]);
void estimate_magnitude(struct trigger t[], struct event e[], int i);
float intensity_extrapolate(float dist, float dist_eq_nd, float intensity_in);
void php_event_page(struct trigger t[], int i, struct event e[], char* epath);
void preserve_dir(char * edir, char * epath);
void intensity_map_gmt(struct trigger t[], int i, struct event e[], char* epath);
void intensity_map(struct trigger t[], int i, struct event e[]);

void detect_qcn_event(struct trigger t[], int iCtr, struct event e[]);



//extern bool qcn_post_check();
//extern bool qcn_post_setup();

#endif //_QCN_TRIGMON_H_
