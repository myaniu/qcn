#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UTC (0)
#define Vs (3.4)
#define Vp (6.4)

#define n_long (1000)                       // Length of trigger buffer ring
#define n_short (100)

struct trigger {
/*  Data structure for input trigger data to be used with QCN MySQL output & location
    program. This structure written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu                                                  */
   int    hid;                   // Host ID (Sensor number) 
   float  slon, slat;            // Sensor location
   double trig, rec;             // Time of trigger & Time received
   float  sig, mag;              // Significance and magnitude (sig/noise)
   int    c_cnt;                 // Count of correlated triggers
   int    c_ind[n_short];        // Correlated trigger IDs
   int    c_hid[n_short];        // Correlated host IDs
};

struct event {
/*  Data structure for events. To be used with QCN location
    program. This structure written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu                                                  */
   
   int    eid;                   // Event ID
   float  elon,elat,edep;        // Event Longitude, Latitude, & Depth
   double  e_time;               // Event Origin Time
   float  e_r2;                  // r-squared correlation
   float  e_mag;                 // Event magnitude
   float  e_cnt;
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


int i_loop(int i) {
/*  This function loops over a vector length n_long. If initial value 0<=i<=n_long,
    then it returns the initial value.  If i<0 or i>n_long then it loops it back around
    so that 0<i<n_long is true.  This function written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu           */
   int j = i;
   if (j < 0) { j = j + n_long; }
   if (j > n_long) { j = j - n_long;}
   return j;
};

int read_trigger_data (struct trigger t[], FILE *fp1, int i) {
   
   int m,n=0;                               // Index variable 
   if (fscanf(fp1,"%d %f %f %lf %lf %f %f",
              &t[i].hid, &t[i].slat, &t[i].slon, &t[i].trig, &t[i].rec,
              &t[i].sig, &t[i].mag) == 7){ // Read in trigger data
   t[i].c_cnt=0;                           // Zero the count of correlated triggers
   t[i].c_hid[0]=t[i].hid;
   t[i].c_ind[0]=i;
   for (m = 0; m<=1000; m++) {
    t[i].c_ind[m]=0;                       // Zero the correlated trigger index
    t[i].c_hid[m]=0;                       // Zero the correlated host ID index
   };
   } else {
    printf("READ ERROR: %d \n",n);
    return -1;
   }
   return 1;

};



float ang_dist_km(float lon1, float lat1, float lon2, float lat2) {
/* This function returns the distance (in km) between two points given as
   (lon1,lat1) & (lon2,lat2).This function written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu   
*/
   float pi = atan(1.f)*4.f;            // set pi = 3.14....
   float d2r = pi/180.f;                // Degree to radian Conversion
   double x1,y1,z1,x2,y2,z2;             // Cartesian coordinates of two locs 
   
   x1 = sin(lat1*d2r)*cos(lon1*d2r);    // (x,y,z unit vector in cartesian from origin)
   y1 = sin(lat1*d2r)*sin(lon1*d2r);
   z1 = cos(lat1*d2r);
   
   x2 = sin(lat2*d2r)*cos(lon2*d2r);    // (x,y,z unit vector in cartesian from origin)
   y2 = sin(lat2*d2r)*sin(lon2*d2r);
   z2 = cos(lat2*d2r);

   double cd  = x1*x2 + y1*y2 + z1*z2;   // Cosine of distance
   double re   = 6371.f;                 // Radius of the earth
   float delt = acos(cd)*re;            // Distance in km
   
   return delt;
};




int check_hid1_encountered( struct trigger ti, struct trigger tk)

{
   int l;
      
   if ( ti.c_cnt>0 ) {
    for (l=1 ; l<=ti.c_cnt; l++) {
     if (tk.hid == ti.c_hid[l]) {
      return 1;
     }
    }
   }

   if ( tk.c_cnt>0 ) {
    for (l=1 ; l<=tk.c_cnt; l++) {
     if (ti.hid == tk.c_hid[l]) {
      return 1;
     }
    }
   }

   
   return 0;
}



void detect_qcn_event(struct trigger t[], struct qcn_host h[], int i) {
/* This subroutine determines if a set of triggers is correlated in time and space.
   The subroutine is used by program main, which assumes that the triggers have already been read in.
   The subroutine uses ang_dist_km, which provides the distance between two points in kilometers. 
   The subroutine uses hid_yn, which determines if the primary trigger has already encountered a host ID or not.
   
   A correlated station pair occurs when the distance is < 100 km apart and the trigger time difference 
   is less than the velocity divided by the station separation.
*/
   int j,k;                             // Index variables
   double  dt;                          // Time between triggers
   float   dist;                        // Distance between triggers
   

   if (t[i].trig - h[t[i].hid].t_last < 90.) {return;};
   
   
   for (j = 1; j <=1000-1; j++) {       // Loop back over triggers
    k = i_loop(i-j);                    // Index within ring trigger buffer
    if (t[k].trig < 123.f) {return;};   // Skip if not trigger time (buffer not filled yet).
    if (t[i].hid!=t[k].hid) {           // Check to make sure not the same station
     dist=ang_dist_km(t[i].slon,t[i].slat,t[k].slon,t[k].slat);//Distance between triggers
     dt  = abs(t[i].trig-t[k].trig);    // Time between triggers
     int hid_yn = check_hid1_encountered( t[i],t[k]);//Check if station pair already evalued
     if ( (dist < 100.f) && (dt < dist/Vs+1.f) && (abs(dt) < 90.) && (hid_yn == 0) ) {
      t[i].c_cnt = t[i].c_cnt + 1;      // Count # of correlated triggers
      t[i].c_ind[t[i].c_cnt]=k;         // Store correlated triggers
      t[i].c_hid[t[i].c_cnt]=t[k].hid;  // Store correlated host IDs
     }
    }  
   }
   t[i].c_ind[0]=i;
   return;
};




void set_grid2D( struct bounds g[], float elon, float elat, float width, float dx, int j) {
/* This subroutine sets up a 3D grid search (x,y, & z) for source location.  This is a brute force location
   scheme.  Slower than it should be.  The gridding is made into a subroutine because I do several iterations,
   that focus in on the best solution of the prior grid, which is much faster than a fine grid for a large area.
   
   The grid uses structure bounds.  The center of the grid is assumed to be the best eq estimate.
   
   The grid has horizontal dimensions width with horizontal node intervals of dx.
   
   The grid has vertical dimension zrange with vertical node interval of dz.

*/   float d2r = atan(1.f)/45.f;                    // Degrees to radians 
   g[j].yw = width;                               // Latitudinal width of grid
   g[j].xw = width;///g[j].lon_factor;            // Latitudinal width of grid
   g[j].zw = 0;
   
   g[j].x_min = elon-g[j].xw/2.f;                 // Set bounds of grid
   g[j].x_max = elon+g[j].xw/2.f;
   g[j].y_min = elat-g[j].yw/2.f;
   g[j].y_max = elat+g[j].yw/2.f;
   g[j].z_min = 0.;
   g[j].z_max = 0.;

   g[j].dy = dx;                                  // Set latitudinal step size
   g[j].dx = dx;///g[j].lon_factor;               // Set longitudinal step size
   g[j].dz = 0;
   
   g[j].ny = (int) ((g[j].y_max-g[j].y_min)/g[j].dy);// Set number of latitudinal steps
   g[j].nx = (int) ((g[j].x_max-g[j].x_min)/g[j].dx);// Set number of longitudinal steps
   g[j].nz = 1;                                      // Set number of depth steps
//   printf("SET_GRID: %f %f %f %d %f %f %f \n", d2r, g[j].x_min, g[j].dy, g[j].nx,elon,elat,g[j].lon_factor);
};




void set_grid3D( struct bounds g[], float elon, float elat, float edep, float width, float dx, float zrange, float dz, int j) {
/* This subroutine sets up a 3D grid search (x,y, & z) for source location.  This is a brute force location
   scheme.  Slower than it should be.  The gridding is made into a subroutine because I do several iterations,
   that focus in on the best solution of the prior grid, which is much faster than a fine grid for a large area.
   
   The grid uses structure bounds.  The center of the grid is assumed to be the best eq estimate.
   
   The grid has horizontal dimensions width with horizontal node intervals of dx.
   
   The grid has vertical dimension zrange with vertical node interval of dz.

*/   float d2r = atan(1.f)/45.f;                    // Degrees to radians 
   g[j].yw = width;                               // Latitudinal width of grid
//   g[j].lon_factor = abs(cos( (elat * d2r)));   // Longitude to Latitude ratio (lon<lat near poles)
   g[j].xw = width;///g[j].lon_factor;            // Latitudinal width of grid
   g[j].zw = zrange;
   
   g[j].x_min = elon-g[j].xw/2.f;                 // Set bounds of grid
   g[j].x_max = elon+g[j].xw/2.f;
   g[j].y_min = elat-g[j].yw/2.f;
   g[j].y_max = elat+g[j].yw/2.f;
   g[j].z_min = edep-g[j].zw/2.f;if (g[j].z_min < 0.) {g[j].z_min=0.f;};
   g[j].z_max = edep+g[j].zw/2.f;

   g[j].dy = dx;                                  // Set latitudinal step size
   g[j].dx = dx;///g[j].lon_factor;               // Set longitudinal step size
   g[j].dz = dz;
   
   g[j].ny = (int) ((g[j].y_max-g[j].y_min)/g[j].dy);// Set number of latitudinal steps
   g[j].nx = (int) ((g[j].x_max-g[j].x_min)/g[j].dx);// Set number of longitudinal steps
   g[j].nz = (int) ((g[j].z_max-g[j].z_min)/g[j].dz);// Set number of depth steps
//   printf("SET_GRID: %f %f %f %d %f %f %f \n", d2r, g[j].x_min, g[j].dy, g[j].nx,elon,elat,g[j].lon_factor);
};


void vel_calc(float dep, float v[]) {
/* This subroutine is a kluge - it increases velocity with depth, but assuming path averaged velocity for an earthquake that
   occurs at depth.  This is only a kluge, because the path average velocity changes as a function of depth/distance.  This
   subroutine needs updating.  Jesse has accurate versions in fortran, but they need to be converted to c.

*/
   float dep2=dep;if (dep < 5.f) {dep2=5.f;};
   v[1] = 0.34*log(dep2)+2.56;
   v[0] = 1.86*v[1];
}


void locate_event(struct trigger t[], struct event e[], int i, int n_ev) {
/* This subroutine locates an event given a series of triggers (t) with structure trigger.  The event (e) with structure event
   is both an input and an output.  The initial location is updated.  The event is located from the ith trigger's perspective.
   n_ev is the nth iteration of the trigger location.
   
   Note - triggers from the current setup are not ideal for locating events because we don't know if they are P-waves or S-waves,
   so this program guesses on a trigger pair basis.  This really should be done on a one relative to all other triggers basis.
   
    
*/   
   int    h,j,k,l,m,n,o,p,q,r,j_min;
   double t_min = t[i].trig;                // Minimum trigger time
   struct bounds g[4];                      // Bounds of grid
   float  width = 4.f; float zrange=150.f;  // Lateral and vertical grid size
   float  dx = 0.1f; float dz = 10.f;       // Lateral and vertical grid step size                  
   int    n_iter=3;                         // Number of grid search iterations
   float  ln_x,lt_x,dp_x;
   float  dn,dp;                            // Distances to the nth and pth trigger location
   float  tn_S,tp_S,tn_P,tp_P;              // Travel times to n&pth trigger For S & P waves.
   float  vn,vo;
   double  dt,dt_min;
   double  ls_mf,ls_ct;                     // Least-squares misfit & count (For normalization)
   float   v[2];
   
   srand ( time(NULL) );


   t_min = t[i].trig;
   for (j = 1; j<=t[i].c_cnt; j++) {
    if (t_min > t[t[i].c_ind[j]].trig) {
    t_min = t[t[i].c_ind[j]].trig;
    j_min = t[i].c_ind[j];
    };//Find first trigger time
   }
//   float qlon=e[n_ev].elon; float qlat=e[n_ev].elat;
   for (j = 1; j<=n_iter; j++) {           // For each iteration 
    set_grid3D(g, e[n_ev].elon, e[n_ev].elat, e[n_ev].edep, width, dx, zrange, dz, j);        // Set bounds of grid search
    
    
       
    float ls_mf_min = 9999999999.f;         // Set obserdly high misfit minimum
    for (h = 1; h<=g[j].nz; h++) {
     dp_x = g[j].z_min + g[j].dz * (float) (h-1);
     vel_calc(dp_x,v);                  // Calculate mean path velocity
     for (k = 1; k<=g[j].nx; k++) {            // For each x node
      ln_x = g[j].x_min + g[j].dx * (float) (k-1);// Longitude of grid point
      for (l = 1; l<=g[j].nx; l++) {           // For each y node
       lt_x = g[j].y_min + g[j].dy * (float) (l-1);// Latitude of grid point
       ls_mf = 0.; ls_ct = 0.;              // Zero least-squares misfit & count
      

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
         e[n_ev].elon=ln_x; e[n_ev].elat=lt_x; e[n_ev].edep=dp_x;// Save location of best fit
         e[n_ev].e_r2 = r2;                              // Save correlation of best fit
        }                                                // End loop over 2nd trigger of pair
       }                                                 // End loop over 1st trigger of pair
      }                                                  // End loop over latitude
     }                                                   // End loop over Longitude
    }                                                    // End loop over depth
    dx = dx / 10.f; width = width/10.f; dz = dz / 10.f; zrange=zrange/10.f;
   
   }
   
   dn=ang_dist_km(e[n_ev].elon,e[n_ev].elat,t[i].slon,t[i].slat);//Distance between triggers in horizontal plain
   dn = sqrt(dn*dn + e[n_ev].edep*e[n_ev].edep);                 // Distance between triggers in 3D (direct line, not ray path).
   vel_calc(e[n_ev].edep,v);                                     // Get velocity (note - depth dependant - not done well).
   e[n_ev].e_time = t[i].trig - dn/v[1];                         // time of event calculated from ith trigger.
   return;
};

void ave_locate_ini(struct trigger t[], struct event e[], int i, int n_ev){
/* This subroutine determines the initial location based on the location of the first host in the trigger sequence.
   I also calculate the correlation of time with distance from the source.  Ideally for all P-waves, this would be
   1, so that as the wave expands, it triggers perfectly on the P wave with time.  This doesn't happen with imperfect
   sensors.  With better sensors we should with time get better correlations.
*/   
   int j;                                               //
   double t_min=t[t[i].c_ind[1]].trig;                  //
   e[n_ev].elon = t[t[i].c_ind[1]].slon;                // Set initial event location to primary trigger location
   e[n_ev].elat = t[t[i].c_ind[1]].slat;                // 
   for (j = 1; j<=t[i].c_cnt; j++) {
      if (t[t[i].c_ind[j]].trig < t_min) {              // Use earliest trigger host location as event initial location
         e[n_ev].elon = t[t[i].c_ind[j]].slon;          // Initial Lon
         e[n_ev].elat = t[t[i].c_ind[j]].slat;          // Initial Lat
      }
   }




/* Direct method: correlate time & node-sensor distance - (not yet accounting for P or S wave question). */
      int m,n; float d_av=0.; double t_av=0.; float dn; // Correlate trigger times & location
      for (m=1; m<=t[i].c_cnt; m++){
       n = t[i].c_ind[m];                               // Index of correlated trigger
       dn=ang_dist_km(e[n_ev].elon,e[n_ev].elat,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
       d_av = d_av + dn/ (float) t[i].c_cnt;            //average distance (sum first, normalize later)
       t_av = t_av + t[n].trig/ (float)t[i].c_cnt;      // Average trigger time (sum first, normalize later)
      }
      float num=0.; float den1=0.; float den2=0.;
      for (m=1; m<=t[i].c_cnt; m++){
       n = t[i].c_ind[m];                               // Index of correlated trigger
       dn=ang_dist_km(e[n_ev].elon,e[n_ev].elat,t[n].slon,t[n].slat);   // Horizontal distance between event and host/station
       num = num + (dn-d_av)*(t[n].trig-t_av);          // Numerator of correlation (Sxy)
       den1 = den1 + (dn-d_av)*(dn-d_av);               // Denominator part 1 of (Sxx)
       den2 = den2 + (t[n].trig-t_av)*(t[n].trig-t_av); // Denominator part 2 (Syy)
      };
      if (num<0) num=0.000001;                          // Only use positive correlation (not negative correlation)
      e[n_ev].e_r2 = num/sqrt(den1)/sqrt(den2);         // Normalize correlation Sxy/(Sxx^0.5 * Syy^0.5)
      e[n_ev].edep = 75.f;                              // Set depth (not actual) to ave depth of search area

};


void estimate_magnitude(struct trigger t[], struct event e[], int i, int n_ev) {
/* We need to come up with good magnitude/amplitude relationships.  There are some good ones for peak displacement v. dist.
   We need some for peak acceleration v. distance.  Note - they may vary from location to location.
   We will need to adjust this for characterization of P & S wave values.  It may also be sensor specific.
M = a * LN( accel * b) + c * LN(dist) + d
*/   
   float a=1.25f; float b=.9f; float c=0.8f; float d=3.25f; float pi = atan(1.f)*4.f;
   int j, n; float dn; 
   float v[2],ts,tp,mul_amp;
    
   e[n_ev].e_mag = 0.f;                                    // Zero magnitude
   for (j = 1; j <=t[i].c_cnt; j++) {
      n = t[i].c_ind[j];                                // Index of correlated trigger
      dn=ang_dist_km(e[n_ev].elon,e[n_ev].elat,t[n].slon,t[n].slat);//Distance between triggers in horizontal plain
      dn = sqrt(dn*dn + e[n_ev].edep*e[n_ev].edep);     // Distance between triggers in 3D (direct line, not ray path).
      vel_calc(e[n_ev].edep, v);
      tp = abs(t[n].trig-e[n_ev].e_time-dn/v[0]);
      ts = abs(t[n].trig-e[n_ev].e_time-dn/v[1]);
      if ( ts < tp ) { mul_amp = 1.f; } else { mul_amp = 2.f;};
      e[n_ev].e_mag = e[n_ev].e_mag + a*log(t[n].mag*b*mul_amp) + c*log(dn) + d;
   }
   e[n_ev].e_mag = e[n_ev].e_mag / (float) t[i].c_cnt;

}


void update_qcn_event_db(struct trigger t[], struct event e[], int i, int n_ev, int nev) {
/* This subroutine should update the MySQL event database - Carl - this will have to be all you

*/



}


void create_event_php(struct trigger t[], struct event e[], int i, int n_ev, int nev) {
/* This subroutine should create a web page for the event

*/



}


void email_qcn_personnel(struct trigger t[], struct event e[], int i, int n_ev, int nev) {
/* This subroutine should email qcn personnel that an event has been detected

*/



}


void add_triggers_to_event(struct trigger t[], struct trigger et[], int i, int nev) {
   
   int j,k,jj;int found=0;

   if (et[nev].c_cnt < 1) {
      et[nev] = t[i];
   } else {

    for (j = 1; j<=t[i].c_cnt; j++) {                      //Compare each new trigger host ID with
     found = 0;
     for (k = 1; k<=et[nev].c_cnt; k++) {               //with each event trigger host 
      if (et[nev].c_hid[k]==t[i].c_hid[j]) {            //If host IDs match, don't add trigger to event  
       found = 1;   
      }
     }   
     if (found == 0) {                                  // No host ID match so add trigger
      et[nev].c_cnt++;k=et[nev].c_cnt;                  // Add one trigger
      et[nev].c_ind[k]=t[i].c_ind[j];                   // Add trigger index
      et[nev].c_hid[k]=t[i].c_hid[j];                   // Add trigger index
     }
    }

   }
};


float intensity_extrapolate(float dist, float dist_eq_nd, float intensity_in) {
 if (dist       < 1.f) {dist      =1.f;}
 if (dist_eq_nd < 1.f) {dist_eq_nd=1.f;}
 float intensity_out = intensity_in * ((dist_eq_nd) / (dist))*1.7 ;
 return intensity_out;
}

void intensity_map(struct trigger t[], struct trigger et[], int i, float elon, float elat, int nev) {
   struct bounds g[3];                                           // Grid structure
   float width=5; float dx=0.05;                              // Dimensions of grid
   float   dist,dist_min,dist_min2,dist_eq_st,dist_eq_st2,dist_eq_nd;                                          // Min distance from triger host to grid node
   float   ln_x,lt_x,imap;                                    // Location & intensity at grid node
   char ifile[sizeof "INTENSITIES/intensity_100.xyz"];          // intensity file
   char efile[sizeof "EVENTS/event_100.xy"];               // Event file
   char sfile[sizeof "STATIONS/stations_100.xyz"];           // station info
   FILE *fp10;                                                // Output file(s)
   int j,k,l,m,n,il,il2,il3;                                          // Index variables
   float pi=atan(1.f)*4.f;
   j=1;
   set_grid2D(g, elon, elat, width, dx, j);                      // Set grid
   
   printf("DONG %d \n",et[nev].c_cnt);
   sprintf(efile, "EVENTS/event_%03d.xy", nev);            // Create event output file name
   fp10 = fopen(efile,"w+");                                  // Open event output file
   fprintf(fp10,"%f %f\n",elon,elat);                         // Output event location
   fclose(fp10);                                                // Close event output file name
   
   sprintf(sfile, "STATIONS/stations_%03d.xyz", nev);        // Create station output file name
   fp10 = fopen(sfile,"w+");                                  // Open station output file
   for (k = 0; k<=et[nev].c_cnt; k++) {                          // For each correlated trigger
     n = et[nev].c_ind[k];                                       // Index of correlated trigger
     fprintf(fp10,"%f %f %f \n",t[n].slon,t[n].slat,t[n].mag*1.7);// Output correlated trigger loc & magnitude
   }
   fclose(fp10);                                               // Close station output file name
   printf("DING\n");
   sprintf(ifile, "INTENSITY/intensity_%03d.xyz", nev);       // Intensity file name
   fp10 = fopen(ifile,"w+");                                  // Open intensity file
   
   
   for (j=1; j<=g[1].nx; j++) {                                   // For each longitudinal node
     ln_x = g[1].x_min + g[1].dx * (float) (j-1);            // Longitude of grid point
     for (k=1; k<=g[1].ny; k++) {                                 // For each latitudinal node
       lt_x = g[1].y_min + g[1].dy * (float) (k-1);          // Latitude of grid point
       float dist_min = 9999999.;                            // Set unreasonably high min distance
       dist_eq_nd = ang_dist_km(ln_x,lt_x,elon,elat);   // Horizontal distance from event to station/host
       imap = 0.f;
       for (l=0; l<=et[nev].c_cnt; l++) {                       // For each trigger
        n = et[nev].c_ind[l];                                   // Index of lth trigger
	dist = ang_dist_km(ln_x,lt_x,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
	if (dist_min > dist) {dist_min=dist;il=n;};          // Set minimum distance and lth trigger at dist
	dist = ang_dist_km(elon,elat,t[n].slon,t[n].slat);   // Horizontal distance from event to station/host
        imap=imap+intensity_extrapolate(dist_eq_nd, dist, t[n].mag)/ (float) (et[nev].c_cnt+1);                  // 
       }
	dist = ang_dist_km(elon,elat,t[il].slon,t[il].slat);   // Horizontal distance from event to station/host
        imap=(2.f*imap+intensity_extrapolate(dist_eq_nd, dist, t[il].mag))/3.f;                  // 
       fprintf(fp10,"%f %f %f \n",ln_x,lt_x,imap);
     }
   }
   fclose(fp10);

   return;
}


int main (){
/*  This main section of the program:
    1) reads in triggers (input file created by script from MySQL DB on kew.stanford.edu).
    2) runs detection algorithm
    3) checks to see if there are sufficient triggers to warrent a probable detection.
    4) Attempt to locate events given probable detection. 


*/
   struct trigger t[n_long],et[n_long];                            // Trigger buffer ring
   int    i,j,k,l,m,n;                                  // Index variables
   struct event e[n_long],e_ave,e_std;                   // Event data
   struct qcn_host  h[n_long];                          // Host info
   
   time_t time_new;                                     // Date in seconds since 1970
   struct tm * dtm;                                     // Structure of date-time variable

   FILE  *fp1,*fp2,*fp3,*fp4;                           // Input (1) & output (2-4) files
   
   char   fname[64];                                    // Input file name:
   
   float  elon,elat,edep,r2;                            // Event location
   double e_time;int sec;                               // Event time
   float  qlon,qlat;
   
   int    cnt;int hid_last=0;
   int i_l;
   printf("Enter input file name: \n");                 // Request input file name
   scanf("%s",fname);                                   // Read input file name
   fp1 = fopen(fname,"r");                              // Open input file
   fp2 = fopen("qcn_earthquakes.txt","w+");
   fp3 = fopen("qcn_sensors.txt","w+");                 // QCN sensor locations output file
   fp4 = fopen("qcn_intensity.txt","w+");               // QCN sensor locations output file
   
   
   int n_ev = 0;                                        // Start with zeroth event
   int e_id = 0;
   int nev  = 0;
   e[0].e_time = 0.;                                    // Initial time
   i = 0;                                               // Zero index before using
   while (i > -1) {                                     // Indefinite loop
      if (read_trigger_data(t,fp1,i) <0){break;};                       // Read in trigger data
      if (t[i].mag < 0.2) {continue;};
//      if ( abs(t[i].trig-t[i].rec) > 20.f) {continue;};
      if (e[0].e_time<=0.f) {e[0].e_time=t[i].trig;};
      detect_qcn_event(t,h,i);                          // Detect a possible new event
      cnt = cnt + t[i].c_cnt;                           // Count triggers
      if ( (t[i].c_cnt > 4) && (t[i].hid!=hid_last) ) {
       if (t[i].trig-e[n_ev].e_time > 90.) { 
	fprintf(fp2,"%f %f \n",e_ave.elon,e_ave.elat);
        time_new = t[i].rec; dtm = gmtime(&time_new); sec=(int) ((t[i].rec-(int)t[i].rec)*1000.f);
        n_ev ++;
	e[n_ev].e_time = t[i].trig;                        // Set initial event time to trigger time
	e_id = 0;
	if (nev > 0) {
	 intensity_map(t, et, i_l, e_ave.elon, e_ave.elat, nev);
	}
	nev ++;
	add_triggers_to_event(t,et,i,nev);
	ave_locate_ini(t,e,i,n_ev);                     // Get initial location from first trigger location
	locate_event(t,e,i,n_ev);                       // Try a better location of the event
	time_new = e[n_ev].e_time; dtm = gmtime(&time_new);sec=(int)((e[n_ev].e_time-(int)e[n_ev].e_time)*10.f)*1000;
        printf("\nCEED/QCN, %d %d/%02d/%02d %02d:%02d:%02d.%04d +/-?????,	Trigs:%d ",nev,
	           dtm->tm_year+1900,dtm->tm_mon+1,dtm->tm_mday,
                   dtm->tm_hour,dtm->tm_min,dtm->tm_sec,sec,t[i].c_cnt);
        printf("	LOC%d:	%f, %f, %f,	R=%f",e_id,e[n_ev].elon,e[n_ev].elat, 0.f ,e[n_ev].e_r2);
	estimate_magnitude(t,e,i,n_ev);
	printf("	MAG: %f +/- ?????",e[n_ev].e_mag);
        e[n_ev].eid = e_id;
        e_ave = e[n_ev];
	

        time_new=time(NULL);e[n_ev].e_t_detect=time_new;  // Store Time detected
	e[n_ev].e_dt_detect=t[i].rec-e[n_ev].e_time;// Detection latency
	printf("	DT: %f \n", e[n_ev].e_dt_detect);
	
	
        update_qcn_event_db(t,e,i,n_ev,nev);             // Update QCN event database
        create_event_php(t,e,i,n_ev,nev);                // To Do - Create a web page for event
	email_qcn_personnel(t,e,i,n_ev,nev);             // Email QCN Personnel about event
	i_l = i;
       }
       hid_last = t[i].hid;

       if (t[i].c_cnt > 4+(e_id+1)) {
	n_ev++;                                          // Next event location when new triggers added
	e_id++;
	add_triggers_to_event(t,et,i,nev);
	ave_locate_ini(t,e,i,n_ev);                      // Get initial location from first 
	locate_event(t,e,i,n_ev);                        // Better location
        e[n_ev].eid = e_id;

	estimate_magnitude(t,e,i,n_ev);
        e_ave.elon   = ( e_ave.elon  +e[n_ev].elon  *(float) nev ) / (float) ( nev + 1);
        e_ave.elat   = ( e_ave.elat  +e[n_ev].elat  *(float) nev ) / (float) ( nev + 1);
        e_ave.edep   = ( e_ave.edep  +e[n_ev].edep  *(float) nev ) / (float) ( nev + 1);
	e_ave.e_time = ( e_ave.e_time+e[n_ev].e_time*(float) nev ) / (float) ( nev + 1);
        e_ave.e_mag  = ( e_ave.e_mag +e[n_ev].e_mag *(float) nev ) / (float) ( nev + 1);
        e_ave.e_r2   = ( e_ave.e_r2  +e[n_ev].e_r2  *(float) nev ) / (float) ( nev + 1);

	e_std.elon   = ( e_ave.elon  -e[n_ev-e_id].elon  )*( e_ave.elon  -e[n_ev-e_id].elon  ) ;
        e_std.elat   = ( e_ave.elat  -e[n_ev-e_id].elat  )*( e_ave.elat  -e[n_ev-e_id].elat  ) ;
        e_std.edep   = ( e_ave.edep  -e[n_ev-e_id].edep  )*( e_ave.edep  -e[n_ev-e_id].edep  ) ;
	e_std.e_time = ( e_ave.e_time-e[n_ev-e_id].e_time)*( e_ave.e_time-e[n_ev-e_id].e_time) ;
        e_std.e_mag  = ( e_ave.e_mag -e[n_ev-e_id].e_mag )*( e_ave.e_mag -e[n_ev-e_id].e_mag ) ;
        for (k = n_ev-e_id+1; k >= n_ev; k--) {
	 e_std.elon   = e_std.elon  + ( e_ave.elon  -e[k].elon  )*( e_ave.elon  -e[k].elon  ) ;
         e_std.elat   = e_std.elat  + ( e_ave.elat  -e[k].elat  )*( e_ave.elat  -e[k].elat  ) ;
         e_std.edep   = e_std.edep  + ( e_ave.edep  -e[k].edep  )*( e_ave.edep  -e[k].edep  ) ;
	 e_std.e_time = e_std.e_time+ ( e_ave.e_time-e[k].e_time)*( e_ave.e_time-e[k].e_time) ;
         e_std.e_mag  = e_std.e_mag + ( e_ave.e_mag -e[k].e_mag )*( e_ave.e_mag -e[k].e_mag ) ;
        }	        
	time_new = e_ave.e_time; dtm = gmtime(&time_new);sec=(int)((e_ave.e_time-(int)e_ave.e_time)*10.f)*1000;
        printf("CEED/QCN, %d %d/%02d/%02d %02d:%02d:%02d.%04d +/-%5.2f,	Trigs:%d ",nev,dtm->tm_year+1900,dtm->tm_mon+1,dtm->tm_mday,
                   dtm->tm_hour,dtm->tm_min,dtm->tm_sec,sec,2.f*sqrt(e_std.e_time/(float)e_id),t[i].c_cnt);
        printf("	LOC%d:	%f, %f, %f,	R=%f",e_id,e_ave.elon,e_ave.elat,e_ave.edep,e_ave.e_r2);
        printf("	MAG: %f +/- %5.2f",e_ave.e_mag,2.f*sqrt(e_std.e_mag/(float)e_id));

	time_new=time(NULL);e[n_ev].e_t_detect = time_new;// Time event detected 
	e[n_ev].e_dt_detect=t[i].rec-e[n_ev].e_time;// Detection latency
	printf("	DT: %f \n", e[n_ev].e_dt_detect);
	
        update_qcn_event_db(t,e,i,n_ev,nev);             // Update QCN event database
        for (k = 1; k<=t[i].c_cnt; k++) {
	   n = t[i].c_ind[k];
	   fprintf(fp3,"%f %f \n",t[n].slon,t[n].slat);
	   fprintf(fp4,"%f %f %f \n",t[n].slon,t[n].slat,t[n].mag);
        }
	fprintf(fp4,"\n");
//	i_l = i;
       }

      }
      h[t[i].hid].hid = t[i].hid;                        // Update the host info
      h[t[i].hid].slat = t[i].slat;                      //
      h[t[i].hid].slon = t[i].slon;                      //
      h[t[i].hid].t_last = t[i].trig;                    // 
      i++; if (i > n_long) i = 0;                        // Next i & loop back over ring if needed
   }

   time_new = t[i-1].trig; dtm = gmtime(&time_new);sec=(int)((e_ave.e_time-(int)e_ave.e_time)*10.f)*1000;
   printf("LAST TIME: %d/%02d/%02d %02d:%02d:%02d %f",dtm->tm_year+1900,dtm->tm_mon+1,dtm->tm_mday,
              dtm->tm_hour,dtm->tm_min,dtm->tm_sec,t[i-1].trig);
   fprintf(fp2,"%f %f \n",e_ave.elon,e_ave.elat);
   
   fclose(fp1);                                          // Close input trigger list file
   fclose(fp2);                                          // Close output event list file
   fclose(fp3);                                          // Close output station list file
   fclose(fp4);
   
   
};




