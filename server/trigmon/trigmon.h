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
#include "qcn_post.h"

#define Vs 3.4                              // S wave velocity (km/s)
#define Vp 6.4                              // P wave velocity (km/s)
#define T_max 90.                           // Maximum time between triggers in seconds
#define D_max 200.                          // Maximum distance between triggers in km
#define C_CNT_MIN 5                         // Min # of correlated triggers for event detect
#define EVENT_MASK 0755

#define CSHELL_CMD      "/bin/csh"
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

int QCN_GetTriggers();
void QCN_EventLocate(const bool& bEventFound, struct event& e, const int& ciOff);
void QCN_DetectEvent();

int QCN_IntensityMapGMT(struct event& e, const char* epath);
int QCN_IntensityMap(struct event& e);
void QCN_EstimateMagnitude(struct event& e, const int& ciOff);

float average(float* dat, const int& ndat);
float std_dev(float* dat, const int& ndat, const float& dat_ave);
float correlate(float* datx, float* daty, const int& ndat);

float ang_dist_km(const float& lon1, const float& lat1, const float& lon2, const float& lat2);
void vel_calc(float& dep, float* v);
void set_grid3D(struct bounds& g, const float& elon, const float& elat, const float& edep, 
   const float& width, const float& dx, const float& zrange, const float& dz);

float intensity_extrapolate(int pors, float dist, float dist_eq_nd, float intensity_in);

void estimate_magnitude_bs(struct trigger& t, struct event& e);
void scatter_plot_gmt(const char* epath);

void preserve_dir(const char* edir, const char* epath);
//void get_bad_hosts();

void php_event_email(const struct event& e, char* epath);
void php_event_page(const struct event& e, char* epath);

#endif //_QCN_TRIGMON_H_
