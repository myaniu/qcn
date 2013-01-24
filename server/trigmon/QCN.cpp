#include <iostream>
#include <cassert>

#include "QCN.h"
//#include "QCNTrigger.h"
#include "QCNEvent.h"
#include "QCNBounds.h"
#include "sched_msgs.h"
#include "sched_config.h"
#include "qcn_types.h"
#include "Crust2.h"
#include <sys/stat.h>
#include "sched_util.h"

using namespace std;

DB_CONN trigmem_db;

QCN::QCN(const Crust2& crust2, bool isDBActivate):
        _crust2(crust2),
        ID_USB_SENSOR_START(100),
        C_CNT_MIN(5),
        N_LONG(1000),
        T_MAX(90),
        Vs(3.4),
        Vp(6.4),
        D_MAX(200.0),
        MAX_PATH(255),
        TRIGGER_SLEEP_INTERVAL(3.0),
        TRIGGER_TIME_INTERVAL (10),
        EVENT_URL_BASE("http://qcn.stanford.edu/yildirim/earthquakes/"),
        EVENT_PATH("/var/www/qcn/earthquakes/yildirim/"),
        PHP_CMD("/usr/local/bin/php"),
        GMT_MAP_PHP("/var/www/qcn/earthquakes/inc/gmt_map.php"),
        CSHELL_CMD("/bin/csh"),
        EMAIL_PATH("/var/www/boinc/sensor/html/user/earthquake_email.php"),
        EMAIL_DIR("/var/www//boinc/sensor/html/user"),
        EMAIL_INC("/var/www//boinc/sensor/html/inc/earthquake_email.inc"),
        _dSleepInterval(-1.0),
        _iTriggerTimeInterval(-1),
        _iTriggerDeleteInterval(-1),
        _EMAIL(false),
        _mapGMT(true),
        _isUpdateQuakeOut(false),
        _isDBActivate(isDBActivate)
{
    if (_isDBActivate){
        init();
    }
}

//Destructor
QCN::~QCN()
{
    if (_isDBActivate){
       DBClose();
    }
}

/////////////////////////////////PUBLIC~ QCN() METHODS::///////////////////////////////////////////

void  
QCN::execute()
{ 
    install_stop_signal_handler(); //   /home/boinc/projects/boinc/sched/sched_util.h
    
    double dtDelete = 0.; // time to delete old triggers from memory
    double dtEnd = 0.; // sleep cutoff
    int k = 0;
    while (1) {
        double dTimeCurrent = dtime();
        dtEnd = dTimeCurrent + _dSleepInterval;

        // the qcn_trigmon program which checks against known USGS quakes takes care of deleting
        if (dTimeCurrent > dtDelete) {
            do_delete_trigmem();  // get rid of triggers every once in awhile
            dtDelete = dTimeCurrent + _iTriggerDeleteInterval;
        }

        getTriggers();  // reads the memories from the trigger memory table into a global vector
        detectEvent();     // searches the vector of triggers for matching events
        k++;
        cout << " HI " << k << endl;

        check_stop_daemons();  // checks for a quit request /home/boinc/projects/boinc/sched/sched_util.h
        dTimeCurrent = dtime();
        if (dTimeCurrent < dtEnd && (dtEnd - dTimeCurrent) < 60.0) { // sleep a bit if not less than 60 seconds
            log_messages.printf(MSG_DEBUG, "Sleeping %f seconds....\n", dtEnd - dTimeCurrent);
            boinc_sleep(dtEnd - dTimeCurrent);// call from /home/boinc/projects/boinc/lib/util.cpp
        }
    }

}


////////////////////////////////PRIVATE METHODS::///////////////////////////////////////////

// get the latest triggers from the database memory table and place into k////
int
QCN::getTriggers()
{ 
    // flush out old events every few minutes
    const int iTestTime = 120;  // number of seconds to flush out quake events so our vector doesn't get too big
    static int iCounter = 0; // use this to keep track of
    if ( iCounter++ == int(float(iTestTime / _dSleepInterval )) ) {
        iCounter = 0;
        if (_ve.size() > 0 ) {
            // flush out old events
            log_messages.printf(MSG_DEBUG, "QCN_GetTriggers: erasing old quake events...\n");
            iCounter = 0;
            // get time from database
            long int lTest = getMySQLUnixTime() - iTestTime;
            vector<QCNEvent>::iterator it = _ve.begin();
            while (it != _ve.end()) {
                if (it->e_time < lTest) { // remove this record
                    log_messages.printf(MSG_DEBUG,
                                        "QCN_GetTriggers: erased old quake id %d %d\n", it->eventid, it->qcn_quakeid);
                    it = _ve.erase(it);  // erase and set new iterator start as elements will shift
                } else { // just increment iterator
                    it++;
                }
            }
        }
    }

    // clear our local variables
    DB_QCN_TRIGGER_MEMORY qtm; //  /home/boinc/projects/qcn/server/trigger/qcn_types.h
    qtm.clear();
    QCNTrigger t;
    //t.clear();

    _vt.clear();  // clear our trigger vector since we will rebuild it from the database below
    
    //char strKeyTrigger[32];
    char strWhere[64];
    sprintf(strWhere, "WHERE time_trigger > (unix_timestamp()-%d)", _iTriggerTimeInterval);
    while (!qtm.enumerate(strWhere)) {   // /home/boinc/projects/boinc/db/db_base.cpp  has enumerate
        // just print a line out of trigger info i.e. all fields in qtm
        /* fprintf(stdout, "%d %s %d %d %s %s %f %f %f %f %f %f %f %f %f %d %d %f %d %d %d %d %d\n",
           ++iCtr, qtm.db_name, qtm.triggerid, qtm.hostid, qtm.ipaddr, qtm.result_name, qtm.time_trigger,
           qtm.time_received, qtm.time_sync, qtm.sync_offset, qtm.significance, qtm.magnitude, qtm.latitude,
           qtm.longitude, qtm.levelvalue, qtm.levelid, qtm.alignid, qtm.dt, qtm.numreset, qtm.qcn_sensorid,
           qtm.varietyid, qtm.qcn_quakeid, qtm.posted
        );*/
        /* CMC suppress the bad host stuff now as all clients should be behaving with my "throttling trickle" code by now
        int bad_host=-999;
        for (j=1; j<bh.nh;j++) {
          if (qtm.hostid==bh.hostid[j]) bad_host=j;  
         }
         */
        // Note this will get previous quake eventid info (if updated) for the trigger
        if ( qtm.qcn_sensorid >= ID_USB_SENSOR_START && qtm.numreset < 10) { //&&(bad_host<0) ) {
            // Only use triggers from usb accelerometers
            t.copyFromUSBTrigger(qtm);
            // now insert trigger
            _vt.push_back(t);  // insert into our vector of triggers
        } // USB sensors only
    }

    return int(_vt.size());


}

/* This subroutine determines if a set of triggers is correlated in time and space.
   The subroutine is used by program main, which assumes that the triggers have already been read in.
   The subroutine uses ang_dist_km, which provides the distance between two points in kilometers.
   The subroutine uses hid_yn, which determines if the primary trigger has already encountered a host ID or not.
 
   A correlated station pair occurs when the distance is < 100 km apart and the trigger time difference
   is less than the velocity divided by the station separation.*/
void
QCN::detectEvent()
{
 
    int iCtr = _vt.size() - 1;   // max index of the trigger vector
        
    if (iCtr < (C_CNT_MIN - 1)) {
        log_messages.printf(MSG_DEBUG, "QCN_DetectEvent: Not enough triggers %d\n", iCtr+1);
        return;
    }
    log_messages.printf(MSG_DEBUG, "QCN_DetectEvent: Scanning %d triggers\n", iCtr+1);


    for (int i=iCtr; i>=2; i--) {                     // For each trigger (go backwards: triggers go last to 1st, & we want 1st first)
        QCNTrigger& vi = _vt[i];
        vi.c_cnt=0;                               // Zero the count of correlated triggers
        vi.c_ind[0]=i;                            // Index the same trigger as zeroth trigger
        vi.c_hid[0] = vi.hostid;

        bool isRepeatTrigger = false;
        if ( i > 0 ){
             isRepeatTrigger = (vi.hostid != _vt[i-1].hostid);
        } else {
             isRepeatTrigger = true;
        }     


        if ( isRepeatTrigger ) {        // Do not use repeating triggers
            for (int j = i-1; j>=0; j--) {                  // For every other trigger
                QCNTrigger& vj = _vt[j];


               bool isCorrelated = false;
               if ( j > 0){
                  isCorrelated  = ( (vj.hostid != vi.hostid) && (vj.hostid != _vt[j-1].hostid) &&
                                     (fabs(vi.time_trigger - vj.time_trigger) <= T_MAX) );
               } else {   //j==0
                  isCorrelated  = ( (vj.hostid != vi.hostid) &&         true                   &&
                                     (fabs(vi.time_trigger - vj.time_trigger) <= T_MAX) );
               }
               
                if ( isCorrelated ) {
                    //For non-repeating triggers & triggers less than t_max apart
                    float dist=ang_dist_km(vi.longitude,vi.latitude,vj.longitude,vj.latitude);//Distance between triggers
                    if ( (fabs(vi.time_trigger-vj.time_trigger) < dist/Vs + 3.f) && (dist<=D_MAX) ) {
                        vi.c_cnt++;                       // Add count of correlated triggers for this trigger
                        if (vi.c_cnt > (QCNTrigger::N_SHORT-1) ) {  // Make sure we don't use more correlations than array size
                            vi.c_cnt = QCNTrigger::N_SHORT-1;     // Set count to max array size
                            break;                          // Done
                        }
                        vi.c_ind[vi.c_cnt] = j;          // index of all correlaed triggers
                        vi.c_hid[vi.c_cnt]= vi.hostid;  // Index of host ids
                    } // if abs
                } // if vt[=j]
            } // for j
        }  // if vt[i] hostid
     
    } // for i                                    // Done correlating

    /* Now we correlate triggers that are currently correlated with triggers that are correlated with the initial trigger, but not
       correlated with the initial trigger itself */
    bool bInsertEvent = false; // if true then we should insert this into qcn_quake table
    for (int i =iCtr; i>1; i--) {                     // For each trigger
        QCNTrigger& vi = _vt[i];
        if (vi.c_cnt > C_CNT_MIN) {               // If more than 4 correlated triggers, possible regional event
            bool bEventFound = false;
            QCNEvent e;
            int kIndx = -1;
            for (int k = 0; k < int(_ve.size()); k++) {    // go through all the events for a match, if not make a new event
                if ( ( vi.time_trigger <= T_MAX+_ve[k].e_time) &&
                        (fabs(vi.latitude-_ve[k].latitude)<=3.) ) {
                    // this is a match by time & location (just using lat though?)
                    bEventFound = true;
                    e = _ve[k];  // should have a qcn_quakeid here as it should have been inserted in the original pass
                    kIndx = k;
                    break;
                }
            } // for k

            if (!bEventFound) {                       // need to add a new quake event
                e.eventid = (long) vi.time_trigger; //g_idEvent++; // If new Time or location, then new event
                e.e_cnt=0;                            // Zero trigger count for event count for if new location/time
                e.dirty = true;
            }

            if ( !bEventFound && (vi.time_trigger - e.e_t_detect > 4.) ) {
                // Only do new event location ... if more triggers for same event (or new event - no prior triggers)
                bool isEvent = eventLocate(bEventFound, e, i);
                bool isContinue = (isEvent && e.e_r2 >= 0.5);
                if (isContinue){
                   e.e_cnt = vi.c_cnt;
                   bInsertEvent = (bool) (e.qcn_quakeid == 0); // if 0 then we don't have a quake id yet hence need to insert
                   estimateMagnitude(e, i);                // Estimate the magnitude of the earthquake
                   intensityMap(bInsertEvent, e, i);       // This generates the intensity map -- also where quake info updated in the qcn_quake & qcn_trigger tables    }
                }
            }

            if (bEventFound) {                            // replace element
                _ve[kIndx] = e;
            } else {
                _ve.push_back(e);
            }
        } // if vt[i]
    } // for i


}



////////////////PRIVATE METHODS//////////////////////////////
void
QCN::init()
{
    int retval = DBOpen();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
    }
    assert(retval == 0); //make sure retval is true 


}


//boinc_db and config are defined in Carl's code or Boinc library find them out
int
QCN::DBOpen()
{

    int retval = boinc_db.open( config.db_name, config.db_host,
                                config.db_user, config.db_passwd);
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
    retval = trigmem_db.set_isolation_level(REPEATABLE_READ);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
                            "trigmem_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
                           );
    }
    return retval;


}

void
QCN::DBClose()
{
    log_messages.printf(MSG_DEBUG, "Closing databases.\n");
    boinc_db.close();
    trigmem_db.close();
}



long int
QCN::getMySQLUnixTime()
{
    long int lTime = 0L;
    int numRows = 0;
    int retval = trigmem_db.do_query("SELECT unix_timestamp()");
    if (retval) { // big error, should probably quit as may have lost database connection
        log_messages.printf(MSG_CRITICAL,
                            "getMySQLUnixTime() error: %s - %s\n", "Query Error", boincerror(retval)
                           );
        return lTime;
    }

    MYSQL_ROW row;
    MYSQL_RES* rp;
    rp = mysql_store_result(trigmem_db.mysql);
    while (rp && (row = mysql_fetch_row(rp))) {
        numRows++;
        lTime = atol(row[0]);
    }
    mysql_free_result(rp);

    return lTime;
}



/* This function returns the distance (in km) between two points given as
   (lon1,lat1) & (lon2,lat2).This function written by Jesse Lawrence (April 2010) - 
    Contact: jflawrence@stanford.edu   
*/
inline float
QCN::ang_dist_km(const float lon1, const float lat1, const float lon2, const float lat2)
{
    float pi = atan(1.f)*4.f;                              // set pi = 3.14....
    float d2r = pi/180.f;                                  // Degree to radian Conversion
    float dlat = lat2-lat1;                                // Latitudinal distance (in degrees)
    float dlon = (lon2-lon1)*cos((lat2+lat1)/2.f*d2r);     // Longitudeinal distance (corrected by mean latitude)
    float delt = 111.19*sqrt(dlat*dlat+dlon*dlon);         // Distance in km = degrees*111.19

    return delt;                                           // Return with distance
};



// CMC the next line was handled above in QCN_Detect and basically we can use bEventFound if this event was already used
//if ( ( vt[j].time_trigger > T_max+e.e_time) || (abs(t[i].latitude-e.latitude)>3.) ) e.eventid=0;
// If new Time or location, then new event
bool
QCN::eventLocate(const bool bEventFound, QCNEvent& e, const int ciOff)
{
    log_messages.printf(MSG_DEBUG,
                        "Locate possible event ID %d, Count %d \n", e.eventid, _vt[ciOff].c_cnt);

    float  dn=0.; float dp=0.;                             // Distances to the nth and pth trigger location

    double t_min = _vt[ciOff].time_trigger;                              // Minimum trigger time

    int j_min = ciOff;                               // Assume first trigger is i'th trigger (not necessarily the case and will be checked)
    QCNTrigger vciOff = _vt[ciOff];
    for (int j = 0; j <= vciOff.c_cnt; j++) {       // Find earliest trigger
        const int n = vciOff.c_ind[j];                       // index of correlated series
        if (t_min < _vt[n].time_trigger) {              // If earlier trigger, then use that as first time
            t_min = _vt[n].time_trigger;                //
            j_min = n;                                 //
        }
    }       

    e.longitude = _vt[j_min].longitude;               // Start with assumption of quake at/near earliest trigger
    e.latitude  = _vt[j_min].latitude;
    e.depth = 33.;                                       // Start with default Moho depth eq.


    float dp_x = 0.0;
    int    n_iter=3;                                       // Number of grid search iterations
    vector<float> v(2,0.0);
    
    float  width = 4.f; float zrange=150.f;                // Lateral and vertical grid size
    float  dx = 0.1f; float dz = 10.f;                     // Lateral and vertical grid step size
    for (int j = 1; j<=n_iter; j++) {                          // For each iteration
        QCNBounds g(e.longitude, e.latitude, e.depth, width, dx, zrange, dz);
        float ls_mf_min = 9999999999.f;                       // Set obserdly high misfit minimum
        for (int h = 0; h < g.nz; h++) {                           // For each vertical grid step
            dp_x = g.z_min + g.dz * float(h);               // Calculate depth
            //Get depth-averaged velocity for location from CRUST2.0
            _crust2.getMeanVel(dp_x, _vt[j_min].longitude, _vt[j_min].latitude,v);
            for (int k = 0; k < g.nx; k++) {                          // For each x node
                const float ln_x = g.x_min + g.dx * float(k);              // Longitude of grid point
                for (int l = 0; l < g.ny; l++) {                         // For each y node
                    const float lt_x = g.y_min + g.dy * float(l);             // Latitude of grid point
                    double ls_mf = 0.;
                    double ls_ct = 0.;                            // Zero least-squares misfit & count
                    // Compare times and distance difference between each station pair
                    for (int m = 0; m < vciOff.c_cnt; m++) {                  // For each trigger
                        const int n = vciOff.c_ind[m];                        // Index of correlated trigger
                        //cout << " m  = " << m  << "    n = " << n << endl;
                        // Horizontal distance between node and host/station
                        float dn = ang_dist_km(ln_x, lt_x, _vt[n].longitude, _vt[n].latitude);
                        // Actual distance between event & station/host
                        dn = sqrt(dn*dn + dp_x*dp_x);
                        // Compare with each other trigger
                        for (int o = m+1; o <= vciOff.c_cnt; o++) {
                            // Index of correlated trigger
                            const int p  = vciOff.c_ind[o];
                            dp = ang_dist_km(ln_x, lt_x, _vt[p].longitude, _vt[p].latitude);   // Distance between triggers
                            dp = sqrt(dp*dp + dp_x*dp_x);                    // Distance corrected for depth
                            // Travel time difference between two stations (dt), dt_min: minimum for P & S wave
                            double dt_min = 9999999.0;                              // Search For best P-wave & S-wave pairing
                            for (int q=0; q<=1; q++) {                             //    First station P(0) or S(1)
                                for (int r=0;r<=1; r++) {                            //    Second station P(0) or S(1)
                                    //Interstation time difference - theoretical time difference
                                    const double dt = (_vt[n].time_trigger - _vt[p].time_trigger) -
                                                      (dn/v[q] - dp/v[r]);
                                    if (dt*dt < dt_min*dt_min) {
                                        //Use the smallest value (assume if neerer zero then probably true)
                                        dt_min = dt;
                                    }
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
                            e.longitude=ln_x;
                            e.latitude=lt_x;
                            e.depth=dp_x;  // Save location of best fit
                        }                   // End loop over 2nd trigger of pair
                    }                      // End loop over 1st trigger of pair
                } // l                         // End loop over latitude
            } // k                          // End loop over Longitude
        }  // h (depth)                  // End loop over depth
        // Reduce grid dimensions by an order of magnitude
        dx /= 10.f;
        width /= 10.f;
        dz /= 10.f;
        zrange /= 10.f;

        // Re-compare times and distance difference between each station pair for best location and identify which phase each trigger is
        // Get depth-averaged velocity for location from CRUST2.0
        _crust2.getMeanVel(dp_x, _vt[j_min].longitude, _vt[j_min].latitude, v);
        int q_save=0;
        e.e_time=0.;
        for (int m = 0; m <= vciOff.c_cnt; m++) {                     // For each trigger
            const int n = vciOff.c_ind[m];                                   // Index of correlated trigger
            _vt[n].pors = 0;
            // Horizontal distance between node and host/station
            dn = ang_dist_km(e.longitude,e.latitude, _vt[n].longitude, _vt[n].latitude);
            dn = sqrt(dn*dn + e.depth*e.depth);                 // Actual distance between event & station/host
            for (int o = 0; o <= vciOff.c_cnt; o++) {          // Compare with each other trigger
                const int p = vciOff.c_ind[o];                          // Index of correlated trigger
                if (p!=n) {
                    dp=ang_dist_km(e.longitude,e.latitude, _vt[p].longitude, _vt[p].latitude);  // Distance between triggers
                    dp = sqrt(dp*dp + e.depth*e.depth);
                    double dt_min = 9999999.f;                                // Search For best P-wave & S-wave pairing
                    for (int q=0; q<=1; q++) {                                //    First station P(0) or S(1)
                        for (int r=0; r<=1; r++) {                              //    Second station P(0) or S(1)
                            //Interstation time difference - theoretical time difference
                            const double dt = (_vt[n].time_trigger - _vt[p].time_trigger) -
                                              (dn/v[q]-dp/v[r]);
                            if (dt*dt < dt_min*dt_min) {                     //    Use lowest time misfit to identify P or S
                                q_save = q;                                     //    Save P or S
                                dt_min = dt;                                    //    Save minimum time
                            }                                                //
                        }                                                 //
                    }                                                  //
                    _vt[n].pors += q_save;                                 // Sum most optimal values to see if it lies closer to P or S
                }
            }  // o
            // Normalize P or S to see if it is closer to P or S
            _vt[n].pors = (int) round( float(_vt[n].pors) / float(vciOff.c_cnt + 1.0));
            _vt[n].dis=dn;                    // Save distance
            e.e_time += _vt[n].time_trigger - dn/v[_vt[n].pors];          // Time of event calculated from ith trigger.
        } // m                                                     //

        // Time of earthquake:
        e.e_time = e.e_time / ((float) vciOff.c_cnt + 1.0 ); // Time of earthquake calculated by averaging time w.r.t each station normalized by number of stations
        
        if (!bEventFound) {// e.eventid<=0) {                                    // For New earthquake ID if earthquake time wasn't set
            e.eventid = ((int) e.e_time);
            log_messages.printf(MSG_DEBUG, "NEW EID: %d \n", e.eventid);
        }
    } // j  ie big outer loop ending

    //  Determine maximum dimension of array
    float ss_dist_max = -999999999.;                       // Start with large station to station maximum array dimension
    for ( int j = 0; j < vciOff.c_cnt; j++) {                       // For each trigger/station
        const int n = vciOff.c_ind[j];                                    // Index of jth station
        for (int k = j+1; k <= vciOff.c_cnt; k++) {                   // For each other trigger/station
            const int m  = vciOff.c_ind[k];                                   // Index of kth station
            dn = ang_dist_km(_vt[m].longitude, _vt[m].latitude, _vt[n].longitude, _vt[n].latitude);// Distance between stations
            if (dn > ss_dist_max)
                ss_dist_max = dn;              // Store if maximum distance
        }                                                     //
    }                                                      //

    //  Require that the earthquake-to-array distance be less than four times the dimension of the array
    if (ss_dist_max*4. < _vt[j_min].dis) {                   // If the event-to-station distance > 4 times the array dimension
        e.e_r2=-1.;                                        // Set correlation to -1 (will reject earthquake)
        log_messages.printf(MSG_DEBUG,
                            "Event poorly located: Array dimension=%f EQ Dist=%f.\n",ss_dist_max,dn);
        return false;                                               // Return so done
    } else {                                               // Otherwise output status report
        log_messages.printf(MSG_DEBUG, "Event located: %f %f\n",e.longitude,e.latitude);
    }

    //Calculate the estimated arrival time of the phase
    vector<float> t_obs(vciOff.c_cnt+1,0.0);                               // Observed phase arrival times (from triggers)
    vector<float> t_est(vciOff.c_cnt+1,0.0);                               // Estimated phase arrival times (from earthquake location estimate)
    float dt_ave=0;                                        // Average model-data time variance
    e.e_msfit = 0.;                                     // Zero misfit for model given set P & S wave times
    for (int j=0; j <= vciOff.c_cnt; j++) {                          // For each trigger
        const int n = vciOff.c_ind[j];                                  // Index of triggers
        _vt[n].time_est = e.e_time + _vt[n].dis / v[_vt[n].pors];   // Estimated time is event origin time plus distance divided by velocity
        t_obs[j]=_vt[n].time_trigger-e.e_time;                    // Store observed times
        t_est[j]=_vt[n].time_est-e.e_time;                      // Store estimated times
        dt_ave += abs(t_obs[j]-t_est[j]);                     // Sum averaged model-data time variance
        e.e_msfit += (t_obs[j]-t_est[j])*(t_obs[j]-t_est[j]);// Sum L2 misfit
    }
    e.e_msfit /= float( vciOff.c_cnt + 1.0);            // Normalize the event misfit
    e.e_msfit = sqrt(e.e_msfit);                     //
    //Require that the model variance is less than two seconds
    dt_ave /= float(vciOff.c_cnt + 1.0);                  // Normalize the data-model variance
    if (dt_ave > 2.) {                                     // If the average travel time misfit is greater than 2 seconds, reject the event detection
        e.e_r2 = -1.;                                      // Set correlation to < 0 for rejection
        log_messages.printf(MSG_DEBUG,
                            "Event rejected - average travel time misfit greater than 2 seconds \n");
        return false;                                               // Return with bad correlation
    }

    //correlate observed and estimated travel times
    e.e_r2 = correlate(t_obs, t_est, vciOff.c_cnt+1);  // Correlate observed & estimated times
    log_messages.printf(MSG_DEBUG, "Estimated times correlate at r^2= %f \n",e.e_r2);
    return true;
}

//  This function correlates two data sets (datx & daty) of length (ndat).  It returns the R^2 value (not r).
float
QCN::correlate(const vector<float>& datx, vector<float>& daty, const int ndat)
{
    float Sxy=0.;
    float Sxx=0.;
    float Syy=0.;       // Zero summed numerator and denominator values
    for (int j=0;j < ndat; j++) {                             // For each point
        Sxy = Sxy + datx[j] * daty[j];                         // Sum x*y
        Sxx = Sxx + datx[j] * datx[j];                         // Sum x*x
        Syy = Syy + daty[j] * daty[j];                         // Sum y*y
    }

    float datx_ave = average(datx,ndat);                   // Average of x series
    float daty_ave = average(daty,ndat);                   // Average of y series

    Sxy     -= float(ndat) * datx_ave * daty_ave;   // Sum x*y - x_ave*y_ave
    datx_ave = float(ndat) * datx_ave * datx_ave;   // Sum x*x - N*x_ave*x_ave
    daty_ave = float(ndat) * daty_ave * daty_ave;   // Sum y*y - N*y_ave*y_ave
    Sxx -= datx_ave;
    Syy -= daty_ave;
    float R = Sxy*Sxy / Sxx / Syy;                         // Corrlatitudeion coefficient
    return R;                                              // Return with correlation coefficient
}

inline float
QCN::stdDev(const vector<float>& dat, const int ndat, const float dat_ave)
{
    //  This function calculates the standard deviation of a data set (dat) of length (ndat) with a mean of (dat_ave)
    float dat_std = 0.;                                    // Zero the standard deviation
    for (int j = 0; j < ndat; j++) {                            // For each point
        dat_std = dat_std + pow(dat[j]-dat_ave,2);// Sum squared difference from mean
    }                                                      //
    dat_std = dat_std / float(ndat);              // Normalize squared differences
    dat_std = sqrt(dat_std);                           // Standard deviation once square root taken
    return dat_std;                                     // Return standard deviation
}

inline float
QCN::stdDev(const vector<float>& dat, const int ndat)
{    
    float avg = 0.;                                    // Zero the standard deviation
    for (int j = 0; j < ndat; j++) {                            // For each point
        avg +=  dat[j];// Sum squared difference from mean
    }  
    avg /=  float(ndat);
    
    //  This function calculates the standard deviation of a data set (dat) of length (ndat) with a mean of (dat_ave)
    float dat_std = 0.;                                    // Zero the standard deviation
    for (int j = 0; j < ndat; j++) {                            // For each point
        dat_std = dat_std + pow(dat[j]-avg,2);// Sum squared difference from mean
    }                                                      //
    dat_std = dat_std / float(ndat);              // Normalize squared differences
    dat_std = sqrt(dat_std);                           // Standard deviation once square root taken
    return dat_std;                                     // Return standard deviation
}
//  This fuction calculates the average of a data set (dat) of length (ndat).
inline float
QCN::average(const vector<float>& dat, const int ndat)
{
    float dat_ave = 0.;                      // Start with zero as average
    for (int j = 0; j<ndat; j++) {
        dat_ave = dat_ave + dat[j];
    }
    dat_ave = dat_ave / float(ndat);              // Normalize sum by quantity for average
    return dat_ave;                                        // Return with average
}

/* We need to come up with good magnitude/amplitude relationships.  There are some good ones for peak displacement v. dist.
   We need some for peak acceleration v. distance.  Note - they may vary from location to location.
   We will need to adjust this for characterization of P & S wave values.  It may also be sensor specific.
 
   This code bootstraps over the data to determine how cerntain the estimated magnitude is.  Use 3X the standard error for 99% confidence.
 
   The magnitude relation takes the form:
           M = a * LN( accel * b) + c * LN(dist) + d
   The
   This subroutine was written by Jesse F. Lawrence (jflawrence@stanford.edu).
 
*/
void
QCN::estimateMagnitude(QCNEvent& e, const int ciOff)
{
    //Constants for equation above (Need to be adjusted)
    float a=0.544f;
    float b=2.0f;
    float c=0.03085f; float d=4.28f;
    srand ( time(NULL) );                                // Set randomization kernel

    e.magnitude = 0.f;                                   // Zero magnitude
    const int c_cnt = _vt[ciOff].c_cnt;
    for (int j = 0; j <= c_cnt; j++) {              // Bootstrap once for each trigger 
       float mul_amp = 1.0; 
       const int n = _vt[ciOff].c_ind[j]; 
       e.magnitude += a * log(_vt[n].magnitude * b * mul_amp) +
                          c * log(_vt[n].dis) + d; // Sum magnitude estimate from each trigger for average estimate
    }
    e.magnitude /= ( (float) _vt[ciOff].c_cnt + 1.0);            // Normalize summed mag estimates for average magnitude estimate

  // Average magnitude
   vector<float> mag_ave(c_cnt+1,0.0);
    for (int j = 0; j <= c_cnt; j++) {              // Bootstrap once for each trigger
        for (int k = 0; k <= c_cnt; k++) {            // Select one point for each trigger
            const int kk = rand() % (c_cnt);                 // Use random trigger
            const int n = _vt[ciOff].c_ind[kk];                           // Index of correlated trigger
            assert( n >= 0 );
            float mul_amp = 1.0;                               // Multiplication factor depends on P & S waves currently set to 1
            if ( _vt[n].pors == 0 ) {                           // Use appropriate multilication factor (currently not used but will eventually)
                mul_amp = 1.0;                                    //
            } else {                                           //
                mul_amp = 1.0;                                    //
            }
            ;                                                 //
            mag_ave[j] += a * log(_vt[n].magnitude * b * mul_amp) +
                          c * log(_vt[n].dis) + d; // Sum magnitude estimate from each trigger for average estimate
        }
        mag_ave[j]/= ( (float) _vt[ciOff].c_cnt + 1.0);            // Normalize summed mag estimates for average magnitude estimate
    }
    e.e_std     = stdDev (mag_ave, c_cnt+1)*3.;// 3 sigma is the 99 % confidence (assuming a statistically large enough data set)
    e.dirty = true; // flag that we should update this event
}


// insert or update this event in the qcn_quake table
// we will also want to get the triggers and update their data in qcn_trigger as well,
// including sending a file upload request if one wasn't posted already
void
QCN::updateQuake(const bool bInsert, QCNEvent& e, const int ciOff)
{
    //if (!e.dirty) return;
    DB_QCN_QUAKE dqq;

    bool bFound = false;
    if (bInsert) {
        dqq.clear();
    } else {
        bFound = bool(dqq.lookup_id(e.qcn_quakeid) == 0); // boinc db fn 0 means found
    }

    dqq.time_utc = e.e_time;
    dqq.magnitude = e.magnitude;
    dqq.depth_km = e.depth;
    dqq.latitude = e.latitude;
    dqq.longitude = e.longitude;
    sprintf(dqq.description, "QCN Detected Event # %d : #Sens=%d Std=%f r2=%f mft=%f dt=%f",
            e.eventid, e.e_cnt, e.e_std, e.e_r2, e.e_msfit, e.e_dt_detect);
    dqq.processed = 1;
    sprintf(dqq.url,"%s/%d", EVENT_URL_BASE.c_str(), e.eventid);
    sprintf(dqq.guid,"QCNTRIGMON%d", e.eventid);

    // get/update qcn_quakeid and set e.qcn_quakeid
    // now insert
    int retval = 0;
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
    } else { // update
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
    //   note to do this in the trigmem as well as the sensor/continual database!
    const char strBaseTrigMem[] = {"UPDATE trigmem.qcn_trigger_memory SET qcn_quakeid=%d, posted=1 WHERE db_name='%s' AND triggerid=%d"};
    const char strBaseTrig[] = {"UPDATE %s.qcn_trigger SET qcn_quakeid=%d, time_filereq=unix_timestamp(), "
                                "received_file=1 WHERE id=%d AND (received_file IS NULL or received_file != 100)"
                               };
    char strSQL[MAX_PATH];

    // loop over all correlated triggers and update as appropriate & send file upload request
    for (int m = 0; m <= _vt[ciOff].c_cnt; m++) {  // For each trigger
        int n = _vt[ciOff].c_ind[m];         // Index of correlated trigger so just use vt[n]

        // first update the trigmemdb trigger table with quake id & posted (since we will request uploads)
        if (! _vt[n].posted) { // only need to update if wasn't already posted
            sprintf(strSQL, strBaseTrigMem, e.qcn_quakeid, _vt[n].db, _vt[n].triggerid);
            retval = trigmem_db.do_query(strSQL);
            log_messages.printf(MSG_DEBUG,
                                "QCN_UpdateQuake Updating trigmem trigger %s %d\n", _vt[n].db, _vt[n].triggerid
                               );
            if (retval) { // big error, should probably quit as may have lost database connection
                log_messages.printf(MSG_CRITICAL,
                                    "QCN_UpdateQuake trigmem db update error: %s - %s\n", strSQL, boincerror(retval)
                                   );
            }
        }

        // now do the appropriate main table
        sprintf(strSQL, strBaseTrig, _vt[n].db, e.qcn_quakeid, _vt[n].triggerid);
        retval = boinc_db.do_query(strSQL);
        log_messages.printf(MSG_DEBUG,
                            "QCN_UpdateQuake Updating %s trigger %d\n", _vt[n].db, _vt[n].triggerid
                           );
        if (retval) { // big error, should probably quit as may have lost database connection
            log_messages.printf(MSG_CRITICAL,
                                "QCN_UpdateQuake %s db update error: %s - %s\n", _vt[n].db, strSQL, boincerror(retval)
                               );
        }

        // do the upload file request here (if wasn't already posted)
        if (! _vt[n].posted) { // only need to update if wasn't already posted
            sendTriggerFileRequest(_vt[n].file, _vt[n].result_name, _vt[n].hostid, _vt[n].db);
            // now set memvar to be posted in casea it's reused since we don't want duplicate msg_to_host trickle down requests
            _vt[n].posted = 1;
        }
    } // end for loop over vt[n]

}

/* // use this to get the latest result name sent out for given hostid
   const char strFileReq[] = "insert into msg_to_host " 
               "(create_time,hostid,variety,handled,xml) " 
               "values (unix_timestamp(), %d, 'filelist', 0, "
               "concat('<trickle_down>\n<result_name>', r.name, '</result_name>\n<filelist>\n" 
               "%s</filelist>\n</trickle_down>\n') " 
               "from result r where r.hostid=%d"
               "  and r.sent_time=(select max(rr.sent_time) from result rr where rr.hostid=r.hostid) "};
*/
bool
QCN::sendTriggerFileRequest(const char* strFile, const char* strResult, const int hostid, const char* strDB)
{
    // since these are live triggers the following should be fine as it's the latest result hostid working on
    const char strFileReq[] = { "insert into %s.msg_to_host "
                                "(create_time,hostid,variety,handled,xml) "
                                "values (unix_timestamp(), %d, 'filelist', 0, "
                                "concat('<trickle_down>\n<result_name>', '%s', '</result_name>\n<filelist>\n', "
                                "'%s', '</filelist>\n</trickle_down>\n') )"
                              };
    char strSQL[MAX_PATH];
    sprintf(strSQL, strFileReq, strDB, hostid, strResult, strFile);
    log_messages.printf(MSG_DEBUG,
                        "sendTriggerFileRequest host %d %s %s\n", hostid, strFile, strResult);
    int retval = boinc_db.do_query(strSQL);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
                            "sendTriggerFileRequest error: %s - %s\n", strSQL, boincerror(retval));
        return false;
    }
    return true;
}




int
QCN::intensityMap(const bool bInsertEvent, QCNEvent& e, const int ciOff)
{
    //float width=5; float dx=0.05;                              // Physical dimensions of grid
    //int   nx = ((int) (width/dx)) + 1; int ny = nx;            // array dimension of grid
    //float   dist,dist_eq_nd, imap;                                          // Min distance from triger host to grid node

    float longitude = e.longitude;                                    // Copy even longitude & Latitude
    float latitude = e.latitude;
    //float x_min = longitude - width/2.f;                            // Minimum longitude of map
    //float y_min = latitude - width/2.f;                            // Min Latitude

    float pi = atan(1.)*4.;                                   // pi = 3.14....
    float latr = e.latitude*pi/180.;                           // Latitude in radians

    time_t t_eq = int(e.e_time);

    time_t t_now;
    time(&t_now);
    double t_dif = difftime(t_now,t_eq);

    // Create an event directory name
    char *edir = new char[MAX_PATH];
    char *epath = new char[MAX_PATH];
    char *epath2 = new char[MAX_PATH];
    memset(edir, 0x00, sizeof(char) * MAX_PATH);
    memset(epath, 0x00, sizeof(char) * MAX_PATH);
    memset(epath2, 0x00, sizeof(char) * MAX_PATH);


    sprintf(edir,"%d", e.eventid);
    sprintf(epath,"%s%s",EVENT_PATH.c_str(), edir);

    // Create event base directory path
    int retval = 0;
    mode_t E_MASK=0777;                                        // File Permissions for new directory to create
    struct stat st;                                            // I/O status for checking existance of directory/file
    if(stat(epath,&st) != 0) {                                 // If the path does not exist,
        retval = mkdir(epath,E_MASK);                            // Make new directory
    }


    int email = 1;                                             // email=1 if we want to email people about new event
    // Create iteration directory
    char ABC[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";                   // All caps variables
    int jIndx = -1;
    for (int j = 0; j<64; j++) {                                   // For all letters
        sprintf(epath2,"%s/%c",epath,ABC[j]); // Create full path directory name
        if (stat(epath2,&st) != 0 ) {                            // If the directory doesn't exist, creat it
            retval = mkdir(epath2,E_MASK);                     // Make new directory
            jIndx = j;
            break;                                                 // Stop here because this is where we want to put the files
        }
    }

    if (jIndx<1) {
        email=1;
    } else {
        email=0;
    }                // Set email to send if first iteration


    FILE *fp[6] = {NULL, NULL, NULL, NULL, NULL, NULL}
                  ;                                            // Output file(s)
    // Generate file names
    char* strPath[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    for (int ii = 0; ii < 6; ii++) {
        strPath[ii] = new char[MAX_PATH];
        memset(strPath[ii], 0x00, sizeof(char) * MAX_PATH);
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
                            "Error in intensity_map OUT_EVENT file creation %s\n", strPath[OUT_EVENT]);
        goto close_output_files;
    }

    e.e_t_now = int(t_now);    // Current time
    fprintf(fp[OUT_EVENT],
            "%4.4f,%4.4f,%1.4f,%1.2f,%d,%f,%d,%1.1f,%1.2f,%1.2f\n",
            longitude,latitude,e.depth,e.magnitude, _vt[ciOff].c_cnt+1,e.e_time,e.e_t_now,e.e_std,e.e_r2,e.e_msfit);// Output event location
    fclose(fp[OUT_EVENT]);                                              // Close event output file name
    fp[OUT_EVENT] = NULL;

    // Create a file with the station information
    fp[OUT_STATION] = fopen(strPath[OUT_STATION],"w");                                  // Open station output file
    if (!fp[OUT_STATION]) {
        retval = 1;
        log_messages.printf(MSG_CRITICAL,
                            "Error in intensity_map OUT_STATION file creation\n"
                           );
        goto close_output_files;
    }

    for (int k = 0; k <= _vt[ciOff].c_cnt; k++) {                          // For each correlated trigger
        const int n = _vt[ciOff].c_ind[k];                                       // Index of correlated trigger
        fprintf(fp[OUT_STATION],
                "%f,%f,%f,%d,%d,%s,%f,%d,%f,%f",
                _vt[n].longitude,_vt[n].latitude,_vt[n].magnitude,
                _vt[n].hostid, _vt[n].triggerid,_vt[n].file,
                _vt[n].time_trigger,(int) _vt[n].time_received,
                _vt[n].significance, _vt[n].dis);
        fprintf(fp[OUT_STATION],
                ",%f,%f,%f,%f,%f,%f,%f,%f \n",
                _vt[n].pgah[0],_vt[n].pgaz[0],_vt[n].pgah[1],_vt[n].pgaz[1],_vt[n].pgah[2],_vt[n].pgaz[2],_vt[n].pgah[3],_vt[n].pgaz[3]);
        // Output correlated trigger loc & magnitude
        //t[ij].triggerid,t[ij].longitude,t[ij].latitude,t[ij].time_trigger,(int) t[ij].time_received,t[ij].sig,t[ij].mag,t[ij].dis)
    }
    fclose(fp[OUT_STATION]);                                  // Close station output file name
    fp[OUT_STATION] = NULL;

    // Create contours for time rlatitudeive to identification time
    fp[OUT_CONT_TIME] = fopen(strPath[OUT_CONT_TIME],"w");    // Open time contours output file.
    fp[OUT_CONT_LABEL] = fopen(strPath[OUT_CONT_LABEL],"w");  // label file for contours
    if (!fp[OUT_CONT_LABEL] || !fp[OUT_CONT_TIME]) {
        retval = 1;
        log_messages.printf(MSG_CRITICAL,
                            "Error in intensity_map OUT_CONT_TIME/LABEL file creation %lx %lx\n", (unsigned long) fp[OUT_CONT_TIME], (unsigned long) fp[OUT_CONT_LABEL]
                           );
        goto close_output_files;
    }


    float   ln_x, lt_x;                                    // Location & intensity at grid node
    for (int j = 1; j<=9; j++) {                                  // For five distances
        float dti =  (float) (j-3) * 10.;                        // Time offset from detection time
        float dis = ((float) (j-3) * 10.+t_dif)*3.;              // Distance of time contours (10 s interval at 3km/s)
        if (dis > 0.) {                                          // Only use if distance greater than zero
            for (int k=0; k<=180; k++) {                                // for each azimuth
                float az = (float) k * 2 * pi / 180.;                  // azimuth in radians
                float dlon = sin(az)*dis/111.19/abs(cos(latr));        // Longitudinal distance
                ln_x = e.longitude + dlon;                             // New longitude
                lt_x = e.latitude + cos(az)*dis/111.19;                // New latitude
                fprintf(fp[OUT_CONT_TIME],"%f,%f\n",ln_x,lt_x);        // Output contour
            }                                                       //
            fprintf(fp[OUT_CONT_TIME],">\n");                       // Deliminator for separation between line segments
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
                            "Error in intensity_map OUT_TIME_SCATTER file creation\n");
        goto close_output_files;
    }

    for (int j = 0; j <= _vt[ciOff].c_cnt; j++) {                       // For each correlated trigger
        const int n = _vt[ciOff].c_ind[j];                                       // index of correlated triggers
        fprintf(fp[OUT_TIME_SCATTER],"%f,%f\n",_vt[n].time_trigger-e.e_time,_vt[n].time_est-e.e_time); // Print out travel times
    }
    fclose(fp[OUT_TIME_SCATTER]);                                             // Close scatter plot file
    fp[OUT_TIME_SCATTER] = NULL;

    if (_mapGMT )
        intensityMapGMT(epath2);                              // Run Scripts for plotting (GMT)

    // quake event info
    if( _isUpdateQuakeOut )
       updateQuake(bInsertEvent, e, ciOff);

    //php_event_page(e,epath2);                             // Output event Page
    if (email==1 && _EMAIL) {
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

int
QCN::intensityMapGMT(const char* epath)
{
    int retval = 0;
    char *gmtfile = new char[MAX_PATH];
    char *syscmd = new char[MAX_PATH];
    memset(gmtfile, 0x00, sizeof(char) * MAX_PATH);
    memset(syscmd, 0x00, sizeof(char) * MAX_PATH);

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
    fprintf(fpGMT,"%s %s\n", PHP_CMD.c_str(), GMT_MAP_PHP.c_str());
    fclose(fpGMT);     // Close script

    //Execute GMT script
    sprintf(syscmd,"%s %s", CSHELL_CMD.c_str(), gmtfile);
    retval = system(syscmd);

ints_map_gmt_cleanup:
    if (gmtfile) delete [] gmtfile;
    if (syscmd) delete [] syscmd;

    return retval;
}



// This subroutine creates a web page for the event.
//void
//QCN::php_event_page(const QCNEvent& e, const char* epath)
//{
   
//}

// This subroutine should email us when we detect an earthquake
void
QCN::php_event_email(const QCNEvent& e, const char* epath)
{

    FILE *fpMail  = fopen(EMAIL_PATH.c_str(), "w");                       // Open web file
    if (!fpMail) {
        log_messages.printf(MSG_CRITICAL,
                            "Error in php_event_email - could not open file %s\n", EMAIL_PATH.c_str() );
        return;  //error
    }

    log_messages.printf(MSG_DEBUG,
                        "Sending email notices.\n"
                       );

    fprintf(fpMail,"<?php\n");
    fprintf(fpMail,"chdir(\"%s\");\n", EMAIL_DIR.c_str());
    // Include email php function
    fprintf(fpMail,"require_once(\"%s\");\n", EMAIL_INC.c_str());

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

    char *sys_cmd = new char[MAX_PATH];
    memset(sys_cmd, 0x00, sizeof(char) * MAX_PATH);
    sprintf(sys_cmd,"%s %s", PHP_CMD.c_str(), EMAIL_PATH.c_str());
    int retval = system(sys_cmd);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
                            "Error on email shell script %s\n", sys_cmd
                           );
    }

    delete [] sys_cmd;
}

void
QCN::do_delete_trigmem()
{
    char strDelete[128]; 
    sprintf(strDelete,
            "DELETE FROM trigmem.qcn_trigger_memory WHERE time_trigger<(unix_timestamp() - %d"
            ") OR time_trigger>(unix_timestamp()+10.0)",
            _iTriggerDeleteInterval
           );

    int retval = trigmem_db.do_query(strDelete);

    if (retval) {
        log_messages.printf(MSG_CRITICAL,
                            "do_delete_trigmem() error: %s\n", boincerror(retval)
                           );
    } else {  // free memory
        sprintf(strDelete,
                "ALTER TABLE trigmem.qcn_trigger_memory ENGINE=MEMORY"
               );
        retval = trigmem_db.do_query(strDelete);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                                "do_delete_trigmem() error freeing memory: %s\n", boincerror(retval)
                               );
        } else {
            log_messages.printf(MSG_DEBUG,
                                "do_delete_trigmem(): Removed old triggers from memory and freed memory\n"
                               );
        }
    }
  
}


