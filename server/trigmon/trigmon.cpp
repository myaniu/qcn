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

//int g_idEvent = 1;  // internal counter of event id, eventually we use database qcn_quakeid

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
   static int iCounter = 0; // use this to keep track of 
   DB_QCN_TRIGGER_MEMORY qtm;
   struct trigger t;
   int j;
   //char strKeyTrigger[32];
   char strWhere[64];
   const int iTestTime = 120;  // number of seconds to flush out quake events so our vector doesn't get too big

   // flush out old events every few minutes
   if ( iCounter++ == (int) ( (float) iTestTime / g_dSleepInterval ) && ve.size() > 0 ) {
       // flush out old events
       iCounter = 0;
       // get time from database
       long int lTest = getMySQLUnixTime() - iTestTime;
       vector<struct event>::iterator it = ve.begin();
       while (it != ve.end()) {
         if (it->e_time < lTest) { // remove this record 
           it = ve.erase(it);  // erase and set new iterator start as elements will shift
         }
         else { // just increment iterator
           it++;
         }
       }
   }

   // clear our local variables
   qtm.clear();
   t.clear();

   vt.clear();  // clear our trigger vector since we will rebuild it from the database below

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
      t.hostid  = qtm.hostid;            // Host ID
      t.triggerid  = qtm.triggerid;      // Trigger ID
      t.qcn_quakeid = qtm.qcn_quakeid;   // bring in the matching quake id, if any yet!
      t.posted = qtm.posted;             // file req posted?
      strncpy(t.db, qtm.db_name, sizeof(t.db)-1);   // Database Name
      strncpy(t.file, qtm.file, sizeof(t.file)-1);  // File name
      t.latitude = qtm.latitude;         // Latitude
      t.longitude= qtm.longitude;        // Longitude
      t.time_trigger = qtm.time_trigger; // Trigger Time
      t.time_received = qtm.time_received; // Time Trigger received
      t.significance  = qtm.significance;  // Significance (Trigger detection filter)
      t.magnitude  = qtm.magnitude;        // set mag to magnitude at time of trigger
      t.pgah[0] = qtm.mxy1p;               // Peak Ground Acceleration (1 second before and during trigger) (m/s/s)
      t.pgaz[0] = qtm.mz1p;                // Peak Ground Acceleration (1 seconds before and during trigger) (m/s/s)
    
      t.pgah[1] = qtm.mxy1a;               // Peak Ground Acceleration (1 second after trigger)  (m/s/s)
      t.pgaz[1] = qtm.mz1a;                // Peak Ground Acceleration (1 second after trigger)  (m/s/s)
    
      t.pgah[2] = qtm.mxy2a;               // Peak Ground Acceleration (2 seconds after trigger)  (m/s/s)
      t.pgaz[2] = qtm.mz2a;                // Peak Ground Acceleration (2 seconds after trigger)  (m/s/s)
    
      t.pgah[3] = qtm.mxy4a;               // Peak Ground Acceleration (4 seconds after trigger) (m/s/s)
      t.pgaz[3] = qtm.mz4a;                // Peak Ground Acceleration (4 seconds after trigger) (m/s/s)

      for (j=0;j<=3;j++) {
       if (t.pgah[j] > t.magnitude) t.magnitude = t.pgah[j];  
       if (t.pgaz[j] > t.magnitude) t.magnitude = t.pgah[j];  
      }   
      // now insert trigger 
      vt.push_back(t);  // insert into our vector of triggers

     }  // USB sensors only
   }
   return (int) vt.size();
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

   for (i=0; i<mx_cr_lt; i++) { crm.lat[i]=90.f-( ((float)i)+0.5)*dx_cr ;  }                      // for each latitude

   for (i=0; i<mx_cr_ln; i++) { crm.lon[i]=     ( ((float)i)+0.5)*dx_cr ;  }                      // for each longitude

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

      v[0]/=d;  // Normalize weighted sum by cumulative weight (cumulative depth) for average
      v[1]/=d;  // Normalize weighted sum by cumulative weight (cumulative depth) for average

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

float correlate(float* datx, float* daty, const int& ndat) {
//  This function correlates two data sets (datx & daty) of length (ndat).  It returns the R^2 value (not r). 
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
   return R;                                              // Return with correlation coefficient
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

bool QCN_EventLocate(const bool& bEventFound, struct event& e, const int& ciOff)
{
   log_messages.printf(MSG_DEBUG,
      "Locate possible event ID %d, Count %d \n", e.eventid, vt[ciOff].c_cnt
   );

   // CMC the next line was handled above in QCN_Detect and basically we can use bEventFound if this event was already used
   //if ( ( vt[j].time_trigger > T_max+e.e_time) || (abs(t[i].latitude-e.latitude)>3.) ) e.eventid=0;         // If new Time or location, then new event
   
   float  width = 4.f; float zrange=150.f;                // Lateral and vertical grid size
   float  dx = 0.1f; float dz = 10.f;                     // Lateral and vertical grid step size                  
   int    n_iter=3;                                       // Number of grid search iterations
   float  ln_x=0.,lt_x=0.,dp_x=0.;                                 // Lon, Lat, & Depth of each grid point
   float  dn=0.; float dp=0.;                                // Distances to the nth and pth trigger location
   double  dt,dt_min;                                     // Travel time difference between two stations (and minimum for P & S wave)
   double  ls_mf,ls_ct;                                   // Least-squares misfit & count (For normalization)
   float   v[2];                                          // Seismic Velocities (P & S)
   int     h,j,k,l,m,n,o,p,q,r,j_min;                     // Index variables
   double t_min = vt[ciOff].time_trigger;                              // Minimum trigger time
   struct bounds g;                                       // Bounds of grid

   j_min = ciOff;                               // Assume first trigger is i'th trigger (not necessarily the case and will be checked)
   for (j = 0; j<=vt[ciOff].c_cnt; j++) {                      // Find earliest trigger
    n = vt[ciOff].c_ind[j];                                    // index of correlated series
    if (t_min < vt[n].time_trigger) {                              // If earlier trigger, then use that as first time
     t_min = vt[n].time_trigger;                                   //
     j_min = n;                                           //
    }
   }                                                      //
   e.longitude = vt[j_min].longitude;                             // Start with assumption of quake at/near earliest trigger
   e.latitude = vt[j_min].latitude;
   e.depth = 33.;                                       // Start with default Moho depth eq.
   
   for (j = 1; j<=n_iter; j++) {                          // For each iteration 
//    set_grid3D(&g, e.longitude, e.latitude, e.depth, width, dx, zrange, dz);        // Set bounds of grid search
    g.yw = width;                                         // Latitudinal width of grid
    g.xw = width;                                         // Latitudinal width of grid
    g.zw = zrange;                                        // Vertical dimension of grid
    
    g.dx = dx;                                            // Set grid interval
    g.dz = dz;
    g.dy = dx;

    g.x_min = e.longitude-g.xw/2.f;                         // Set bounds of grid
    g.x_max = e.longitude+g.xw/2.f;
    g.y_min = e.latitude-g.yw/2.f;
    g.y_max = e.latitude+g.yw/2.f;
    g.z_min = e.depth-g.zw/2.f;if (g.z_min < 0.) {g.z_min=0.f;};
    g.z_max = e.depth+g.zw/2.f;
 
    g.ny = (int) ((g.y_max-g.y_min)/g.dy);                // Set number of latitudinal steps
    g.nx = (int) ((g.x_max-g.x_min)/g.dx);                // Set number of longitudinal steps
    g.nz = (int) ((g.z_max-g.z_min)/g.dz);                // Set number of depth steps  
    float ls_mf_min = 9999999999.f;                       // Set obserdly high misfit minimum
    for (h = 1; h<=g.nz; h++) {                           // For each vertical grid step
     dp_x = g.z_min + g.dz * (float) (h-1);               // Calculate depth
//     vel_calc(dp_x,v);                                    // Calculate mean path velocity
     crust2_get_mean_vel(dp_x,vt[j_min].longitude,vt[j_min].latitude,v); // Get depth-averaged velocity for location from CRUST2.0
     for (k = 1; k<=g.nx; k++) {                          // For each x node
      ln_x = g.x_min + g.dx * (float) (k-1);              // Longitude of grid point
      for (l = 1; l<=g.nx; l++) {                         // For each y node
       lt_x = g.y_min + g.dy * (float) (l-1);             // Latitude of grid point
       ls_mf = 0.; ls_ct = 0.;                            // Zero least-squares misfit & count

// Compare times and distance difference between each station pair 
       for (m = 0; m < vt[ciOff].c_cnt; m++) {                  // For each trigger
        n = vt[ciOff].c_ind[m];                                // Index of correlated trigger
        dn=ang_dist_km(ln_x,lt_x,vt[n].longitude,vt[n].latitude);    // Horizontal distance between node and host/station
        dn = sqrt(dn*dn + dp_x*dp_x);                     // Actual distance between event & station/host
        
        for (o = m+1; o<=vt[ciOff].c_cnt; o++) {               // Compare with each other trigger
         p = vt[ciOff].c_ind[o];                               // Index of correlated trigger
         dp=ang_dist_km(ln_x,lt_x,vt[p].longitude,vt[p].latitude);   // Distance between triggers
         dp = sqrt(dp*dp + dp_x*dp_x);                    // Distance corrected for depth
         dt_min = 9999999.f;                              // Search For best P-wave & S-wave pairing
         for (q=0;q<=1;q++) {                             //    First station P(0) or S(1)
          for (r=0;r<=1;r++) {                            //    Second station P(0) or S(1)
           dt = (vt[n].time_trigger-vt[p].time_trigger) - (dn/v[q]-dp/v[r]);//    Interstation time difference - theoretical time difference
           if (dt*dt < dt_min*dt_min) {
            dt_min = dt;};                                //    Use the smallest value (assume if neerer zero then probably true)
          }
         }
         ls_mf = ls_mf + (dt_min*dt_min);                 // Sum L2 misfit 
         ls_ct = ls_ct + 1.f;                             // Count For normalizing misfit
        } // o
       } // m      
       if (ls_ct > 0.) {                                  // make sure at least one point used
        ls_mf = ls_mf/ls_ct;                              // Normalize misfit by count
        if (ls_mf < ls_mf_min) {                          // If minimum misfit/max correlation save location & r2
         ls_mf_min = ls_mf;                               // Save new misfit minimum
         e.longitude=ln_x; e.latitude=lt_x; e.depth=dp_x;  // Save location of best fit
        }                                                 // End loop over 2nd trigger of pair
       }                                                  // End loop over 1st trigger of pair
      } // l                                                   // End loop over latitude
     } // k                                                    // End loop over Longitude
    }  // h (depth)                                                     // End loop over depth
    dx /=10.f; width/=10.f; dz/=10.f; zrange/=10.f;       // Reduce grid dimensions by an order of magnitude

// Re-compare times and distance difference between each station pair for best location and identify which phase each trigger is 
//    vel_calc(e.depth,v);                                // Get velocity (note - depth dependant - not done well).
    crust2_get_mean_vel(dp_x,vt[j_min].longitude,vt[j_min].latitude,v); // Get depth-averaged velocity for location from CRUST2.0
    int q_save=0;
    e.e_time=0.;
    for (m = 0; m<=vt[ciOff].c_cnt; m++) {                     // For each trigger
     n = vt[ciOff].c_ind[m];                                   // Index of correlated trigger
     vt[n].pors = 0;
     dn=ang_dist_km(e.longitude,e.latitude, vt[n].longitude, vt[n].latitude);// Horizontal distance between node and host/station
     dn = sqrt(dn*dn + e.depth*e.depth);                 // Actual distance between event & station/host
        
     for (o = 0; o<=vt[ciOff].c_cnt; o++) {                    // Compare with each other trigger
      p = vt[ciOff].c_ind[o];                                  // Index of correlated trigger
      if (p!=n) {
       dp=ang_dist_km(e.longitude,e.latitude,vt[p].longitude,vt[p].latitude);  // Distance between triggers
       dp = sqrt(dp*dp + e.depth*e.depth);
       dt_min = 9999999.f;                                // Search For best P-wave & S-wave pairing
       for (q=0;q<=1;q++) {                               //    First station P(0) or S(1)
        for (r=0;r<=1;r++) {                              //    Second station P(0) or S(1)
         dt = (vt[n].time_trigger-vt[p].time_trigger) - (dn/v[q]-dp/v[r]);  //    Interstation time difference - theoretical time difference
         if (dt*dt < dt_min*dt_min) {                     //    Use lowest time misfit to identify P or S
          q_save = q;                                     //    Save P or S
          dt_min = dt;                                    //    Save minimum time
         }                                                //
        }                                                 //
       }                                                  //
       vt[n].pors+=q_save;                                 // Sum most optimal values to see if it lies closer to P or S
      }
     }  // o     
     vt[n].pors = (int) round( (float) vt[n].pors / ((float) vt[ciOff].c_cnt +1.));  // Normalize P or S to see if it is closer to P or S
     vt[n].dis=dn;                                         // Save distance
     e.e_time += vt[n].time_trigger - dn/v[vt[n].pors];          // Time of event calculated from ith trigger.
    } // m                                                     //

// Time of earthquake: 
   e.e_time = e.e_time / ((float) vt[ciOff].c_cnt + 1.); // Time of earthquake calculated by averaging time w.r.t each station normalized by number of stations
   if (!bEventFound) {// e.eventid<=0) {                                    // For New earthquake ID if earthquake time wasn't set
      e.eventid = ((int) e.e_time);                       //
      log_messages.printf(MSG_DEBUG,
         "NEW EID: %d \n", e.eventid
      );
   }
 } // j  ie big outer loop ending

//  Determine maximum dimension of array  
   float ss_dist_max = -999999999.;                       // Start with large station to station maximum array dimension
   for (j = 0; j<vt[ciOff].c_cnt; j++) {                       // For each trigger/station
    n = vt[ciOff].c_ind[j];                                    // Index of jth station
    for (k = j+1; k<=vt[ciOff].c_cnt; k++) {                   // For each other trigger/station
     m = vt[ciOff].c_ind[k];                                   // Index of kth station
     dn = ang_dist_km(vt[m].longitude,vt[m].latitude,vt[n].longitude,vt[n].latitude);// Distance between stations
     if (dn > ss_dist_max) ss_dist_max = dn;              // Store if maximum distance
    }                                                     // 
   }                                                      //

//  Require that the earthquake-to-array distance be less than four times the dimension of the array 
   if (ss_dist_max*4. < vt[j_min].dis) {                   // If the event-to-station distance > 4 times the array dimension
    e.e_r2=-1.;                                        // Set correlation to -1 (will reject earthquake)
    log_messages.printf(MSG_DEBUG,
       "Event poorly located: Array dimension=%f EQ Dist=%f.\n",ss_dist_max,dn //Status report
    );
    return false;                                               // Return so done
   } else {                                               // Otherwise output status report
    log_messages.printf(MSG_DEBUG,
       "Event located: %f %f\n",e.longitude,e.latitude
    );
   }
   
//  Calculate the estimated arrival time of the phase 
   float t_obs[vt[ciOff].c_cnt];                               // Observed phase arrival times (from triggers)
   float t_est[vt[ciOff].c_cnt];                               // Estimated phase arrival times (from earthquake location estimate)
   float dt_ave=0;                                        // Average model-data time variance
   e.e_msfit = 0.;                                     // Zero misfit for model given set P & S wave times
   for (j=0;j<=vt[ciOff].c_cnt;j++) {                          // For each trigger
    n = vt[ciOff].c_ind[j];                                    // Index of triggers
    vt[n].time_est = e.e_time + vt[n].dis / v[vt[n].pors];   // Estimated time is event origin time plus distance divided by velocity
    t_obs[j]=vt[n].time_trigger-e.e_time;                       // Store observed times
    t_est[j]=vt[n].time_est-e.e_time;                      // Store estimated times
    dt_ave += abs(t_obs[j]-t_est[j]);                     // Sum averaged model-data time variance
    e.e_msfit += (t_obs[j]-t_est[j])*(t_obs[j]-t_est[j]);// Sum L2 misfit
   }
   e.e_msfit /= ( (float) vt[ciOff].c_cnt + 1.);            // Normalize the event misfit
   e.e_msfit = sqrt(e.e_msfit);                     // 
//  Require that the model variance is less than two seconds 
   dt_ave /= ( (float) vt[ciOff].c_cnt + 1.);                  // Normalize the data-model variance
   if (dt_ave > 2.) {                                     // If the average travel time misfit is greater than 2 seconds, reject the event detection
    e.e_r2 = -1.;                                      // Set correlation to < 0 for rejection
    return false;                                               // Return with bad correlation
   }

//  correlate observed and estimated travel times  
    e.e_r2 = correlate(t_obs, t_est, vt[ciOff].c_cnt);  // Correlate observed & estimated times
    log_messages.printf(MSG_DEBUG,
       "Estimated times correlate at r^2= %f \n",e.e_r2
    );
   return true;
}


float intensity_extrapolate(int pors, float dist, float dist_eq_nd, float pga1) {
/* This is a simplistic aproach to wave amplitude decay: 
   The amplitude for a cylindrically expanding wave should be 
   proportional to 1./sqrt(distance).  This blows up near zer 
   distance.  We should take attenuation into account. */

 float fact=1.;// = dist_eq_nd / dist;                        // Factor of node distance (for intensity map) to event-station distance (from trigger)
 if (pors == 0) fact = 2.;
// if (fact <=0.01) fact = 0.01;                          // If the factor will lead to order of magnitude stronger shaking, then cap it.

// float intensity_out = intensity_in/((fact+sqrt(fact))/2.);                   // Calculate the projected intensity

   float pga2 = pga1 * exp( -0.05*(dist_eq_nd-dist) );

 return pga2;                                  // 
}

void php_event_email(const struct event& e, const char* epath) {
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

   fprintf(fpMail,"$mag  = %1.1f; \n",e.magnitude);                     // Set eq magnitude
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
   if (retval) {
    log_messages.printf(MSG_CRITICAL,
       "Error on email shell script %s\n", sys_cmd
    );
   }

   delete [] sys_cmd;
}


void php_event_page(const struct event& e, const char* epath) {
// This subroutine creates a web page for the event. 
   
   
}


int QCN_IntensityMapGMT(struct event& e, const char* epath)
{
    int retval = 0;
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


int QCN_IntensityMap(const bool& bInsertEvent, struct event& e, const int& ciOff)
{
   time_t t_now; time(&t_now); e.e_t_now = (int) t_now;    // Current time
   //float width=5; float dx=0.05;                              // Physical dimensions of grid
   //int   nx = ((int) (width/dx)) + 1; int ny = nx;            // array dimension of grid
   //float   dist,dist_eq_nd, imap;                                          // Min distance from triger host to grid node
   float   ln_x,lt_x;                                    // Location & intensity at grid node
   FILE *fp[6] = {NULL, NULL, NULL, NULL, NULL, NULL};                                            // Output file(s)
   int j,k,n;
   //int l,il;                                            // Index variables
   mode_t E_MASK=0777;                                        // File Permissions for new directory to create
   int email = 0;                                             // email=1 if we want to email people about new event
   int retval = 0;

   float longitude = e.longitude;                                    // Copy even longitude & Latitude
   float latitude = e.latitude;
   //float x_min = longitude - width/2.f;                            // Minimum longitude of map
   //float y_min = latitude - width/2.f;                            // Min Latitude

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

   sprintf(edir,"%d", e.eventid);
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
   for (int ii = 0; ii < 6; ii++) {
     strPath[ii] = new char[_MAX_PATH]; 
     memset(strPath[ii], 0x00, sizeof(char) * _MAX_PATH);
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
     longitude,latitude,e.depth,e.magnitude, vt[ciOff].c_cnt+1,e.e_time,e.e_t_now,e.e_std,e.e_r2,e.e_msfit);// Output event location
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

   for (k = 0; k<=vt[ciOff].c_cnt; k++) {                          // For each correlated trigger
     n = vt[ciOff].c_ind[k];                                       // Index of correlated trigger
     fprintf(fp[OUT_STATION],
        "%f,%f,%f,%d,%d,%s,%f,%d,%f,%f",
             vt[n].longitude,vt[n].latitude,vt[n].magnitude,
             vt[n].hostid,vt[n].triggerid,vt[n].file,
            vt[n].time_trigger,(int) vt[n].time_received,vt[n].significance,vt[n].dis);
     fprintf(fp[OUT_STATION],
         ",%f,%f,%f,%f,%f,%f,%f,%f \n",
            vt[n].pgah[0],vt[n].pgaz[0],vt[n].pgah[1],vt[n].pgaz[1],vt[n].pgah[2],vt[n].pgaz[2],vt[n].pgah[3],vt[n].pgaz[3]);
     // Output correlated trigger loc & magnitude
     //t[ij].triggerid,t[ij].longitude,t[ij].latitude,t[ij].time_trigger,(int) t[ij].time_received,t[ij].sig,t[ij].mag,t[ij].dis)
   }
   fclose(fp[OUT_STATION]);                                               // Close station output file name
   fp[OUT_STATION] = NULL;

// Create contours for time rlatitudeive to identification time 
   fp[OUT_CONT_TIME] = fopen(strPath[OUT_CONT_TIME],"w");     // Open time contours output file.
   fp[OUT_CONT_LABEL] = fopen(strPath[OUT_CONT_LABEL],"w");     // label file for contours
   if (!fp[OUT_CONT_LABEL] || !fp[OUT_CONT_TIME]) {
      retval = 1;
      log_messages.printf(MSG_CRITICAL,
         "Error in intensity_map OUT_CONT_TIME/LABEL file creation %lx %lx\n", (unsigned long) fp[OUT_CONT_TIME], (unsigned long) fp[OUT_CONT_LABEL]
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

   for (j = 1; j <= vt[ciOff].c_cnt; j++) {                       // For each correlated trigger
    n = vt[ciOff].c_ind[j];                                       // index of correlated triggers
    fprintf(fp[OUT_TIME_SCATTER],"%f,%f\n",vt[n].time_trigger-e.e_time,vt[n].time_est-e.e_time); // Print out travel times
   }
   fclose(fp[OUT_TIME_SCATTER]);                                             // Close scatter plot file
   fp[OUT_TIME_SCATTER] = NULL;


   QCN_IntensityMapGMT(e,epath2);                              // Run Scripts for plotting (GMT)

   // quake event info
   QCN_UpdateQuake(bInsertEvent, e, ciOff);

   php_event_page(e,epath2);                             // Output event Page
   if (email==1) {
    php_event_email(e,edir);                            // Email if a new event
   }

close_output_files:
   if (edir) delete [] edir;
   if (epath) delete [] epath;
   if (epath2) delete [] epath2;
   for (int i = 0; i < 6; i++) {
      if (fp[i]) fclose(fp[i]);
      fp[i] = NULL;
      if (strPath[i]) {
        delete [] strPath[i];
        strPath[i] = NULL;
      }
   }
   return retval;                                                   //
}

// this is the main routine for searching through the triggers (now in a vector vt) and see if they are correlated in time & space
void QCN_DetectEvent()
{
/* This subroutine determines if a set of triggers is correlated in time and space.
   The subroutine is used by program main, which assumes that the triggers have already been read in.
   The subroutine uses ang_dist_km, which provides the distance between two points in kilometers.
   The subroutine uses hid_yn, which determines if the primary trigger has already encountered a host ID or not.

   A correlated station pair occurs when the distance is < 100 km apart and the trigger time difference
   is less than the velocity divided by the station separation.*/
   int i,j,k,l,kl;                             // Index variables
   int iCtr = vt.size() - 1;   // max index of the trigger vector
//   double  dt;                               // Time between triggers
   struct event e;  // temp test event
   float   dist;                               // Distance between triggers
   int   nh = 0;int ih=0;                      // Number of hosts, ith host
   int   h[N_LONG]; int ind[N_LONG];           // host ids already used
  
   memset(h, 0x00, sizeof(int) * N_LONG);
   memset(ind, 0x00, sizeof(int) * N_LONG);
 
   h[0]=vt[iCtr].hostid;                           // First host id is last in trigger list
   ind[0]=iCtr;                                // Index of host id's start at last trigger first

   if (iCtr < (C_CNT_MIN - 1)) return; // not enough triggers to do anything with

   for (i=iCtr; i>=2; i--) {          // For each trigger (go backwards because triggers in order of latest first, and we want first first)
    vt[i].c_cnt=0;                              // Zero the count of correlated triggers
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
     if ( (vt[i].time_trigger > vt[ind[ih]].time_trigger) && (vt[1].time_trigger < vt[ind[ih]].time_trigger + 5.) ) { 
       // Trigger is older than prior, but less than prior + 5 seconds use the higher value
      if (vt[i].magnitude > vt[ind[nh]].magnitude) {vt[ind[nh]].magnitude=vt[i].magnitude;} // Use largest fmag for primary trigger
     }
    }

    if ( (vt[i].hostid != vt[i-1].hostid) && (ih < 0) ) {        // Do not use repeating triggers
     for (j = i-1; j>=1; j--) {                // For every other trigger
       if ( (vt[j].hostid != vt[i].hostid) && (vt[j].hostid != vt[j-1].hostid) && (abs(vt[i].time_trigger - vt[j].time_trigger) <= T_max) ) {
        //For non-repeating triggers & triggers less than t_max apart
        dist=ang_dist_km(vt[i].longitude,vt[i].latitude,vt[j].longitude,vt[j].latitude);//Distance between triggers
        if ( (abs(vt[i].time_trigger-vt[j].time_trigger)<dist/Vs + 3.f) && (dist<=D_max) ) {
         vt[i].c_cnt++;                          // Add count of correlated triggers for this trigger
         if (vt[i].c_cnt>N_SHORT) {              // Make sure we don't use more correlations than array size
          vt[i].c_cnt=N_SHORT;                   // Set count to max array size
          break;                                // Done
         }
         vt[i].c_ind[vt[i].c_cnt] = j;              // index of all correlaed triggers
         vt[i].c_hid[vt[i].c_cnt] = vt[i].hostid;       // Index of host ids
        } // if abs
       } // if vt[=j]
     } // for j
    }  // if vt[i] hostid
   } // for i                                          // Done correlating



/* Now we correlate triggers that are currently correlated with triggers that are correlated with the initial trigger, but not
   correlated with the initial trigger itself */
   bool bInsertEvent = false; // if true then we should insert this into qcn_quake table
   for (i =iCtr; i>1; i--) {                   // For each trigger
    if (vt[i].c_cnt > C_CNT_MIN) {              // If more than 4 correlated triggers, possible regional event
     for (j = i-1;j>=0; j--) {                 // Compare with all later triggers
      if (vt[j].c_cnt > C_CNT_MIN) {            // Make sure this trigger is an event all of it's own
       kl = -10;
       for (k = 0; k<=vt[j].c_cnt;k++) {        // Compare all potential secondary correlated triggers
        for (l = 0; l<=vt[i].c_cnt;l++) {       // Make sure trigger isn't same host as prior trigger
         if (vt[i].c_ind[l]==vt[j].c_ind[k]) {
          kl = l;break;
         }
        }
        if (kl < 0) {                         // If no matching trigger, then add secondary trigger to primary trigger list
         vt[i].c_cnt++;
         vt[i].c_ind[vt[i].c_cnt]=vt[j].c_ind[k];
         vt[i].c_hid[vt[i].c_cnt]=vt[j].c_hid[k];
        }
        vt[k].c_cnt = 0;                        // get rid of correlated triggers (now that they are primary triggers)
       } // k
      } // vt[j].c_cnt
     } // j 
     j = i;
     bool bEventFound = false;
     e.clear(); // CMC use a temp event struct that we can fill in and update the array as necessary with the dirty bit (or insert if a new one)
     for (k = 0; k < (int) ve.size(); k++)  { // go through all the events for a match, if not make a new event
       if ( ( vt[i].time_trigger <= T_max+ve[k].e_time) && (abs(vt[i].latitude-ve[k].latitude)<=3.) ) {
             // this is a match by time & location (just using lat though?)
          bEventFound = true;
          e = ve[k];  // should have a qcn_quakeid here as it should have been inserted in the original pass
          break;
       }
      /* if ( ( vt[i].time_trigger > T_max+ve[k].e_time)||(abs(vt[i].latitude-e.latitude)>3.) ) {
       } */
     } // for k
     if (!bEventFound) { // need to add a new quake event
        e.eventid = (long) vt[i].time_trigger; //g_idEvent++; // If new Time or location, then new event
        e.e_cnt=0;                            // Zero trigger count for event count for if new location/time
        e.dirty = true;
     }

     if ((vt[i].c_cnt > e.e_cnt)||((vt[i].time_trigger-e.e_t_detect>5.)&&(vt[i].c_cnt=e.e_cnt))) { 
       // Only do new event location ... if more triggers for same event (or new event - no prior triggers)
       if (!QCN_EventLocate(bEventFound, e, i) || e.e_r2 < 0.5) { 
          break; 
       }          // Stop event if no event located
       e.e_cnt = vt[i].c_cnt;
       bInsertEvent = (bool) (e.qcn_quakeid == 0); // if 0 then we don't have a quake id yet hence need to insert
       QCN_EstimateMagnitude(e, i); //  // Estimate the magnitude of the earthquake
       QCN_IntensityMap(bInsertEvent, e, i);   // This generates the intensity map -- also where quake info updated in the qcn_quake & qcn_trigger tables
     }
     if (bEventFound) { // replace element
        ve[k] = e;
     }
     else {
        ve.push_back(e);
     }
   } // if vt[i]
 } // for i
}

// insert or update this event in the qcn_quake table
// we will also want to get the triggers and update their data in qcn_trigger as well,
// including sending a file upload request if one wasn't posted already
void QCN_UpdateQuake(const bool& bInsert, struct event& e, const int& ciOff)
{
   //if (!e.dirty) return; 
   int retval = 0;
   bool bFound = false;
   DB_QCN_QUAKE dqq;

   if (bInsert) {
     dqq.clear();
   }
   else {
     bFound = (bool) (dqq.lookup_id(e.qcn_quakeid) == 0); // boinc db fn 0 means found
   }

   dqq.time_utc = e.e_time;
   dqq.magnitude = e.magnitude;
   dqq.depth_km = e.depth;
   dqq.latitude = e.latitude;
   dqq.longitude = e.longitude;
   sprintf(dqq.description, "QCN Detected Event # %d : #Sens=%d Std=%f r2=%f mft=%f dt=%f", 
         e.eventid, e.e_cnt, e.e_std, e.e_r2, e.e_msfit, e.e_dt_detect);
   dqq.processed = 1;
   sprintf(dqq.url,"%s/%d", EVENT_URL_BASE, e.eventid);
   sprintf(dqq.guid,"QCNTRIGMON%d", e.eventid);

   // get/update qcn_quakeid and set e.qcn_quakeid 
   // now insert
   if (bInsert || !bFound) {
     retval = dqq.insert();
     if (retval) {
        log_messages.printf(
          MSG_CRITICAL, "QCN_UpdateQuake Insert qcn_quake error event ID %d , errno %d, %s\n",
              e.eventid, retval, boincerror(retval)
        );
      } else { // trigger got in OK
         e.qcn_quakeid = dqq.db->insert_id();
      }
   }
   else { // update
      retval = dqq.update();
      if (retval) {
        log_messages.printf(
          MSG_CRITICAL, "QCN_UpdateQuake error event ID %d , errno %d, %s\n",
              e.eventid, retval, boincerror(retval)
        );
       }
   }
   // set e.dirty to false after insert/update?
   e.dirty = false;
   if (e.qcn_quakeid == 0) {
      log_messages.printf(
        MSG_CRITICAL, "QCN_UpdateQuake error no qcn_quakeid allocated ID %d\n",
          e.eventid
      );
      return; // something is really wrong if we don't have a quakeid at this point
   }

   // now go through all the correlated triggers for this event, post file upload requests, set qcn_quakeid
   //   note to do this in the trigmem as well as the qcnalpha/continual database!
   const char strBaseTrigMem[] = {"UPDATE trigmem.qcn_trigger_memory SET qcn_quakeid=%d, posted=1 WHERE db='%s' AND id=%d"};
   const char strBaseTrig[] = {"UPDATE %s.qcn_trigger SET qcn_quakeid=%d, time_filereq=unix_timestamp() WHERE id=%d"};
   char strSQL[_MAX_PATH];
   for (int m = 0; m < vt[ciOff].c_cnt; m++) {  // For each trigger
        int n = vt[ciOff].c_ind[m];         // Index of correlated trigger so just use vt[n]

        // first update the trigmemdb trigger table with quake id & posted (since we will request uploads)
        if (! vt[n].posted) { // only need to update if wasn't already posted
          sprintf(strSQL, strBaseTrigMem, e.qcn_quakeid, vt[n].db, vt[n].triggerid);
          retval = trigmem_db.do_query(strSQL);
          if (retval) { // big error, should probably quit as may have lost database connection
             log_messages.printf(MSG_CRITICAL,
                "QCN_UpdateQuake trigmem db update error: %s - %s\n", strSQL, boincerror(retval)
             );
          }
        }
 
        // now do the appropriate main table 
        sprintf(strSQL, strBaseTrig, vt[n].db, e.qcn_quakeid, vt[n].triggerid);
        retval = boinc_db.do_query(strSQL);
        if (retval) { // big error, should probably quit as may have lost database connection
           log_messages.printf(MSG_CRITICAL,
              "QCN_UpdateQuake %s db update error: %s - %s\n", vt[n].db, strSQL, boincerror(retval)
           );
         }


        // do the upload file request here
//CMC HERE
     }
/*
trigmem_db.do_query("SELECT unix_timestamp()");
    if (retval) { // big error, should probably quit as may have lost database connection
        log_messages.printf(MSG_CRITICAL,
            "getMySQLUnixTime() error: %s - %s\n", "Query Error", boincerror(retval)
        );
        goto done;
    }

    MYSQL_ROW row;
    MYSQL_RES* rp;
    rp = mysql_store_result(trigmem_db.mysql);
    while (rp && (row = mysql_fetch_row(rp))) {
      numRows++;
      lTime = atol(row[0]);
    }
    mysql_free_result(rp);
*/

   // loop across all triggers for this event

   // update the trigmem entries to reflect this

/*
struct trigger
   int    triggerid;                   // Trigger ID

   char   db[64];               // Database
   char   file[64];              // File name
   float  longitude, latitude;            // Sensor location
   double time_trigger, time_received, time_est;       // Time of trigger & Time received
   float  significance, magnitude;              // Significance and magnitude (sig/noise)
   float  pgah[4],pgaz[4];       // Peak Ground Acceleration (Horizontal & vertical)
   int    c_cnt;                 // Count of correlated triggers
   int    c_ind[N_SHORT];        // Correlated trigger IDs
   int    c_hid[N_SHORT];        // Correlated host IDs
   float  dis;                   // Event to station distance (km)
   int    pors;                   // 1=P, 2=S wave
   bool   dirty;    // if this is true, we changed and should update the qcn_trigger table ie for quakeid etc
   void clear() { memset(this, 0x00, sizeof(trigger)); }
   trigger() { clear(); };
};
*/

}

void QCN_EstimateMagnitude(struct event& e, const int& ciOff)
{
/* We need to come up with good magnitude/amplitude relationships.  There are some good ones for peak displacement v. dist.
   We need some for peak acceleration v. distance.  Note - they may vary from location to location.
   We will need to adjust this for characterization of P & S wave values.  It may also be sensor specific.

   This code bootstraps over the data to determine how cerntain the estimated magnitude is.  Use 3X the standard error for 99% confidence.

   The magnitude relation takes the form:
           M = a * LN( accel * b) + c * LN(dist) + d

   The

   This subroutine was written by Jesse F. Lawrence (jflawrence@stanford.edu).

*/
   float a=0.544f; float b=2.0f; float c=0.03085f; float d=4.28f;// Constants for equation above (Need to be adjusted)
   int j, k, kk, n;                                     // Index variables
   float mul_amp;                                       // Multiplication factor depends on P & S waves currently set to 1
   srand ( time(NULL) );                                // Set randomization kernel
   float mag_ave[N_SHORT];                              // Average magnitude

   e.magnitude = 0.f;                                    // Zero magnitude
   for (j = 0; j <=vt[ciOff].c_cnt; j++) {                   // Bootstrap once for each trigger
    mag_ave[j]=0.;                                      // Zero the average magnitude for this bootstrap
    for (k = 0; k <= vt[ciOff].c_cnt; k++) {                 // Select one point for each trigger
     kk = rand() % (vt[ciOff].c_cnt+1);                      // Use random trigger
     n = vt[ciOff].c_ind[kk];                                // Index of correlated trigger
     if ( vt[n].pors == 0 ) {                            // Use appropriate multilication factor (currently not used but will eventually)
      mul_amp = 1.f;                                    //
     } else {                                           //
      mul_amp = 1.f;                                    //
     };                                                 //
     mag_ave[j] += a*log(vt[n].magnitude * b * mul_amp) + c*log(vt[n].dis) + d; // Sum magnitude estimate from each trigger for average estimate
    }
    mag_ave[j]/= ( (float) vt[ciOff].c_cnt + 1.);            // Normalize summed mag estimates for average magnitude estimate
   }
   e.magnitude = average( mag_ave, vt[ciOff].c_cnt);          // Average the averages
   e.e_std = std_dev( mag_ave, vt[ciOff].c_cnt, e.magnitude)*3.;// 3 sigma is the 99 % confidence (assuming a statistically large enough data set)
   e.dirty = true; // flag that we should update this event
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
    if ((retval = crust2_load())) {
        log_messages.printf(MSG_CRITICAL,
            "Can't load crust2 data %d\n", retval
        );
        return -1;
    }

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

