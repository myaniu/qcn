#ifndef _CRUST_H_
#define _CRUST_H_

enum crust_types { CRUST_KEY, CRUST_MAP, CRUST_ELEV };

#define CRUST_KEY_FILE     "/var/www/qcn/earthquakes/inc/CNtype2_key.txt"
#define CRUST_MAP_FILE     "/var/www/qcn/earthquakes/inc/CNtype2.txt"
#define CRUST_ELEV_FILE    "/var/www/qcn/earthquakes/inc/CNelevatio2.txt"
 
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


int  crust2_load();                         // Loads crust2.0 into memory
// retrieves mean velocity to depth from CRUST2.0
void crust2_get_mean_vel(const float& qdep, const float& qlon, const float& qlat, float* v);
// Retrieves crustal type from lon & lat of CRUST2.0
int crust2_type(const float& lon, const float& lat);

extern struct cr_mod crm;                     // The Crust2.0 model map
extern struct cr_key crk[mx_cr_type];         // The Crust2.0 model key

#endif // _CRUST_H_
