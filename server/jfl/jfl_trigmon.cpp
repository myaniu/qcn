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
     if (qtm.type_sensor>=100) {                           // Only use triggers from usb accelerometers
      ++iCtr2;                                             // Count triggers
      t[iCtr2].hid  = qtm.hostid;                          // Host ID
      t[iCtr2].tid  = qtm.triggerid;                       // Trigger ID
      sprintf(t[iCtr2].db,"%s",qtm.db_name);               // Database Name
      t[iCtr2].slat = qtm.latitude;                        // Latitude
      t[iCtr2].slon = qtm.longitude;                       // Longitude
      t[iCtr2].trig = qtm.time_trigger;                    // Trigger Time
      t[iCtr2].rec  = qtm.time_received;                   // Time Trigger received
      t[iCtr2].sig  = qtm.significance;                    // Significance (Trigger detection filter)
      t[iCtr2].mag  = qtm.magnitude;                       // |acceleration| (m/s/s)
     }     
   }

   return iCtr2;                                           // Return with number of triggers

}




float average( float dat[], int ndat) {
/*  This fuction calculates the average of a data set (dat) of length (ndat). */
   float dat_ave = 0.;                                    // Start with zero as average
   int j;                                                 // Index variable
   for (j = 0; j<=ndat; j++) {                            // For each point
    dat_ave = dat_ave + dat[j];                           // Add values
   }                                                      // 
   dat_ave = dat_ave / ( (float) ndat + 1.);              // Normalize sum by quantity for average
   return dat_ave;                                        // Return with average
}

float std_dev( float dat[], int ndat, float dat_ave) {
/*  This function calculates the standard deviation of a data set (dat) of length (ndat) with a mean of (dat_ave) */
   float dat_std = 0.;                                    // Zero the standard deviation
   int j;                                                 // Index variable
   for (j = 0; j<=ndat; j++) {                            // For each point
    dat_std = dat_std + (dat[j]-dat_ave)*(dat[j]-dat_ave);// Sum squared difference from mean
   }                                                      // 
   dat_std = dat_std / ( (float) ndat + 1.);              // Normalize squared differences
   dat_std = sqrt(dat_std);                               // Standard deviation once square root taken
   return dat_std;                                        // Return standard deviation
}

float correlate( float datx[], float daty[], int ndat) {
/*  This function correlates two data sets (datx & daty) of length (ndat).  It returns the R^2 value (not r). */
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
   float R = Sxy*Sxy / Sxx / Syy;                         // Correlation coefficient
   return R;                                              // Return with correlation coefficient
}



float ang_dist_km(float lon1, float lat1, float lon2, float lat2) {
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


void set_grid3D( struct bounds g, float elon, float elat, float edep, float width, float dx, float zrange, float dz) {
/* This subroutine sets up a 3D grid search (x,y, & z) for source location.  This is a brute force location
   scheme.  Slower than it should be.  The gridding is made into a subroutine because I do several iterations,
   that focus in on the best solution of the prior grid, which is much faster than a fine grid for a large area.
   
   The grid uses structure bounds.  The center of the grid is assumed to be the best eq estimate.
   
   The grid has horizontal dimensions width with horizontal node intervals of dx.
   
   The grid has vertical dimension zrange with vertical node interval of dz.

*/ 
   g.yw = width;                                          // Latitudinal width of grid
   g.xw = width;                                          // Latitudinal width of grid
   g.zw = zrange;
   
   g.x_min = elon-g.xw/2.f;                               // Set bounds of grid
   g.x_max = elon+g.xw/2.f;
   g.y_min = elat-g.yw/2.f;
   g.y_max = elat+g.yw/2.f;
   g.z_min = edep-g.zw/2.f;if (g.z_min < 0.) {g.z_min=0.f;};
   g.z_max = edep+g.zw/2.f;

   g.dy = dx;                                             // Set latitudinal step size
   g.dx = dx;                                             // Set longitudinal step size
   g.dz = dz;
   
   g.ny = (int) ((g.y_max-g.y_min)/g.dy);                 // Set number of latitudinal steps
   g.nx = (int) ((g.x_max-g.x_min)/g.dx);                 // Set number of longitudinal steps
   g.nz = (int) ((g.z_max-g.z_min)/g.dz);                 // Set number of depth steps
}

void vel_calc(float dep, float v[]) {
/* This subroutine is a kluge - it increases velocity with depth, but assuming path averaged velocity for an earthquake that
   occurs at depth.  This is only a kluge, because the path average velocity changes as a function of depth/distance.  This
   subroutine needs updating.  Jesse has accurate versions in fortran, but they need to be converted to c.

*/
   float dep2=dep;if (dep < 5.f) {dep2=5.f;};             // Set minimum depth for equation to 5
   v[1] = 0.34*log(dep2)+2.56;                            // Vs (simple equation expression - not actual)
   v[0] = 1.86*v[1];                                      // Vp = 1.86*Vs
}

void qcn_event_locate(struct trigger t[], int i, struct event e[]) {
   fprintf(stdout,"Locate possible event %d \n",t[i].c_cnt);
   if ( ( t[i].trig > T_max+e[1].e_time)||(abs(t[i].slat-e[1].elat)>3.) ) e[1].eid=0;         // If new Time or location, then new event
   
   
   float  width = 4.f; float zrange=150.f;                // Lateral and vertical grid size
   float  dx = 0.1f; float dz = 10.f;                     // Lateral and vertical grid step size                  
   int    n_iter=3;                                       // Number of grid search iterations
   float  ln_x,lt_x,dp_x;                                 // Lon, Lat, & Depth of each grid point
   float  dn=0.; float dp;                                // Distances to the nth and pth trigger location
   double  dt,dt_min;                                     // Travel time difference between two stations (and minimum for P & S wave)
   double  ls_mf,ls_ct;                                   // Least-squares misfit & count (For normalization)
   float   v[2];                                          // Seismic Velocities (P & S)
   int     h,j,k,l,m,n,o,p,q,r,j_min;                     // Index variables
   double t_min = t[i].trig;                              // Minimum trigger time
   struct bounds g;                                       // Bounds of grid

   j_min = i;                                             // Assume first trigger is i'th trigger (not necessarily the case and will be checked)
   for (j = 0; j<=t[i].c_cnt; j++) {                      // Find earliest trigger
    n = t[i].c_ind[j];                                    // index of correlated series
    if (t_min < t[n].trig) {                              // If earlier trigger, then use that as first time
     t_min = t[n].trig;                                   //
     j_min = n;                                           //
    }
   }                                                      //
   e[1].elon = t[j_min].slon;                             // Start with assumption of quake at/near earliest trigger
   e[1].elat = t[j_min].slat;
   e[1].edep = 33.;                                       // Start with default Moho depth eq.
   
   for (j = 1; j<=n_iter; j++) {                          // For each iteration 
//    set_grid3D(g, e[1].elon, e[1].elat, e[1].edep, width, dx, zrange, dz);        // Set bounds of grid search
    g.yw = width;                                         // Latitudinal width of grid
    g.xw = width;                                         // Latitudinal width of grid
    g.zw = zrange;                                        // Vertical dimension of grid
    
    g.dx = dx;                                            // Set grid interval
    g.dz = dz;
    g.dy = dx;

    g.x_min = e[1].elon-g.xw/2.f;                         // Set bounds of grid
    g.x_max = e[1].elon+g.xw/2.f;
    g.y_min = e[1].elat-g.yw/2.f;
    g.y_max = e[1].elat+g.yw/2.f;
    g.z_min = e[1].edep-g.zw/2.f;if (g.z_min < 0.) {g.z_min=0.f;};
    g.z_max = e[1].edep+g.zw/2.f;
 
    g.ny = (int) ((g.y_max-g.y_min)/g.dy);                // Set number of latitudinal steps
    g.nx = (int) ((g.x_max-g.x_min)/g.dx);                // Set number of longitudinal steps
    g.nz = (int) ((g.z_max-g.z_min)/g.dz);                // Set number of depth steps  
    float ls_mf_min = 9999999999.f;                       // Set obserdly high misfit minimum
    for (h = 1; h<=g.nz; h++) {                           // For each vertical grid step
     dp_x = g.z_min + g.dz * (float) (h-1);               // Calculate depth
     vel_calc(dp_x,v);                                    // Calculate mean path velocity
     for (k = 1; k<=g.nx; k++) {                          // For each x node
      ln_x = g.x_min + g.dx * (float) (k-1);              // Longitude of grid point
      for (l = 1; l<=g.nx; l++) {                         // For each y node
       lt_x = g.y_min + g.dy * (float) (l-1);             // Latitude of grid point
       ls_mf = 0.; ls_ct = 0.;                            // Zero least-squares misfit & count

/* Compare times and distance difference between each station pair */
       for (m = 0; m< t[i].c_cnt; m++) {                  // For each trigger
        n = t[i].c_ind[m];                                // Index of correlated trigger
        dn=ang_dist_km(ln_x,lt_x,t[n].slon,t[n].slat);    // Horizontal distance between node and host/station
        dn = sqrt(dn*dn + dp_x*dp_x);                     // Actual distance between event & station/host
        
        for (o = m+1; o<=t[i].c_cnt; o++) {               // Compare with each other trigger
         p = t[i].c_ind[o];                               // Index of correlated trigger
         dp=ang_dist_km(ln_x,lt_x,t[p].slon,t[p].slat);   // Distance between triggers
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
        if (ls_mf < ls_mf_min) {                          // If minimum misfit/max correlation save location & r2
         ls_mf_min = ls_mf;                               // Save new misfit minimum
         e[1].elon=ln_x; e[1].elat=lt_x; e[1].edep=dp_x;  // Save location of best fit
        }                                                 // End loop over 2nd trigger of pair
       }                                                  // End loop over 1st trigger of pair
      }                                                   // End loop over latitude
     }                                                    // End loop over Longitude
    }                                                     // End loop over depth
    dx /=10.f; width/=10.f; dz/=10.f; zrange/=10.f;       // Reduce grid dimensions by an order of magnitude


/* Re-compare times and distance difference between each station pair for best location and identify which phase each trigger is */
    vel_calc(e[1].edep,v);                                // Get velocity (note - depth dependant - not done well).
    int q_save=0;
    e[1].e_time=0.;
    for (m = 0; m<=t[i].c_cnt; m++) {                     // For each trigger
     n = t[i].c_ind[m];                                   // Index of correlated trigger
     t[n].pors = 0;
     dn=ang_dist_km(e[1].elon,e[1].elat,t[n].slon,t[n].slat);// Horizontal distance between node and host/station
     dn = sqrt(dn*dn + e[1].edep*e[1].edep);                 // Actual distance between event & station/host
        
     for (o = 0; o<=t[i].c_cnt; o++) {                    // Compare with each other trigger
      p = t[i].c_ind[o];                                  // Index of correlated trigger
      if (p!=n) {
       dp=ang_dist_km(e[1].elon,e[1].elat,t[p].slon,t[p].slat);  // Distance between triggers
       dp = sqrt(dp*dp + e[1].edep*e[1].edep);
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
     e[1].e_time += t[n].trig - dn/v[t[n].pors];          // Time of event calculated from ith trigger.
    }                                                     //

/* Time of earthquake: */
   e[1].e_time = e[1].e_time / ((float) t[i].c_cnt + 1.); // Time of earthquake calculated by averaging time w.r.t each station normalized by number of stations
   if (e[1].eid <=0) {                                    // For New earthquake ID if earthquake time wasn't set
    e[1].eid = ((int) e[1].e_time);                       //
    fprintf(stdout,"NEW EID: %d \n",e[1].eid);}           //
   }

/*  Determine maximum dimension of array  */
   float ss_dist_max = -999999999.;                       // Start with large station to station maximum array dimension
   for (j = 0; j<t[i].c_cnt; j++) {                       // For each trigger/station
    n = t[i].c_ind[j];                                    // Index of jth station
    for (k = j+1; k<=t[i].c_cnt; k++) {                   // For each other trigger/station
     m = t[i].c_ind[k];                                   // Index of kth station
     dn = ang_dist_km(t[m].slon,t[m].slat,t[n].slon,t[n].slat);// Distance between stations
     if (dn > ss_dist_max) ss_dist_max = dn;              // Store if maximum distance
    }                                                     // 
   }                                                      //

/*  Require that the earthquake-to-array distance be less than four times the dimension of the array */
   if (ss_dist_max*4. < t[j_min].dis) {                   // If the event-to-station distance > 4 times the array dimension
    e[1].e_r2=-1.;                                        // Set correlation to -1 (will reject earthquake)
    fprintf(stdout,"Event poorly located: Array dimension=%f EQ Dist=%f.\n",ss_dist_max,dn); //Status report
    return;                                               // Return so done
   } else {                                               // Otherwise output status report
    fprintf(stdout,"Event located: %f %f\n",e[1].elon,e[1].elat);
   }
   
/*  Calculate the estimated arrival time of the phase */ 
   float t_obs[t[i].c_cnt];                               // Observed phase arrival times (from triggers)
   float t_est[t[i].c_cnt];                               // Estimated phase arrival times (from earthquake location estimate)
   float dt_ave=0;                                        // Average model-data time variance
   e[1].e_msfit = 0.;                                     // Zero misfit for model given set P & S wave times
   for (j=0;j<=t[i].c_cnt;j++) {                          // For each trigger
    n = t[i].c_ind[j];                                    // Index of triggers
    t[n].t_est = e[1].e_time + t[n].dis / v[t[n].pors];   // Estimated time is event origin time plus distance divided by velocity
    t_obs[j]=t[n].trig-e[1].e_time;                       // Store observed times
    t_est[j]=t[n].t_est-e[1].e_time;                      // Store estimated times
    dt_ave += abs(t_obs[j]-t_est[j]);                     // Sum averaged model-data time variance
    e[1].e_msfit += (t_obs[j]-t_est[j])*(t_obs[j]-t_est[j]);// Sum L2 misfit
   }
   e[1].e_msfit /= ( (float) t[i].c_cnt + 1.);            // Normalize the event misfit
   e[1].e_msfit = sqrt(e[1].e_msfit);                     // 
/*  Require that the model variance is less than two seconds */
   dt_ave /= ( (float) t[i].c_cnt + 1.);                  // Normalize the data-model variance
   if (dt_ave > 2.) {                                     // If the average travel time misfit is greater than 2 seconds, reject the event detection
    e[1].e_r2 = -1.;                                      // Set correlation to < 0 for rejection
    return;                                               // Return with bad correlation
   }

/*  Correlate observed and estimated travel times  */
    e[1].e_r2 = correlate(t_obs, t_est, t[i].c_cnt);  // Correlate observed & estimated times
    fprintf(stdout,"Estimated times correlate at r^2= %f \n",e[1].e_r2);

}











void estimate_magnitude_bs(struct trigger t[], struct event e[], int i) {
/* We need to come up with good magnitude/amplitude relationships.  There are some good ones for peak displacement v. dist.
   We need some for peak acceleration v. distance.  Note - they may vary from location to location.
   We will need to adjust this for characterization of P & S wave values.  It may also be sensor specific.

   This code bootstraps over the data to determine how cerntain the estimated magnitude is.  Use 3X the standard error for 99% confidence.

   The magnitude relation takes the form:
           M = a * LN( accel * b) + c * LN(dist) + d

   The

   This subroutine was written by Jesse F. Lawrence (jflawrence@stanford.edu).

*/   
   float a=1.25f; float b=1.8f; float c=0.8f; float d=3.25f;// Constants for equation above (Need to be adjusted)
   int j, k, kk, n;                                     // Index variables
   float mul_amp;                                       // Multiplication factor depends on P & S waves currently set to 1
   srand ( time(NULL) );                                // Set randomization kernel
   float mag_ave[n_short];                              // Average magnitude

   e[1].e_mag = 0.f;                                    // Zero magnitude
   for (j = 0; j <=t[i].c_cnt; j++) {                   // Bootstrap once for each trigger
    mag_ave[j]=0.;                                      // Zero the average magnitude for this bootstrap
    for (k = 0; k <= t[i].c_cnt; k++) {                 // Select one point for each trigger
     kk = rand() % (t[i].c_cnt+1);                      // Use random trigger
     n = t[i].c_ind[kk];                                // Index of correlated trigger
     if ( t[n].pors == 0 ) {                            // Use appropriate multilication factor (currently not used but will eventually)
      mul_amp = 1.f;                                    //
     } else {                                           //
      mul_amp = 1.f;                                    //
     };                                                 //
     mag_ave[j] += a*log(t[n].mag*b*mul_amp) + c*log(t[n].dis) + d; // Sum magnitude estimate from each trigger for average estimate
    }
    mag_ave[j]/= ( (float) t[i].c_cnt + 1.);            // Normalize summed mag estimates for average magnitude estimate
   }
   e[1].e_mag = average( mag_ave, t[i].c_cnt);          // Average the averages
   e[1].e_std = std_dev( mag_ave, t[i].c_cnt, e[1].e_mag)*3.;// 3 sigma is the 99 % confidence (assuming a statistically large enough data set)
}






float intensity_extrapolate(int pors, float dist, float dist_eq_nd, float intensity_in) {
/* This is a simplistic aproach to wave amplitude decay: 
   The amplitude for a cylindrically expanding wave should be 
   proportional to 1./sqrt(distance).  This blows up near zer 
   distance.  We should take attenuation into account. */

 float fact = dist_eq_nd / dist;                        // Factor of node distance (for intensity map) to event-station distance (from trigger)
 if (pors == 0) fact *= 2.;
 if (fact <=0.01) fact = 0.01;                          // If the factor will lead to order of magnitude stronger shaking, then cap it.
 float intensity_out = intensity_in/((fact+sqrt(fact))/2.);                   // Calculate the projected intensity
 return intensity_out;                                  // 
}




void php_event_email(struct trigger t[], int i, struct event e[], char* epath) {
/* This subroutine should email us when we detect an earthquake */   

}


void php_event_page(struct trigger t[], int i, struct event e[], char* epath) {
/* This subroutine creates a web page for the event. */
   
   char phpfile[sizeof epath + sizeof "/index.php"]; sprintf(phpfile,"%s/index.php",epath);
   FILE *fp11; fp11 = fopen(phpfile,"w+");                       // Open web file
   //fprintf(stdout,"HI2:\n");
   fprintf(fp11,"<?php\n");                                      // Start of PHP
   fprintf(fp11,"require_once('/var/www/qcn/inc/utils.inc');\n");// Includes for page design
   fprintf(fp11,"require_once('/var/www/qcn/earthquakes/inc/qcn_auto_detect.inc');\n");// Includes for autodetection information
   fprintf(fp11,"page_top();\n");                                // Standard QCN formatting of page
   fprintf(fp11,"echo \"\n");                                    // Just a space

/* Earthquake Information */
   fprintf(fp11,"<h1>Earthquake</h1>");                          // Title
   time_t t_eq; struct tm * t_eq_gmt; t_eq = (int) e[1].e_time; t_eq_gmt = gmtime(&t_eq); // Earthquake time
   fprintf(fp11,"<p><strong>Date and Time:</strong> %s </p>\n",asctime(t_eq_gmt));// Earthquake time
   fprintf(fp11,"<p><strong>Longitude:</strong> %4.4f <strong>Latitude:</strong> %4.4f <strong>Depth:</strong> %4.1f km \n",e[1].elon,e[1].elat,e[1].edep);
   fprintf(fp11,"<p><strong>Magnitude:</strong> %1.2f &plusmn;%1.1f(Local estimate - for scientific use only)\n",e[1].e_mag,e[1].e_std);

/* Table for intensity maps */
   fprintf(fp11,"<p><table><tr>");
   fprintf(fp11,"<td width=\\\"50\\\" align=\\\"center\\\"><img src=\\\"./intensity_02.jpg\\\" width=\\\"325\\\"><br><a href=\\\"./intensity_02.ps\\\">PS</a> or <a href=\\\"./intensity_02.jpg\\\">JPEG</a> file.</td>\n");
   fprintf(fp11,"<td width=\\\"50\\\" align=\\\"center\\\"><img src=\\\"./intensity_01.jpg\\\" width=\\\"325\\\"><br>Download: <a href=\\\"./intensity_01.ps\\\">PS</a> or <a href=\\\"./intensity_01.jpg\\\">JPEG</a> file.</td>\n");
   fprintf(fp11,"</tr></table>\n");
   fprintf(fp11,"<a href=\\\"http://qcn.stanford.edu/images/ShakeMap_Scale.png\\\"><img src=\\\"http://qcn.stanford.edu/images/ShakeMap_Scale.png\\\" width=\\\"600\\\"></a> \n");
   fprintf(fp11,"\\n\";");
   
/* List Quakes under if listed beneath this one */
   fprintf(fp11,"echo list_quakes(); \n");
   fprintf(fp11,"echo \" \n");

/* Table of triggers */
   fprintf(fp11,"<h2>Triggers:</h2>\n");                           // Table Title
   fprintf(fp11,"<table>\n");
   fprintf(fp11,"<tr><td><strong>Host ID</strong></td><td><strong>Trigger ID</strong></td><td><strong>Longitude</strong></td><td><strong>Latitude</strong></td><td><strong>Trig Time</strong></td><td><strong>Time Received</strong></td><td><strong>Significance</strong></td><td><strong>|acceleration| (m/s/s)</strong></td><td><strong>Distance (km)</strong></td></tr>");
   int j;int ji = 0;                                               // Index of triggers 
   for (j=0;j<=t[i].c_cnt;j++) {                                   // For each trigger
    int ij = t[i].c_ind[j];
    if(ji>1){fprintf(fp11,"<tr bgcolor=\\\"#FFFFFF\\\"> \n");ji=0;}// Alternate color from white to gray 
    else {fprintf(fp11,"<tr bgcolor=\\\"#DDDDDD\\\"> \n");ji++; }  // Color gray
    fprintf(fp11,"<td><a href=\\\"http://qcn.stanford.edu/%s/show_host_detail.php?hostid=%d\\\">%d</a></td>",t[ij].db,t[ij].hid,t[ij].hid);
    fprintf(fp11,"<td>%d</td><td>%2.4f</td><td>%2.4f</td><td>%f</td><td>%d</td><td>%4.2f</td><td>%4.2f</td><td>%2.5f</td>\n",t[ij].tid,t[ij].slon,t[ij].slat,t[ij].trig,(int) t[ij].rec,t[ij].sig,t[ij].mag,t[ij].dis);
    fprintf(fp11,"</tr> \n");
   }
   fprintf(fp11,"</table> \n");


/* Plot scatter plot of observed to estimated travel times */
   fprintf(fp11,"<hr><table><tr>\n");                                   //
   fprintf(fp11,"<td><img src=\\\"./t_scatter.jpg\\\" width=\\\"325\\\"></td> \n");   
   fprintf(fp11,"<td><p><h2>Travel Time Fit</h2>\n");
   fprintf(fp11,"<p>The quality of the earthquake location depends on the match between the travel times estimated from the earthquake location to the observed travel times.\n");
   fprintf(fp11,"The observed travel times come from the time the earthquake triggered the volunteer sensor computer.\n");
   fprintf(fp11,"The estimated times are determined from the event distance divided by the speed of the wave.\n");
   fprintf(fp11,"The correlation (R) is a measure of the similarity between observed and estimated travel times.\n");
   fprintf(fp11,"The misfit is another measure of the similarity\n");
   fprintf(fp11,"</td></tr></table>\n");
   time_t t_now; struct tm * t_now_gmt; t_now = e[1].e_t_now;  t_now_gmt = gmtime(&t_now); // Current time
   fprintf(fp11,"<hr> \n");
   fprintf(fp11,"<p aling=\\\"justify\\\">Page created on: %s at %f after the event origin. \n",asctime(t_now_gmt),difftime(t_now,t_eq));
   fprintf(fp11,"<hr> \n");
   fprintf(fp11,"<p>The information contained on this page is not intended for official use.  This is a scientific project aiming to validate the methods used to produce these data.  For official earthquake characterization, please obtain the appropriate information from your national earthquake program or the <a href=\\\"http://earthquake.usgs.gov/earthquakes/\\\">USGS.</a>\n");   

/* Output a line stating the time from the event to detection */
   fprintf(fp11,"\n\";\n");
   fprintf(fp11,"echo \"<p>Page viewed on: \". date(\'M d Y\'). \" at \". date('h:i:s'); echo \" (UTC)\";\n");
   
/* Finish the page formatting */
   fprintf(fp11,"page_end();\n");                              // Finish the page formatting
   fprintf(fp11,"?>\n");                                       // Finish the php command
   fclose(fp11);                                               // Close html file 
   
}


void intensity_map_gmt(struct event e[], char* epath){
   fprintf(stdout,"Create/run GMT map script \n");
   int k;
  
    char gmtfile[sizeof epath + sizeof "/gmt_script.csh"]; sprintf(gmtfile,"%s/gmt_script.csh",epath);
    fprintf(stdout,gmtfile);
    FILE *fp10; fp10 = fopen(gmtfile,"w+");                      // gmt script
    fprintf(fp10,"set GMT      = \"/usr/local/gmt/bin\"\n");                                                    // Set GMT bin directory
    fprintf(fp10,"set GRID     = \"-I0.01/0.01\" \n");                                                        // Set grid inerval
    
    float elon = (float) ((int) (e[1].elon*100))/100.;
    float elat = (float) ((int) (e[1].elat*100))/100.;
    fprintf(fp10,"set BOUNDS   = \"-R%2.4f/%2.4f/%2.4f/%2.4f\"\n",elon-2,elon+2,elat-2,elat+2);   // Set bound of intensity map
   
    fprintf(fp10,"set OUTDIR   = \"%s\" \n",epath);                               // Set Output Directory
    fprintf(fp10,"set GRDFILE  = \"$OUTDIR/grid.grd\" \n");                      // Set grid file 
    fprintf(fp10,"set GRADFILE = \"$OUTDIR/grad.grd\" \n");                      // Set grid file 
    fprintf(fp10,"set TOPO     = \"/usr/local/gmt/share/topo/topo30.grd\" \n");                                 // Set topography file 
    fprintf(fp10,"set CITIES   = \"/var/www/qcn/earthquakes/inc/worldcitiespop.gmt\"\n");
    fprintf(fp10,"set CITY_NAMES = \"/var/www/qcn/earthquakes/inc/worldcities_names.gmt\"\n");
    fprintf(fp10,"set EVENT    = \"$OUTDIR/event.xy\" \n");                      // Set event file 
    fprintf(fp10,"set STATIONS = \"$OUTDIR/stations.xyz\" \n");                  // Set station file     
    fprintf(fp10,"set TCONTOUR = \"$OUTDIR/t_contour.xy\" \n");                  // Set contour file
    fprintf(fp10,"set TXTCON   = \"$OUTDIR/t_contour.txt\" \n");                 // Set contour text description
    fprintf(fp10,"set IFILE    = \"$OUTDIR/intensity_map.xyz\" \n");             // Set input file name
    fprintf(fp10,"set TEMP     = \"$OUTDIR/.temp\" \n");                         // Set temp file name
    fprintf(fp10,"set CPTFILE  = \"/var/www/qcn/earthquakes/inc/int.cpt\" \n");              // Set GMT CPT color definition file
    fprintf(fp10,"set X1Y1     = \"-X3 -Y10\" \n");                                                             // Set location of file
    fprintf(fp10,"set PROJ     = \"-JM4i\" \n");                                                              // Set projection of map plot (Mercator)
    fprintf(fp10,"set FLAGS1   = \"-K -P \" \n");                                                               // Set Flags for first plot layer
    fprintf(fp10,"set FLAGS2   = \"-K -O \" \n");                                                               // Set Flags for second plot layer
    fprintf(fp10,"set FLAGS3   = \"-O \" \n");                                                                  // Set flags for last plot layer
    fprintf(fp10,"set COASTS   = \"-Df -N2 -N1 -W1.0p/0 \" \n");                                                // Set coasts

/*  Cut topography */
    fprintf(fp10,"$GMT/grdcut $TOPO -G$GRDFILE $BOUNDS \n");                                                    // Cut grid topography file
    fprintf(fp10,"$GMT/grd2xyz $GRDFILE $BOUNDS > $TEMP \n");
    fprintf(fp10,"$GMT/surface -S2 $TEMP -G$GRDFILE $GRID $BOUNDS \n");                                           // Convert grid to xyz
    fprintf(fp10,"$GMT/grdgradient $GRDFILE -A270/20 -Ne0.5 -G$GRADFILE \n");                                // Create gradient shadow from topogray
  
/*  Creat grid from intensity */ 
    fprintf(fp10,"$GMT/surface $IFILE -S2 -T0.9 $GRID $BOUNDS -G$GRDFILE \n");                                  // Contour a surface to the intensity data    
   for (k=1;k<=2;k++) { 

    fprintf(fp10,"set PSFILE   = \"$OUTDIR/intensity_%02d.ps\"\n",k);                  // Set post script file name
    fprintf(fp10,"set BOUNDS   = \"-R%2.4f/%2.4f/%2.4f/%2.4f\"\n",e[1].elon-k,e[1].elon+k,e[1].elat-k,e[1].elat+k);   // Set bound of intensity map
    fprintf(fp10,"set B        = \"-B%dg%d\" \n",k,k);                                                                // Set tick marks
/*  Plot the grid to an image */
    fprintf(fp10,"$GMT/grdimage $GRDFILE -I$GRADFILE -C$CPTFILE $BOUNDS $PROJ $X1Y1 $FLAGS1 > $PSFILE \n");        // Plot grid to postscript

/*  Plot the coastline:  */
    fprintf(fp10,"$GMT/pscoast $COASTS $PROJ $BOUNDS -W1p/0 $FLAGS2>> $PSFILE \n");

/*  Plot Earthquake Location on plot */
    fprintf(fp10,"$GMT/psxy $EVENT $BOUNDS $PROJ $FLAGS2 -Sa0.75 -W1p/255/0/0 >> $PSFILE \n"); 

/*  Plot Station Locations with color of measured intensity */
    fprintf(fp10,"$GMT/psxy $STATIONS $BOUNDS $PROJ $FLAGS2 -St0.25 -C$CPTFILE -W0.5p/100 >> $PSFILE \n"); 

/*  Plot City Locations */
    fprintf(fp10,"$GMT/psxy $CITIES $BOUNDS $PROJ $FLAGS2 -Sc0.2 -G0 -W1p/255 >> $PSFILE\n");
    fprintf(fp10,"$GMT/pstext $CITY_NAMES $BOUNDS $PROJ $FLAGS2 -G0 >> $PSFILE\n");


/*  Plot time contours */
    fprintf(fp10,"$GMT/psxy $TCONTOUR $BOUNDS $PROJ $FLAGS2 -m -W1p/175 >> $PSFILE \n");
    fprintf(fp10,"$GMT/pstext $TXTCON $BOUNDS $PROJ $FLAGS3 $B -S0.5p >> $PSFILE \n");

/*  Convert PS file to jpeg file: */
    fprintf(fp10,"$GMT/ps2raster $PSFILE -D$OUTDIR -A -P -Tj \n");   
    fprintf(fp10,"if ( ! -e %s/../event.xy ) then\n",epath);
    fprintf(fp10," cp %s/event.xy %s/../.\n",epath,epath);
    fprintf(fp10," cp /var/www/qcn/earthquakes/inc/index_earthquake.php %s/../index.php\n",epath); 
    fprintf(fp10,"endif\n");
   }

   fprintf(fp10,"set BOUNDS   = \"-R0/100/0/100\"\n");   // Set bound of intensity map
   fprintf(fp10,"set T_SCAT   = \"$OUTDIR/t_scatter.xy\" \n");                     // Set contour text description
   fprintf(fp10,"set PROJ     = \"-JX4i/4i\" \n");                                                             // Set projection of map plot (Mercator)
   fprintf(fp10,"set PSFILE   = \"$OUTDIR/t_scatter.ps\"\n");                  // Set post script file name
   fprintf(fp10,"set B        = \"-Ba20f10:T_observed(s):/a20f10:T_estimated(s):WSne\"\n"); // Set tick marks

/* Plot observed v. estimated travel times against each other */
   fprintf(fp10,"$GMT/psxy $T_SCAT $BOUNDS $PROJ $FLAGS1 -Sx0.2 -W1p/255/0/0 >> $PSFILE \n"); 

/* Plot travel time ideal matchup between observed & estimated travel time */
   fprintf(fp10,"$GMT/psxy         $BOUNDS $PROJ $FLAGS2 $B -m -Wthick,- << EOF >> $PSFILE \n"); 
   fprintf(fp10,"  0.0,  0.0\n");
   fprintf(fp10,"100.0,100.0\n");
   fprintf(fp10,"EOF\n");
   fprintf(fp10,"$GMT/psxy         $BOUNDS $PROJ $FLAGS2 $B -m -Wthick,- << EOF >> $PSFILE \n"); 
   fprintf(fp10,"  0.0,  0.0\n");
   fprintf(fp10,"100.0,55.8\n");
   fprintf(fp10,"EOF\n");
   fprintf(fp10,"$GMT/psxy         $BOUNDS $PROJ $FLAGS2 $B -m -Wthick,- << EOF >> $PSFILE \n"); 
   fprintf(fp10,"  0.0,  0.0\n");
   fprintf(fp10,"55.8.0,100.0\n");
   fprintf(fp10,"EOF\n");

/* Label with R^2 correlation and misfit */
   fprintf(fp10,"$GMT/pstext       $BOUNDS $PROJ $FLAGS3 $B << EOF >> $PSFILE \n");
   fprintf(fp10,"5.0 87.0 18 0 0 5 \\ R^2 = %2.2f \n",e[1].e_r2);
   fprintf(fp10,"5.0 80.0 18 0 0 5 \\ Misfit = %2.2f \n",e[1].e_msfit);
   fprintf(fp10,"EOF\n");

/*  Convert PS file to jpeg file: */
   fprintf(fp10,"$GMT/ps2raster $PSFILE -D$OUTDIR -A -P -Tj \n");   

   fclose(fp10);     // Close script

/*  Execute GMT script  */
   char syscmd[sizeof "csh " + sizeof gmtfile + sizeof " &"]; sprintf(syscmd,"csh %s&",gmtfile);
   int retval = system(syscmd);

}



void get_loc(float ilon, float ilat, float dis, float az, float olon, float olat) {
/* This subroutine determins a new location based on a starting location, an azimuth and a distance to project to. */
  float pi = atan(1.)*4.;                                     // pi = 3.14....
  float az_r = az*pi/180.;                                    // azimuth in radians
  float latr = ilat*pi/180.;                                  // Latitude in radians
  float dlon = sin(az_r)*dis/111.19/abs(cos(latr));olon = ilon+dlon;// Longitude difference and longitude
  olat = ilat + cos(az_r)*dis/111.19;                         // New latitude
   
}


void intensity_map(struct trigger t[], int i, struct event e[]) {
   fprintf(stdout,"Calculate intensit map\n");
   float width=5; float dx=0.05;                              // Physical dimensions of grid
   int   nx = ((int) (width/dx)) + 1; int ny = nx;            // array dimension of grid
   float   dist,dist_eq_nd;                                          // Min distance from triger host to grid node
   float   ln_x,lt_x,imap;                                    // Location & intensity at grid node
   FILE *fp10; FILE *fp11;                                    // Output file(s)
   int j,k,l,n,il;                                            // Index variables
   mode_t E_MASK=0777;                                        // File Permissions for new directory to create
   int email = 0;                                             // email=1 if we want to email people about new event
   
   float elon = e[1].elon;                                    // Copy even longitude & Latitude
   float elat = e[1].elat; 
   float x_min = elon - width/2.f;                            // Minimum longitude of map
   float y_min = elat - width/2.f;                            // Min Latitude

/* Create an event directory name                             */
   char edir[]="00000000"; sprintf(edir,"%08d", e[1].eid);
   char epath[sizeof EVENT_PATH + sizeof edir]; sprintf(epath,"%s%s",EVENT_PATH,edir);

/* Create event base directory path                           */
   struct stat st;                                            // I/O status for checking existance of directory/file
   if(stat(epath,&st) != 0) {                                 // If the path does not exist,  
     int retval = mkdir(epath,E_MASK);                        // Make new directory
   }   

/* Create iteration directory                                 */
   char epath2[sizeof epath + sizeof "/A"];                   // Set size of sub directory
   char ABC[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";                   // All caps variables
   for (j = 0; j<64; j++) {                                   // For all letters
     sprintf(epath2,"%s/%c",epath,ABC[j]); // Create full path directory name
     if (stat(epath2,&st) != 0 ) {                            // If the directory doesn't exist, creat it
       int retval = mkdir(epath2,E_MASK);                     // Make new directory
       break;                                                 // Stop here because this is where we want to put the files
     }
   }
   if (j<1) {email=1;} else {email=0;}                // Set email to send if first iteration

/* Generate file names */
   char efile[sizeof epath2 + sizeof "/event.xy"]; sprintf(efile,"%s/event.xy",epath2);
   char sfile[sizeof epath2 + sizeof "/stations.xyz"]; sprintf(sfile,"%s/stations.xyz",epath2);
   char ifile[sizeof epath2 + sizeof "/intensity_map.xyz"]; sprintf(ifile,"%s/intensity_map.xyz",epath2);
   char tfile[sizeof epath2 + sizeof "/t_contour.xy"]; sprintf(tfile,"%s/t_contour.xy",epath2);   
   char txtfile[sizeof epath2 + sizeof "/t_contour.txt"]; sprintf(txtfile,"%s/t_contour.txt",epath2);
   char tscfile[sizeof epath2 + sizeof "/t_scatter.xy"]; sprintf(tscfile,"%s/t_scatter.xy",epath2);
   
/* Create a file with the event location (lon,lat only)       */
   fp10 = fopen(efile,"w+");                                  // Open event output file
   time_t t_now; time(&t_now); e[1].e_t_now = (int) t_now;    // Current time
   fprintf(fp10,"%4.4f,%4.4f,%1.4f,%1.2f,%d,%f,%d,%1.1f\n",elon,elat,e[1].edep,e[1].e_mag, t[i].c_cnt+1,e[1].e_time,e[1].e_t_now,e[1].e_std);// Output event location
   fclose(fp10);                                              // Close event output file name
   
/* Create a file with the station information                 */
   fp10 = fopen(sfile,"w+");                                  // Open station output file
   for (k = 0; k<=t[i].c_cnt; k++) {                          // For each correlated trigger
     n = t[i].c_ind[k];                                       // Index of correlated trigger
     fprintf(fp10,"%f,%f,%f,%d \n",t[n].slon,t[n].slat,t[n].mag,t[n].hid);// Output correlated trigger loc & magnitude
   }
   fclose(fp10);                                               // Close station output file name


/* Create an interpolated intensity map:                      */
   fp10 = fopen(ifile,"w+");                                  // Open intensity file
   float wt;
   for (j=1; j<=nx; j++) {                                   // For each longitudinal node
     ln_x = x_min + dx * (float) (j-1);                      // Longitude of grid point
     for (k=1; k<=ny; k++) {                                 // For each latitudinal node
       lt_x = y_min + dx * (float) (k-1);                    // Latitude of grid point
       float dist_min = 9999999.;                            // Set unreasonably high min distance
       dist_eq_nd = ang_dist_km(ln_x,lt_x,elon,elat);        // Horizontal distance from event to station/host
       imap = 0.f;
       il = -999;                                               // Initialize with obviously bad value
       wt = 0.;
       for (l=0; l<=t[i].c_cnt; l++) {                       // For each trigger
        n = t[i].c_ind[l];                                   // Index of lth trigger
	dist = ang_dist_km(ln_x,lt_x,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
	if (dist_min > dist) {dist_min=dist;il=n;};          // Set minimum distance and lth trigger at dist
	dist = ang_dist_km(elon,elat,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
        imap=imap+intensity_extrapolate(t[n].pors,dist, dist_eq_nd, t[n].mag)/ dist/dist;//(float) (t[i].c_cnt+1);                  // 
        wt = wt + 1./dist/dist;
       }
       imap=imap/wt;                                         // Normalized intensity
       fprintf(fp10,"%f,%f,%f \n",ln_x,lt_x,imap);           // Output locatin & intensity for GMT mapping
    }
   }
   fclose(fp10);                                             // Close intensity map
   

/* Create contours for time relative to identification time */
   fp10 = fopen(tfile,"w+");                                 // Open time contours output file.
   fp11 = fopen(txtfile,"w+");                               // Label file for contours
   float pi = atan(1.)*4.;                                   // pi = 3.14....
   float latr = e[1].elat*pi/180.;                           // Latitude in radians
   time_t t_eq; t_eq = (int) e[1].e_time;double t_dif = difftime(t_now,t_eq);
   for (j = 1; j<=9; j++) {                                  // For five distances
    float dti =  (float) (j-3) * 10.;                        // Time offset from detection time
    float dis = ((float) (j-3) * 10.+t_dif)*3.;              // Distance of time contours (10 s interval at 3km/s)
    if (dis > 0.) {                                          // Only use if distance greater than zero
     for (k=0; k<=360; k++) {                                // for each azimuth
      float az = (float) k * pi / 180.;                      // azimuth in radians
      float dlon = sin(az)*dis/111.19/abs(cos(latr));        // Longitudinal distance
      ln_x = e[1].elon + dlon;                               // New longitude
      lt_x = e[1].elat + cos(az)*dis/111.19;                 // New latitude
      fprintf(fp10,"%f,%f\n",ln_x,lt_x);                     // Output contour
     }                                                       //
     fprintf(fp10,">\n");                                    // Deliminator for separation between line segments
     fprintf(fp11,"%f %f 12 0 1 5 \\ %d \n",ln_x,lt_x,(int) dti );// Output labels for each contour
    }
   }                                                         //
   fclose(fp10);                                             // Close time contour
   fclose(fp11);                                             // Close time contour labels

/* Create scatter plot data for observed v. estimated travel time */
   fp10 = fopen(tscfile,"w+");                               // Open time scatter plot file
   for (j = 1; j <= t[i].c_cnt; j++) {                       // For each correlated trigger
    n = t[i].c_ind[j];                                       // index of correlated triggers
    fprintf(fp10,"%f,%f\n",t[n].trig-e[1].e_time,t[n].t_est-e[1].e_time); // Print out travel times
   }
   fclose(fp10);                                             // Close scatter plot file



   intensity_map_gmt(e,epath2);                              // Run Scripts for plotting (GMT)
   php_event_page(t,i,e,epath2);                             // Output event Page
   if (email==1) {
    php_event_email(t,i,e,epath2);                            // Email if a new event
   }

   return;                                                   // 
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
//   double  dt;                               // Time between triggers
   float   dist;                               // Distance between triggers
   int   nh = 0;int ih=0;                      // Number of hosts, ith host
   int   h[n_long]; int ind[n_long];           // host ids already used
   h[0]=t[iCtr].hid;                           // First host id is last in trigger list
   ind[0]=iCtr;                                // Index of host id's start at last trigger first
   fprintf(stdout,"New possible event: Correlate triggers: %d \n",iCtr);
   for (i=iCtr; i>=2; i--) {                   // For each trigger (go backwards because triggers in order of latest first, and we want first first)
    t[i].c_cnt=0;                              // Zero the count of correlated triggers 
    t[i].c_ind[0]=i;                           // Index the same trigger as zeroth trigger
    ih = -10;                                    // Unassigned host id
    for (j = 0; j<=nh;j++) {                  // search through the assigned host ids
     if (t[i].hid == h[j]) {                  // to find a match
      ih = j;                                 // Match found
     }   
    }
    if (ih<0) {                               // If no match found, then
     nh++;                                    // add a new assigned host id 
     h[nh]=t[i].hid;                          // assign the new host id
     ind[nh]=i;                               // Index the trigger
    } else {                                  // If a prior match is found, then
/*   Save peak fmag for all triggers within 5 seconds of first arrival */
     if ( (t[i].trig>t[ind[ih]].trig) && (t[1].trig<t[ind[ih]].trig + 5.) ) { // Trigger is older than prior, but less than prior + 5 seconds use the higher value
      if (t[i].mag > t[ind[nh]].mag) {t[ind[nh]].mag=t[i].mag;} // Use largest fmag for primary trigger
     }
    }
    
    if ( (t[i].hid!=t[i-1].hid) && (ih<0) ) {        // Do not use repeating triggers
     for (j = i-1; j>=1; j--) {                // For every other trigger
      if ( (t[j].hid!=t[i].hid) && (t[j].hid!=t[j-1].hid) && (abs(t[i].trig-t[j].trig) <= T_max) ) {//For non-repeating triggers & triggers less than t_max apart
       dist=ang_dist_km(t[i].slon,t[i].slat,t[j].slon,t[j].slat);//Distance between triggers
       if ( (abs(t[i].trig-t[j].trig)<dist/Vs + 3.f) && (dist<=D_max) ) {
        t[i].c_cnt++;                          // Add count of correlated triggers for this trigger
        if (t[i].c_cnt>n_short) {              // Make sure we don't use more correlations than array size
         t[i].c_cnt=n_short;                   // Set count to max array size
         break;                                // Done
        }
        t[i].c_ind[t[i].c_cnt]=j;              // index of all correlaed triggers
        t[i].c_hid[t[i].c_cnt]=t[i].hid;       // Index of host ids
       }
      }     
     }
    }
   }                                           // Done correlating



/* Now we correlate triggers that are currently correlated with triggers that are correlated with the initial trigger, but not
   correlated with the initial trigger itself */
   for (i =iCtr; i>1; i--) {                   // For each trigger 
    if (t[i].c_cnt > C_CNT_MIN) {              // If more than 4 correlated triggers, possible regional event
     for (j = i-1;j>=0; j--) {                 // Compare with all later triggers
      if (t[j].c_cnt > C_CNT_MIN) {            // Make sure this trigger is an event all of it's own 
       kl = -10;
       for (k = 0; k<=t[j].c_cnt;k++) {        // Compare all potential secondary correlated triggers 
        for (l = 0; l<=t[i].c_cnt;l++) {       // Make sure trigger isn't same host as prior trigger
         if (t[i].c_ind[l]==t[j].c_ind[k]) {
          kl = l;break;
         }   
        }
        if (kl < 0) {                         // If no matching trigger, then add secondary trigger to primary trigger list
         t[i].c_cnt++;
         t[i].c_ind[t[i].c_cnt]=t[j].c_ind[k];
         t[i].c_hid[t[i].c_cnt]=t[j].c_hid[k];
        }
        t[k].c_cnt = 0;                        // get rid of correlated triggers (now that they are primary triggers)
       }
      }
     }
     j = i; 
     if ( ( t[i].trig > T_max+e[1].e_time)||(abs(t[i].slat-e[1].elat)>3.) ) {
      e[1].eid++;                              // If new Time or location, then new event
      e[1].e_cnt=0;                            // Zero trigger count for event count for if new location/time
     }
     if (t[i].c_cnt > e[1].e_cnt) {            // Only do new event location ... if more triggers for same event (or new event - no prior triggers)
      
      qcn_event_locate(t,j,e);                 // Try to locate event
      if (e[1].e_r2 < 0.5) { break; }          // Stop event if no event located
      e[1].e_cnt = t[i].c_cnt;
      estimate_magnitude_bs(t, e, j);          // Estimate the magnitude of the earthquake
      intensity_map(t,j,e);                    // This generates the intensity map
     }
    } 
   }
   
   return;                                     // Done
};

void get_bad_hosts(struct bad_hosts bh) {
/*  This subrouting retrieves the bad host names */
   FILE *fp10; fp10 = fopen(BAD_HOSTS_FILE,"r+");
   bh.nh = -1;
   while (feof(fp10) == 0) {
    bh.nh++;
    fscanf(fp10,"%d",bh.hid[bh.nh]);
   }
   fclose(fp10);
   return;
}





int main(int argc, char** argv) 
{
    struct trigger t[n_long];                            // Trigger buffer ring
    struct event   e[2];e[1].eid=0;                      // event
    int retval;
    int tidl=0; int hidl=0;                                         // default last host id
    //struct bad_hosts(bh);
    //get_bad_hosts(bh);

    
/* initialize random seed: */
    srand ( time(NULL) );

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
      
      if (iCtr>C_CNT_MIN) {
       if ( (t[1].tid != tidl) && (t[1].hid != hidl) ) {  // Dont allow repeat trigger id or host id as last entry otherwise redundant process
        detect_qcn_event(t,iCtr,e);       
        tidl=t[1].tid; hidl=t[1].hid;                    // Save last trigger id & host id (so no repeats)
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



