#include "main.h"
#include "qcn_thread.h"
#include "qcn_thread_time.h"
#include "str_util.h"  // boinc string utils e.g. strlcpy

//forward declarations
bool parseReplyToTimeOffset(const char* strReply, const double dTimeLocal);
//void addTimeSync(const double& dTimeLocal, const double& dTimeOffset);

// CMC -- the library for ntpd/libntp is pretty crap as far as conflicts with other libs & compiler flags
//   the ntpdatemain (as in client/util/qcn_ntpdate.c) hangs for awhile, so it seems actually safer
//   and faster to just call the external exec as before

// #define _USE_NTPDATE_EXEC_  // this is defined in Makefile.am to denote we're using an external ntpdate program

#ifndef _USE_NTPDATE_EXEC_
// forward declaration for the ntpdatemain proc
extern "C"
int
ntpdatemain (
	double volatile *pdTimeOffset,
	const int volatile *pbStop,
        int argc,
	char *argv[]
);
#endif

#ifdef _WIN32  // Win fn ptr
// UINT WINAPI QCNThreadTime(LPVOID /* lpParameter */)
DWORD WINAPI QCNThreadTime(LPVOID)
#else  // Mac & Linux thread function ptr
void* QCNThreadTime(void*)
#endif
{ 
    if (qcn_main::g_threadTime) qcn_main::g_threadTime->SetRunning(true);  // mark the entry point of the thread
    double dTimeStart = dtime();
    fprintf(stdout, "Server time synchronization started at   %f\n", dTimeStart);
    fflush(stdout);

    // CMC changed so we don't need the external executable anymore...
#ifdef _USE_NTPDATE_EXEC_

    // this just calls the ntpdate executable and then gets & sets the sm->dOffset time stuff
    bool bRet;
    const int iLenReply = 1024;  // that should be plenty for the return value, it's probably more like 100 chars across two lines
    char* strReply = new char[iLenReply];
    memset(strReply, 0x00, iLenReply);
    char strExec[_MAX_PATH];  // set exec & boinc resolved filename for ntpdate
  //#define NTPDATE_ARGS { "ntpdatemainthread", "-p", "8", "-t", "20", "-u", "-b", "-q", "qcn-upl.stanford.edu" }
  //#define NTPDATE_ARGC 9
  //  const char* ntpdate_argv[NTPDATE_ARGC] = NTPDATE_ARGS;

//#ifdef QCNLIVE
   #ifdef _WIN32
      sprintf(strExec, "%s_%s", NTPDATE_EXEC_VERSION, "windows_intelx86.exe");
   #else // note here you need the ./ since it's not a full path, we're already in the working directory
      #ifdef __APPLE_CC__  // apple, now check endian-ness
         sprintf(strExec, "%s%s_%s-apple-darwin", 
// if using the GUI (qcnwx) need to prepend ./ as we are already in the working directory
#ifdef QCNLIVE
                 "./",
#else
                 "",
#endif
                 NTPDATE_EXEC_VERSION, 
                 (qcn_main::g_endian == ENDIAN_LITTLE ? "i686" : "powerpc")
         );
      #else // Linux
         sprintf(strExec, "./%s_i686-pc-linux-gnu", NTPDATE_EXEC_VERSION);
      #endif
   #endif

    if (!sm || qcn_main::g_iStop || qcn_main::g_threadTime->IsSuspended()) {
        goto done; // try a graceful exit if shutting down
    }

    bRet = execproc::execute_program(strExec, NTPDATE_ARGS,
#ifdef QCNLIVE
		NULL,
#else
		(const char*) qcn_util::dataBOINC.project_dir, 
#endif
		strReply, iLenReply);

    double dTimeNow = dtime();  // CMC note -- the exit of the process above should be the point to get the time?  any delay in the above proc?
    // perhaps it's fine as we just need a local reference time, the offset in the reply is the exact time difference around this time
    sm->setTriggerLock();
    if (bRet) {  // this sets the sm->dtimeSync etc
       bRet = parseReplyToTimeOffset(strReply, dTimeNow);
    } 
    if (bRet) { // successful, set it for 15 minutes
       fprintf(stdout, "Synchronized server time at local time = %f   offset = %f   elapsed time = %f\n",
           qcn_main::g_dTimeSync, qcn_main::g_dTimeOffset, qcn_main::g_dTimeSync - dTimeStart);
       fflush(stdout);
       qcn_main::g_dTimeSyncRetry = dTimeNow + 900.0f;  // 15 minutes until the next sync
    }
    else { // try again in two minutes
       if (qcn_main::g_iStop) {
          fprintf(stderr, "Time synchronization failed local time = %f, stop request received - elapsed time = %f\n", dTimeNow, dTimeNow - dTimeStart);
          fprintf(stdout, "Time synchronization failed local time = %f, stop request received - elapsed time = %f\n", dTimeNow, dTimeNow - dTimeStart);
       }
       else {
          fprintf(stderr, "Time synchronization failed local time = %f, will retry in 3 minutes - elapsed time = %f\n", dTimeNow, dTimeNow - dTimeStart);
          fprintf(stdout, "Time synchronization failed local time = %f, will retry in 3 minutes - elapsed time = %f\n", dTimeNow, dTimeNow - dTimeStart);
       }
       fflush(stdout);
       fflush(stderr);
       qcn_main::g_dTimeSyncRetry = dTimeNow + 180.0f;  // set next sync try in 3 minutes if still running!
       qcn_main::g_dTimeSync = 0.0f;
       qcn_main::g_dTimeOffset = 0.0f;
    } 
    sm->releaseTriggerLock();

#else // ndef _USE_NTPDATE_EXEC_ --- just use the ntpdatemain() linked in here
    char* strReply = NULL; 
    const char* ntpdateargs[NTPDATE_ARGC] = NTPDATE_ARGS; 
    double volatile myTimeOffset = 0.0f;
    if (!sm || qcn_main::g_iStop || qcn_main::g_threadTime->IsSuspended()) {
        qcn_main::g_dTimeSyncRetry = dtime() + 120.0f;  // set next sync try in 2 minute if still running!
        goto done; // try a graceful exit if shutting down
    }

    // this directly calls the function ntpdatemain which is in util/qcn_ntpdate.cpp, i.e. not an external program
    // since this can take awhile, note I'm passing in a pointer to the global stop flag, in the hopes I can read this and exit gracefully
    int iRetVal = ntpdatemain(&myTimeOffset, 
        (const int volatile *) &qcn_main::g_iStop, 
        NTPDATE_ARGC, 
        (char**) ntpdateargs
    );  
    if (!sm || qcn_main::g_iStop || qcn_main::g_threadTime->IsSuspended()) {
        if (iRetVal == -1) {
            fprintf(stderr, "Stop signal received in ntpdate at %f - leaving thread\n", dtime());
        }
        qcn_main::g_dTimeSyncRetry = dtime() + 120.0f;  // set next sync try in 2 minute if still running!
        goto done; // try a graceful exit if shutting down
    }

    sm->setTriggerLock();
    if (!iRetVal) { // good return value
       qcn_main::g_dTimeSync = dtime();
       qcn_main::g_dTimeOffset = myTimeOffset;
       //qcn_main::g_dTimeSyncRetry = qcn_main::g_dTimeSync + 900.0f;  // next sync in 15 minutes 
       qcn_main::g_dTimeSyncRetry = qcn_main::g_dTimeSync + 1.0f;  // next sync in 15 minutes 
        fprintf(stdout, "Synchronized server time at local time = %f   offset = %f   elapsed time = %f\n",
           qcn_main::g_dTimeSync, qcn_main::g_dTimeOffset, qcn_main::g_dTimeSync - dTimeNow);
           //addTimeSync(dTimeLocal, myTimeOffset);
    } 
    else { // at the very least we know to set btimeOffset to false
       // hmm, should we reset if failed, but we have a good time 10 minutes ago?
       qcn_main::g_dTimeSync = 0.0f;
       qcn_main::g_dTimeOffset = 0.0f;
       qcn_main::g_dTimeSyncRetry = dtime() + 180.0f;  // next sync try in 3 minutes 
       fprintf(stderr, "Server time synchronization failed - elapsed time = %f - retry at %.2f\n", 
           dtime() - dTimeNow, qcn_main::g_dTimeSyncRetry);
       fflush(stderr);
    }
    sm->releaseTriggerLock();
#endif

done:
    if (strReply) delete [] strReply;
    if (qcn_main::g_threadTime) qcn_main::g_threadTime->SetRunning(false);  // mark the exit point of the thread
#ifdef _WIN32
	return 0;
#else
	return (void*) 0; //bRet ? 0 : 1;
#endif
}

bool parseReplyToTimeOffset(const char* strReply, const double dTimeLocal)
{
    // parse, /if it's a valid offset then update the array of time offsets 
    // need to reverse find offset & sec, what's between is the double offset:  offset -0.649662 sec
    const char strSearch[3][20] = { 
       NTPDATE_STR_SEARCH_1,
       NTPDATE_STR_SEARCH_2,
       NTPDATE_STR_SEARCH_3
     };

    char* strFind[3] = {NULL, NULL, NULL};
    double dTimeOffset;
 
    // look for the strings which surround the offset float
    strFind[0] = (char*) strstr(strReply, strSearch[0]);
    if (strFind[0]) { // search for sec & offset from the point of the first string
       strFind[1] = (char*) strstr(strFind[0], strSearch[1]);
       strFind[2] = (char*) strstr(strFind[0], strSearch[2]);
    }

    // now these are unique strings in the reply, and should be in order, so make sure both are there (not null ptr) & strFind[1] > strFind[0]
    if (!strFind[0] || !strFind[1] || !strFind[2] || strFind[1] <= strFind[0] || strFind[2] <= strFind[0] || strFind[2] <= strFind[1]) return false;

    // OK, if here we should have a proper reply with an offset float between strFind[1] and strFind[2]
    // so we'll want to atof the bit between the end of "offset" and the beginning of " sec"
    // advance strFind[1] by strlen("offset")
    strFind[1] += strlen(strSearch[1]);
    int iLen = (int) (strFind[2] - strFind[1]);
    char* strOffset = new char[iLen+1]; 
    memset(strOffset, 0x00, sizeof(char) * iLen+1); // pad for a \0
    // copy the substr between the end of strFind[1] & strFind[2]
    strlcpy(strOffset, strFind[1], iLen);
    dTimeOffset = atof(strOffset);
    delete [] strOffset;

    // OK, if we made it here then we can set the qcn_main::g_dTimeOffset & qcn_main::g_dTimeSync
    //addTimeSync(dTimeLocal, dTimeOffset);
    qcn_main::g_dTimeOffset = dTimeOffset;
    qcn_main::g_dTimeSync = dTimeLocal;

    return true;
}

/*
void addTimeSync(const double& dTimeLocal, const double& dTimeOffset)
{
    // whew, OK, now pop this dTimeLocal & dTimeOffset into our array, loop through until find a final value open, or else just overwrite last value
    // NB:  need to shift array down if writing a last value
    int iOpen;
    for (iOpen = 0; iOpen < MAX_TIME_ARRAY; iOpen++) {
        if (sm->dTimeServerTime[iOpen] == 0.0f) break;
    }
    if (iOpen == MAX_TIME_ARRAY) { // we ran out of slots and have to shift the array down and then set this at MAX_TIME_ARRAY-1
        for (iOpen = 0; iOpen < MAX_TIME_ARRAY - 1; iOpen++) {
          sm->dTimeServerTime[iOpen]   = sm->dTimeServerTime[iOpen+1];       
          sm->dTimeServerOffset[iOpen] = sm->dTimeServerOffset[iOpen+1];       
        }
        sm->dTimeServerTime[MAX_TIME_ARRAY-1]   = dTimeLocal;       
        sm->dTimeServerOffset[MAX_TIME_ARRAY-1] = dTimeOffset;       
    }
    else { // we have an open slot at iOpen
        sm->dTimeServerTime[iOpen]   = dTimeLocal;       
        sm->dTimeServerOffset[iOpen] = dTimeOffset;       
    }
    fprintf(stdout, "Synchronized server time at local time = %f   offset = %f\n",
       dTimeLocal, dTimeOffset);
    fflush(stdout);
}
*/
