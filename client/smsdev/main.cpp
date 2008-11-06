#include "define.h"
#include "csensor_test.h"

#ifndef _WIN32
  #include <sys/time.h>   // for gettimeofday()
#endif

CQCNShMem* sm = NULL;

// add your sensor class below, i.e. CSensorTest
int main(int argc, char** argv)
{
    const float RUN_SECONDS = 10.0f; // how many seconds to run -- max is 200 seconds (or bump up MAXI in define.h to RUN_SECONDS / DT )

    int iRetVal = 0, iErrCnt = 0;
    sm = new CQCNShMem();

    CSensorTest sms;

    if (sms.detect()) {
       double tstart = dtime(), tend;
       // initialize timers
       sm->t0active = tstart; // use the function in boinc/lib
       sm->t0check = sm->t0active + sm->dt;

       // assuming we're at 50Hz, run 500 times for 10 seconds of output, note array only holds 10,000 so don't go past that!
       for (sm->lOffset = 0; sm->lOffset < (int) (RUN_SECONDS / DT); sm->lOffset++) {
           if (!sms.mean_xyz(true)) iErrCnt++;   // pass in true for verbose output, false for silent
       }

       tend = dtime();
       fprintf(stdout, "%f seconds of samples read from %f to %f in %f seconds real time -- error of %3.3f %c\n"
               "%d Timing Errors Encountered\n",
	  RUN_SECONDS, tstart, tend, tend - tstart, ((RUN_SECONDS - (tend - tstart)) / RUN_SECONDS) * 100.0f, '%', iErrCnt);
    }
    else {
       fprintf(stdout, "No sensor detected!\n");
       iRetVal = 1;
    }

    if (sm) {
        delete sm;
        sm = NULL;
    }

    return iRetVal;

}

// handle function from boinc/lib/util.C to get epoch time as a double
// return time of day (seconds since 1970) as a double
//
double dtime() {
#ifdef _WIN32
	LARGE_INTEGER time;
    FILETIME sysTime;
    double t;
    GetSystemTimeAsFileTime(&sysTime);
    time.LowPart = sysTime.dwLowDateTime;
    time.HighPart = sysTime.dwHighDateTime;  // Time is in 100 ns units
    t = (double)time.QuadPart;    // Convert to 1 s units
    t /= TEN_MILLION;                /* In seconds */
    t -= EPOCHFILETIME_SEC;     /* Offset to the Epoch time */
    return t;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec + (tv.tv_usec/1.e6);
#endif
}

