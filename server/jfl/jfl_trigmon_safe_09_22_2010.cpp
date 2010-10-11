/* version of qcn_trigmon for Jesse

  This program will monitor the "live" triggers in memory 
   (the mysql trigmem.qcn_trigger_memory table)

The general idea is that a query is run every few seconds to see if any quakes were
detected by QCN sensors via lat/lng & time etc

if there were "hits" in a certain area, then flag this as a quake and put in the qcn_quake table

logic:
  1) check for numerous trickles within a region in a short time period (i.e. 10 seconds) 
  2) if there are numerous trickles - see if this event has been reported in qcnalpha.qcn_quake - lookup by time/lat/lng
  3) if has been reported, use that event to tag trickles; else make an entry in qcn_quake and tag triggers
  4) request uploads from these triggers as appropriate

Example usage:
./jfl_trigmon -d 3 -sleep_interval 10 -time_interval 100

(run every 10 seconds, show all debug (-d 3), triggers in last 100 seconds)

(c) 2010  Stanford University School of Earth Sciences

*/

#include "jfl_trigmon.h"

DB_CONN trigmem_db;

double g_dTimeCurrent = 0.0;  // global for current time

// global params for trigger monitoring behavior
double g_dSleepInterval = -1.0;   // number of seconds to sleep between trigmem enumerations
int g_iTriggerTimeInterval = -1;  // number of seconds to check for triggers (i.e. "time width" of triggers for an event)

// keep a global vector of recent QCN quake events, remove them after an hour or so
// this way follup triggers can be matched to a known QCN quake event (i.e. qcn_quake table)
//vector<QCN_QUAKE_EVENT> vQuakeEvent;

void close_db()
{
   log_messages.printf(MSG_DEBUG, "Closing databases.\n");
   boinc_db.close();
   trigmem_db.close();
}

int do_trigmon(struct trigger t[]) 
{
   DB_QCN_TRIGGER_MEMORY qtm;
   qtm.clear();
   int iCtr = 0; int iCtr2 = 0;
   char strWhere[64];
   sprintf(strWhere, "WHERE time_trigger > (unix_timestamp()-%d)", g_iTriggerTimeInterval);
   while (!qtm.enumerate(strWhere))  {
    // just print a line out of trigger info i.e. all fields in qtm
/*     fprintf(stdout, "%d %s %d %d %s %s %f %f %f %f %f %f %f %f %f %d %d %f %d %d %d %d %d\n",
        ++iCtr, qtm.db_name, qtm.triggerid, qtm.hostid, qtm.ipaddr, qtm.result_name, qtm.time_trigger,
        qtm.time_received, qtm.time_sync, qtm.sync_offset, qtm.significance, qtm.magnitude, qtm.latitude,
         qtm.longitude, qtm.levelvalue, qtm.levelid, qtm.alignid, qtm.dt, qtm.numreset, qtm.type_sensor,
         qtm.varietyid, qtm.qcn_quakeid, qtm.posted
     );*/
    ++iCtr;
     if (qtm.type_sensor>=100) {
      ++iCtr2;
      t[iCtr2].hid  = qtm.hostid;
      t[iCtr2].tid  = qtm.triggerid;
      t[iCtr2].slat = qtm.latitude;
      t[iCtr2].slon = qtm.longitude;
      t[iCtr2].trig = qtm.time_trigger;
      t[iCtr2].rec  = qtm.time_received;
      t[iCtr2].sig  = qtm.significance;
      t[iCtr2].mag  = qtm.magnitude;
     }     
   }
   return iCtr2;
}




float ang_dist_km(float lon1, float lat1, float lon2, float lat2) {
/* This function returns the distance (in km) between two points given as
   (lon1,lat1) & (lon2,lat2).This function written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu   
*/
   float pi = atan(1.f)*4.f;            // set pi = 3.14....
   float d2r = pi/180.f;                // Degree to radian Conversion
   
   float dlat = lat2-lat1;
   float dlon = (lon2-lon1)*cos((lat2+lat1)/2.f*d2r);

   float delt = 111.19*sqrt(dlat*dlat+dlon*dlon);
 
   return delt;
};


void set_grid3D( struct bounds g, float elon, float elat, float edep, float width, float dx, float zrange, float dz) {
/* This subroutine sets up a 3D grid search (x,y, & z) for source location.  This is a brute force location
   scheme.  Slower than it should be.  The gridding is made into a subroutine because I do several iterations,
   that focus in on the best solution of the prior grid, which is much faster than a fine grid for a large area.
   
   The grid uses structure bounds.  The center of the grid is assumed to be the best eq estimate.
   
   The grid has horizontal dimensions width with horizontal node intervals of dx.
   
   The grid has vertical dimension zrange with vertical node interval of dz.

*/ 
   g.yw = width;                               // Latitudinal width of grid
   g.xw = width;                               // Latitudinal width of grid
   g.zw = zrange;
   
   g.x_min = elon-g.xw/2.f;                    // Set bounds of grid
   g.x_max = elon+g.xw/2.f;
   g.y_min = elat-g.yw/2.f;
   g.y_max = elat+g.yw/2.f;
   g.z_min = edep-g.zw/2.f;if (g.z_min < 0.) {g.z_min=0.f;};
   g.z_max = edep+g.zw/2.f;

   g.dy = dx;                                  // Set latitudinal step size
   g.dx = dx;                                  // Set longitudinal step size
   g.dz = dz;
   
   g.ny = (int) ((g.y_max-g.y_min)/g.dy);      // Set number of latitudinal steps
   g.nx = (int) ((g.x_max-g.x_min)/g.dx);      // Set number of longitudinal steps
   g.nz = (int) ((g.z_max-g.z_min)/g.dz);      // Set number of depth steps
}

void vel_calc(float dep, float v[]) {
/* This subroutine is a kluge - it increases velocity with depth, but assuming path averaged velocity for an earthquake that
   occurs at depth.  This is only a kluge, because the path average velocity changes as a function of depth/distance.  This
   subroutine needs updating.  Jesse has accurate versions in fortran, but they need to be converted to c.

*/
   float dep2=dep;if (dep < 5.f) {dep2=5.f;};
   v[1] = 0.34*log(dep2)+2.56;
   v[0] = 1.86*v[1];
}

void qcn_event_locate(struct trigger t[], int i, struct event e[]) {
   
   if ( ( t[i].trig > T_max+e[1].e_time)||(abs(t[i].slat-e[1].elat)>5.) ) e[1].eid++;         // If new Time or location, then new event
   
   float  width = 4.f; float zrange=150.f;       // Lateral and vertical grid size
   float  dx = 0.1f; float dz = 10.f;            // Lateral and vertical grid step size                  
   int    n_iter=3;                              // Number of grid search iterations
   float  ln_x,lt_x,dp_x;
   float  dn,dp;                                 // Distances to the nth and pth trigger location
   double  dt,dt_min;
   double  ls_mf,ls_ct;                          // Least-squares misfit & count (For normalization)
   float   v[2];
   int     h,j,k,l,m,n,o,p,q,r,j_min;
   double t_min = t[i].trig;                     // Minimum trigger time
   struct bounds g;                              // Bounds of grid
   
   j_min = i;
   for (j = 1; j<=t[i].c_cnt; j++) {             // Find earliest trigger
    n = t[i].c_ind[j];                           // index of correlated series
    if (t_min < t[n].trig) {
     t_min = t[n].trig;
     j_min = n;
    }
   } 
   e[1].elon = t[j_min].slon;                       // Start with assumption of quake at earliest trigger
   e[1].elat = t[j_min].slat;
   
   for (j = 1; j<=n_iter; j++) {                 // For each iteration 
//    set_grid3D(g, e[1].elon, e[1].elat, e[1].edep, width, dx, zrange, dz);        // Set bounds of grid search
    
    g.yw = width;                               // Latitudinal width of grid
    g.xw = width;                               // Latitudinal width of grid
    g.zw = zrange;
 
    g.x_min = e[1].elon-g.xw/2.f;                    // Set bounds of grid
    g.x_max = e[1].elon+g.xw/2.f;
    g.y_min = e[1].elat-g.yw/2.f;
    g.y_max = e[1].elat+g.yw/2.f;
    g.z_min = e[1].edep-g.zw/2.f;if (g.z_min < 0.) {g.z_min=0.f;};
    g.z_max = e[1].edep+g.zw/2.f;
 
    g.dy = dx;                                  // Set latitudinal step size
    g.dx = dx;                                  // Set longitudinal step size
    g.dz = dz;
 
    g.ny = (int) ((g.y_max-g.y_min)/g.dy);      // Set number of latitudinal steps
    g.nx = (int) ((g.x_max-g.x_min)/g.dx);      // Set number of longitudinal steps
    g.nz = (int) ((g.z_max-g.z_min)/g.dz);      // Set number of depth steps

    float ls_mf_min = 9999999999.f;              // Set obserdly high misfit minimum
    for (h = 1; h<=g.nz; h++) {
     dp_x = g.z_min + g.dz * (float) (h-1);
     vel_calc(dp_x,v);                           // Calculate mean path velocity
     for (k = 1; k<=g.nx; k++) {                 // For each x node
      ln_x = g.x_min + g.dx * (float) (k-1);     // Longitude of grid point
      for (l = 1; l<=g.nx; l++) {                // For each y node
       lt_x = g.y_min + g.dy * (float) (l-1);    // Latitude of grid point
       ls_mf = 0.; ls_ct = 0.;                   // Zero least-squares misfit & count
      

/* Direct method: correlate time & node-sensor distance - (not yet accounting for P or S wave question). */
       float d_av=0.; double t_av=0.;                    // Correlate trigger times & location
       for (m=1; m<=t[i].c_cnt; m++){
        n = t[i].c_ind[m];                               // Index of correlated trigger
        dn=ang_dist_km(ln_x,lt_x,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
        dn = sqrt(dn*dn + dp_x*dp_x);                    // Actual distance between event to station/host
        d_av = d_av + dn/ (float) t[i].c_cnt;            //average distance (sum first, normalize later)
        t_av = t_av + t[n].trig/ (float)t[i].c_cnt;      // Average trigger time (sum first, normalize later)
       }
       float num=0.; float den1=0.; float den2=0.;
       for (m=1; m<=t[i].c_cnt; m++){
        n = t[i].c_ind[m];                               // Index of correlated trigger
        dn=ang_dist_km(ln_x,lt_x,t[n].slon,t[n].slat);   // Horizontal distance between event and host/station
        dn = sqrt(dn*dn + dp_x*dp_x);                    // Actual distance between event to station/host
        num = num + (dn-d_av)*(t[n].trig-t_av);          // Numerator of correlation (Sxy)
        den1 = den1 + (dn-d_av)*(dn-d_av);               // Denominator part 1 of (Sxx)
        den2 = den2 + (t[n].trig-t_av)*(t[n].trig-t_av); // Denominator part 2 (Syy)
        
       };
       if (num<0) {num=0.000001;}                        // Only use positive correlation (not negative correlation)
       float r2 = num/sqrt(den1)/sqrt(den2);             // Normalize correlation Sxy/(Sxx^0.5 * Syy^0.5)
       
       


/* Indirect method: compare times and distance difference between each station pair */
       for (m = 1; m< t[i].c_cnt; m++) {                 // For each trigger
        n = t[i].c_ind[m];                               // Index of correlated trigger
        dn=ang_dist_km(ln_x,lt_x,t[n].slon,t[n].slat);   // Horizontal distance between node and host/station
        dn = sqrt(dn*dn + dp_x*dp_x);                    // Actual distance between event & station/host
        
        for (o = m+1; o<=t[i].c_cnt; o++) {              // Compare with each other trigger
         p = t[i].c_ind[o];                              // Index of correlated trigger
         dp=ang_dist_km(ln_x,lt_x,t[p].slon,t[p].slat);  // Distance between triggers
         dp = sqrt(dp*dp + dp_x*dp_x);
//         avel = (dn-dp)/(t[n].trig-t[p].trig);
//	 printf("V=%f %d %d \n",(dn-dp)/(t[n].trig-t[p].trig),m,o);
	 	
         dt_min = 9999999.f;                             // Search For best P-wave & S-wave pairing
         for (q=0;q<=1;q++) {                            //    First station P(0) or S(1)
         for (r=0;r<=1;r++) {                            //    Second station P(0) or S(1)
          dt = (t[n].trig-t[p].trig) - (dn/v[q]-dp/v[r]);//    Interstation time difference - theoretical time difference
          if (dt*dt < dt_min*dt_min) {
	   dt_min = dt;};     //    Use the smallest value (assume if neerer zero then probably true)
         }
        }
         ls_mf = ls_mf + (dt_min*dt_min);                // Sum L2 misfit 
         ls_ct = ls_ct + 1.f;                            // Count For normalizing misfit
        }
       }      
      ls_mf = ls_mf;                                   // Option 1 - just use l2 misfit (indirect method) 
//      ls_mf = 1./r2;                                   // Option 2 - just use correlation (direct method)
//       ls_mf = ls_mf/r2;                                 // Option 3 - just use correlation & l2 misfit (direct & indirect method) 
       if (ls_ct > 0.) {                                 // 
        ls_mf = ls_mf/ls_ct;                             // Normalize misfit by count
        if (ls_mf < ls_mf_min) {                         // If minimum misfit/max correlation save location & r2
         ls_mf_min = ls_mf;                              // Save new misfit minimum
         e[1].elon=ln_x; e[1].elat=lt_x; e[1].edep=dp_x;// Save location of best fit
         e[1].e_r2 = r2;                              // Save correlation of best fit
        }                                                // End loop over 2nd trigger of pair
       }                                                 // End loop over 1st trigger of pair
      }                                                  // End loop over latitude
     }                                                   // End loop over Longitude
    }                                                    // End loop over depth
    dx = dx / 10.f; width = width/10.f; dz = dz / 10.f; zrange=zrange/10.f;

   dn=ang_dist_km(e[1].elon,e[1].elat,t[i].slon,t[i].slat);//Distance between triggers in horizontal plain
   dn = sqrt(dn*dn + e[1].edep*e[1].edep);                 // Distance between triggers in 3D (direct line, not ray path).
   vel_calc(e[1].edep,v);                                     // Get velocity (note - depth dependant - not done well).
   e[1].e_time = t[i].trig - dn/v[1];                         // time of event calculated from ith trigger.

   }
 
  fprintf(stdout,"T=%f Loc: %f %f \n",e[1].e_time,e[1].elon,e[1].elat); 
   
   
    
}


void estimate_magnitude(struct trigger t[], struct event e[], int i) {
/* We need to come up with good magnitude/amplitude relationships.  There are some good ones for peak displacement v. dist.
   We need some for peak acceleration v. distance.  Note - they may vary from location to location.
   We will need to adjust this for characterization of P & S wave values.  It may also be sensor specific.
M = a * LN( accel * b) + c * LN(dist) + d
*/   
   float a=1.25f; float b=.9f; float c=0.8f; float d=3.25f;
   int j, n; float dn; 
   float v[2],ts,tp,mul_amp;
    
   e[1].e_mag = 0.f;                                    // Zero magnitude
   for (j = 1; j <=t[i].c_cnt; j++) {
      n = t[i].c_ind[j];                                // Index of correlated trigger
      dn=ang_dist_km(e[1].elon,e[1].elat,t[n].slon,t[n].slat);//Distance between triggers in horizontal plain
      dn = sqrt(dn*dn + e[1].edep*e[1].edep);     // Distance between triggers in 3D (direct line, not ray path).
      vel_calc(e[1].edep, v);
      tp = abs(t[n].trig-e[1].e_time-dn/v[0]);
      ts = abs(t[n].trig-e[1].e_time-dn/v[1]);
      if ( ts < tp ) { mul_amp = 1.f; } else { mul_amp = 2.f;};
      e[1].e_mag = e[1].e_mag + a*log(t[n].mag*b*mul_amp) + c*log(dn) + d;
   }
   e[1].e_mag = e[1].e_mag / (float) t[i].c_cnt;

}



float intensity_extrapolate(float dist, float dist_eq_nd, float intensity_in) {
 if (dist       < 1.f) {dist      =1.f;}
 if (dist_eq_nd < 1.f) {dist_eq_nd=1.f;}
 float intensity_out = intensity_in * ((dist_eq_nd) / (dist)) ;
 return intensity_out;
}


void intensity_map(struct trigger t[], int i, struct event e[]) {
   float width=5; float dx=0.05;                              // Physical dimensions of grid
   int   nx = ((int) (width/dx)) + 1; int ny = nx;            // array dimension of grid
   float   dist,dist_eq_nd;                                          // Min distance from triger host to grid node
   float   ln_x,lt_x,imap;                                    // Location & intensity at grid node
   char ifile[sizeof "INTENSITIES/intensity_100.xyz"];        // intensity file
   char efile[sizeof "EVENTS/event_100.xy"];                  // Event file
   char sfile[sizeof "STATIONS/stations_100.xyz"];            // station info
   FILE *fp10;                                                // Output file(s)
   int j,k,l,n,il;                                  // Index variables
   j=1;
   
   float elon = e[1].elon;
   float elat = e[1].elat; 
   float x_min = elon - width/2.f;                       // Minimum longitude of map
   float y_min = elat - width/2.f;                       // Min Latitude
   int eid = e[1].eid;

/* Create an event directory */
   sprintf(efile, "/data/cees2/QCN/trigger/autodetect/%08d", eid);// Create event output directory name
   int imkdir = boinc_mkdir(efile);                           // Make directory

/* Create a file with the event location (lon,lat only)       */
   sprintf(efile, "/data/cees2/QCN/trigger/autodetect/%08d/event_%08d.xy", eid,eid);        // Create event output file name
   fp10 = fopen(efile,"w+");                                  // Open event output file
   fprintf(fp10,"%f %f\n",elon,elat);               // Output event location
   fclose(fp10);                                              // Close event output file name
   
/* Create a file with the station information                 */
   sprintf(sfile, "/data/cees2/QCN/trigger/autodetect/%08d/stations_%08d.xyz", eid,eid);        // Create station output file name
   fp10 = fopen(sfile,"w+");                                  // Open station output file
   for (k = 0; k<=t[i].c_cnt; k++) {                          // For each correlated trigger
     n = t[i].c_ind[k];                                       // Index of correlated trigger
     fprintf(fp10,"%f %f %f \n",t[n].slon,t[n].slat,t[n].mag);// Output correlated trigger loc & magnitude
   }
   fclose(fp10);                                               // Close station output file name

/* Create an interpolated intensity map:                      */
   sprintf(ifile, "/data/cees2/QCN/trigger/autodetect/%08d/intensity_%08d.xyz",eid,eid);       // Intensity file name
   fp10 = fopen(ifile,"w+");                                  // Open intensity file
   
   for (j=1; j<=nx; j++) {                                   // For each longitudinal node
     ln_x = x_min + dx * (float) (j-1);                      // Longitude of grid point
     for (k=1; k<=ny; k++) {                                 // For each latitudinal node
       lt_x = y_min + dx * (float) (k-1);                    // Latitude of grid point
       float dist_min = 9999999.;                            // Set unreasonably high min distance
       dist_eq_nd = ang_dist_km(ln_x,lt_x,elon,elat);        // Horizontal distance from event to station/host
       imap = 0.f;
       for (l=0; l<=t[i].c_cnt; l++) {                       // For each trigger
        n = t[i].c_ind[l];                                   // Index of lth trigger
	dist = ang_dist_km(ln_x,lt_x,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
	if (dist_min > dist) {dist_min=dist;il=n;};          // Set minimum distance and lth trigger at dist
	dist = ang_dist_km(elon,elat,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
        imap=imap+intensity_extrapolate(dist_eq_nd, dist, t[n].mag)/ (float) (t[i].c_cnt+1);                  // 
       }
	dist = ang_dist_km(elon,elat,t[il].slon,t[il].slat);   // Horizontal distance from event to station/host
        imap=(imap+intensity_extrapolate(dist_eq_nd, dist, t[il].mag));                  // 
       fprintf(fp10,"%f %f %f \n",ln_x,lt_x,imap);
     }
   }
   fclose(fp10);

   return;
}


void detect_qcn_event(struct trigger t[], int iCtr, struct event e[]) {
/* This subroutine determines if a set of triggers is correlated in time and space.
   The subroutine is used by program main, which assumes that the triggers have already been read in.
   The subroutine uses ang_dist_km, which provides the distance between two points in kilometers. 
   The subroutine uses hid_yn, which determines if the primary trigger has already encountered a host ID or not.
   
   A correlated station pair occurs when the distance is < 100 km apart and the trigger time difference 
   is less than the velocity divided by the station separation.
*/
   int i,j,k,l,kl;                             // Index variables
//   double  dt;                          // Time between triggers
   float   dist;                        // Distance between triggers
   int   nh = 0;int ih=0;
   int   h[1000];
   time_t t_now; time_t t_eq;
   struct tm * t_now_gmt; struct tm * t_eq_gmt;
 
   for (i = 1; i<iCtr; i++) {          // For each trigger
    t[i].c_cnt=0;                      // Zero the count of correlated triggers 
    ih = 0;                            // Unassigned host id
    if (nh>0) {                        // if there are assigned host ids, then
     for (j = 1; j<=nh;j++) {          // search through the assigned host ids
      if (t[i].hid == h[j]) {          // to find a match
       ih = j;                         // Match found
      }   
     }
     if (ih==0) {                      // If no match found, then
      nh++;                            // add a new assigned host id 
      h[nh]=t[i].hid;                  // assign the new host id
     }                                 //
    } else {                           // If no previously assigned host ids, then
     nh = 1;                           // add the first assigned host id
     h[nh]=t[i].hid;                   // assign the first host id
    }                                  //
    
    if ( (t[i].hid!=t[i-1].hid) ) { // && (ih==0) ) {        // Do not use repeating triggers
     for (j = i+1; j<=iCtr; j++) {        // For every other trigger
      if ( (t[j].hid!=t[i].hid) && (t[j].hid!=t[j-1].hid) && (abs(t[i].trig-t[j].trig) <= T_max) ) {//For non-repeating triggers & triggers less than t_max apart
       dist=ang_dist_km(t[i].slon,t[i].slat,t[j].slon,t[j].slat);//Distance between triggers
       if ( (abs(t[i].trig-t[j].trig)<dist/Vs + 3.f) && (dist<=D_max) ) {
        t[i].c_cnt++;                  // Add count of correlated triggers for this trigger
        if (t[i].c_cnt>n_short) {      // Make sure dont use more correlations than array size
         t[i].c_cnt=n_short; 
         break;
        }
        t[i].c_ind[t[i].c_cnt]=j;      // index of all correlaed triggers
        t[i].c_hid[t[i].c_cnt]=t[i].hid; // Index of host ids
       }
      }     
     }
    }
   }                                  // Done correlating




/* Now we correlate triggers that are currently correlated with triggers that are correlated with the initial trigger, but not
   correlated with the initial trigger itself */
   for (i = 1; i<iCtr; i++) {                  // For each trigger 
    if (t[i].c_cnt > C_CNT_MIN) {              // If more than 4 correlated triggers, possible regional event
     for (j = i+1;j<=iCtr; j++) {              // Compare with all later triggers
      if (t[j].c_cnt > C_CNT_MIN) {            // Make sure this trigger is an event all of it's own 
       kl = 0;
       for (k = 1; k<=t[j].c_cnt;k++) {        // Compare all potential secondary correlated triggers 
        for (l = 1; l<=t[i].c_cnt;l++) {       // Make sure trigger isn't same host as prior trigger
         kl = l;
        }
        if (kl == 0) {                         // If no matching trigger, then add secondary trigger to primary trigger list
         t[i].c_cnt++;
         t[i].c_ind[t[i].c_cnt]=t[j].c_ind[k];
         t[i].c_hid[t[i].c_cnt]=t[j].c_hid[k];
        }
        t[k].c_cnt = 0;                       // get rid of correlated triggers (now that they are primary triggers)
       }
      }
     }
     j = i; 
     if ( ( t[i].trig > T_max+e[1].e_time)||(abs(t[i].slat-e[1].elat)>5.) ) {
      e[1].eid++;         // If new Time or location, then new event
      e[1].e_cnt=0;
     }
     if (t[i].c_cnt > e[1].e_cnt) {
      qcn_event_locate(t,j,e); 
      e[1].e_cnt = t[i].c_cnt;
      estimate_magnitude(t, e, j);      // Estimate the magnitude of the earthquake
      time(&t_now);  t_now_gmt = gmtime(&t_now); // Current time
      t_eq = e[1].e_time;  t_eq_gmt = gmtime(&t_eq);  // Time of the Earthquake
      fprintf(stdout,"%d %d %f %f %f %f %s %s %f \n",i,t[i].c_cnt,e[1].elon,e[1].elat,e[1].edep,e[1].e_mag,asctime(t_now_gmt),asctime(t_eq_gmt),e[1].e_mag);

     }
    } 
   }
    
   

   
   
   return;                            // Done
};





int main(int argc, char** argv) 
{
    struct trigger t[n_long];                            // Trigger buffer ring
    struct event   e[2];                                    // event
    int retval;
    int tidl=0; int hidl=0;                                         // default last host id
    
//    int j;
    //vQuakeEvent.clear();
 
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
                "Example usage: jfl_trigmon -d 3 -sleep_interval 3 -count 10 -time_interval 10\n\n"
             , argv[i]
            );
            return 2;
        }
    }
    if (g_dSleepInterval < 0) g_dSleepInterval = TRIGGER_SLEEP_INTERVAL;
    if (g_iTriggerTimeInterval < 0) g_iTriggerTimeInterval = TRIGGER_TIME_INTERVAL;

    install_stop_signal_handler();
    atexit(close_db);

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        return 3;
    }
    retval = trigmem_db.open(
        config.trigmem_db_name, config.trigmem_db_host, config.trigmem_db_user, config.trigmem_db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "trigmem_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        return 4;
    }
    retval = boinc_db.set_isolation_level(REPEATABLE_READ);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
        );
    }

    log_messages.printf(MSG_NORMAL,
            "jfl_trigmon started with the following options:\n"
            "  -time_interval   = %d\n" 
            "  -sleep_interval  = %f\n",
         g_iTriggerTimeInterval,
         g_dSleepInterval
    ); 

    //signal(SIGUSR1, show_state);
    while (1) {
      g_dTimeCurrent = dtime();
      double dtEnd = g_dTimeCurrent + g_dSleepInterval;
      int iCtr = do_trigmon(t);          // the main trigger monitoring routine
      
      if (iCtr>0) {
       if ( (t[1].tid != tidl) && (t[1].hid != hidl) ) {  // Dont allow repeat trigger id or host id
//        for (j=1; j<iCtr; j++) {
//         fprintf(stdout,"%d %d %d %f %f %f %f %f %f\n",j,t[j].tid,t[j].hid,t[j].slat,t[j].slon,
//                  t[j].trig,t[j].rec,t[j].sig,t[j].mag);
         detect_qcn_event(t,iCtr,e);       
         tidl=t[1].tid; hidl=t[1].hid;                    // Save last trigger id & host id (so no repeats)
//        }
       }
      }
      check_stop_daemons();  // checks for a quit request
      g_dTimeCurrent = dtime();
      if (g_dTimeCurrent < dtEnd && (dtEnd - g_dTimeCurrent) < 60.0) { // sleep a bit if not less than 60 seconds
          log_messages.printf(MSG_DEBUG, "Sleeping %f seconds....\n", dtEnd - g_dTimeCurrent);
          boinc_sleep(dtEnd - g_dTimeCurrent);
      } 
    }
    return 0;
}



