#include "qcn_thread_sensor_util.h"
#include "qcn_thread_sensor_loop.h"

// this is the main thread for monitoring the sensor and generating triggers/trickles as they occur
// to make it more readable, it has been split into three files, this one, and sensor_util and sensor_loop
// most function calls in here are in sensor_util, except for the TriggerDetectionAlgorithm which is in sensor_loop  (to make it easier to try other algorithms)

// so this file pretty much will try to detect and open a sensor and read it at 50Hz, then pass the data to the TriggerDetectionAlgorithm for analysis and making a trigger

// some globals which are extern'd in qcn_thread_sensor_util.h
// mainly if bDemo is true so we can have continual trigger output
bool   g_bSensorTrickle = false;  // set to true if no sensor found, so we can trickle out this info (disabled now)
bool   g_bRecordState = false;    // internal sensor thread flag that we are recording
double g_dStartDemoTime = 0.0f;
bool   g_bStartDemoEvenTime = false;  // after the first demo trigger write (10 minutes), set it to be an even 10 minute interval start time
long   g_lDemoOffsetStart = 0L;
long   g_lDemoOffsetEnd = 0L;
float  g_fThreshold = 0.10f;
float  g_fSensorDiffFactor = 1.3333f;

#ifdef _DEBUG
   int iCtrStart = 0;
#endif

#ifdef _WIN32
DWORD WINAPI QCNThreadSensor(LPVOID)
#else
extern void* QCNThreadSensor(void*)
#endif
{ 

  if (qcn_main::g_threadSensor) qcn_main::g_threadSensor->SetRunning(true);  // mark the entry point of the thread

  initDemoCounters();

//#ifdef __APPLE_CC__
  //fprintf(stdout, "Initializing Objective-C library...\n");
  //_objcInit();  // init objective-c
//#endif

    // this is the function which does the monitoring, it is launched in a separate thread from the main() program
    sm->setTriggerLock();
    sm->lOffset = 0;
	sm->lWrap = 0;
    sm->releaseTriggerLock();

    // seed random number generator
    srand((unsigned) time(NULL));

    int iNumResetInitial = sm->iNumReset;  // initialize to total num reset, so we can see how many failures in current session
    //const int iNumResetInitial = 0; // change to 0, don't use the saved reset number, reset to 0 every wraparound
    qcn_main::g_vectTrigger.clear();

    // the main loop
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
            //fprintf(stderr, "Motion sensor initialized of type %d - %s.\n", qcn_main::g_psms->getTypeEnum(), qcn_main::g_psms->getTypeStr());
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
         }  // sensor detected or not

// 1)
         // reset max/min, we could be wrapping around from MAXI
         sm->fmag[0] = 0.;                                  /*  INITIAL ZERO POSITION           */
         sm->amag[0] = 0.;                                  /*  INITIAL ZERO AVERAGE            */
         sm->vari[0] = 0.;                                  /*  STANDARD DEVIATION              */

         sm->resetSampleClock();
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
		  
		  SetSensorThresholdAndDiffFactor();

         fprintf(stdout,"Start of monitoring at time %f  interval %f  threshold %f\n", sm->t0start, sm->dt, g_fThreshold);
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
         // save the current sensor for use below (random data upload)
         e_sensor esType = qcn_main::g_psms ? qcn_main::g_psms->getTypeEnum() : SENSOR_NOTFOUND;  
         char strTypeSensor[8];
         memset(strTypeSensor, 0x00, sizeof(char) * 8);
         if (qcn_main::g_psms) strncpy(strTypeSensor, qcn_main::g_psms->getTypeStrShort(), 7);
#endif
         // close the port first -- because if running as a service (Mac JoyWarrior) this will cause a timing lag/reset error
         // as the port is still monitoring, but the big file I/O below will cause this thread to suspend a few seconds
         sm->iNumReset = 0;  // let's reset our reset counter every wraparound (1 hour)
         iNumResetInitial = 0; // "local" reset counter
         sm->lOffset = 0;  // don't reset, that's only for drastic errors i.e. bad timing errors
		 sm->lWrap++; // increment how many times we've been through the array without a reset

         // close the open port
         if (qcn_main::g_psms) {
	         qcn_main::g_psms->closePort();  // close the port, it will be reopened above
             delete qcn_main::g_psms;
             qcn_main::g_psms = NULL;
         }
 
 // CMC - randomly upload whole array for USB sensors on a random basis
#if defined(RANDOM_USB_UPLOAD) && !defined(QCNLIVE) && !defined(QCN_USB)
        if (!qcn_main::g_bDemo && !qcn_main::g_bQCNLive && 
           (esType == SENSOR_USB_JW24F8 
         || esType == SENSOR_USB_JW24F14
         || esType == SENSOR_USB_MOTIONNODEACCEL)) { 
            // they're using a JW -- do a random test to see if we want to upload this array
            long lCurTime = QCN_ROUND(dtime() + qcn_main::g_dTimeOffset);
            fprintf(stderr, "%ld - USB Sensor - End of array - reloop # %d\n", lCurTime, sm->lWrap);
            if (sm->iNumUpload < 5 && rand() < (RAND_MAX/5)) { // 20% chance to do an upload
                //if (sm->iNumUpload < 5 && (sm->lWrap == (1 + (rand() % 10)))) { 
                 // this will get a number from 1 to 10 which should match our continuous counter
                 fprintf(stderr, "%ld - Random upload scheduled # %d\n", lCurTime, sm->iNumUpload);
                 uploadSACMem(lCurTime, strTypeSensor); 
            }
         }
#endif
         //ResetCounter(WHERE_WRAPAROUND);  // don't reset, that's only for drastic errors i.e. bad timing errors
         //sm->bWrapped = true;
         qcn_util::set_qcn_counter();
         continue;
      } // >=MAXI
#ifdef _DEBUG
      iCtrStart++;
#endif

		sm->itl++;  // our placeholder for max sig position
		
		// now read the sensor value
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
		
        // call the main loop trigger detection routine...
		qcn_thread_sensor_loop::TriggerDetectionAlgorithm();

    }  // outermost while loop

done: // ending, perhaps from a g_iStop request in the wxWidgets myApp::OnExit()
    // if in continual mode and workunit finished normally, may want to send last bit of data?
    if (g_bRecordState || (qcn_main::g_bContinual && sm->eStatus == ERR_FINISHED)) {
     // && (g_dStartDemoTime - (sm->t0active + qcn_main::g_dTimeOffset)) > (DEMO_TRIGGER_TIME_SECONDS - 60)) { 
         fprintf(stderr, "Sensor thread - eEnding workunit - force one last continual trigger file\n");
         checkDemoTrigger(true); 
    }
    if (qcn_main::g_psms) {
         qcn_main::g_psms->closePort();  // close the port, it will be reopened above
         delete qcn_main::g_psms;
	 qcn_main::g_psms = NULL;
    }
    if (qcn_main::g_threadSensor) qcn_main::g_threadSensor->SetRunning(false);  // mark the thread as being done
    return 0;
}

