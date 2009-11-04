#ifndef _CQCNSHMEM_H_
#define _CQCNSHMEM_H_
// Carl Christensen, copyright 2007 Stanford University
// this class will be instantiated in shared memory, for updating of sensor data and sharing with an external OpenGL graphics app
// note that some variables that are important are global in qcn_main namespace, as they are not necessary for graphics, so don't
// need to be here

#include "boinc_api.h"
#include "define.h"
#include "cserialize.h"
#ifndef _WIN32
#include <unistd.h>
#endif

class CQCNShMem : public CSerialize
{
  public:
    CQCNShMem();
    ~CQCNShMem();

    bool setTriggerLock();
    bool releaseTriggerLock(); // mutex control for trigger vars such as lTriggerNew, lTriggerFile etc
    void resetSampleClock();   // sets t0check & t0actual to proper values before start calling sensor mean_xyz function

    void clear(bool bAll = false);
    //void resetMinMax();
    //void testMinMax(const float fVal, const e_maxmin eType);
    const double TimeError();
    float averageSamples();
    float averageDT();

    // data fields below

    // this section should not be reset on a clear as their values are good as long as QCN is running
    double dt; // this is the delta-time between point readings, currently .02 for the Mac sensor

    int iContinuousCounter; // keeps count of how many times (without reset) we've been through the array (i.e. 1.5 hours
    //char strUploadLogical[_MAX_PATH_LOGICAL];
    //char strUploadResolve[_MAX_PATH];

    // some important sensor stuff - iWindow is the "time window" i.e. 1 minute of data points
    long iWindow;
    char strPathImage[_MAX_PATH];  // path to the images

    // the next 8 vars are read from qcnprefs.xml
    int iNumTrigger;        // the total number of triggers for this workunit
    int iNumUpload;         // the total number of uploads for this workunit
    int iNumReset;          // the number of timing resets this session has had (diags which can be trickled up)
    double dMyLatitude;     // 'station' lat -- from here down gets written to SAC files in QCNLive
    double dMyLongitude;    // 'station' lng
    double dMyElevationMeter;   // 'station' elevation in meters
    int    iMyElevationFloor;   // 'station' floor (-1=basement, 0=ground floor, 1=first floor, etc)
	int iMySensor;   // user pref for preferred sensor
    char strMyStation[SIZEOF_STATION_STRING]; // 'station' name

	double clock_time;
	double cpu_time;
	double update_time;
	double fraction_done;

	BOINC_STATUS statusBOINC;
    APP_INIT_DATA dataBOINC;
	char strProjectPreferences[SIZEOF_PROJECT_PREFERENCES];

        double dTimeStart;  // the start time to the nearest second, used for graphics timer tick marks primarily
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

    // keep an array of the last 10 triggers, separate from the vector as this list will not get deleted (so can show in graphics etc)
    double dTriggerLastTime[MAX_TRIGGER_LAST];    // time of the latest trigger, so we don't have them less than a second away, note unadjusted wrt server time!
    long lTriggerLastOffset[MAX_TRIGGER_LAST];  // the last offset (i.e. lOffset) time of the latest trigger
	//int iTriggerLastElement;

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
    //float fmin[4];
    //float fmax[4];

    double t0[MAXI]; // keep track of each time?

    double t0active; // for use by the sensor polling, this value is the real system time (unadjusted from server time)
    double t0check;  // used to see what the timing error is from constant polling of sensor, it's start time + # of dt's/lOffsets

    long lOffset; // current position/index into the arrays

    double dTimeError; // a percentage error between the real time and the t0 time 
    long lSampleSize; // the sample size for this reading at lOffset
    unsigned long long ullSampleCount;  // used to compute a running average of sample count
    unsigned long long ullSampleTotal;
    float fRealDT;

    //double dTimeInteractive; // time that interactive use started, should bypass triggers if this is recent
};

#endif
