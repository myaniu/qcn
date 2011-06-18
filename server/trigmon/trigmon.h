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
#include "common.h"

#define Vs 3.4                              // S wave velocity (km/s)
#define Vp 6.4                              // P wave velocity (km/s)
#define T_max 90.                           // Maximum time between triggers in seconds
#define D_max 200.                          // Maximum distance between triggers in km
#define C_CNT_MIN 5                         // Min # of correlated triggers for event detect
#define EVENT_MASK 0755

#define PHP_CMD         "/usr/local/bin/php"
#define EVENT_PATH      "/var/www/qcn/earthquakes/"
#define BAD_HOSTS_FILE  "/var/www/qcn/earthquakes/inc/bad_hosts.txt"
#define PATH_EMAIL      "/var/www/boinc/sensor/html/user/earthquake_email.php"
#define GMT_MAP_PHP     "/var/www/qcn/earthquakes/inc/gmt_map.php"

enum eOutput { OUT_EVENT, OUT_STATION, OUT_INTENSITY_MAP, OUT_CONT_TIME, OUT_CONT_LABEL, OUT_TIME_SCATTER };

// returns quakeid if event found/created (0 if not)
int getQCNQuakeID(
    const double& dLat, 
    const double& dLng, 
    const int& iCtr, 
    const double& dTimeMin, 
    const double& dTimeMax);

void close_db();
void setQueries();
int do_trigmon(struct trigger t[],struct bad_hosts bh);

float average( float dat[], int ndat);
float std_dev( float dat[], int ndat, float dat_ave);
float correlate( float datx[], float daty[], int ndat);

float ang_dist_km(float lon1, float lat1, float lon2, float lat2);
void vel_calc(float dep, float v[]);
void set_grid3D( struct bounds g, float elon, float elat, float edep, float width, float dx, float zrange, float dz);

float intensity_extrapolate(int pors, float dist, float dist_eq_nd, float intensity_in);

void qcn_event_locate(struct trigger t[], int i, struct event e[]);
void estimate_magnitude_bs(struct trigger t[], struct event e[], int i);
void php_event_email(struct trigger t[], int i, struct event e[], char* epath);
void php_event_page(struct trigger t[], int i, struct event e[], char* epath);
int intensity_map_gmt(struct event e[], char* epath);
void scatter_plot_gmt(struct event e[], char* epath);
int intensity_map(struct trigger t[], int i, struct event e[]);
void detect_qcn_event(struct trigger t[], int iCtr, struct event e[]);

void preserve_dir(char * edir, char * epath);
void get_bad_hosts(struct bad_hosts bh[]);

#endif //_QCN_TRIGMON_H_
