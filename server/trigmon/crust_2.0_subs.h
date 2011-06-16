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

//#include "util.h"
//#include "str_util.h"
//#include "sched_config.h"
//#include "sched_msgs.h"
//#include "sched_util.h"


char crust_key_file[]  = {"/var/www/qcn/earthquakes/inc/CNtype2_key.txt"};
char crust_map_file[]  = {"/var/www/qcn/earthquakes/inc/CNtype2.txt"};
char crust_elev_file[] = {"/var/www/qcn/earthquakes/inc/CNelevatio2.txt"};
 
#define mx_cr_type 360                // Number of crust2.0 crust types
#define mx_cr_lt    90                // Number of latitude nodes
#define mx_cr_ln   180                // Number of longitude nodes
#define dx_cr        2                // Sampling interval of Crust2.0

struct cr_key {
 float vp[8];
 float vs[8];
 float rh[8];
 float dp[8];
};

struct cr_mod {
 float  elev[mx_cr_lt][mx_cr_ln];
 int    ikey[mx_cr_lt][mx_cr_ln];
 float  lat[mx_cr_lt];
 float  lon[mx_cr_ln];
};


int  crust2_load();                                                        // Loads crust2.0 into memory
int  crust2_type(float lon, float lat);                                       // Retrieves crustal type from lon & lat of CRUST2.0
void crust2_get_mean_vel(float qdep, float qlon, float qlat, float v[]);  // retrieves mean velocity to depth from CRUST2.0

extern struct cr_mod crm;                     // The Crust2.0 model map
extern struct cr_key crk[mx_cr_type];         // The Crust2.0 model key


