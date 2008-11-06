#ifndef _DEFINE_H_
#define _DEFINE_H_

#ifndef __APPLE_CC__
   #define __USE_XOPEN_EXTENDED
#endif

#include <stdlib.h>
#include <vector>
#include <string>

using std::string;
using std::vector;

extern double dtime();

const long MAXI = 10001L;  // number of x/y/z/t points in the shared memory array

// for the sensor try/catch
const int EXCEPTION_SHUTDOWN = 1;
const float DEFAULT_SIGCUTOFF = 3.0f;  // significance cutoff threshold for triggers
const float cfTimeWindow = 60.0f;  // time window in seconds
const double DT = 0.02f;          // delta t sampling interval, i.e. target time between points
const double DT_SLOW = 0.10f;      // delta t sampling interval for slow/troublesome machines (i.e. can't keep up at <3 samples per dt=.02)

// emulates the full-blown QCN Shared Memory struct
class CQCNShMem
{
  public:
    CQCNShMem() 
    { 
         memset(this, 0x00, sizeof(CQCNShMem)); 
         dt = DT; 
         fSignificanceFilterCutoff = DEFAULT_SIGCUTOFF;
         t0check = dtime();
         t0active = dtime();
    };
    ~CQCNShMem() {};

    double dt; // this is the delta-time between point readings, currently .02 for the Mac sensor, not needed to be volatile
    float fSignificanceFilterCutoff;
    int iNumReset; 

    // arrays for the timeseries data -- will be in shared memory to share between procs, i.e. main app & graphics app
    float  x0[MAXI];
    float  y0[MAXI];
    float  z0[MAXI];             /*  DECLARE TIME SERIES             */

    double t0[MAXI]; // keep track of each time?

    double t0active; // for use by the sensor polling, this value is the real system time (unadjusted from server time)
    double t0check;  // used to see what the timing error is from constant polling of sensor, it's start time + # of dt's/lOffsets
    double tstart;   // start time

    long lOffset; // current position/index into the arrays

    double dTimeError; // a percentage error between the real time and the t0 time 
    long lSampleSize; // the sample size for this reading at lOffset
        unsigned long long ullSampleCount;  // used to compute a running average of sample count
        unsigned long long ullSampleTotal;
    float fRealDT;

    double dTimeInteractive; // time that interactive use started, should bypass triggers if this is recent
};

#ifndef _chdir
  #define _chdir chdir
#endif

// first define some useful match functions
#define QCN_SQR(A)   ((A)*(A))
#define QCN_ROUND(A) (((A)>=0) ? (long)((A) + .5) : (long)((A) - .5))

#ifdef _WIN32
  #include <windows.h>

   #define EPOCHFILETIME_SEC (11644473600.)
   #define TEN_MILLION 10000000.

// make a usleep function for Windows Sleep (which is in milliseconds)
  #define usleep(A) ::Sleep((DWORD) ((long) A/1000L))
#endif

// common QCN defines & return codes can go here

enum e_os      { OS_MAC_INTEL, OS_MAC_PPC, OS_WINDOWS, OS_LINUX };
enum e_retcode { ERR_NO_SENSOR = 99, ERR_SHMEM, ERR_TIME_CHANGE, ERR_INIT, ERR_CRASH, ERR_DIR_TRIGGER, ERR_ABORT, ERR_NUM_RESET };

// enum of quake types for the globe list
enum e_quake   { QUAKE_CURRENT, QUAKE_WORLD85, QUAKE_DEADLIEST };

// enumerate the various sensor types, we can trickle this int back for easier comparisons
enum e_sensor  { SENSOR_NOTFOUND = 0,
                 SENSOR_MAC_PPC_TYPE1, 
                 SENSOR_MAC_PPC_TYPE2, 
                 SENSOR_MAC_PPC_TYPE3, 
                 SENSOR_MAC_INTEL, 
                 SENSOR_WIN_THINKPAD,
		 SENSOR_WIN_HP,
		 SENSOR_USB,
		 SENSOR_TEST
               };
 
// USB id's
#define USB_MOUSEWARRIOR 0x1114
#define USB_JOYWARRIOR 0x1113
#define USB_VENDOR 0x07C0

// ERR_NO_SENSOR   = no sensor was detected in the CSensor::detect() call
// ERR_SHMEM       = could not create/setup shared memory block for the CQCNShMem class
// ERR_TIME_CHANGE = err in the clocks, user may have changed the clock in the middle of running QCN?
// ERR_INIT        = call to boinc_init failed -- possibly out of memory?
// ERR_CRASH       = an unspecified catch-all for a QCN crash

// the number of seconds lag before we consider it an error an reset the sensor
#define TIME_ERROR_SECONDS .90f         // an error of more than this (2000%!) is critical and we should reset timer
#define SLUGGISH_MACHINE_THRESHOLD 30   // if exceededs this many time resets in a session, bump up DT to DT_SLOW

#define THREAD_EXIT -1
#define QCN_SHMEM "quake"
#ifndef FALSE
  #define FALSE 0
#endif
#ifndef TRUE
  #define TRUE  1
#endif
#define WHERE_MAIN 0
#define WHERE_PREFS 10

#define SAC_NUMHEADBYTES 632 // floats and longs are 440 rest are characters 
#define SAC_VERSION 6L

// this is used for the boinc_sleep interval for a sensor, it's basically dt/SAMPLING_FREQUENCY each sleep interval
// which seems to yield 5-10 samples per dt
// this is DT/SAMPLING_FREQUENCY * 1e6
#define DT_MICROSECOND_SAMPLE 2000
#define SAMPLING_FREQUENCY  10.0f
#define SAMPLE_SIZE 10

#endif

