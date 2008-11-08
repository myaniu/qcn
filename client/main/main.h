#ifndef _QCN_MAIN_H_
#define _QCN_MAIN_H_
/*
 *  main.h -- main header file for the Quake Catcher Network
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 */

#include "define.h"
#include "qcn_util.h"
#include "qcn_shmem.h"
#include "sac.h"
#include "qcn_thread.h"
#include "qcn_thread_time.h"
#include "qcn_thread_sensor.h"
//#include "checkboincstatus.h"
#include "qcn_signal.h"

#include "execproc.h"
//#include "md5.h"

#ifdef _WIN32
  #include <windows.h>
  #include "config.h.win" // this is from the qcn/ directory
  #include "boinc_win.h"
#else // Mac & Linux
  #include "config.h" // this is from the qcn/ directory
  #include <cstdio>
  #include <cctype>
  #include <ctime>
  #include <cstring>
  #include <cstdlib>
  #include <csignal>
  #include <unistd.h>
#endif

// setup the shared memory segment (a class that resides in shared mem) for the QCN (& graphics) app
#include <list>
using std::list;
using std::vector;

#include "csensor.h"

// Mac specific libraries
#ifdef __APPLE_CC__    
  #include <IOKit/IOKitLib.h>
  #include <CoreFoundation/CoreFoundation.h>
#endif

// BOINC & esp boinc graphics stuff:
#include "boinc_api.h"  // default to a BOINC build (standalone or live) if not using the 2-D X11 graphics
#include "diagnostics.h"
#include "str_util.h"
#include "filesys.h"
#include "parse.h"
#include "mfile.h"
#include "graphics2.h"
#include "util.h"  // this is boinc util

#ifdef _ZLIB
#include "qcn_gzip.h"
#endif
#include "trickleup.h"
#include "trickledown.h"

using std::string;

#ifdef QCN_USB
#include "qcn_shmem_usb.h"
extern CQCNUSBSensor* volatile sm;
extern CQCNUSBState* volatile smState;
#else
extern CQCNShMem* volatile sm;
#endif

#ifndef _WIN32
extern char **environ;   // environment for Mac & Linux to call execve (in execproc.cpp)
#endif

struct STriggerInfo
{ // info needed to describe a trigger
  public:
    STriggerInfo() { clear(); };
    void clear() { memset(this, 0x00, sizeof(STriggerInfo)); };

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
};

namespace qcn_main  {

extern int qcn_main(int argc, char **argv);
void signal_handler(int iSignal);
extern void parseArgs(int argc, char*argv[]); // startup arguments such as --demo and --dump are found here
extern void doMainQuit(const bool& bFinish = false, const int& errcode = 0);

#ifndef QCN_USB
bool CheckTriggers(bool bForce);
bool CheckTriggerFile(struct STriggerInfo* ti, bool bForce);      // write the trickle file for the latest trigger
bool CheckTriggerTrickle(struct STriggerInfo* ti);   // send a trickle when a trigger is hit
#endif

int do_checkpoint(MFILE& mf, int nchars);
void update_sharedmem();

// global data within the namespace
// some globals to extern within the qcn_main namespace
extern CSensor* volatile g_psms; // pointer to the CSensor - so we can closePort() on a signal error
extern e_endian volatile g_endian;

#ifndef QCNDEMO // qcndemo needs the g_iStop internally
extern int volatile g_iStop;
#endif
extern bool volatile g_bFinished;
extern bool volatile g_bSuspended;

extern CQCNThread* volatile g_threadSensor;
extern CQCNThread* volatile g_threadTime;
extern CQCNThread* volatile g_threadMain;

extern int  volatile g_iQCNReturn; // qcn return code

#ifndef GRAPHICS_PROGRAM
  extern const float g_cfTimeWindow;  // time window in seconds
  extern const double g_DT;    // delta t sampling interval
#endif

  extern float g_fPerturb[MAX_PERTURB];  // define 10 vars we will perturb

  extern bool g_bDemo;
  extern bool g_bReadOnly;

  extern char g_strPathTrigger[_MAX_PATH];  // this is the path to trigger, doesn't change after startup

  extern double g_dTimeOffset;  // the time offset between client & server, +/- in seconds difference from server
  extern double g_dTimeSync;    // the (unadjusted client) time this element was retrieved
  extern double g_dTimeSyncRetry; // retry time for the time sync thread

  extern vector<struct STriggerInfo> g_vectTrigger;  // a list for all the trigger info, use bTriggerLock to control access for writing

} // namespace qcn_main

#endif //_MAIN_H_
