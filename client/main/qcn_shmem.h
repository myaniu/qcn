#ifndef _CQCNSHMEM_H_
#define _CQCNSHMEM_H_
// Carl Christensen, copyright 2007 Stanford University
// this class will be instantiated in shared memory, for updating of sensor data and sharing with an external OpenGL graphics app

#include "boinc_api.h"
#include "define.h"
#include "cserialize.h"
#ifndef _WIN32
#include <unistd.h>
#endif

using std::vector;

class CTriggerInfo 
{ // info needed to describe a trigger
  public:
    CTriggerInfo();
    long lOffsetStart;  // the array offset the trigger occurred
    long lOffsetEnd;    // the array offset the trigger occurred
    int iWUEvent;      // the number of this trigger within the workunit (i.e. 1 through 9999)
    int iLevel;        // what level of file I/O this trigger has had (see the enum in define.h, i.e. TRIGGER_IMMEDIATE, TRIGGER_ALL)
    char strFile[_MAX_PATH]; // the filename associated with this trigger
    char strChecksum[SIZEOF_CHECKSUM]; // the checksum for the strFile filename associated with this trigger
    bool bSent;        // whether the trickle has been sent for this trigger
    bool bDemo;        // flag if this is a real reportable event or just a per-minute trickle trigger in Demo mode
    bool bInteractive; // this trigger happened in interactive mode, so don't really trickle
    bool bRemove;      // flag that it's safe to remove this trigger, all processed
    void clear();
};

class CQCNShMem : public CSerialize
{
  public:
    CQCNShMem();
    ~CQCNShMem();

    bool setTriggerLock();
    bool releaseTriggerLock(); // mutex control for trigger vars such as lTriggerNew, lTriggerFile etc
    void resetSampleClock();   // sets t0check & t0actual to proper values before start calling sensor mean_xyz function

    void clear(bool bAll = false);
    void resetMinMax();
    void testMinMax(const float fVal, const e_maxmin eType);
    const double TimeError();
    float averageSamples();
    float averageDT();

    // data fields below

    // this section should not be reset on a clear as their values are good as long as QCN is running
    double dt; // this is the delta-time between point readings, currently .02 for the Mac sensor
    float fSignificanceFilterCutoff;

    // some important sensor stuff - iWindow is the "time window" i.e. 1 minute of data points
    long iWindow;

    bool bReadOnly;  // denotes we're not recording
    bool bDemo;      // we're in demo mode (i.e. QCNLive)

    // the next 8 vars are read from qcnprefs.xml
    int iNumTrigger;        // the total number of triggers for this workunit
    int iNumUpload;         // the total number of uploads for this workunit
    int iNumReset;          // the number of timing resets this session has had (diags which can be trickled up)
    double dMyLatitude;     // 'station' lat -- from here down gets written to SAC files in QCNLive
    double dMyLongitude;    // 'station' lng
    double dMyElevationMeter;   // 'station' elevation in meters
    int    iMyElevationFloor;   // 'station' floor (-1=basement, 0=ground floor, 1=first floor, etc)
    char strMyStation[SIZEOF_STATION_STRING]; // 'station' name

    char strPathTrigger[_MAX_PATH];  // this is the path to trigger, doesn't change after startup

    APP_INIT_DATA dataBOINC;  // useful BOINC user prefs, i.e. for graphics
    char strProjectPreferences[MAX_PROJPREFS]; // need to copy this separately as dataBOINC.project_preferences is dynamic string

    // BOINC status values -- called regularly in main::update_sharedmem() to update
    double fraction_done;
    double update_time;
    double cpu_time;
    double clock_time;
    BOINC_STATUS statusBOINC;

    /*
    double dTimeServerOffset[MAX_TIME_ARRAY];  // the time offset between client & server, +/- in seconds difference from server
    double dTimeServerTime[MAX_TIME_ARRAY];    // the (unadjusted client) time this element was retrieved
    bool   bTimeOffset;  // boolean so we easily know if we have successfully sync'd with the server
    bool   bTimeCheck;   // boolean so we check time in the main loop as needed (either auto or from reset in csensor)
    */
    double dTimeOffset;  // the time offset between client & server, +/- in seconds difference from server
    double dTimeSync;    // the (unadjusted client) time this element was retrieved
    double dTimeSyncRetry; // retry time for the time sync thread

    // end of "persistent" values

    // the following section of data is more dynamic and can be safely cleared

    // add serialization here for reading/writing
    // note: use the CSerialize class

    // readpos & writepos are for IPC
    //int readpos;
    //int writepos;

    // note the use of for vars that can be read/written by other threads/procs
    bool bWriting;
    bool bSensorFound;
    bool bTriggerLock;

    e_sensor eSensor;     // an enum of the sensor type
    e_retcode eStatus;    // basically to store ERR_ABORT so we can quit this workunit

    vector<CTriggerInfo> vectTrigger;  // a list for all the trigger info, use bTriggerLock to control access for writing

    // keep an array of the last 10 triggers, separate from the vector as this list will not get deleted (so can show in graphics etc)
    double dTriggerLastTime[MAX_TRIGGER_LAST];    // time of the latest trigger, so we don't have them less than a second away, note unadjusted wrt server time!
    long lTriggerLastOffset[MAX_TRIGGER_LAST];  // the last offset (i.e. lOffset) time of the latest trigger

/*
    char strLatLng[5][32];  // user lat/long array from proj prefs
    char strGeoIP[5][32];  // geoip lat/long
    char strWhere[5][32];  // identifier for lat/long index (i.e. "home" "work" "school" etc)
*/
    char strSensor[_MAX_PATH];
    char strMemFile[_MAX_PATH];  // a serialize file that can be read in

    // this is the dynamic data area
 
    long itl, itm, iout;

    // arrays for the timeseries data -- will be in shared memory to share between procs, i.e. main app & graphics app
    float  x0[MAXI];
    float  y0[MAXI];
    float  z0[MAXI];             /*  DECLARE TIME SERIES             */

    float  fmag[MAXI];
    float  amag[MAXI];
    float  f1;          /*  MAGNITUDE OF SIGNAL   */

    float  vari[MAXI];
    float  fsig[MAXI];
    float  sgmx;             /*  VARIANCE & STANDARD DEVIATION   */

    float  xa[MAXI];
    float  ya[MAXI];
    float  za[MAXI];             /*  AVERAGE VALUES OF x,y, &z*/

    // uses e_maxtype i.e. DX, DY, DZ, DS
    float fmin[4];
    float fmax[4];

    double t0[MAXI]; // keep track of each time?

    double t0active; // for use by the sensor polling, this value is the real system time (unadjusted from server time)
    double t0check;  // used to see what the timing error is from constant polling of sensor, it's start time + # of dt's/lOffsets
    double tstart;   // start time

    long lOffset; // current position/index into the arrays
	//bool bWrapped; // boolean to denote that we've wrapped around the array, so start point is lOffset - MAXI + 1 (rather than 1)

    double dTimeError; // a percentage error between the real time and the t0 time 
    long lSampleSize; // the sample size for this reading at lOffset
	unsigned long long ullSampleCount;  // used to compute a running average of sample count
	unsigned long long ullSampleTotal;
    float fRealDT;

    double dTimeInteractive; // time that interactive use started, should bypass triggers if this is recent
};

#endif
