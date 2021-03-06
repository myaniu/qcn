#ifndef _COMMON_H_
#define _COMMON_H_



#define CSHELL_CMD    "/bin/csh"
#define SHELL_CMD     "/bin/bash"
#define PHP_CMD       "/usr/local/bin/php"

#define WEB_BASE_DIR "/var/www"
#define EVENT_URL_BASE  "http://qcn.stanford.edu/earthquakes"

#define DB_TRIGMEM "trigmem"

#define N_SHORT 300
#define N_LONG  1000

#define FILE_NAME_TRIGGER_LAPTOP  WEB_BASE_DIR "/qcn/rt_image/rt_triggers_LTN.xyz"
#define FILE_NAME_TRIGGER_DESKTOP WEB_BASE_DIR "/qcn/rt_image/rt_triggers_DTN.xyz"

// note using bash on this
#define PLOT_CMD  SHELL_CMD " " WEB_BASE_DIR "/qcn/rt_image/inc/rt_images.sh " WEB_BASE_DIR

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

// usb sensors start with type id # 100
#define ID_USB_SENSOR_START 100

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
   int    qcn_quakeid;           // QCN database (qcn_quake table) ID of this event
   bool   posted;   // was this updated in the database

   int    hostid;                   // Host ID (Sensor number) 
   int    triggerid;                   // Trigger ID

   char   db[64];               // Database
   char   file[64];              // File name
   char   result_name[64];       // qcn db result name
   float  longitude, latitude;            // Sensor location
   double time_trigger, time_received, time_est;       // Time of trigger & Time received
   float  significance, magnitude;              // Significance and magnitude (sig/noise)
   float  pgah[4],pgaz[4];       // Peak Ground Acceleration (Horizontal & vertical)
   int    c_cnt;                 // Count of correlated triggers
   int    c_ind[N_SHORT];        // Correlated trigger IDs  (really the index into the trigger vector)
   int    c_hid[N_SHORT];        // Correlated host IDs
   float  dis;                   // Event to station distance (km)
   int    pors;                   // 1=P, 2=S wave             
   bool   dirty;    // if this is true, we changed and should update the qcn_trigger table ie for quakeid etc 
   void clear() { memset(this, 0x00, sizeof(trigger)); }
   trigger() { clear(); };
};

struct event {
/*  Data structure for events. To be used with QCN location
    program. This structure written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu                                                  */

   int    eventid;                   // internal Event ID
   int    qcn_quakeid;           // QCN database (qcn_quake table) ID of this event
   float  longitude, latitude, depth;        // Event Longitude, Latitude, & Depth
   double  e_time;               // Event Origin Time
   int    e_t_now;               // Event ID Time
   float  e_r2;                  // r-squared correlation
   float  magnitude; float e_std;    // Event magnitude & magnitude standard deviation
   int    e_cnt;
   float  e_msfit;                 // event misfit
   double e_t_detect;            // Time detected
   float  e_dt_detect;           // Time from event origin time to detection 
   bool   dirty;    // if this is true, we changed and should update the qcn_trigger table ie for quakeid etc 

   void clear() { memset(this, 0x00, sizeof(event)); }
   event() { clear(); };
};

struct qcn_host {
   int    hid;                   // Host ID (Sensor number) 
   float  slon, slat;            // Sensor location
   double t_last;                // Last detection by host
   
   void clear() { memset(this, 0x00, sizeof(qcn_host)); }
   qcn_host() { clear(); };
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
   
   void clear() { memset(this, 0x00, sizeof(bounds)); }
   bounds() { clear(); };
};

/* CMC not needed now
struct bad_hosts {
   int   nh;                      // Number of bad host ids
   int   hid[N_LONG];              // Bad host ids

};
*/

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




#endif // _COMMON_H_
