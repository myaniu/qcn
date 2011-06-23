/* version of qcn_trigmon for Jesse

  This program will monitor the "live" triggers in memory 
   (the mysql trigmem.qcn_trigger_memory table)

The general idea is that a query is run frequently (<1 secod) to see if any quakes were
detected by QCN sensors via lat/lng & time etc

if there were "hits" in a certain area, then flag this as a quake and put in the qcn_quake table

logic:
  1) check for numerous trickles within a region in a short time period (i.e. 10 seconds) 
  2) if there are numerous trickles - see if this event has been reported in qcnalpha.qcn_quake - lookup by time/lat/lng
  3) if has been reported, use that event to tag trickles; else make an entry in qcn_quake and tag triggers
  4) request uploads from these triggers as appropriate

Example usage:
./trigmonitor -d 3 -sleep_interval .2 -time_interval 100

(run every .2 seconds, show all debug (-d 3), triggers in last 100 seconds)

(c) 2010  Stanford University School of Earth Sciences

*/

#include "trigmon.h"
// Include crust_2.0 information
#include "crust_2.0_subs.h"

DB_CONN trigmem_db;
   
double g_dTimeCurrent = 0.0;  // global for current time

// global params for trigger monitoring behavior
double g_dSleepInterval = -1.0;   // number of seconds to sleep between trigmem enumerations
int g_iTriggerTimeInterval = -1;  // number of seconds to check for triggers (i.e. "time width" of triggers for an event)

// keep a global vector of recent QCN quake events, remove them after an hour or so
// this way follup triggers can be matched to a known QCN quake event (i.e. qcn_quake table)
//vector<QCN_QUAKE_EVENT> vQuakeEvent;

//map<string, struct trigger> mapTrigger;
//map<int, struct event> mapEvent;
//vector<struct bad_hosts> vbh;
vector<struct trigger> vt;
vector<struct event> ve;

// defined as extern in the header so other progs/files can include crust_2.0_subs.h
struct cr_mod crm;                     // The Crust2.0 model map
struct cr_key crk[mx_cr_type];         // The Crust2.0 model key

/*
void make_strKeyTrigger(char* strKey, const int& iLen, const char* db, const int& id)
{
  if (!strKey) return;
  memset(strKey, 0x00, iLen);
  sprintf(strKey, "%s%020d", db, id);
}
*/

// get the latest triggers from the database memory table and place into k////
int QCN_GetTriggers()
{
   DB_QCN_TRIGGER_MEMORY qtm;
   struct trigger t;
   int j;
   //char strKeyTrigger[32];
   char strWhere[64];

   // clear our local variables
   qtm.clear();
   t.clear();

   //mapTrigger.clear(); // clear old triggers
   vt.clear();  // clear our trigger vector

   sprintf(strWhere, "WHERE time_trigger > (unix_timestamp()-%d)", g_iTriggerTimeInterval);

   while (!qtm.enumerate(strWhere))  {
    // just print a line out of trigger info i.e. all fields in qtm
/*     fprintf(stdout, "%d %s %d %d %s %s %f %f %f %f %f %f %f %f %f %d %d %f %d %d %d %d %d\n",
        ++iCtr, qtm.db_name, qtm.triggerid, qtm.hostid, qtm.ipaddr, qtm.result_name, qtm.time_trigger,
        qtm.time_received, qtm.time_sync, qtm.sync_offset, qtm.significance, qtm.magnitude, qtm.latitude,
         qtm.longitude, qtm.levelvalue, qtm.levelid, qtm.alignid, qtm.dt, qtm.numreset, qtm.type_sensor,
         qtm.varietyid, qtm.qcn_quakeid, qtm.posted
     );*/
/* CMC suppress the bad host stuff now as all clients should be behaving with my "throttling trickle" code by now
    int bad_host=-999;
    for (j=1; j<bh.nh;j++) {
     if (qtm.hostid==bh.hostid[j]) bad_host=j;  
    }
*/
    // Note this will get previous quake eventid info (if updated) for the trigger
    if ( qtm.type_sensor >= ID_USB_SENSOR_START ) { //&&(bad_host<0) ) { 
      // Only use triggers from usb accelerometers
      t.hostid  = qtm.hostid;                          // Host ID
      t.triggerid  = qtm.triggerid;                       // Trigger ID
      t.qcn_quakeid = qtm.qcn_quakeid;                // bring in the matching quake id, if any yet!
      t.posted = qtm.posted;                // file req posted?
      strncpy(t.db, qtm.db_name, sizeof(t.db)-1);               // Database Name
      strncpy(t.file, qtm.file, sizeof(t.file)-1);                // File name
      t.latitude = qtm.latitude;                        // Latitude
      t.longitude= qtm.longitude;                       // Longitude
      t.time_trigger = qtm.time_trigger;                    // Trigger Time
      t.time_received = qtm.time_received;                   // Time Trigger received
      t.significance  = qtm.significance;                    // Significance (Trigger detection filter)
      t.magnitude  = qtm.magnitude;                       // set mag to magnitude at time of trigger
      t.pgah[0] = qtm.mxy1p;                        // Peak Ground Acceleration (1 second before and during trigger) (m/s/s)
      t.pgaz[0] = qtm.mz1p;                         // Peak Ground Acceleration (1 seconds before and during trigger) (m/s/s)
    
      t.pgah[1] = qtm.mxy1a;                        // Peak Ground Acceleration (1 second after trigger)  (m/s/s)
      t.pgaz[1] = qtm.mz1a;                         // Peak Ground Acceleration (1 second after trigger)  (m/s/s)
    
      t.pgah[2] = qtm.mxy2a;                        // Peak Ground Acceleration (2 seconds after trigger)  (m/s/s)
      t.pgaz[2] = qtm.mz2a;                         // Peak Ground Acceleration (2 seconds after trigger)  (m/s/s)
    
      t.pgah[3] = qtm.mxy4a;                        // Peak Ground Acceleration (4 seconds after trigger) (m/s/s)
      t.pgaz[3] = qtm.mz4a;                         // Peak Ground Acceleration (4 seconds after trigger) (m/s/s)

      for (j=0;j<=3;j++) {
       if (t.pgah[j] > t.magnitude) t.magnitude = t.pgah[j];  
       if (t.pgaz[j] > t.magnitude) t.magnitude = t.pgah[j];  
      }   
      // now insert trigger 
      vt.push_back(t);  // insert into our vector of triggers

     }  // USB sensors only
   }
   //return (int) vt.size();
}

// The following codes determine the depth-averaged seismic velocity for a location (lon,lat,depth) from CRUST2.0 

int crust2_load()
// This function reads in the crust2.0 model and indexes the model map by number rather than by letter 
{

   FILE *fpCrust[3] = {NULL, NULL, NULL};  // Crust2 files
   int retval = 0;

// Open input files. These are defined in crust_2.0_subs.h 
   fpCrust[CRUST_KEY]  = fopen(CRUST_KEY_FILE,  "r");     // Open key file describing each model
   fpCrust[CRUST_MAP]  = fopen(CRUST_MAP_FILE,  "r");     // Open map file with key at each lat/lon location
   fpCrust[CRUST_ELEV] = fopen(CRUST_ELEV_FILE, "r");    // Open elevation file w/ data at each lat/lon

   if (!fpCrust[CRUST_KEY] || !fpCrust[CRUST_MAP] || !fpCrust[CRUST_ELEV]) {
      log_messages.printf(MSG_CRITICAL,
         "File Open Error %lx %lx %lx\n", 
           (unsigned long) fpCrust[CRUST_KEY], 
           (unsigned long) fpCrust[CRUST_MAP], 
           (unsigned long) fpCrust[CRUST_ELEV]
      );
      retval = 1;
      goto crust2_close;
   }

   int   i, j, k;                                                                                    // Index variables
   int   ilat;                                                                                       // Index of latitude
   char  aline[1000];                                                                                // A full line of characters

// Temporary variales because I was worried about reading in directly to the variable 
   char key_string[mx_cr_type][255];   // Temporary key string variable (easier to index by number)
   char mod_string[mx_cr_lt][mx_cr_ln][255];  // Temporary map key string variable (easier to index by number)

   for (i=0;i<=4;i++) {
      fgets(aline,1000,fpCrust[CRUST_KEY]); //printf("Skipping Line: %s \n",aline);
   }            // Skip first 5 lines of key file
   fgets(aline,1000,fpCrust[CRUST_MAP]);                                                       // Skip first 1 lines of map file
   fgets(aline,1000,fpCrust[CRUST_ELEV]);                                                       // Skip first 1 lines of elevation file

// Read in model Key (the two-letter ID and the Vp, Vs, rho, thickness model): 
   for (i=0;i<mx_cr_type;i++) {   // Read in Crust 2.0 key
     fgets(aline,1000,fpCrust[CRUST_KEY]);        // retrieve whole line of Key file
    sscanf(aline,"%s",key_string[i]);  // Read in key code
    fgets(aline,1000,fpCrust[CRUST_KEY]);     // retrieve whole line of Key file
    sscanf(aline,"%f %f %f %f %f %f %f %f",
           &crk[i].vp[1],&crk[i].vp[0],&crk[i].vp[2],&crk[i].vp[3],
           &crk[i].vp[4],&crk[i].vp[5],&crk[i].vp[6],&crk[i].vp[7]);    // Read P velocity

    fgets(aline,1000,fpCrust[CRUST_KEY]);                                         // retrieve whole line of Key file
    sscanf(aline,"%f %f %f %f %f %f %f %f",
           &crk[i].vs[1],&crk[i].vs[0],&crk[i].vs[2],&crk[i].vs[3],
           &crk[i].vs[4],&crk[i].vs[5],&crk[i].vs[6],&crk[i].vs[7]);  // Read S velocity

    fgets(aline,1000,fpCrust[CRUST_KEY]);                                           // retrieve whole line of Key file
    sscanf(aline,"%f %f %f %f %f %f %f %f",
           &crk[i].rh[1],&crk[i].rh[0],&crk[i].rh[2],&crk[i].rh[3],
           &crk[i].rh[4],&crk[i].rh[5],&crk[i].rh[6],&crk[i].rh[7]);   // Read Density

    fgets(aline,1000,fpCrust[CRUST_KEY]);                                          // Retrieve whole line of Key file
    sscanf(aline,"%f %f %f %f %f %f %f   ",
           &crk[i].dp[1],&crk[i].dp[0],&crk[i].dp[2],&crk[i].dp[3],
           &crk[i].dp[4],&crk[i].dp[5],&crk[i].dp[6]              ); // Read layer thickness 

   }

// Read in map and associate letters with key index:
   for (i=0; i<=mx_cr_lt-1; i++) {                          // For each latitude
      fscanf(fpCrust[CRUST_MAP],"%d",&ilat);                              // Read latitude at beginning of each line
      fscanf(fpCrust[CRUST_ELEV],"%d",&ilat);                              // Read latitude at beginning of each line

      for(j=0; j<=mx_cr_ln-1; j++) {                        // For each longitude
         fscanf(fpCrust[CRUST_MAP],"%s",mod_string[i][j]);                // Read in key letters for that lat/lon location
         fscanf(fpCrust[CRUST_ELEV],"%f",&crm.elev[i][j]);                 // Read elevations for each longitude at this latitude

         for (k=0;k<=mx_cr_type-1; k++) {                   // Index the map key by searching through crk.tp

            if (memcmp(key_string[k],mod_string[i][j],2)==0) {    // If key found, store the key and stop searching
               break;                                             // Stop looking for additional matches since match found
            }
         }
      }
   }

// Set the longitude and latitude for each node: 

   for (i=0; i<=mx_cr_lt-1; i++) { crm.lat[i]=90.f-( ((float)i)+0.5)*dx_cr ;  }                      // for each latitude

   for (i=0; i<=mx_cr_ln-1; i++) { crm.lon[i]=     ( ((float)i)+0.5)*dx_cr ;  }                      // for each longitude

crust2_close:
   for (i = 0; i < 3; i++) {
     if (fpCrust[i]) fclose(fpCrust[i]);
     fpCrust[i] = NULL; 
   }
   return retval;                                                                                         // Done with function

}


int crust2_type(const float& lon, const float& lat) 
{
// This function returns the index of the model key for the map

   int ilat,ilon;
   float lon2=lon; if (lon2<0.f) {lon2+=360.f;}                                                    // make sure 0<lon<360 rather than -180<lon<180

   ilat = (int) (90.f-lat)/dx_cr;                                                                  // index of longitude
   ilon = (int) (    lon2)/dx_cr;                                                                  // index of latitude
   log_messages.printf(MSG_DEBUG,
      "ilon=%d    ilat=%d     type=%d\n",ilon,ilat,crm.ikey[ilat][ilon]
   );

   return crm.ikey[ilat][ilon];                                                                    // index key for this lon/lat location

}


void crust2_get_mean_vel(const float& qdep, const float& qlon, const float& qlat, float* v) {

/* 
   This fuction returns the average seismic velocity integrated from the surface to some depth (qdep). 
        The velocity is for P:v[0] and S:v[1] waves.  Note: for a zero depth qdep, use the first 
        non-zero thickness layer.
*/

   int i;                                          // Index variables
   v[0]=0.f; v[1]=0.f;                               // set P(0) and S(1) velocity to 0
   float d=0.f;                                        // set depth to 0
   int itype = crust2_type(qlon,qlat);                   // determine the crust type

// For an event at depth, integrate down to depth (layers 0-6 in crust, 7 is mantle): 

   if (qdep>0.f) {                                   // If depth greater than 0

      for (i=1;i<7;i++) {                            // go through each layer

         if (qdep>=d+crk[itype].dp[i]) {// If the depth is greater than the last depth plus the thickness of this layer, then it goes all the way through

            v[0]+=crk[itype].vp[i]*crk[itype].dp[i]; // Add thickness weighted Vp from crust2.0
            v[1]+=crk[itype].vs[i]*crk[itype].dp[i]; // Add Thickness weighted Vs from crust2.0
            d   +=crk[itype].dp[i];                  // Add layer thickness to accumulated depth.

         } else {          // If the depth is not greater than the last depth plust the layer thickness, use the difference between qdep and last depth

            v[0]+=crk[itype].vp[i]*(qdep-d);         // Add to average 
            v[1]+=crk[itype].vs[i]*(qdep-d);         // Add to average 
            d=qdep;                                  // Total depth is the depth of the earthquake
            break;                                   // Stop looking though layers if goal depth is achieved.

         }

      }

      if (qdep>d) {                                  // If depth greater than crustal depth, then ...

       v[0]+=crk[itype].vp[7]*(qdep-d);              // Add mantle velocity to average 
       v[1]+=crk[itype].vs[7]*(qdep-d);              // Add mantle velocity to average 
       d=qdep;                                       // Total depth is depth of the earthquake

      }

      v[0]/=d;                                       // Normalize weighted sum by cumulative weight (cumulative depth) for average
      v[1]/=d;                                       // Normalize weighted sum by cumulative weight (cumulative depth) for average

// Zero depth (surface rupture), means we need velocity below surface at first non-zero thickness layer

   } else {                             // If zero depth, use the velocity of the first layer

      for (i=1;i<7;i++) {               // go through layers until first non-zero layer is encountered.

         if (crk[itype].dp[i]>0.f) {    // if non-zero layer encountered, then 

            v[0]=crk[itype].vp[i];      // set Vp to Crust2.0
            v[1]=crk[itype].vs[i];      // set Vs to Crust2.0
            break;                      // stop going through layers

         }

      }

   }

   return;

}




inline float average(float* dat, const int& ndat) {
//  This fuction calculates the average of a data set (dat) of length (ndat). 
   float dat_ave = 0.;                                    // Start with zero as average
   int j;                                                 // Index variable
   for (j = 0; j<=ndat; j++) {                            // For each point
    dat_ave = dat_ave + dat[j];                           // Add values
   }                                                      // 
   dat_ave = dat_ave / ( (float) ndat + 1.);              // Normalize sum by quantity for average
   return dat_ave;                                        // Return with average
}

inline float std_dev(float* dat, const int& ndat, const float& dat_ave) {
//  This function calculates the standard deviation of a data set (dat) of length (ndat) with a mean of (dat_ave) 
   float dat_std = 0.;                                    // Zero the standard deviation
   int j;                                                 // Index variable
   for (j = 0; j<=ndat; j++) {                            // For each point
    dat_std = dat_std + (dat[j]-dat_ave)*(dat[j]-dat_ave);// Sum squared difference from mean
   }                                                      // 
   dat_std = dat_std / ( (float) ndat + 1.);              // Normalize squared differences
   dat_std = sqrt(dat_std);                               // Standard deviation once square root taken
   return dat_std;                                        // Return standard deviation
}

float corrlatitudee(float* datx, float* daty, const int& ndat) {
//  This function corrlatitudees two data sets (datx & daty) of length (ndat).  It returns the R^2 value (not r). 
   float datx_ave = average(datx,ndat);                   // Average of x series
   float daty_ave = average(daty,ndat);                   // Average of y series

   int j; float Sxy=0.; float Sxx=0.; float Syy=0.;       // Zero summed numerator and denominator values
   for (j=0;j <= ndat; j++) {                             // For each point
     Sxy = Sxy + datx[j]*daty[j];                         // Sum x*y
     Sxx = Sxx + datx[j]*datx[j];                         // Sum x*x
     Syy = Syy + daty[j]*daty[j];                         // Sum y*y
   }
   Sxy -= ((float) ndat+1.) * datx_ave*daty_ave;          // Sum x*y - x_ave*y_ave
   datx_ave=(float) (ndat+1)*datx_ave*datx_ave;           // Sum x*x - N*x_ave*x_ave
   daty_ave=(float) (ndat+1)*daty_ave*daty_ave;           // Sum y*y - N*y_ave*y_ave
   Sxx -= datx_ave;                                       
   Syy -= daty_ave;
   float R = Sxy*Sxy / Sxx / Syy;                         // Corrlatitudeion coefficient
   return R;                                              // Return with corrlatitudeion coefficient
}



inline float ang_dist_km(const float& lon1, const float& lat1, const float& lon2, const float& lat2) {
/* This function returns the distance (in km) between two points given as
   (lon1,lat1) & (lon2,lat2).This function written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu   
*/
   float pi = atan(1.f)*4.f;                              // set pi = 3.14....
   float d2r = pi/180.f;                                  // Degree to radian Conversion
   float dlat = lat2-lat1;                                // Latitudinal distance (in degrees)
   float dlon = (lon2-lon1)*cos((lat2+lat1)/2.f*d2r);     // Longitudeinal distance (corrected by mean latitude)
   float delt = 111.19*sqrt(dlat*dlat+dlon*dlon);         // Distance in km = degrees*111.19
 
   return delt;                                           // Return with distance
};


void set_grid3D(struct bounds *g, const float& longitude, const float& latitude, const float& depth,    
    const float& width, const float& dx, const float& zrange, const float& dz)
{
//
//void set_grid3D( struct bounds g, float longitude, float latitude, float depth, float width, float dx, float zrange, float dz) {
/* This subroutine sets up a 3D grid search (x,y, & z) for source location.  This is a brute force location
   scheme.  Slower than it should be.  The gridding is made into a subroutine because I do several iterations,
   that focus in on the best solution of the prior grid, which is much faster than a fine grid for a large area.
   
   The grid uses structure bounds.  The center of the grid is assumed to be the best eq estimate.
   
   The grid has horizontal dimensions width with horizontal node intervals of dx.
   
   The grid has vertical dimension zrange with vertical node interval of dz.

*/ 
   if (!g) return; // null ptr
   g->yw = width;                                          // Latitudinal width of grid
   g->xw = width;                                          // Latitudinal width of grid
   g->zw = zrange;
   
   g->x_min = longitude-g->xw/2.f;                               // Set bounds of grid
   g->x_max = longitude+g->xw/2.f;
   g->y_min = latitude-g->yw/2.f;
   g->y_max = latitude+g->yw/2.f;
   g->z_min = depth-g->zw/2.f;if (g->z_min < 0.) {g->z_min=0.f;};
   g->z_max = depth+g->zw/2.f;

   g->dy = dx;                                             // Set latitudinal step size
   g->dx = dx;                                             // Set longitudinal step size
   g->dz = dz;
   
   g->ny = (int) ((g->y_max-g->y_min)/g->dy);                 // Set number of latitudinal steps
   g->nx = (int) ((g->x_max-g->x_min)/g->dx);                 // Set number of longitudinal steps
   g->nz = (int) ((g->z_max-g->z_min)/g->dz);                 // Set number of depth steps
}

void vel_calc(float dep, float* v) {
/* This subroutine is a kluge - it increases velocity with depth, but assuming path averaged velocity for an earthquake that
   occurs at depth.  This is only a kluge, because the path average velocity changes as a function of depth/distance.  This
   subroutine needs updating.  Jesse has accurate versions in fortran, but they need to be converted to c.

*/
   float dep2=dep;if (dep < 5.f) {dep2=5.f;};             // Set minimum depth for equation to 5
   v[1] = 0.34*log(dep2)+2.56;                            // Vs (simple equation expression - not actual)
   v[0] = 1.86*v[1];                                      // Vp = 1.86*Vs
}

void QCN_EventLocate(const int& j)
{
   log_messages.printf(MSG_DEBUG,
      "Locate possible event %d \n",vt[j].c_cnt
   );

   if ( ( vt[j].time_trigger > T_max+ve[1].e_time)||(abs(t[i].latitude-ve[1].latitude)>3.) ) ve[1].eventid=0;         // If new Time or location, then new event
   
   
   float  width = 4.f; float zrange=150.f;                // Lateral and vertical grid size
   float  dx = 0.1f; float dz = 10.f;                     // Lateral and vertical grid step size                  
   int    n_iter=3;                                       // Number of grid search iterations
   float  ln_x=0.,lt_x=0.,dp_x=0.;                                 // Lon, Lat, & Depth of each grid point
   float  dn=0.; float dp=0.;                                // Distances to the nth and pth trigger location
   double  dt,dt_min;                                     // Travel time difference between two stations (and minimum for P & S wave)
   double  ls_mf,ls_ct;                                   // Least-squares misfit & count (For normalization)
   float   v[2];                                          // Seismic Velocities (P & S)
   int     h,j,k,l,m,n,o,p,q,r,j_min;                     // Index variables
   double t_min = t[i].trig;                              // Minimum trigger time
   struct bounds g;                                       // Bounds of grid

   j_min = i;                                             // Assume first trigger is i'th trigger (not necessarily the case and will be checked)
   for (j = 0; j<=t[i].c_cnt; j++) {                      // Find earliest trigger
    n = t[i].c_ind[j];                                    // index of corrlatitudeed series
    if (t_min < t[n].trig) {                              // If earlier trigger, then use that as first time
     t_min = t[n].trig;                                   //
     j_min = n;                                           //
    }
   }                                                      //
   ve[1].longitude = t[j_min].longitude;                             // Start with assumption of quake at/near earliest trigger
   ve[1].latitude = t[j_min].latitude;
   ve[1].depth = 33.;                                       // Start with default Moho depth eq.
   
   for (j = 1; j<=n_iter; j++) {                          // For each iteration 
//    set_grid3D(&g, ve[1].longitude, ve[1].latitude, ve[1].depth, width, dx, zrange, dz);        // Set bounds of grid search
    g.yw = width;                                         // Latitudinal width of grid
    g.xw = width;                                         // Latitudinal width of grid
    g.zw = zrange;                                        // Vertical dimension of grid
    
    g.dx = dx;                                            // Set grid interval
    g.dz = dz;
    g.dy = dx;

    g.x_min = ve[1].longitude-g.xw/2.f;                         // Set bounds of grid
    g.x_max = ve[1].longitude+g.xw/2.f;
    g.y_min = ve[1].latitude-g.yw/2.f;
    g.y_max = ve[1].latitude+g.yw/2.f;
    g.z_min = ve[1].depth-g.zw/2.f;if (g.z_min < 0.) {g.z_min=0.f;};
    g.z_max = ve[1].depth+g.zw/2.f;
 
    g.ny = (int) ((g.y_max-g.y_min)/g.dy);                // Set number of latitudinal steps
    g.nx = (int) ((g.x_max-g.x_min)/g.dx);                // Set number of longitudinal steps
    g.nz = (int) ((g.z_max-g.z_min)/g.dz);                // Set number of depth steps  
    float ls_mf_min = 9999999999.f;                       // Set obserdly high misfit minimum
    for (h = 1; h<=g.nz; h++) {                           // For each vertical grid step
     dp_x = g.z_min + g.dz * (float) (h-1);               // Calculate depth
//     vel_calc(dp_x,v);                                    // Calculate mean path velocity
     crust2_get_mean_vel(dp_x,t[j_min].longitude,t[j_min].latitude,v); // Get depth-averaged velocity for location from CRUST2.0
     for (k = 1; k<=g.nx; k++) {                          // For each x node
      ln_x = g.x_min + g.dx * (float) (k-1);              // Longitude of grid point
      for (l = 1; l<=g.nx; l++) {                         // For each y node
       lt_x = g.y_min + g.dy * (float) (l-1);             // Latitude of grid point
       ls_mf = 0.; ls_ct = 0.;                            // Zero least-squares misfit & count

// Compare times and distance difference between each station pair 
       for (m = 0; m< t[i].c_cnt; m++) {                  // For each trigger
        n = t[i].c_ind[m];                                // Index of corrlatitudeed trigger
        dn=ang_dist_km(ln_x,lt_x,t[n].longitude,t[n].latitude);    // Horizontal distance between node and host/station
        dn = sqrt(dn*dn + dp_x*dp_x);                     // Actual distance between event & station/host
        
        for (o = m+1; o<=t[i].c_cnt; o++) {               // Compare with each other trigger
         p = t[i].c_ind[o];                               // Index of corrlatitudeed trigger
         dp=ang_dist_km(ln_x,lt_x,t[p].longitude,t[p].latitude);   // Distance between triggers
         dp = sqrt(dp*dp + dp_x*dp_x);                    // Distance corrected for depth
         dt_min = 9999999.f;                              // Search For best P-wave & S-wave pairing
         for (q=0;q<=1;q++) {                             //    First station P(0) or S(1)
          for (r=0;r<=1;r++) {                            //    Second station P(0) or S(1)
           dt = (t[n].trig-t[p].trig) - (dn/v[q]-dp/v[r]);//    Interstation time difference - theoretical time difference
           if (dt*dt < dt_min*dt_min) {
            dt_min = dt;};                                //    Use the smallest value (assume if neerer zero then probably true)
          }
         }
         ls_mf = ls_mf + (dt_min*dt_min);                 // Sum L2 misfit 
         ls_ct = ls_ct + 1.f;                             // Count For normalizing misfit
        }
       }      
       if (ls_ct > 0.) {                                  // make sure at least one point used
        ls_mf = ls_mf/ls_ct;                              // Normalize misfit by count
        if (ls_mf < ls_mf_min) {                          // If minimum misfit/max corrlatitudeion save location & r2
         ls_mf_min = ls_mf;                               // Save new misfit minimum
         ve[1].longitude=ln_x; ve[1].latitude=lt_x; ve[1].depth=dp_x;  // Save location of best fit
        }                                                 // End loop over 2nd trigger of pair
       }                                                  // End loop over 1st trigger of pair
      }                                                   // End loop over latitude
     }                                                    // End loop over Longitude
    }                                                     // End loop over depth
    dx /=10.f; width/=10.f; dz/=10.f; zrange/=10.f;       // Reduce grid dimensions by an order of magnitude


// Re-compare times and distance difference between each station pair for best location and identify which phase each trigger is 
//    vel_calc(ve[1].depth,v);                                // Get velocity (note - depth dependant - not done well).
    crust2_get_mean_vel(dp_x,t[j_min].longitude,t[j_min].latitude,v); // Get depth-averaged velocity for location from CRUST2.0
    int q_save=0;
    ve[1].e_time=0.;
    for (m = 0; m<=t[i].c_cnt; m++) {                     // For each trigger
     n = t[i].c_ind[m];                                   // Index of corrlatitudeed trigger
     t[n].pors = 0;
     dn=ang_dist_km(ve[1].longitude,ve[1].latitude,t[n].longitude,t[n].latitude);// Horizontal distance between node and host/station
     dn = sqrt(dn*dn + ve[1].depth*ve[1].depth);                 // Actual distance between event & station/host
        
     for (o = 0; o<=t[i].c_cnt; o++) {                    // Compare with each other trigger
      p = t[i].c_ind[o];                                  // Index of corrlatitudeed trigger
      if (p!=n) {
       dp=ang_dist_km(ve[1].longitude,ve[1].latitude,t[p].longitude,t[p].latitude);  // Distance between triggers
       dp = sqrt(dp*dp + ve[1].depth*ve[1].depth);
       dt_min = 9999999.f;                                // Search For best P-wave & S-wave pairing
       for (q=0;q<=1;q++) {                               //    First station P(0) or S(1)
        for (r=0;r<=1;r++) {                              //    Second station P(0) or S(1)
         dt = (t[n].trig-t[p].trig) - (dn/v[q]-dp/v[r]);  //    Interstation time difference - theoretical time difference
         if (dt*dt < dt_min*dt_min) {                     //    Use lowest time misfit to identify P or S
          q_save = q;                                     //    Save P or S
          dt_min = dt;                                    //    Save minimum time
         }                                                //
        }                                                 //
       }                                                  //
       t[n].pors+=q_save;                                 // Sum most optimal values to see if it lies closer to P or S
      }
     }      
     t[n].pors = (int) round( (float) t[n].pors / ((float) t[i].c_cnt +1.));  // Normalize P or S to see if it is closer to P or S
     t[n].dis=dn;                                         // Save distance
     ve[1].e_time += t[n].trig - dn/v[t[n].pors];          // Time of event calculated from ith trigger.
    }                                                     //

// Time of earthquake: 
   ve[1].e_time = ve[1].e_time / ((float) t[i].c_cnt + 1.); // Time of earthquake calculated by averaging time w.r.t each station normalized by number of stations
   if (ve[1].eventid<=0) {                                    // For New earthquake ID if earthquake time wasn't set
    ve[1].eventid= ((int) ve[1].e_time);                       //
    log_messages.printf(MSG_DEBUG,
       "NEW EID: %d \n", ve[1].eventid
    );
   }

//  Determine maximum dimension of array  
   float ss_dist_max = -999999999.;                       // Start with large station to station maximum array dimension
   for (j = 0; j<t[i].c_cnt; j++) {                       // For each trigger/station
    n = t[i].c_ind[j];                                    // Index of jth station
    for (k = j+1; k<=t[i].c_cnt; k++) {                   // For each other trigger/station
     m = t[i].c_ind[k];                                   // Index of kth station
     dn = ang_dist_km(t[m].longitude,t[m].latitude,t[n].longitude,t[n].latitude);// Distance between stations
     if (dn > ss_dist_max) ss_dist_max = dn;              // Store if maximum distance
    }                                                     // 
   }                                                      //

//  Require that the earthquake-to-array distance be less than four times the dimension of the array 
   if (ss_dist_max*4. < t[j_min].dis) {                   // If the event-to-station distance > 4 times the array dimension
    ve[1].e_r2=-1.;                                        // Set corrlatitudeion to -1 (will reject earthquake)
    log_messages.printf(MSG_DEBUG,
       "Event poorly located: Array dimension=%f EQ Dist=%f.\n",ss_dist_max,dn //Status report
    );
    return;                                               // Return so done
   } else {                                               // Otherwise output status report
    log_messages.printf(MSG_DEBUG,
       "Event located: %f %f\n",ve[1].longitude,ve[1].latitude
    );
   }
   
//  Calculate the estimated arrival time of the phase 
   float t_obs[t[i].c_cnt];                               // Observed phase arrival times (from triggers)
   float t_est[t[i].c_cnt];                               // Estimated phase arrival times (from earthquake location estimate)
   float dt_ave=0;                                        // Average model-data time variance
   ve[1].e_msfit = 0.;                                     // Zero misfit for model given set P & S wave times
   for (j=0;j<=t[i].c_cnt;j++) {                          // For each trigger
    n = t[i].c_ind[j];                                    // Index of triggers
    t[n].t_est = ve[1].e_time + t[n].dis / v[t[n].pors];   // Estimated time is event origin time plus distance divided by velocity
    t_obs[j]=t[n].trig-ve[1].e_time;                       // Store observed times
    t_est[j]=t[n].t_est-ve[1].e_time;                      // Store estimated times
    dt_ave += abs(t_obs[j]-t_est[j]);                     // Sum averaged model-data time variance
    ve[1].e_msfit += (t_obs[j]-t_est[j])*(t_obs[j]-t_est[j]);// Sum L2 misfit
   }
   ve[1].e_msfit /= ( (float) t[i].c_cnt + 1.);            // Normalize the event misfit
   ve[1].e_msfit = sqrt(ve[1].e_msfit);                     // 
//  Require that the model variance is less than two seconds 
   dt_ave /= ( (float) t[i].c_cnt + 1.);                  // Normalize the data-model variance
   if (dt_ave > 2.) {                                     // If the average travel time misfit is greater than 2 seconds, reject the event detection
    ve[1].e_r2 = -1.;                                      // Set corrlatitudeion to < 0 for rejection
    return;                                               // Return with bad corrlatitudeion
   }

//  Corrlatitudee observed and estimated travel times  
    ve[1].e_r2 = corrlatitudee(t_obs, t_est, t[i].c_cnt);  // Corrlatitudee observed & estimated times
    log_messages.printf(MSG_DEBUG,
       "Estimated times corrlatitudee at r^2= %f \n",ve[1].e_r2
    );

}


void QCN_EstimateMagnitude(int i)
{
/* We need to come up with good magnitude/amplitude rlatitudeionships.  There are some good ones for peak displacement v. dist.
   We need some for peak acceleration v. distance.  Note - they may vary from location to location.
   We will need to adjust this for characterization of P & S wave values.  It may also be sensor specific.

   This code bootstraps over the data to determine how cerntain the estimated magnitude is.  Use 3X the standard error for 99% confidence.

   The magnitude rlatitudeion takes the form:
           M = a * LN( accel * b) + c * LN(dist) + d

   The

   This subroutine was written by Jesse F. Lawrence (jflawrence@stanford.edu).

*/
   float a=0.544f; float b=2.0f; float c=0.03085f; float d=4.28f;// Constants for equation above (Need to be adjusted)
   int j, k, kk, n;                                     // Index variables
   float mul_amp;                                       // Multiplication factor depends on P & S waves currently set to 1
   srand ( time(NULL) );                                // Set randomization kernel
   float mag_ave[N_SHORT];                              // Average magnitude

   ve[1].e_mag = 0.f;                                    // Zero magnitude
   for (j = 0; j <=t[i].c_cnt; j++) {                   // Bootstrap once for each trigger
    mag_ave[j]=0.;                                      // Zero the average magnitude for this bootstrap
    for (k = 0; k <= t[i].c_cnt; k++) {                 // Select one point for each trigger
     kk = rand() % (t[i].c_cnt+1);                      // Use random trigger
     n = t[i].c_ind[kk];                                // Index of corrlatitudeed trigger
     if ( t[n].pors == 0 ) {                            // Use appropriate multilication factor (currently not used but will eventually)
      mul_amp = 1.f;                                    //
     } else {                                           //
      mul_amp = 1.f;                                    //
     };                                                 //
     mag_ave[j] += a*log(t[n].mag*b*mul_amp) + c*log(t[n].dis) + d; // Sum magnitude estimate from each trigger for average estimate
    }
    mag_ave[j]/= ( (float) t[i].c_cnt + 1.);            // Normalize summed mag estimates for average magnitude estimate
   }
   ve[1].e_mag = average( mag_ave, t[i].c_cnt);          // Average the averages
   ve[1].e_std = std_dev( mag_ave, t[i].c_cnt, ve[1].e_mag)*3.;// 3 sigma is the 99 % confidence (assuming a statistically large enough data set)

}





//void QCN_EstimateMagnitude(struct trigger t[], struct event e[], int i) {
/* We need to come up with good magnitude/amplitude rlatitudeionships.  There are some good ones for peak displacement v. dist.
   We need some for peak acceleration v. distance.  Note - they may vary from location to location.
   We will need to adjust this for characterization of P & S wave values.  It may also be sensor specific.

   This code bootstraps over the data to determine how cerntain the estimated magnitude is.  Use 3X the standard error for 99% confidence.

   The magnitude rlatitudeion takes the form:
           M = a * LN( accel * b) + c * LN(dist) + d

   The

   This subroutine was written by Jesse F. Lawrence (jflawrence@stanford.edu).

*/   
/*   float a=1.25f; float b=1.8f; float c=0.8f; float d=3.25f;// Constants for equation above (Need to be adjusted)
   int j, k, kk, n;                                     // Index variables
   float mul_amp;                                       // Multiplication factor depends on P & S waves currently set to 1
   srand ( time(NULL) );                                // Set randomization kernel
   float mag_ave[N_SHORT];                              // Average magnitude

   ve[1].e_mag = 0.f;                                    // Zero magnitude
   for (j = 0; j <=t[i].c_cnt; j++) {                   // Bootstrap once for each trigger
    mag_ave[j]=0.;                                      // Zero the average magnitude for this bootstrap
    for (k = 0; k <= t[i].c_cnt; k++) {                 // Select one point for each trigger
     kk = rand() % (t[i].c_cnt+1);                      // Use random trigger
     n = t[i].c_ind[kk];                                // Index of corrlatitudeed trigger
     if ( t[n].pors == 0 ) {                            // Use appropriate multilication factor (currently not used but will eventually)
      mul_amp = 1.f;                                    //
     } else {                                           //
      mul_amp = 1.f;                                    //
     };                                                 //
     mag_ave[j] += a*log(t[n].mag*b*mul_amp) + c*log(t[n].dis) + d; // Sum magnitude estimate from each trigger for average estimate
    }
    mag_ave[j]/= ( (float) t[i].c_cnt + 1.);            // Normalize summed mag estimates for average magnitude estimate
   }
   ve[1].e_mag = average( mag_ave, t[i].c_cnt);          // Average the averages
   ve[1].e_std = std_dev( mag_ave, t[i].c_cnt, ve[1].e_mag)*3.;// 3 sigma is the 99 % confidence (assuming a statistically large enough data set)
}
*/


float intensity_extrapolate(int pors, float dist, float dist_eq_nd, float pga1) {
/* This is a simplistic aproach to wave amplitude decay: 
   The amplitude for a cylindrically expanding wave should be 
   proportional to 1./sqrt(distance).  This blows up near zer 
   distance.  We should take attenuation into account. */

 float fact=1.;// = dist_eq_nd / dist;                        // Factor of node distance (for intensity map) to event-station distance (from trigger)
 if (pors == 0) {fact = 2.;}
// if (fact <=0.01) fact = 0.01;                          // If the factor will lead to order of magnitude stronger shaking, then cap it.

// float intensity_out = intensity_in/((fact+sqrt(fact))/2.);                   // Calculate the projected intensity

   float pga2 = pga1 * exp( -0.05*(dist_eq_nd-dist) );

 return pga2;                                  // 
}





//float intensity_extrapolate(int pors, float dist, float dist_eq_nd, float intensity_in) {
///* This is a simplistic aproach to wave amplitude decay: 
//   The amplitude for a cylindrically expanding wave should be 
//   proportional to 1./sqrt(distance).  This blows up near zer 
//   distance.  We should take attenuation into account. */
//
// float fact = dist_eq_nd / dist;                        // Factor of node distance (for intensity map) to event-station distance (from trigger)
// if (pors == 0) fact *= 2.;
// if (fact <=0.01) fact = 0.01;                          // If the factor will lead to order of magnitude stronger shaking, then cap it.
// float intensity_out = intensity_in/((fact+sqrt(fact))/2.);                   // Calculate the projected intensity
// return intensity_out;                                  // 
//}




void php_event_email(int i, const struct event& e, char* epath) {
// This subroutine should email us when we detect an earthquake 

   FILE *fpMail  = fopen(PATH_EMAIL, "w");                       // Open web file
   if (!fpMail) {
    log_messages.printf(MSG_CRITICAL,
       "Error in php_event_email - could not open file %s\n", PATH_EMAIL
    );
    return;  //error
   }

   fprintf(fpMail,"<?php\n");
   fprintf(fpMail,"chdir(\"/var/www/boinc/sensor/html/user/\");\n");
   fprintf(fpMail,"require_once(\"/var/www/boinc/sensor/html/inc/earthquake_email.inc\");\n");        // Include email php function

   time_t t_eq; struct tm * t_eq_gmt; t_eq = (int) e.e_time; t_eq_gmt = gmtime(&t_eq);     // Earthquake time

   fprintf(fpMail,"$mag  = %1.1f; \n",e.e_mag);                     // Set eq magnitude
   fprintf(fpMail,"$longitude = %4.3f; \n",e.longitude);                      // Set eq Lon 
   fprintf(fpMail,"$latitude = %4.3f; \n",e.latitude);                      // Set eq Lat
   fprintf(fpMail,"$depth = %2.1f; \n",e.depth);                      // Set Depth
   fprintf(fpMail,"$n_stations = %d; \n",e.e_cnt+1);                // Set # of stations used
   fprintf(fpMail,"$etime = %f; \n",e.e_time);                      // Set earthquake time
   fprintf(fpMail,"$dtime = %d; \n",e.e_t_now);                     // Set current time
   fprintf(fpMail,"$dt_detect  = %3.1f; \n",e.e_t_now-e.e_time); // Calculate time to detection
   fprintf(fpMail,"$edir       = %s; \n",epath);                       // Set directory
   fprintf(fpMail,"\nearthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);\n");


   fprintf(fpMail,"\n");                                                                       // Close php while loop.
   fprintf(fpMail,"?>\n");                                                                      // End php
   fclose(fpMail);

   char *sys_cmd = new char[_MAX_PATH];
   memset(sys_cmd, 0x00, sizeof(char) * _MAX_PATH);
   sprintf(sys_cmd,"%s %s", PHP_CMD, PATH_EMAIL);
   int retval = system(sys_cmd);
   delete [] sys_cmd;
}


void php_event_page(int i, const struct event& e, char* epath) {
// This subroutine creates a web page for the event. 
   
   
}


int QCN_IntensityMapGMT(const struct event& e, char* epath)
{
    int k, retval = 0;
    char *gmtfile = new char[_MAX_PATH];
    char *syscmd = new char[_MAX_PATH];
    memset(gmtfile, 0x00, sizeof(char) * _MAX_PATH);
    memset(syscmd, 0x00, sizeof(char) * _MAX_PATH);

    sprintf(gmtfile,"%s/gmt_script.csh", epath);
    FILE *fpGMT = fopen(gmtfile,"w");                      // gmt script
    log_messages.printf(MSG_DEBUG,
       "Create/run GMT map script %s\n", gmtfile
    );
    if (!fpGMT) {
      log_messages.printf(MSG_CRITICAL,
        "Error in intensity_map_gmt - could not open file %s\n", gmtfile
      );
      retval = 1;  //error
      goto ints_map_gmt_cleanup;
    }
   
    fprintf(fpGMT,"cd %s\n",epath);
    fprintf(fpGMT,"%s %s\n", PHP_CMD, GMT_MAP_PHP);
    fclose(fpGMT);     // Close script

//  Execute GMT script  
   sprintf(syscmd,"%s %s", CSHELL_CMD, gmtfile);
   retval = system(syscmd);

ints_map_gmt_cleanup:
   if (gmtfile) delete [] gmtfile;
   if (syscmd) delete [] syscmd;

   return retval;
}



void get_loc(float ilon, float ilat, float dis, float az, float olon, float olat) {
// This subroutine determins a new location based on a starting location, an azimuth and a distance to project to. 
  float pi = atan(1.)*4.;                                     // pi = 3.14....
  float az_r = az*pi/180.;                                    // azimuth in radians
  float latr = ilat*pi/180.;                                  // Latitude in radians
  float dlon = sin(az_r)*dis/111.19/abs(cos(latr));olon = ilon+dlon;// Longitude difference and longitude
  olat = ilat + cos(az_r)*dis/111.19;                         // New latitude
   
}


int QCN_IntensityMap(const struct event& e)
{
   time_t t_now; time(&t_now); e.e_t_now = (int) t_now;    // Current time
   float width=5; float dx=0.05;                              // Physical dimensions of grid
   int   nx = ((int) (width/dx)) + 1; int ny = nx;            // array dimension of grid
   float   dist,dist_eq_nd;                                          // Min distance from triger host to grid node
   float   ln_x,lt_x,imap;                                    // Location & intensity at grid node
   FILE *fp[6] = {NULL, NULL, NULL, NULL, NULL, NULL};                                            // Output file(s)
   int j,k,l,n,il;                                            // Index variables
   mode_t E_MASK=0777;                                        // File Permissions for new directory to create
   int email = 0;                                             // email=1 if we want to email people about new event
   int retval = 0;

   float longitude = e.longitude;                                    // Copy even longitude & Latitude
   float latitude = e.latitude;
   float x_min = longitude - width/2.f;                            // Minimum longitude of map
   float y_min = latitude - width/2.f;                            // Min Latitude

   float pi = atan(1.)*4.;                                   // pi = 3.14....
   float latr = e.latitude*pi/180.;                           // Latitude in radians
   time_t t_eq; t_eq = (int) e.e_time;double t_dif = difftime(t_now,t_eq);

// Create an event directory name                             
   char *edir = new char[_MAX_PATH];
   char *epath = new char[_MAX_PATH];
   char *epath2 = new char[_MAX_PATH];
   memset(edir, 0x00, sizeof(char) * _MAX_PATH);
   memset(epath, 0x00, sizeof(char) * _MAX_PATH);
   memset(epath2, 0x00, sizeof(char) * _MAX_PATH);

   sprintf(edir,"%08d", e.eventid);
   sprintf(epath,"%s%s",EVENT_PATH,edir);

// Create event base directory path                           
   struct stat st;                                            // I/O status for checking existance of directory/file
   if(stat(epath,&st) != 0) {                                 // If the path does not exist,
     retval = mkdir(epath,E_MASK);                            // Make new directory
   }

// Create iteration directory                                 
   char ABC[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";                   // All caps variables
   for (j = 0; j<64; j++) {                                   // For all letters
     sprintf(epath2,"%s/%c",epath,ABC[j]); // Create full path directory name
     if (stat(epath2,&st) != 0 ) {                            // If the directory doesn't exist, creat it
       retval = mkdir(epath2,E_MASK);                     // Make new directory
       break;                                                 // Stop here because this is where we want to put the files
     }
   }
   if (j<1) {email=1;} else {email=0;}                // Set email to send if first iteration

// Generate file names 
   char* strPath[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
   for (i = 0; i < 6; i++) {
     strPath[i] = new char[_MAX_PATH]; 
     memset(strPath[i], 0x00, sizeof(char) * _MAX_PATH);
   }
   sprintf(strPath[OUT_EVENT],         "%s/event.xy",          epath2);
   sprintf(strPath[OUT_STATION],       "%s/stations.xyz",      epath2);
   sprintf(strPath[OUT_INTENSITY_MAP], "%s/intensity_map.xyz", epath2);
   sprintf(strPath[OUT_CONT_TIME],     "%s/t_contour.xy",      epath2);
   sprintf(strPath[OUT_CONT_LABEL],    "%s/t_contour.txt",     epath2);
   sprintf(strPath[OUT_TIME_SCATTER],  "%s/t_scatter.xy",      epath2);

// Create a file with the event location (lon,lat only)       
   fp[OUT_EVENT] = fopen(strPath[OUT_EVENT],"w");      // Open event output file
   if (!fp[OUT_EVENT]) {
      retval = 1;
      log_messages.printf(MSG_CRITICAL,
         "Error in intensity_map OUT_EVENT file creation\n"
      );
      goto close_output_files;
   }

   fprintf(fp[OUT_EVENT],
       "%4.4f,%4.4f,%1.4f,%1.2f,%d,%f,%d,%1.1f,%1.2f,%1.2f\n",
     longitude,latitude,e.depth,e.e_mag, t[i].c_cnt+1,e.e_time,e.e_t_now,e.e_std,e.e_r2,e.e_msfit);// Output event location
   fclose(fp[OUT_EVENT]);                                              // Close event output file name
   fp[OUT_EVENT] = NULL;

// Create a file with the station information                 
   fp[OUT_STATION] = fopen(strPath[OUT_STATION],"w");                                  // Open station output file
   if (!fp[OUT_EVENT]) {
      retval = 1;
      log_messages.printf(MSG_CRITICAL,
         "Error in intensity_map OUT_EVENT file creation\n"
      );
      goto close_output_files;
   }

   for (k = 0; k<=t[i].c_cnt; k++) {                          // For each corrlatitudeed trigger
     n = t[i].c_ind[k];                                       // Index of corrlatitudeed trigger
     fprintf(fp[OUT_STATION],
        "%f,%f,%f,%d,%d,%s,%f,%d,%f,%f",
            t[n].longitude,t[n].latitude,t[n].mag,t[n].hostid,t[n].triggerid,t[n].file,t[n].trig,(int) t[n].rec,t[n].sig,t[n].dis);
     fprintf(fp[OUT_STATION],
         ",%f,%f,%f,%f,%f,%f,%f,%f \n",
            t[n].pgah[0],t[n].pgaz[0],t[n].pgah[1],t[n].pgaz[1],t[n].pgah[2],t[n].pgaz[2],t[n].pgah[3],t[n].pgaz[3]);
     // Output corrlatitudeed trigger loc & magnitude
     //t[ij].triggerid,t[ij].longitude,t[ij].latitude,t[ij].trig,(int) t[ij].rec,t[ij].sig,t[ij].mag,t[ij].dis)
   }
   fclose(fp[OUT_STATION]);                                               // Close station output file name
   fp[OUT_STATION] = NULL;

// Create contours for time rlatitudeive to identification time 
   fp[OUT_CONT_TIME] = fopen(strPath[OUT_CONT_TIME],"w");     // Open time contours output file.
   fp[OUT_CONT_LABEL] = fopen(strPath[OUT_CONT_LABEL],"w");     // label file for contours
   if (!fp[OUT_CONT_LABEL] || !fp[OUT_CONT_TIME]) {
      retval = 1;
      log_messages.printf(MSG_CRITICAL,
         "Error in intensity_map OUT_CONT_TIME/LABEL file creation %x %x\n", fp[OUT_CONT_TIME], fp[OUT_CONT_LABEL]
      );
      goto close_output_files;
   }

   for (j = 1; j<=9; j++) {                                  // For five distances
    float dti =  (float) (j-3) * 10.;                        // Time offset from detection time
    float dis = ((float) (j-3) * 10.+t_dif)*3.;              // Distance of time contours (10 s interval at 3km/s)
    if (dis > 0.) {                                          // Only use if distance greater than zero
     for (k=0; k<=180; k++) {                                // for each azimuth
      float az = (float) k * 2 * pi / 180.;                      // azimuth in radians
      float dlon = sin(az)*dis/111.19/abs(cos(latr));        // Longitudinal distance
      ln_x = e.longitude + dlon;                               // New longitude
      lt_x = e.latitude + cos(az)*dis/111.19;                 // New latitude
      fprintf(fp[OUT_CONT_TIME],"%f,%f\n",ln_x,lt_x);                     // Output contour
     }                                                       //
     fprintf(fp[OUT_CONT_TIME],">\n");                                    // Deliminator for separation between line segments
     fprintf(fp[OUT_CONT_LABEL],"%f %f 12 0 1 5 \\ %d \n",ln_x,lt_x,(int) dti );// Output labels for each contour
    }
   }                                                         //
   fclose(fp[OUT_CONT_TIME]);
   fclose(fp[OUT_CONT_LABEL]);
   fp[OUT_CONT_TIME] = fp[OUT_CONT_LABEL] = NULL;

// Create scatter plot data for observed v. estimated travel time 
   fp[OUT_TIME_SCATTER] = fopen(strPath[OUT_TIME_SCATTER],"w");                               // Open time scatter plot file
   if (!fp[OUT_TIME_SCATTER]) {
      retval = 1;
      log_messages.printf(MSG_CRITICAL,
         "Error in intensity_map OUT_TIME_SCATTER file creation\n"
      );
      goto close_output_files;
   }

   for (j = 1; j <= t[i].c_cnt; j++) {                       // For each corrlatitudeed trigger
    n = t[i].c_ind[j];                                       // index of corrlatitudeed triggers
    fprintf(fp[OUT_TIME_SCATTER],"%f,%f\n",t[n].trig-e.e_time,t[n].t_est-e.e_time); // Print out travel times
   }
   fclose(fp[OUT_TIME_SCATTER]);                                             // Close scatter plot file
   fp[OUT_TIME_SCATTER] = NULL;


   QCN_IntensityMapGMT(e,epath2);                              // Run Scripts for plotting (GMT)
   php_event_page(t,i,e,epath2);                             // Output event Page
   if (email==1) {
    php_event_email(t,i,e,edir);                            // Email if a new event
   }

close_output_files:
   if (edir) delete [] edir;
   if (epath) delete [] epath;
   if (epath2) delete [] epath2;
   for (i = 0; i < 6; i++) {
      if (fp[i]) fclose(fp[i]);
      fp[i] = NULL;
      if (strPath[i]) {
        delete [] strPath[i];
        strPath[i] = NULL;
      }
   }
   return retval;                                                   //
}

// this is the main routine for searching through the triggers (now in a vector vt) and see if they are corrlatitudeed in time & space
void QCN_DetectEvent()
{
/* This subroutine determines if a set of triggers is corrlatitudeed in time and space.
   The subroutine is used by program main, which assumes that the triggers have already been read in.
   The subroutine uses ang_dist_km, which provides the distance between two points in kilometers.
   The subroutine uses hid_yn, which determines if the primary trigger has already encountered a host ID or not.

   A corrlatitudeed station pair occurs when the distance is < 100 km apart and the trigger time difference
   is less than the velocity divided by the station separation.*/

   int i,j,k,l,kl;                             // Index variables
   int iCtr = vt.size() - 1;   // max index of the trigger vector
//   double  dt;                               // Time between triggers
   float   dist;                               // Distance between triggers
   int   nh = 0;int ih=0;                      // Number of hosts, ith host

   map<int, int> mapHost;   // a map of hosts by index & id

   int   h[N_LONG]; int ind[N_LONG];           // host ids already used
   h[0]=t[iCtr].hostid;                           // First host id is last in trigger list
   ind[0]=iCtr;                                // Index of host id's start at last trigger first

   if (iCtr < (C_CNT_MIN - 1)) return; // not enough triggers to do anything with
}


// this is the main routine for searching through the triggers (now in a vector vt) and see if they are corrlatitudeed in time & space
void QCN_DetectEvent_OLD()
{
/* This subroutine determines if a set of triggers is corrlatitudeed in time and space.
   The subroutine is used by program main, which assumes that the triggers have already been read in.
   The subroutine uses ang_dist_km, which provides the distance between two points in kilometers.
   The subroutine uses hid_yn, which determines if the primary trigger has already encountered a host ID or not.

   A corrlatitudeed station pair occurs when the distance is < 100 km apart and the trigger time difference
   is less than the velocity divided by the station separation.*/
   int i,j,k,l,kl;                             // Index variables
   int iCtr = vt.size() - 1;   // max index of the trigger vector
//   double  dt;                               // Time between triggers
   float   dist;                               // Distance between triggers
   int   nh = 0;int ih=0;                      // Number of hosts, ith host
   int   h[N_LONG]; int ind[N_LONG];           // host ids already used
   h[0]=t[iCtr].hostid;                           // First host id is last in trigger list
   ind[0]=iCtr;                                // Index of host id's start at last trigger first

   if (iCtr < (C_CNT_MIN - 1)) return; // not enough triggers to do anything with

   for (i=iCtr; i>=2; i--) {          // For each trigger (go backwards because triggers in order of latest first, and we want first first)
    vt[i].c_cnt=0;                              // Zero the count of corrlatitudeed triggers
    vt[i].c_ind[0]=i;                           // Index the same trigger as zeroth trigger
    ih = -10;                                    // Unassigned host id
    for (j = 0; j<=nh;j++) {                  // search through the assigned host ids
     if (vt[i].hostid == h[j]) {                  // to find a match
      ih = j;                                 // Match found
     }
    }
    if (ih<0) {                               // If no match found, then
     nh++;                                    // add a new assigned host id
     h[nh]=vt[i].hostid;                          // assign the new host id
     ind[nh]=i;                               // Index the trigger
    } else {                                  // If a prior match is found, then
//   Save peak fmag for all triggers within 5 seconds of first arrival
     if ( (vt[i].trig > vt[ind[ih]].trig) && (vt[1].trig < vt[ind[ih]].trig + 5.) ) { 
       // Trigger is older than prior, but less than prior + 5 seconds use the higher value
      if (vt[i].magnitude > vt[ind[nh]].magnitude) {vt[ind[nh]].magnitude=vt[i].magnitude;} // Use largest fmag for primary trigger
     }
    }

    if ( (vt[i].hostid != vt[i-1].hostid) && (ih < 0) ) {        // Do not use repeating triggers
     for (j = i-1; j>=1; j--) {                // For every other trigger
       if ( (vt[j].hostid != vt[i].hostid) && (vt[j].hostid != vt[j-1].hostid) && (abs(vt[i].trig - vt[j].trig) <= T_max) ) {
        //For non-repeating triggers & triggers less than t_max apart
        dist=ang_dist_km(vt[i].longitude,vt[i].latitude,vt[j].longitude,vt[j].latitude);//Distance between triggers
        if ( (abs(vt[i].trig-vt[j].trig)<dist/Vs + 3.f) && (dist<=D_max) ) {
         vt[i].c_cnt++;                          // Add count of corrlatitudeed triggers for this trigger
         if (vt[i].c_cnt>N_SHORT) {              // Make sure we don't use more corrlatitudeions than array size
          vt[i].c_cnt=N_SHORT;                   // Set count to max array size
          break;                                // Done
         }
         vt[i].c_ind[vt[i].c_cnt] = j;              // index of all correlaed triggers
         vt[i].c_hid[vt[i].c_cnt] = vt[i].hostid;       // Index of host ids
        }
       }
     }
    }
   }                                           // Done corrlatitudeing



/* Now we corrlatitudee triggers that are currently corrlatitudeed with triggers that are corrlatitudeed with the initial trigger, but not
   corrlatitudeed with the initial trigger itself */
   for (i =iCtr; i>1; i--) {                   // For each trigger
    if (vt[i].c_cnt > C_CNT_MIN) {              // If more than 4 corrlatitudeed triggers, possible regional event
     for (j = i-1;j>=0; j--) {                 // Compare with all later triggers
      if (vt[j].c_cnt > C_CNT_MIN) {            // Make sure this trigger is an event all of it's own
       kl = -10;
       for (k = 0; k<=vt[j].c_cnt;k++) {        // Compare all potential secondary corrlatitudeed triggers
        for (l = 0; l<=vt[i].c_cnt;l++) {       // Make sure trigger isn't same host as prior trigger
         if (vt[i].c_ind[l]==vt[j].c_ind[k]) {
          kl = l;break;
         }
        }
        if (kl < 0) {                         // If no matching trigger, then add secondary trigger to primary trigger list
         vt[i].c_cnt++;
         vt[i].c_ind[t[i].c_cnt]=t[j].c_ind[k];
         vt[i].c_hid[t[i].c_cnt]=t[j].c_hid[k];
        }
        vt[k].c_cnt = 0;                        // get rid of corrlatitudeed triggers (now that they are primary triggers)
       }
      }
     }
     j = i;
     if ( ( vt[i].trig > T_max+e.e_time)||(abs(vt[i].latitude-e.latitude)>3.) ) {
      e.eventid++;                              // If new Time or location, then new event
      e.e_cnt=0;                            // Zero trigger count for event count for if new location/time
     }
     if ((vt[i].c_cnt > e.e_cnt)||((vt[i].trig-e.e_t_detect>5.)&&(vt[i].c_cnt=e.e_cnt))) {            // Only do new event location ... if more triggers for same event (or new event - no prior triggers)

      QCN_EventLocate(j);                 // Try to locate event
      if (e.e_r2 < 0.5) { break; }          // Stop event if no event located
      e.e_cnt = vt[i].c_cnt;
      estimate_magnitude_bs(j);          // Estimate the magnitude of the earthquake
      intensity_map(j);                    // This generates the intensity map
     }
    }
   }

}

/*
void get_bad_hosts(struct bad_hosts bh) {
//  This subrouting retrieves the bad host names 
   FILE *fpBadHosts = fopen(BAD_HOSTS_FILE,"r");
   bh.nh = -1;
   while (!feof(fpBadHosts)) {
    bh.nh++;
    fscanf(fpBadHosts,"%d",&bh.hostid[bh.nh]);
   }
   fclose(fpBadHosts);
   return;
}
*/

int main(int argc, char** argv) 
{
    int retval;
    // initialize random seed: 
    srand ( time(NULL) );

//  load CRUST2.0 3D seismic velocity model for the crust: 
    int icrust = crust2_load();

    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        return 1;
    }

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-sleep_interval")) {
            g_dSleepInterval = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-time_interval")) {
            g_iTriggerTimeInterval = atoi(argv[++i]);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "bad cmdline arg: %s\n\n"
                "Example usage: trigmon -d 3 -sleep_interval 3 -count 10 -time_interval 10\n\n"
             , argv[i]
            );
            return 2;
        }
    }
    if (g_dSleepInterval < 0) g_dSleepInterval = TRIGGER_SLEEP_INTERVAL;
    if (g_iTriggerTimeInterval < 0) g_iTriggerTimeInterval = TRIGGER_TIME_INTERVAL;

    install_stop_signal_handler();
    atexit(QCN_DBClose);

    retval = QCN_DBOpen();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        return 3;
    }

    log_messages.printf(MSG_NORMAL,
            "trigmon started with the following options:\n"
            "  -time_interval   = %d\n" 
            "  -sleep_interval  = %f\n",
         g_iTriggerTimeInterval,
         g_dSleepInterval
    ); 

    //signal(SIGUSR1, show_state);
    while (1) {
      g_dTimeCurrent = dtime();
      double dtEnd = g_dTimeCurrent + g_dSleepInterval;

      QCN_GetTriggers();  // reads the memories from the trigger memory table into a global vector
      QCN_DetectEvent();     // searches the vector of triggers for matching events

      check_stop_daemons();  // checks for a quit request
      g_dTimeCurrent = dtime();
      if (g_dTimeCurrent < dtEnd && (dtEnd - g_dTimeCurrent) < 60.0) { // sleep a bit if not less than 60 seconds
          log_messages.printf(MSG_DEBUG, "Sleeping %f seconds....\n", dtEnd - g_dTimeCurrent);
          boinc_sleep(dtEnd - g_dTimeCurrent);
      } 
    }
    QCN_DBClose();
    return 0;
}

