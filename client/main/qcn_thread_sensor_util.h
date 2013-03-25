#ifndef _QCN_THREAD_SENSOR_UTIL_H_
#define _QCN_THREAD_SENSOR_UTIL_H_

#include "main.h"
#include "qcn_util.h"
#include "qcn_thread.h"
#include "qcn_thread_sensor.h"
#include "sac.h"
#include "csensor.h"
#include "boinc_zip.h"

#ifdef __APPLE_CC__
  #include "csensor_mac_laptop.h"
  #ifndef QCNLIVE
    #include "csensor_mac_usb_generic.h"
  #endif
  #include "csensor_mac_usb_jw.h"
  #include "csensor_mac_usb_jw24f14.h"
  #include "csensor_mac_usb_onavi.h"
#else
  #ifdef _WIN32
    #include "csensor_win_usb_jw.h"
    #include "csensor_win_usb_jw24f14.h"
    #include "csensor_win_usb_onavi.h"
    #include "csensor_win_laptop_thinkpad.h"
    #include "csensor_win_laptop_hp.h"
    #ifndef QCNLIVE
      //#include "csensor_win_usb_generic.h"
    #endif
  #else // LINUX
    #include "csensor_linux_usb_jw.h"
    #include "csensor_linux_usb_jw24f14.h"
    #include "csensor_linux_usb_onavi.h"
  #endif // Win or Linux
#endif // Apple

// all platforms try the phidgets 1056 and/or 1044
#include "csensor_usb_phidgets.h"

/* MN deprecated as of 01/2013
// all platforms except win64 get MotionNodeAccel USB support!
#if !defined(_WIN64) && !defined(__LP64__) && !defined(_LP64)
#include "csensor_usb_motionnodeaccel.h"
#endif
*/

// global externs defined in qcn_thread_sensor.cpp
// some globals, mainly if bDemo is true so we can have continual trigger output
extern bool   g_bSensorTrickle;  // set to true if no sensor found, so we can trickle out this info (disabled now)
extern bool   g_bRecordState; // internal sensor thread flag that we are recording
extern double g_dStartDemoTime; 
// after the first demo trigger write (10 minutes),
// set it to be an even 10 minute interval start time
extern bool   g_bStartDemoEvenTime;    
extern long   g_lDemoOffsetStart; 
extern long   g_lDemoOffsetEnd; 
extern float  g_fThreshold; 
extern float  g_fSensorDiffFactor; 

// util functions to be used for the sensor thread
// forward declaration for useful functions in this file
extern void checkRecordState();
extern void initDemoCounters(bool bReset);
extern bool getSensor(CSensor* volatile *ppsms);
extern bool getBaseline(CSensor* psms);
extern bool getInitialMean(CSensor* psms);
extern double getNextDemoTimeInterval();
extern void initDemoCounters(bool bReset = false);
extern void checkDemoTrigger(bool bForce = false);
extern void doTrigger(const bool bReal = true, const long lOffsetStart = 0L, const long lOffsetEnd = 0L, const int iVariety = 0);
extern void psmsForceSensor(CSensor* volatile *ppsms);
extern void SetSensorThresholdAndDiffFactor();

#if defined(RANDOM_USB_UPLOAD) && !defined(QCNLIVE) && !defined(QCN_USB)
// use to upload the entire array to a SAC file which in turn gets zipped and uploaded - used to randomly test hosts
extern void uploadSACMem(const long lCurTime, const char* strTypeSensor);
#endif


// callback function for Phidgets 1056 USB sensor detached
extern int CCONV PhidgetsAttachHandler(CPhidgetHandle spatial, void *userPtr);
extern int CCONV PhidgetsDetachHandler(CPhidgetHandle spatial, void *userPtr);


#ifdef _DEBUG
    extern void DebugTime(const int iWhere);
#endif

#endif

