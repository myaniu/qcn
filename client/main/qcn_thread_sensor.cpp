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
#else
  #ifdef _WIN32
    #include "csensor_win_usb_jw.h"
    #include "csensor_win_laptop_thinkpad.h"
    #include "csensor_win_laptop_hp.h"
    #ifndef QCNLIVE
      //#include "csensor_win_usb_generic.h"
    #endif
  #else // LINUX
    #include "csensor_linux_usb_jw.h"
  #endif // Win or Linux
#endif // Apple
// all platforms get MotionNodeAccel USB support!
#include "csensor_usb_motionnodeaccel.h"

// some globals, mainly if bDemo is true so we can have continual trigger output
bool   g_bSensorTrickle = false;  // set to true if no sensor found, so we can trickle out this info (disabled now)
double g_dStartDemoTime = 0.0f;
bool   g_bStartDemoEvenTime = false;  // after the first demo trigger write (10 minutes), set it to be an even 10 minute interval start time
long   g_lDemoOffsetStart = 0L;
long   g_lDemoOffsetEnd = 0L;
float  g_fThreshold = 0.10f;

// forward declaration for useful functions in this file
bool getSensor(CSensor* volatile *ppsms);
bool getBaseline(CSensor* psms);
bool getInitialMean(CSensor* psms);
double getNextDemoTimeInterval();
void initDemoCounters(bool bReset = false);
void checkDemoTrigger(bool bForce = false);
void doTrigger(bool bReal = true, long lOffsetStart = 0L, long lOffsetEnd = 0L);

#if defined(RANDOM_USB_UPLOAD) && !defined(QCNLIVE) && !defined(QCN_USB)
// use to upload the entire array to a SAC file which in turn gets zipped and uploaded - used to randomly test hosts
void uploadSACMem(const long lCurTime, const char* strTypeSensor);  
#endif

void initDemoCounters(bool bReset)
{
   // first, if in demo mode and init'ing from a timing error reset, write out what we can
   if (bReset && qcn_main::g_bDemo && g_dStartDemoTime > 0.0f) {
        checkDemoTrigger(true);
   }
   g_dStartDemoTime = 0.0f;
   g_bStartDemoEvenTime = false;  // after the first demo trigger write (10 minutes), set it to be an even 10 minute interval start time
   g_lDemoOffsetStart = 0L;
   g_lDemoOffsetEnd = 0L;
}


void checkDemoTrigger(bool bForce)
{
    // check with adjusted server time if we went past a 10-minute even boundary
    // here's the annoying bit, we have to keep checking the server adjusted time and then break out to do a trigger, then bump up to next boundary
    if (g_dStartDemoTime > 0.0f) { // we have a valid start time
       //double dTimeOffset, dTimeOffsetTime;      
       //qcn_util::getTimeOffset((const double*) sm->dTimeServerTime, (const double*) sm->dTimeServerOffset, (const double) sm->t0active, dTimeOffset, dTimeOffsetTime);
       if (bForce || ((sm->t0active + qcn_main::g_dTimeOffset) >= g_dStartDemoTime))  { // OK, this time matches what we wanted, so do a trigger now!
          g_lDemoOffsetEnd = sm->lOffset; 
          doTrigger(false, g_lDemoOffsetStart, g_lDemoOffsetEnd);  // note we're passing in the offset which is just before the next 10 minute period
          g_lDemoOffsetStart = sm->lOffset; // set next start point
          g_dStartDemoTime = getNextDemoTimeInterval();  // set next time break point
       }
    }
}

double getNextDemoTimeInterval()
{
   // now roll forward to an even 10-minute boundary, time adjusted (by this point should have checked ntpd server time!)
   // the BOINC lib/util.C dday() function returns the double for the start of the day
   //double dTimeOffset, dTimeOffsetTime;
   //qcn_util::getTimeOffset((const double*) sm->dTimeServerTime, (const double*) sm->dTimeServerOffset, (const double) sm->t0active, dTimeOffset, dTimeOffsetTime);
   double dDay = qcn_util::qcn_dday(sm->t0active + qcn_main::g_dTimeOffset);
   // roll forward to an even 10-minute boundary, time adjusted with the value above
   return ((long)(DEMO_TRIGGER_TIME_SECONDS) * (1 + ((long)((sm->t0active + qcn_main::g_dTimeOffset) - dDay)/((long)(DEMO_TRIGGER_TIME_SECONDS))))) + dDay;
}

// first off let's setup the sensor class detection code
bool getSensor(CSensor* volatile *ppsms)
{ 
   // go through and try the CSensor subclasses and try and detect a sensor
   if (*ppsms) { // previous sensor not deleted
        delete *ppsms;
        *ppsms = NULL;
   }

// for Macs the sensor can either be a CSensorMacLaptop or CSensorMacUSBJW or CSensorUSBMotionNodeAccel
#ifdef __APPLE_CC__
   const int iMaxSensor = 3;
   
   // note we try to detect the USB sensors first (if any), then try the laptop
   for (int i = 0; i < iMaxSensor; i++)  { 
       if (qcn_main::g_iStop || !sm || qcn_main::g_threadSensor->IsSuspended()) return false;  // handle quit request
       switch(i) {
		   case 0:   // try the USB driver first
//#define GENERIC
#ifdef GENERIC
                         if (boinc_is_standalone()) 
	                    *ppsms = (CSensor*) new CSensorMacUSBGeneric();
#ifndef QCNLIVE
	                 else
	                    *ppsms = (CSensor*) new CSensorMacUSBJW();
#endif
#else
                         if (boinc_is_standalone()) 
	                    *ppsms = (CSensor*) new CSensorMacUSBJW();
#ifndef QCNLIVE
	                 else
	                    *ppsms = (CSensor*) new CSensorMacUSBGeneric();
#endif
#endif		
                         break;
		   case 1:  // note it tries to get an external sensor first before using the internal sensor, is this good logic?
			 *ppsms = (CSensor*) new CSensorUSBMotionNodeAccel();
			 break;
		   case 2:  // note it tries to get an external sensor first before using the internal sensor, is this good logic?
			 *ppsms = (CSensor*) new CSensorMacLaptop();
			 break;
       }
#else
#ifdef _WIN32
   const int iMaxSensor = 4;
   // for Windows the sensor can either be a CSensorThinkpad or CSensorWinUSBJW
   // note we try to detect the USB sensors first (if any), then try the laptop
   for (int i = 0; i < iMaxSensor; i++)  {
       switch(i) {
		   case 0:
			   *ppsms = (CSensor*) new CSensorWinUSBJW();
			   break;
		   case 1:
			   *ppsms = (CSensor*) new CSensorUSBMotionNodeAccel();
			   break;
		   case 2:
			   *ppsms = (CSensor*) new CSensorWinThinkpad();
			   break;
		   case 3:
			   *ppsms = (CSensor*) new CSensorWinHP();
			   break;
       }
#else // Linux
   const int iMaxSensor = 2;
   // for Windows the sensor can either be a CSensorThinkpad or CSensorWinUSBJW
   // note we try to detect the USB sensors first (if any), then try the laptop
   for (int i = 0; i < iMaxSensor; i++)  {
       switch(i) {
		   case 0:
			   *ppsms = (CSensor*) new CSensorLinuxUSBJW();
			   break;
		   case 1:
			   *ppsms = (CSensor*) new CSensorUSBMotionNodeAccel();
			   break;
       }
#endif // _WIN32 or Linux
#endif // APPLE

       if (qcn_main::g_iStop || !sm || qcn_main::g_threadSensor->IsSuspended()) return false;  // handle quit request

       if (*ppsms && (*ppsms)->detect()) {
           sm->eSensor = (*ppsms)->getTypeEnum();
           strcpy((char*) sm->strSensor, (*ppsms)->getTypeStr());
           break; // get out of for loop and return since we found an accelerometer
       }
       else { // if here we need to delete the pointer to try again in the loop (or just cleanup in general if nothing found)
           if (*ppsms) delete *ppsms;  // delete this object, not found
           *ppsms = NULL;
       }
   }  // end for loop for trying different sensors

   return (bool)(*ppsms != NULL); // note *ppsms is NULL if no sensor was found, so this can be checked in the calling routine
}

#ifdef _DEBUG
    void DebugTime(const int iWhere);

    static long iCtrStart = 0L;
    static double dError = 0.0f;
    static long lCtrErr = 0L;
    static double dErrorMax = 0.0f;
    static double t0MaxActive, t0MaxCheck;
    //static bool bDebugTest = true;

    void DebugTime(const int iWhere)
    {
      dError = fabs(sm->TimeError());
      if (dError > 35.0) { // if greater than 35% add to our error counter
         lCtrErr++;
      }

      if ( dError > dErrorMax) {
          dErrorMax = dError;
          t0MaxActive = sm->t0active;
          t0MaxCheck = sm->t0check;
      }

	  if (!(sm->lOffset % (sm->dt == .02 ? 3000 : 600)) || iWhere<4) {
         fprintf(stdout,"%d: timeadj=%ld  t0MaxActive=%22.10f  t0MaxCheck=%22.10f  t0active=%22.10f  t0check=%22.10f  tstart=%22.10f  lSampleSize=%ld  TimeErr=%6.2f%c  NumErr>35pct=%ld (%6.2f%c)  sm->lOffset=%ld\n",
              iWhere, sm->iNumReset, t0MaxActive, t0MaxCheck, sm->t0active, sm->t0check, sm->tstart + (sm->dt * (double) iCtrStart), sm->lSampleSize, dErrorMax, '%', lCtrErr, (double) lCtrErr / 3000.0 * 100.0f, '%', sm->lOffset);
		 fprintf(stdout, "Real DT Avg = %f    Avg Sample Size = %f\n",
			 sm->averageDT(), sm->averageSamples()
		 );
         fflush(stdout);
         dErrorMax= 0.0f;
         lCtrErr = 0L;
      }
    }

#endif

bool getInitialMean(CSensor* psms)
{ // note that the first point (0) stores the initial mean
        sm->xa[0]=0.; sm->ya[0]=0. ; sm->za[0]=0.;               /*  INITIAL MEAN IS INITIAL POINT   */
        sm->resetSampleClock();
        for (int j = 1; j < 10; j++) {
// 2)
    sm->lOffset = 0;
#ifdef _DEBUG
    iCtrStart++;
#endif
           if (!psms->mean_xyz()) { // bad error, clock must have changed
               fprintf(stderr, "getInitialMean failed in sensor thread\n");
               return false;
           }

           sm->xa[0] += (sm->x0[0] / 10.0f);
           sm->ya[0] += (sm->y0[0] / 10.0f);
           sm->za[0] += (sm->z0[0] / 10.0f);
        }
        sm->sgmx = 0.0f;
        sm->xa[0]  = sm->x0[0];
        sm->ya[0]  = sm->y0[0];
        sm->za[0]  = sm->z0[0];      /*  INITIAL MEAN IS INITIAL POINT   */

        sm->fmag[0] = sqrt(QCN_SQR(sm->x0[0])+QCN_SQR(sm->y0[0])+QCN_SQR(sm->z0[0]));
        sm->vari[0] = sm->f1;
        sm->fsig[0] = 0.0f;

#ifdef _DEBUG
       DebugTime(2);
#endif

        return true;
}

bool getBaseline(CSensor* psms)
{

// Measure baseline x, y, & z acceleration values for a 1 minute window
       sm->resetSampleClock();
       sm->resetMinMax();
       for (int i = 1; i < sm->iWindow + 1; i++) {             //  CREATE BASELINE AVERAGES
// 3)
#ifdef _DEBUG
    iCtrStart++;
#endif
          sm->lOffset = i;
          if (!psms->mean_xyz()) { // bad error, clock must have changed
              return false;
          }

          // note the first (i-1, or 0) value would have been obtained from getInitialMean() above
          sm->xa[i]  = ((i) * sm->xa[i-1] + sm->x0[i])/(i+1);         //  AVERAGE X
          sm->ya[i]  = ((i) * sm->ya[i-1] + sm->y0[i])/(i+1);         //  AVERAGE Y
          sm->za[i]  = ((i) * sm->za[i-1] + sm->z0[i])/(i+1);         //  AVERAGE Z
          sm->fmag[i] = sqrt(QCN_SQR(sm->x0[i]-sm->xa[i-1])+QCN_SQR(sm->y0[i]-sm->ya[i-1])+QCN_SQR(sm->z0[i]-sm->za[i-1]));

          // test max/min
          sm->testMinMax(sm->x0[i], E_DX);
          sm->testMinMax(sm->y0[i], E_DY);
          sm->testMinMax(sm->z0[i], E_DZ);
       }

#ifdef _DEBUG
       DebugTime(3);
#endif

// Determine base line values of variance & significance for first minute
       for (int i = 1; i < sm->iWindow + 1; i++) {             //  CREATE BASELINE SIGNIFICANCE
            sm->vari[i]= ( (i-1) * sm->vari[i-1] + QCN_SQR(sm->x0[i]-sm->xa[i])
                                       + QCN_SQR(sm->y0[i]-sm->ya[i])
                                       + QCN_SQR(sm->z0[i]-sm->za[i]) ) / i;

            sm->fsig[i]= sm->fmag[i]/sqrt(sm->vari[i-1] + 1.0e-3f);   // .001 to prevent divide-by-zero but so we capture any fmag & vari

            if ( (sm->fsig[i] > sm->sgmx) && (i > 60) ) {   //  LAST MAXIMUM SIG & PLACE
              sm->sgmx = sm->fsig[i];
              sm->itl = sm->iWindow - i + 1;
            }
            sm->fsig[i] = 0.0f;  // ultimately set to 0 in the first minute as fsig doesn't count then
       }
       return true;
}

#ifdef _WIN32
DWORD WINAPI QCNThreadSensor(LPVOID)
#else
extern void* QCNThreadSensor(void*)
#endif
{ 

  if (qcn_main::g_threadSensor) qcn_main::g_threadSensor->SetRunning(true);  // mark the entry point of the thread

  initDemoCounters();

#ifdef __APPLE_CC__
  //fprintf(stdout, "Initializing Objective-C library...\n");
  //_objcInit();  // init objective-c
#endif

    // this is the function which does the monitoring, it is launched in a separate thread from the main() program
    sm->setTriggerLock();
    sm->lOffset = 0;
    sm->releaseTriggerLock();

    // seed random number generator
    srand((unsigned) time(NULL));

    //const int iNumResetInitial = sm->iNumReset;  // initialize to total num reset, so we can see how many failures in current session
    const int iNumResetInitial = 0; // change to 0, don't use the saved reset number, reset to 0 every wraparound
    qcn_main::g_vectTrigger.clear();
    while (sm->lOffset > -1) {  // start the main loop
      if (qcn_main::g_iStop || !sm) goto done;  // handle quit request
      if (qcn_main::g_threadSensor->IsSuspended()) {
           usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
           continue;
      }
      if (sm->lOffset == 0) {
         if ((sm->bSensorFound = getSensor(&qcn_main::g_psms))) {  // g_psms will be set to NULL if no sensor found or a valid sensor subclass of CSensor
            if (qcn_main::g_iStop || !sm) goto done;  // handle quit request
            if (qcn_main::g_threadSensor->IsSuspended()) {
                usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
                continue;
            }
            g_bSensorTrickle = false;
            fprintf(stderr, "Motion sensor initialized of type %d - %s.\n", qcn_main::g_psms->getTypeEnum(), qcn_main::g_psms->getTypeStr());
            fprintf(stdout, "Motion sensor initialized of type %d - %s.\n", qcn_main::g_psms->getTypeEnum(), qcn_main::g_psms->getTypeStr());
         }
         else { // if (!qcn_main::g_psms || qcn_main::g_psms->getTypeEnum() == SENSOR_NOTFOUND) { // no sensor of any type detected
            if (qcn_main::g_psms) {
                delete qcn_main::g_psms;
                qcn_main::g_psms = NULL;
            }
	        if (qcn_main::g_iStop || !sm) goto done;  // handle quit request
            if (qcn_main::g_threadSensor->IsSuspended()) {
                usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
                continue;
            }
            //if (!g_bSensorTrickle && ! boinc_is_standalone()) { // just send max one sensor fail trickle per session
            if (!g_bSensorTrickle) { // just send max one sensor fail trickle per session
               fprintf(stderr, "No motion sensor found!\n");
               fprintf(stdout, "No motion sensor found!\n");
#ifndef QCNLIVE 
               //trickleup::qcnTrickleUp("", "nosensor", (const char*) sm->wu_name);  // trickle the fact that there is no sensor found
#endif
               g_bSensorTrickle = true;
            }
            for (int iSleep = 0; iSleep < 100; iSleep++) { // waits 10 seconds before trying again (100 * .1 seconds so can check for stop request)
               if (qcn_main::g_iStop || !sm) goto done;  // handle quit request               
               if (qcn_main::g_threadSensor->IsSuspended()) {
                  usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
				  iSleep = 0;
                  continue;
               }
               usleep(100000); // boinc_sleep(0.1f); // wait .1 seconds and try again
            }
#ifdef _DEBUG
            fprintf(stdout, "Just slept 10 seconds, look for sensor again...\n");
#endif
            continue;
         }

// 1)
         // reset max/min, we could be wrapping around from MAXI
         sm->fmag[0] = 0.;                                  /*  INITIAL ZERO POSITION           */
         sm->amag[0] = 0.;                                  /*  INITIAL ZERO AVERAGE            */
         sm->vari[0] = 0.;                                  /*  STANDARD DEVIATION              */

         sm->resetMinMax();
         sm->resetSampleClock();
         sm->tstart = sm->t0active;
         try { // sensor mean throws an exception if we're shutting down
            if (!qcn_main::g_psms || ! qcn_main::g_psms->mean_xyz()) {
               fprintf(stderr, "Error 0 in sensor thread mean_xyz()\n");
               fprintf(stdout, "Error 0 in sensor thread mean_xyz()\n");
            }
         }
      	 catch(int ie)  {
            fprintf(stderr, "Exception 1 caught in sensor thread mean_xyz()\n");
            if (ie==EXCEPTION_SHUTDOWN) goto done;
	     }
         if (qcn_main::g_iStop || !sm) goto done;  // handle quit request
         if (qcn_main::g_threadSensor->IsSuspended()) {
             usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
             continue;
         }

         // CMC -- use different threshold values based on sensor type
         switch(sm->eSensor) {
            case SENSOR_MAC_PPC_TYPE1:
            case SENSOR_MAC_PPC_TYPE2:
            case SENSOR_MAC_PPC_TYPE3:
            case SENSOR_MAC_INTEL:
               g_fThreshold = 0.10f;
               break;
            case SENSOR_WIN_HP:
            case SENSOR_WIN_THINKPAD:
               g_fThreshold = 0.20f;
               break;
            case SENSOR_USB_JW:
               g_fThreshold = 0.025f;
               break;
            case SENSOR_USB_MOTIONNODEACCEL:
               g_fThreshold = 0.01f;
               break;
            default:
               g_fThreshold = 0.10f;
         }

         fprintf(stdout,"Start of monitoring at time %f  interval %f  threshold %f\n", sm->tstart, sm->dt, g_fThreshold);
         fprintf(stdout,"Initial sensor values:  x0=%f  y0=%f  z0=%f  sample size=%ld  dt=%f\n", sm->x0[0], sm->y0[0], sm->z0[0], sm->lSampleSize, sm->dt);
         fflush(stdout);

/* In this section we sample the time sensor to provide a basis for the
 * triggering algorithm.
 *
 * Average:           xav(N) = sum(x(i))/N
 *        FASTER AS:  xav(N) = [ (N-1)*xav(N-1) + x(N)]/N
 * Variance:          vari(N) = sum (x(i)-xav(N))^2/N (SUMMED FOR x, y, & z)
 *        FASTER AS:  vari(N) = [ (N-1)*vari(N-1) + (x(N)-xav(N))^2 ]/N
 * Standard Deviation: stdev(N) = sqrt(vari(N))
 * Magnitude: fmag(N) = sqrt( (x(N)-xav(N))^2+(y(N)-yav(N))^2+(z(N)-zav(N))^2 )
 * Significance: fsig(N) = fmag(N)/stdev(N)
 *
 */
// Get a stable initial mean value to start from
     try {
         if (!getInitialMean(qcn_main::g_psms)) {
             // probably a timing error, so retry master (outermost) loop
             initDemoCounters(true);
             qcn_util::ResetCounter(WHERE_THREAD_SENSOR_INITIAL_MEAN, iNumResetInitial);
			if (qcn_main::g_psms) {
				qcn_main::g_psms->closePort();  // close the port, it will be reopened above
                delete qcn_main::g_psms;
                qcn_main::g_psms = NULL;
			}
//#ifdef _DEBUG
//    bDebugTest = true;
//#endif
             continue;
         }
     }
     catch(int ie)  {
             fprintf(stderr, "Exception 2 caught in sensor thread mean_xyz()\n");
             if (ie==EXCEPTION_SHUTDOWN) goto done;
     }
     if (qcn_main::g_iStop || !sm) goto done;  // handle quit request
     if (qcn_main::g_threadSensor->IsSuspended()) {
        usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
        continue;
     }

     try {
         if (!getBaseline(qcn_main::g_psms)) {
             initDemoCounters(true);
             qcn_util::ResetCounter(WHERE_THREAD_SENSOR_BASELINE, iNumResetInitial);  // probably a timing error, so retry master (outermost) loop
			if (qcn_main::g_psms) {
				qcn_main::g_psms->closePort();  // close the port, it will be reopened above
                delete qcn_main::g_psms;
                qcn_main::g_psms = NULL;
			}
//#ifdef _DEBUG
//    bDebugTest = true;
//#endif
             continue;
         }
     }
     catch(int ie)  {
            fprintf(stderr, "Exception 3 caught in sensor thread mean_xyz()\n");
            if (ie==EXCEPTION_SHUTDOWN) goto done;
     }
     if (qcn_main::g_iStop || !sm) goto done;  // handle quit request
     if (qcn_main::g_threadSensor->IsSuspended()) {
            usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
            continue;
     }

// Print a status report to X11 window
     fprintf(stdout, "INITIAL MAX SIGMA: %ld %f\n",sm->iWindow,sm->sgmx);
     fflush(stdout);

// Keep track of start of window relative to ith point
     sm->lOffset = sm->iWindow;  // we've advanced a minute's worth of points...

   } // if sm->lOffset==0

/* Monitor Sensor:
 * Note: Equations are different here because we have a full window already .

 *
 * Average:           xav(N) = sum(x(i))/N
 *        FASTER AS:  xav(N) = xav(N-1) + x(N)/N
 * Variance:          vari(N) = sum (x(i)-xav(N))^2/N (SUMMED FOR x, y, & z)
 *        FASTER AS:  vari(N) = vari(N-1) + [ (x(N)     -xav(N))^2      ]/N
 *                                        - [ (x(N-iWin)-xav(N-iWin))^2 ]/N
 *
 */

/* CMC -- example boinc_time_to_checkpoint call -- probably should be done here
        if (boinc_time_to_checkpoint()) {
            retval = do_checkpoint(out, nchars);
            if (retval) {
                fprintf(stderr, "APP: upper_case checkpoint failed %d\n", retval);
            }
            boinc_checkpoint_completed();
        }
        boinc_fraction_done(??);  // what is the fraction of a QCN workunit?  weekly workunits, so just use t0?
*/

 // 4)
      // increment our main counter, if bigger than array size we have to reset & continue!
      if (++sm->lOffset >= MAXI)  {
#if defined(RANDOM_USB_UPLOAD) && !defined(QCNLIVE) && !defined(QCN_USB)
         e_sensor esType = qcn_main::g_psms ? qcn_main::g_psms->getTypeEnum() : SENSOR_NOTFOUND;  // save the current sensor for use below (random data upload)
         char strTypeSensor[8];
         memset(strTypeSensor, 0x00, sizeof(char) * 8);
         if (qcn_main::g_psms) strncpy(strTypeSensor, qcn_main::g_psms->getTypeStrShort(), 7);
#endif

         sm->iContinuousCounter++; // increment how many times we've been through the array without a reset

         // close the port first -- because if running as a service (Mac JoyWarrior) this will cause a timing lag/reset error
         // as the port is still monitoring, but the big file I/O below will cause this thread to suspend a few seconds
         sm->iNumReset = 0;  // let's reset our reset counter every wraparound (1 hour)
         sm->lOffset = 0;  // don't reset, that's only for drastic errors i.e. bad timing errors

         // close the open port
         if (qcn_main::g_psms) {
	     qcn_main::g_psms->closePort();  // close the port, it will be reopened above
             delete qcn_main::g_psms;
             qcn_main::g_psms = NULL;
         }
 
 // CMC - randomly upload whole array for USB sensors on a random basis
#if defined(RANDOM_USB_UPLOAD) && !defined(QCNLIVE) && !defined(QCN_USB)
         if (!qcn_main::g_bDemo && 
           (esType == SENSOR_USB_JW || esType == SENSOR_USB_MOTIONNODEACCEL)) { 
            // they're using a JW -- do a random test to see if we want to upload this array
            long lCurTime = QCN_ROUND(dtime() + qcn_main::g_dTimeOffset);
            fprintf(stderr, "%ld - USB Sensor - End of array - reloop # %d\n", lCurTime, sm->iContinuousCounter);
            if (sm->iNumUpload < 5 && rand() < (RAND_MAX/5)) { // 20% chance to do an upload
                //if (sm->iNumUpload < 5 && (sm->iContinuousCounter == (1 + (rand() % 10)))) { // this will get a number from 1 to 10 which should match our continuous counter
                 fprintf(stderr, "%ld - Random upload scheduled # %d\n", lCurTime, sm->iNumUpload);
                 uploadSACMem(lCurTime, strTypeSensor); 
            }
         }
#endif
         //ResetCounter(WHERE_WRAPAROUND);  // don't reset, that's only for drastic errors i.e. bad timing errors
         //sm->bWrapped = true;
         qcn_util::set_qcn_counter();
         continue;
      }
#ifdef _DEBUG
      iCtrStart++;
#endif
      sm->itl++;  // our placeholder for max sig position

      try {
         if (!qcn_main::g_psms || ! qcn_main::g_psms->mean_xyz()) {// bad error, clock must have changed or lost sensor
            initDemoCounters(true);
            qcn_util::ResetCounter(WHERE_THREAD_SENSOR_TIME_ERROR, iNumResetInitial);
			if (qcn_main::g_psms) {
				qcn_main::g_psms->closePort();  // close the port, it will be reopened above
                delete qcn_main::g_psms;
                qcn_main::g_psms = NULL;
			}
            continue;
         }  
      }  
      catch(int ie)  {
        fprintf(stderr, "Exception 4 caught in sensor thread mean_xyz()\n");
        if (ie==EXCEPTION_SHUTDOWN) goto done;
      }
        if (qcn_main::g_iStop || !sm) goto done;  // handle quit request
        if (qcn_main::g_threadSensor->IsSuspended()) {
            usleep(MAIN_LOOP_SLEEP_MICROSECONDS); // 5Hz should be fast enough to monitor things such as trickles, stop/suspend requests etc
            continue;
        }

      // CMC note -- in original onepoint.c code, sm->lOffset-1 was iM1 & sm->lOffset-iWindow was IB1, these were
      //     used to get the right "place" in case of the array being looped, but I don't see how this could

      //     happen because after the one minute mean/baseline above, we are past "iWindow" by this point
      //     as if sm->lOffset >= MAXI above will go to top of loop and reinitialize
      sm->xa[sm->lOffset]  = sm->xa[sm->lOffset-1] + ((sm->x0[sm->lOffset] - sm->x0[sm->lOffset-sm->iWindow]) / sm->iWindow);         //  AVERAGE X
      sm->ya[sm->lOffset]  = sm->ya[sm->lOffset-1] + ((sm->y0[sm->lOffset] - sm->y0[sm->lOffset-sm->iWindow]) / sm->iWindow);         //  AVERAGE Y
      sm->za[sm->lOffset]  = sm->za[sm->lOffset-1] + ((sm->z0[sm->lOffset] - sm->z0[sm->lOffset-sm->iWindow]) / sm->iWindow);         //  AVERAGE Z

// note -- the bottom three lines of the vari[i] expression are uncommented per JLF
//   (previously in the original onepoint.c code it ended at the first sm->iWindow)
      sm->vari[sm->lOffset] = sm->vari[sm->lOffset-1] + ( QCN_SQR(sm->x0[sm->lOffset] - sm->xa[sm->lOffset]   )
          + QCN_SQR(sm->y0[sm->lOffset]   - sm->ya[sm->lOffset]   )
          + QCN_SQR(sm->z0[sm->lOffset]   - sm->za[sm->lOffset]   ) ) / sm->iWindow
        - ( QCN_SQR(sm->x0[sm->lOffset-sm->iWindow] - sm->xa[sm->lOffset-sm->iWindow] )
        + QCN_SQR(sm->y0[sm->lOffset-sm->iWindow] - sm->ya[sm->lOffset-sm->iWindow] )
        + QCN_SQR(sm->z0[sm->lOffset-sm->iWindow] - sm->za[sm->lOffset-sm->iWindow] ) ) / sm->iWindow;

      sm->fmag[sm->lOffset]= sqrt(QCN_SQR(sm->x0[sm->lOffset]-sm->xa[sm->lOffset-1])+
         QCN_SQR(sm->y0[sm->lOffset]-sm->ya[sm->lOffset-1])+QCN_SQR(sm->z0[sm->lOffset]-sm->za[sm->lOffset-1]));

// CMC added Jesse's mod to have a short term avg mag, and put this var in shared mem so it can be perturbed
    //  sm->fsig[sm->lOffset]= sm->fmag[sm->lOffset]/sqrt(sm->vari[sm->lOffset-1] + 1.0e-3f);  // .001 to prevent divide-by-zero but so we capture any fmag & vari

// jesse added a short term average magnitude rather than an instantaneous
//    magnitude.  eventually fShortTermAvgMag should be turned into a variable to
//    passed into here.  In theory the larger fShortTermAvgMag is, the harder it will
//    be to trigger with a delta function. fShortTermAvgMag should vary from 1 to ~5
//    (5 is looking at 10Hz variations rather than 50).
//      int fShortTermAvgMag = 3;
      sm->fsig[sm->lOffset]=sm->fmag[sm->lOffset] /
                            sqrt(sm->vari[sm->lOffset-1] + 1.0e-3f);
// .001 to prevent divide-by-zero but so we capture any fmag & vari
      if (qcn_main::g_fPerturb[PERTURB_SHORT_TERM_AVG_MAG] > 0) {
         for (int i_off = 1; i_off < (int) qcn_main::g_fPerturb[PERTURB_SHORT_TERM_AVG_MAG]; i_off++) {
            sm->fsig[sm->lOffset]=sm->fsig[sm->lOffset] +
                             sm->fmag[sm->lOffset-i_off] /
                             sqrt(sm->vari[sm->lOffset-1] + 1.0e-3f);
// .001 to prevent divide-by-zero but so we capture any fmag & vari
         }
         sm->fsig[sm->lOffset]=sm->fsig[sm->lOffset]/ (qcn_main::g_fPerturb[PERTURB_SHORT_TERM_AVG_MAG] + 1.0f);
// Normalize average magnitude over window
      }

      // test max/min
      sm->testMinMax(sm->x0[sm->lOffset], E_DX);
      sm->testMinMax(sm->y0[sm->lOffset], E_DY);
      sm->testMinMax(sm->z0[sm->lOffset], E_DZ);
      sm->testMinMax(sm->fsig[sm->lOffset], E_DS);

// Determine if significance filter is large enough to warrant a trigger
//#ifdef _DEBUG // force a trigger
//      if (bDebugTest) {
//#else
      if (sm->fsig[sm->lOffset] > 1.33*sm->sgmx){                      // 33% larger than others
//#endif
        sm->sgmx = sm->fsig[sm->lOffset];
        sm->itl=0;

        // CMC note: the threshold values are from above after sensor detection, the peturb values for sig cutoff are from the workunit generation
        if ( (sm->fsig[sm->lOffset] > qcn_main::g_fPerturb[PERTURB_SIG_CUTOFF]) && (sm->fmag[sm->lOffset] > g_fThreshold) ) {  // was 0.125,  >2 sigma (90% Conf)
          // lock & update now if this trigger is >.5 second from the last
          double dTime; 
          long lTime;
          qcn_util::getLastTrigger(dTime, lTime);
          if (dTime + 0.50f <= sm->t0[sm->lOffset]) { // if in here, the last trigger was more than a second ago
            doTrigger(); // last trigger time will get set in the proc
          }
        }
      }

// Find the largest significance in the time window
      if (sm->itl > sm->iWindow) {
        sm->sgmx = 0.;
        for (int j=sm->lOffset-(sm->iWindow+1); j<sm->lOffset; j++) {
          if (sm->fsig[j] > sm->sgmx) {          //  MAXIMUM SIGNIFICANCE
            sm->sgmx = sm->fsig[j];
            sm->itl = sm->lOffset-j;
          }
        }
        //fprintf(stdout, "Largest sig at lOffset=%ld iWindow=%d PT:%ld SGMX=%f \n", sm->lOffset, sm->iWindow, sm->itl, sm->sgmx);
        //fflush(stdout);
      }

      // if we've hit a boundary for Demo SAC output (currently 10 minutes) force a "trigger" (writes 0-10 minutes of data)
      // put the demo start time on an even time boundary i.e. every 10 minutes from midnight
      if (qcn_main::g_bDemo) { // have to check versus ntpd server time
           if (g_dStartDemoTime == 0.0f && sm->lOffset > (300.0 / sm->dt)) {
             // five minutes has gone by from start of calibration, long enough for ntpd lookup to have finished with one retry if first failed
             g_dStartDemoTime = getNextDemoTimeInterval();
           }
           // check with adjusted server time if we went past a 10-minute even boundary
           // here's the annoying bit, we have to keep checking the server adjusted time and then break out to do a trigger, then bump up to next boundary
           checkDemoTrigger();  // put all the triggering stuff for demo mode in this function as we may want to call it on a timing reset too
        }

#ifdef _DEBUG
      DebugTime(4);
#endif
    }  // outermost while loop

done: // ending, perhaps from a g_iStop request in the wxWidgets myApp::OnExit()
    if (qcn_main::g_psms) {
         qcn_main::g_psms->closePort();  // close the port, it will be reopened above
         delete qcn_main::g_psms;
		 qcn_main::g_psms = NULL;
    }
    if (qcn_main::g_threadSensor) qcn_main::g_threadSensor->SetRunning(false);  // mark the thread as being done
    return 0;
}

// report a trigger
void doTrigger(bool bReal, long lOffsetStart, long lOffsetEnd)
{
            double dTimeTrigger;
/*
#ifdef _DEBUG // fake a trigger
            bDebugTest = false;
            fprintf(stdout, "Debug Mode: faking a trigger event!\n");
            fflush(stdout);
#endif
*/
            //double dTimeTrigger, dTimeOffset, dTimeOffsetTime;
            STriggerInfo ti;
            // trigger happened when user interacting, so flag so don't really trickle/trigger
            // dTimeInteractive is set in the graphics thread/program when a button is clicked or key is pressed
            // if this interaction time + a decay offset (usually 60 seconds) is exceeded by the trigger time, it's a valid trigger

            if (bReal) {
              ti.lOffsetStart = sm->lOffset - (long)(60.0f / sm->dt);  // for a real trigger we just need the end point as we go back a minute, and forward a minute or two
              ti.lOffsetEnd   = sm->lOffset;  // for a real trigger we just need the end point as we go back a minute, and forward a minute or two
            }
            else {
              ti.lOffsetStart = lOffsetStart; // if not bReal i.e. we're in demo mode we must be passing in an offset
              ti.lOffsetEnd   = lOffsetEnd; // if not bReal i.e. we're in demo mode we must be passing in an offset
            }
            dTimeTrigger = sm->t0[ti.lOffsetEnd]; // "local" trigger time

            // CMC bInteractive can bypass writing trigger files & trickles; but turn off for now
            ti.bInteractive = false;
	    //ti.bInteractive = (bool) ( sm->t0[ti.lOffsetEnd] < sm->dTimeInteractive + DECAY_INTERACTIVE_SECONDS);  
            ti.bDemo = !bReal; // flag if trickle is in demo mode or not so bypasses the trigger trickle (but writes output SAC file)

            sm->setTriggerLock();  // we can be confident we have locked the trigger bool when this returns
            if (bReal) qcn_util::setLastTrigger(dTimeTrigger, ti.lOffsetEnd); // add to our lasttrigger array (for graphics display) if a real trigger
            //if (!ti.bInteractive || ti.bDemo) { // only do filename stuff & inc counter if demo mode or non-interactive trigger
               //qcn_util::getTimeOffset((const double*) sm->dTimeServerTime, (const double*) sm->dTimeServerOffset, (const double) dTimeTrigger, dTimeOffset, dTimeOffsetTime);
               qcn_util::set_trigger_file((char*) ti.strFile,
				   (const char*) sm->dataBOINC.wu_name,
                  bReal ? ++sm->iNumTrigger : sm->iNumTrigger,
                  QCN_ROUND(dTimeTrigger + qcn_main::g_dTimeOffset),
                  bReal
               );
               if (bReal) {
                  qcn_util::set_qcn_counter();
               }
            //}
           
            ti.iWUEvent = sm->iNumTrigger;
            ti.iLevel = bReal ? TRIGGER_IMMEDIATE : TRIGGER_DEMO; // start off with level 1, i.e. an immediate trigger output

            // add this to our vector in sm
            ti.bSent = false;
            qcn_main::g_vectTrigger.push_back(ti);
            sm->iout = ti.lOffsetEnd + (int)(60.0f/sm->dt);                           //  TIME STEP TO OUTPUT FILE
            sm->itm = ti.lOffsetEnd + (2*sm->iWindow); // set end of data collection/file I/O for two minutes from this trigger point
            if (sm->itm > MAXI) sm->itm -= MAXI;  // note itm is adjusted for wraparound
            sm->releaseTriggerLock();  // we can be confident we have locked the trigger bool when this returns
}

#if defined(RANDOM_USB_UPLOAD) && !defined(QCNLIVE) && !defined(QCN_USB)
// use to upload the entire array to a SAC file which in turn gets zipped and uploaded - used to randomly test hosts
void uploadSACMem(const long lCurTime, const char* strTypeSensor)
{ // note -- this will take a little time so we will "miss" a few seconds at most until the recalibration begins again, probably not a big deal...

        int iSlot = (int) sm->iNumUpload;
        char strResolve[_MAX_PATH], strLogical[_MAX_PATH_LOGICAL];
        memset(strResolve, 0x00, _MAX_PATH);
        memset(strLogical, 0x00, _MAX_PATH_LOGICAL);

        // get an empty zip slot to use --- 1 through 20 (MAX_UPLOAD)
        // we already have the slot from sm->iNumUpload
        iSlot++;  // increment the upload slot counter
        if (iSlot<1 || iSlot>MAX_UPLOAD) {  // sanity check, only zip file #'s 1 through MAX_UPLOAD (20) reserved for intermediate uploading
          fprintf(stderr, "%ld - No zip slots left for upload file!\n", lCurTime);
          return;
        }

        // OK now we can resolve the zip filename
        // try and resolve the filename qcnout1.zip
        sprintf(strLogical, "qcnout%d.zip", iSlot);
        if (boinc_resolve_filename(strLogical, strResolve, _MAX_PATH) && !strResolve[0]) {
          // this zip name didn't resolve, free sz mem and return!
          fprintf(stderr, "%ld - Upload zip filename %s not resolved for random upload file!\n", lCurTime, strLogical);
          return;
        }

        // OK if we're here then we have a boinc name for the zip file
        // make a "dummy" trigger for this "event"
        struct STriggerInfo sti;
        memset(&sti, 0x00, sizeof(struct STriggerInfo));
        sti.lOffsetStart = 0L;
        sti.lOffsetEnd = MAXI;
        sti.iWUEvent = sm->iContinuousCounter;
        sti.iLevel = TRIGGER_ALL;

        qcn_util::set_trigger_file(sti.strFile,
                  (const char*) sm->dataBOINC.wu_name,
                  sm->iNumTrigger,
                  lCurTime,
                  true, "usb"
        );

        fprintf(stderr, "%ld - Creating upload file %s\n", lCurTime, sti.strFile);
        sacio::sacio(0, MAXI, &sti, strTypeSensor);

        // now make sure the zip file is stored in sm->strPathTrigger + ti->strFile
        string strZip((const char*) qcn_main::g_strPathTrigger);  // note it DOES NOT HAVE appropriate \ or / at the end
        strZip += qcn_util::cPathSeparator();
        strZip += (const char*) sti.strFile;

        // OK, so zip sti.strFile
        boinc_begin_critical_section(); 
        boinc_zip(ZIP_IT, strResolve, strZip);
        sm->iNumUpload = iSlot;  // set the num upload which was successfully incremented & processed above
        boinc_delete_file(strZip.c_str());  // don't need the original zip, since it's in the strResolve zip name
        //strcpy(sm->strUploadLogical, strLogical);
        //strcpy(sm->strUploadResolve, strResolve);
        boinc_end_critical_section(); 

        // moved from main, this shouldn't bother the sensor thread
        //if (sm 
        //   && strlen(sm->strUploadResolve)>1 
        //   && strlen(sm->strUploadLogical)>1 
        //   && boinc_file_exists(sm->strUploadResolve)) { // send this file, whether it was just made or made previously
        if (boinc_file_exists(strResolve)) { // send this file, whether it was just made or made previously
             // note boinc_upload_file (intermediate uploads) requires the logical boinc filename ("soft link")!
             qcn_util::sendIntermediateUpload(strLogical, strResolve);  // the logical name gets resolved by boinc_upload_file into full path zip file 
             //qcn_util::sendIntermediateUpload(sm->strUploadLogical, sm->strUploadResolve);  // the logical name gets resolved by boinc_upload_file into full path zip file 
             //memset(sm->strUploadResolve, 0x00, sizeof(char) * _MAX_PATH);
             //memset(sm->strUploadLogical, 0x00, sizeof(char) * _MAX_PATH_LOGICAL);
        }
}
#endif


