#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#define DB_TRIGMEM "trigmem"

#define n_short 300
#define n_long 1000

// important constants for the main query
#define TRIGGER_TIME_INTERVAL 10
#define TRIGGER_COUNT 10
#define TRIGGER_SLEEP_INTERVAL 3.0
#define TRIGGER_DELETE_INTERVAL 900

#define ENUM_FIRST_PASS     0
#define ENUM_SECOND_PASS    1
#define ENUM_OVER           2

struct trigger {
/*  Data structure for input trigger data to be used with QCN MySQL output & location
    program. This structure written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu                                                  */
   int    hid;                   // Host ID (Sensor number) 
   int    tid;                   // Trigger ID

   char   db[64];               // Database
   char   file[64];              // File name
   float  slon, slat;            // Sensor location
   double trig, rec,t_est;       // Time of trigger & Time received
   float  sig, mag;              // Significance and magnitude (sig/noise)
   float  pgah[4],pgaz[4];       // Peak Ground Acceleration (Horizontal & vertical)
   int    c_cnt;                 // Count of correlated triggers
   int    c_ind[n_short];        // Correlated trigger IDs
   int    c_hid[n_short];        // Correlated host IDs
   float  dis;                   // Event to station distance (km)
   int    pors;                   // 1=P, 2=S wave             
};

struct event {
/*  Data structure for events. To be used with QCN location
    program. This structure written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu                                                  */

   int    eid;                   // Event ID
   int    qcn_quakeid;           // QCN database (qcn_quake table) ID of this event
   float  elon,elat,edep;        // Event Longitude, Latitude, & Depth
   double  e_time;               // Event Origin Time
   int    e_t_now;               // Event ID Time
   float  e_r2;                  // r-squared correlation
   float  e_mag; float e_std;    // Event magnitude & magnitude standard deviation
   int    e_cnt;
   float  e_msfit;                 // event misfit
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

struct bad_hosts {
   int   nh;                      // Number of bad host ids
   int   hid[n_long];              // Bad host ids

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


#endif // _STRUCTS_H_
