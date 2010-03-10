#include "qcn_thread_sensor_util.h"

#ifndef QCN_USB
void initDemoCounters(bool bReset)
{
   // first, if in demo mode and init'ing from a timing error reset, write out what we can
   if (bReset && (qcn_main::g_bDemo || qcn_main::g_bContinual || g_bRecordState)) {
        checkDemoTrigger(true);
   }
   g_bRecordState = false;   // force recording off on a reset
   sm->bRecording = false;
   g_dStartDemoTime = 0.0f;
   g_bStartDemoEvenTime = false;  // after the first demo trigger write (10 minutes), set it to be an even 10 minute interval start time
   g_lDemoOffsetStart = 0L;
   g_lDemoOffsetEnd = 0L;
}

// check if we're recording in qcnlive mode
void checkRecordState()
{
#ifdef QCNLIVE
	static int iTest = 0;
	if (++iTest % 20) return; // just goes in every .2 second, good enough for recording
	iTest = 0;
	
	// if we've hit a boundary for Demo SAC output (currently 10 minutes) force a "trigger" (writes 0-10 minutes of data)
	if (!g_bRecordState && sm->bRecording) { 
		// quick check to see if they hit recording button and we don't know it
		g_bRecordState = true;
		g_lDemoOffsetStart = sm->lOffset;
	}
#endif
	
	// put the demo start time on an even time boundary i.e. every 10 minutes from midnight
	if (g_bRecordState || qcn_main::g_bDemo || qcn_main::g_bContinual) { // have to check versus ntpd server time
		if (g_dStartDemoTime == 0.0f && (qcn_main::g_dTimeSync > 0.0f || sm->lOffset > (300.0 / sm->dt))) {
			// wait until we get a good offset
			// five minutes has gone by from start of calibration, long enough for ntpd lookup to have finished with one retry if first failed
			g_dStartDemoTime = getNextDemoTimeInterval();
		}
		// check with adjusted server time if we went past a 10-minute even boundary
		// here's the annoying bit, we have to keep checking the server adjusted time and then 
                // break out to do a trigger, then bump up to next boundary
                // put all the triggering stuff for demo mode in this function as we may want to call it on a timing reset too
		checkDemoTrigger(g_bRecordState && !sm->bRecording);  
	}
}


void checkDemoTrigger(bool bForce)
{
    // check with adjusted server time if we went past a 10-minute even boundary
    // here's the annoying bit, we have to keep checking the server adjusted time and then break out to do a trigger, then bump up to next boundary
	if (!bForce && g_bRecordState && !sm->bRecording) {  
		// recording turned off in shared memory but not our local global flag -- means they clicked stopped recording button
		bForce = true;
	}
	else if (bForce && !g_bRecordState && !sm->bRecording) {  // no need to force it, we're not recording anymore
		bForce = false;
	}
    if (bForce || ((sm->bRecording || qcn_main::g_bContinual) && g_dStartDemoTime > 0.0f && ((sm->t0active + qcn_main::g_dTimeOffset) >= g_dStartDemoTime) ) ) { // we have a valid start time and aren't recording
       //double dTimeOffset, dTimeOffsetTime;      
       //qcn_util::getTimeOffset((const double*) sm->dTimeServerTime, (const double*) sm->dTimeServerOffset, (const double) sm->t0active, dTimeOffset, dTimeOffsetTime);
		// this will do every 10 minute interval until quit (bdemo or continual) or hit stop recording button
          g_lDemoOffsetEnd = sm->lOffset; 
	  // send a trigger -- so processed as a normal trigger (i.e. send a trickle at this time)
          doTrigger(false, g_lDemoOffsetStart, g_lDemoOffsetEnd, TRIGGER_VARIETY_CONTINUAL);  // the 2 is for continual variety, note we're passing in the offset which is just before the next 10 minute period
          g_lDemoOffsetStart = sm->lOffset; // set next start point
          g_dStartDemoTime = getNextDemoTimeInterval();  // set next time break point
    }
	if (g_bRecordState && !sm->bRecording) {  // recording turned off in shared memory but not qcn_main -- means they clicked stopped recording button
		g_bRecordState = false;
		g_lDemoOffsetStart = 0L;
		g_lDemoOffsetEnd = 0L;
		g_dStartDemoTime = 0.0f;
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

void psmsForceSensor(CSensor* volatile *ppsms)
{
#ifdef QCNLIVE  // just to be safe, this only applies to qcnlive
	 // see if they want to just use a preferred usb sensor i.e. set in sm->iMySensor - only applies to qcnlive
	 if (sm->iMySensor >= MIN_SENSOR_USB && sm->iMySensor <= MAX_SENSOR_USB ) {
		 switch(sm->iMySensor) {
			 case SENSOR_USB_JW24F8:

	#ifdef _WIN32
					*ppsms = (CSensor*) new CSensorWinUSBJW();
	#else
	#ifdef __APPLE_CC__
					*ppsms = (CSensor*) new CSensorMacUSBJW();
	 #else // Linux
					*ppsms = (CSensor*) new CSensorLinuxUSBJW();
	 #endif
	 #endif
					break;
			 case SENSOR_USB_JW24F14:
#ifdef _WIN32
				 *ppsms = (CSensor*) new CSensorWinUSBJW24F14();
#else
#ifdef __APPLE_CC__
				 *ppsms = (CSensor*) new CSensorMacUSBJW24F14();
#else // Linux
				 *ppsms = (CSensor*) new CSensorLinuxUSBJW24F14();
#endif
#endif
				 break;
#ifndef _WIN64
			case SENSOR_USB_MOTIONNODEACCEL:
					*ppsms = (CSensor*) new CSensorUSBMotionNodeAccel();
					break;
			case SENSOR_USB_ONAVI_1:
#ifdef _WIN32
				 *ppsms = (CSensor*) new CSensorWinUSBONavi01();
#else
#ifdef __APPLE_CC__
				 *ppsms = (CSensor*) new CSensorMacUSBONavi01();
#endif
#endif
#endif // win64
				 break;
		 }
	 }
#endif
}

// first off let's setup the sensor class detection code
bool getSensor(CSensor* volatile *ppsms)
{ 
	static bool bHere = false;   // prevent multiple attempts to detect sensor	
	bool bForceSensor = false;
	
	if (bHere || qcn_main::g_iStop || !sm || qcn_main::g_threadSensor->IsSuspended()) return false;  // handle quit request
	bHere = true;

   // go through and try the CSensor subclasses and try and detect a sensor
   if (*ppsms) { // previous sensor not deleted
        delete *ppsms;
        *ppsms = NULL;
   }

#ifdef QCNLIVE  // forcesensor can only be true if we're running qcnlive and they have selected an iMySensor in the USB range
	bForceSensor = (sm->iMySensor >= MIN_SENSOR_USB && sm->iMySensor <= MAX_SENSOR_USB );
#endif
			
	// for Macs the sensor can either be a CSensorMacLaptop or CSensorMacUSBJW or CSensorUSBMotionNodeAccel
	#ifdef __APPLE_CC__
	#if defined(__LP64__) || defined(_LP64) // no motion node for 64-bit
	   const int iMaxSensor = 4;  // JWF8, JWF14, ONavi, or Mac laptop
	#else
	   const int iMaxSensor = 5;  // JWF8, JWF14, ONavi, MN, or laptop
	#endif
	   
	   // note we try to detect the USB sensors first (if any), then try the laptop
	for (int i = 0; i < (bForceSensor ? 1 : iMaxSensor); i++)  { 
		   if (qcn_main::g_iStop || !sm || qcn_main::g_threadSensor->IsSuspended()) return false;  // handle quit request
		   switch(i) {
			   case 0:   // try the USB driver first
				   if (bForceSensor) {
					   psmsForceSensor(ppsms);
				   }
				   else {
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
				   }
							 break;
			   case 1:   // try the USB driver first
				   if (bForceSensor) {
					   psmsForceSensor(ppsms);
				   }
				   else {
					   //#define GENERIC
#ifdef GENERIC
					   if (boinc_is_standalone()) 
						   *ppsms = (CSensor*) new CSensorMacUSBGeneric();
#ifndef QCNLIVE
					   else
						   *ppsms = (CSensor*) new CSensorMacUSBJW24F14();
#endif
#else
					   if (boinc_is_standalone()) 
						   *ppsms = (CSensor*) new CSensorMacUSBJW24F14();
#ifndef QCNLIVE
					   else
						   *ppsms = (CSensor*) new CSensorMacUSBGeneric();
#endif
#endif		
				   }
				   break;			   
			   case 2:  // try for ONavi
				   *ppsms = (CSensor*) new CSensorMacUSBONavi01();
				   break;

#if defined(__LP64__) || defined(_LP64) // no motion node for 64-bit
			   case 3:  // note it tries to get an external sensor first before using the internal sensor, is this good logic?
				 *ppsms = (CSensor*) new CSensorMacLaptop();
				 break;
	#else
			   case 3:  // note it tries to get an external sensor first before using the internal sensor, is this good logic?
				 *ppsms = (CSensor*) new CSensorUSBMotionNodeAccel();
				 break;
			   case 4:  // note it tries to get an external sensor first before using the internal sensor, is this good logic?
				 *ppsms = (CSensor*) new CSensorMacLaptop();
				 break;
	#endif
		   }
	#else
	#ifdef _WIN32
	#ifdef _WIN64   // just thinkpad & jw8 & jw14 & onavi
	   const int iMaxSensor = 4;
	#else  // thinkpad, jw8, jw14, mn, onavi
	   const int iMaxSensor = 5;
	#endif
	   // for Windows the sensor can either be a CSensorThinkpad or CSensorWinUSBJW
	   // note we try to detect the USB sensors first (if any), then try the laptop
		for (int i = 0; i < (bForceSensor ? 1 : iMaxSensor); i++)  {
		   switch(i) {
			   case 0:
				   if (bForceSensor) {
					   psmsForceSensor(ppsms);
				   }
				   else {
				       *ppsms = (CSensor*) new CSensorWinUSBJW();
				   }
				   break;
			   case 1:
				   *ppsms = (CSensor*) new CSensorWinUSBJW24F14();
				   break;
#ifdef _WIN64
			   // no motionnode support for win64
			   case 2:
				   *ppsms = (CSensor*) new CSensorWinUSBONavi01();
				   break;
			   case 3:
				   *ppsms = (CSensor*) new CSensorWinThinkpad();
				   break;
			   // no motionnode support for win64
#else
			   case 2:
				   *ppsms = (CSensor*) new CSensorUSBMotionNodeAccel();
				   break;
			   case 3:
				   *ppsms = (CSensor*) new CSensorWinUSBONavi01();
				   break;
			   case 4:
				   *ppsms = (CSensor*) new CSensorWinThinkpad();
				   break;
	#endif
	#if 0
			   case 5:
				   *ppsms = (CSensor*) new CSensorWinHP();
				   break;
	#endif // no luck with the HP
		   }
	#else // Linux
	#if defined(__LP64__) || defined(_LP64) // no motion node for 64-bit, just JWF8 and JWF14
	   const int iMaxSensor = 2;
	#else
	   const int iMaxSensor = 3;
	#endif
	   // for Windows the sensor can either be a CSensorThinkpad or CSensorWinUSBJW
	   // note we try to detect the USB sensors first (if any), then try the laptop
		 for (int i = 0; i < (bForceSensor ? 1 : iMaxSensor); i++)  {
		   switch(i) {
			   case 0:
				   if (bForceSensor) {
					   psmsForceSensor(ppsms);
				   }
				   else {
					   *ppsms = (CSensor*) new CSensorLinuxUSBJW();
				   }
				   break;
			   case 1:
					   *ppsms = (CSensor*) new CSensorLinuxUSBJW24F14();
				   break;
#if !defined(__LP64__) && !defined(_LP64) // no motion node for 64-bit
			   case 2:
				   *ppsms = (CSensor*) new CSensorUSBMotionNodeAccel();
				   break;
	#endif
		   }
	#endif // _WIN32 or Linux
	#endif // APPLE

		   // now try out sensor if one is setup in ppsms
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
	bHere = false;
   return (bool)(*ppsms != NULL); // note *ppsms is NULL if no sensor was found, so this can be checked in the calling routine
}

#ifdef _DEBUG
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
              iWhere, sm->iNumReset, t0MaxActive, t0MaxCheck, sm->t0active, sm->t0check, sm->dTimeStart + (sm->dt * (double) iCtrStart), sm->lSampleSize, dErrorMax, '%', lCtrErr, (double) lCtrErr / 3000.0 * 100.0f, '%', sm->lOffset);
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
	   checkRecordState();
        }
        sm->sgmx = 0.0f;
        sm->xa[0]  = sm->x0[0];
        sm->ya[0]  = sm->y0[0];
        sm->za[0]  = sm->z0[0];      /*  INITIAL MEAN IS INITIAL POINT   */

        sm->fmag[0] = sqrt(QCN_SQR(sm->x0[0])+QCN_SQR(sm->y0[0])+QCN_SQR(sm->z0[0]));
        sm->vari[0] = sm->f1;
        sm->fsig[0] = 0.0f;
        if (sm->dTimeStart < 1.0f) sm->dTimeStart = ceil(sm->t0[0]);
#ifdef _DEBUG
       DebugTime(2);
#endif

        return true;
}

bool getBaseline(CSensor* psms)
{

// Measure baseline x, y, & z acceleration values for a 1 minute window
       sm->resetSampleClock();
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

          checkRecordState();

		   // test max/min
          //sm->testMinMax(sm->x0[i], E_DX);
          //sm->testMinMax(sm->y0[i], E_DY);
          //sm->testMinMax(sm->z0[i], E_DZ);
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

// report a trigger
void doTrigger(const bool bReal, const long lOffsetStart, const long lOffsetEnd, const int iVariety)
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
              ti.lOffsetStart = lOffsetStart; // if not bReal or g_bContinual i.e. we're in demo or continual mode we must be passing in an offset
              ti.lOffsetEnd   = lOffsetEnd; 
            }
            dTimeTrigger = sm->t0[ti.lOffsetEnd]; // "local" trigger time

            ti.iVariety = iVariety;

            // CMC bInteractive can bypass writing trigger files & trickles; but turn off for now
            ti.bInteractive = false;
	        //ti.bInteractive = (bool) ( sm->t0[ti.lOffsetEnd] < sm->dTimeInteractive + DECAY_INTERACTIVE_SECONDS);  
		    ti.bReal = bReal || qcn_main::g_bContinual; // flag if trickle is in demo mode or not so bypasses the trigger trickle (but writes output SAC file)

		    sm->setTriggerLock();  // we can be confident we have locked the trigger bool when this returns
	        if (bReal) qcn_util::setLastTrigger(dTimeTrigger, ti.lOffsetEnd); // add to our lasttrigger array (for graphics display) if a real trigger
            //if (!ti.bInteractive || ti.bReal) { // only do filename stuff & inc counter if demo mode or non-interactive trigger
               //qcn_util::getTimeOffset((const double*) sm->dTimeServerTime, (const double*) sm->dTimeServerOffset, (const double) dTimeTrigger, dTimeOffset, dTimeOffsetTime);
               qcn_util::set_trigger_file((char*) ti.strFile,
				   (const char*) sm->dataBOINC.wu_name,
					 bReal ? ++sm->iNumTrigger : sm->iNumTrigger,
                  QCN_ROUND(dTimeTrigger + qcn_main::g_dTimeOffset),
                  ti.bReal
               );
               if (bReal) {
                  qcn_util::set_qcn_counter();
               }
            //}
           
            ti.iWUEvent = sm->iNumTrigger;
            ti.iLevel = (bReal ? TRIGGER_IMMEDIATE : TRIGGER_DEMO); // start off with level 1, i.e. an immediate trigger output

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

void SetSensorThresholdAndDiffFactor()
{	
	// CMC -- use different threshold values based on sensor type
	g_fSensorDiffFactor = 1.3333f;
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
		case SENSOR_USB_JW24F8:
			g_fThreshold = 0.025f;
			g_fSensorDiffFactor = 1.10f;   // note USB sensors get a small diff factor below, instead of 33% just 10%
			break;
		case SENSOR_USB_JW24F14:
		case SENSOR_USB_MOTIONNODEACCEL:
		case SENSOR_USB_ONAVI_1:
			g_fThreshold = 0.01f;
			g_fSensorDiffFactor = 1.10f;   // note USB sensors get a small diff factor below, instead of 33% just 10%
			break;
		default:
			g_fThreshold = 0.10f;
	}
}
		
#endif  //ifndef QCN_USB
