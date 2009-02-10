#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcn_shmem.h"
#include "str_util.h"  // boinc strlcpy etc
#include "qcn_util.h"
#include "util.h" // boinc, for dtime()

CQCNShMem::CQCNShMem()
{
    clear(true);
}

CQCNShMem::~CQCNShMem()
{
	// if (strProjectPreferences) free(strProjectPreferences);
}

bool CQCNShMem::setTriggerLock()
{ // set the bTriggerLock mutex
   //static bool bInHere = false;
   //if (bInHere) return false; // another proc is in here
   //bInHere = true;
   int iLock = 0;
   while (bTriggerLock && iLock++ < 2000) {  // if bTriggerLock is false already, won't even go into this loop             
       usleep(1000L); // wait up to two seconds in .001 second increments
   }  
   if (!bTriggerLock) { // must have just become unlocked, so lock it again!
      bTriggerLock = true;
   } // note that bTriggerLock could have been set to true again in the nanosecond from the break statement?
   //bInHere = false;
   return bTriggerLock;
}

bool CQCNShMem::releaseTriggerLock()
{ // release the mutex bool
   bTriggerLock = false;
   return true; // this is always true
}

float CQCNShMem::averageDT()
{
	if (ullSampleCount)
	{
		return fRealDT / (float) ullSampleCount;
	}
	else
	{
		return 0.0f;
	}
}

float CQCNShMem::averageSamples()
{
	if (ullSampleCount)
	{
		return (float) ullSampleTotal / (float) ullSampleCount;
	}
	else
	{
		return 0.0f;
	}
}

// this resets the t0check & t0active, call right before you start accessing the sensor for mean_xyz
void CQCNShMem::resetSampleClock()
{
    t0active = dtime(); // use the function in boinc/lib
    t0check = t0active + dt;
}

void CQCNShMem::clear(bool bAll)
{
    // don't zap dataBOINC, so copy over first
    if (bAll) { // clear it all, no need to save, this would be the first thing called, basically constructor
        memset(this, 0x00, sizeof(CQCNShMem));

        // start with some values that really shouldn't be 0 ever
        dt = g_DT;
        iWindow = (int) (g_cfTimeWindow / g_DT);  // number of points in time window
        return;
    }

    // for a 'not totally clear' - probably easiest to just make a "new copy"
    CQCNShMem* pshmem = new CQCNShMem();

    // copy over important, persistent fields we want to preserve
    bTriggerLock = true;
    pshmem->bTriggerLock = true;  // rudimentary locking

    pshmem->dt = dt; // this is the delta-time between point readings, currently .02 for the Mac sensor
    pshmem->iContinuousCounter = iContinuousCounter; // keeps count of how many times (without reset) we've been through the array (i.e. 1.5 hours
    pshmem->bFlagUpload = bFlagUpload; // flag that we need to upload
    strcpy(pshmem->strFileUpload, strFileUpload);  // path to the upload file
    pshmem->iWindow = iWindow;
    strcpy(pshmem->strPathImage, strPathImage);  // path to the images

    pshmem->iNumTrigger = iNumTrigger;        // the total number of triggers for this workunit
    pshmem->iNumUpload  = iNumUpload;         // the total number of uploads for this workunit
    pshmem->iNumReset   = iNumReset;          // the number of timing resets this session has had (diags which can be trickled up)
    pshmem->dMyLatitude = dMyLatitude; 
    pshmem->dMyLongitude = dMyLongitude; 
    pshmem->dMyElevationMeter = dMyElevationMeter; 
    pshmem->iMyElevationFloor = iMyElevationFloor; 
    strcpy(pshmem->strMyStation, strMyStation);

    pshmem->clock_time  = clock_time;
	pshmem->cpu_time    = cpu_time;
	pshmem->update_time = update_time;
	pshmem->fraction_done = fraction_done;

    memcpy(&pshmem->statusBOINC, &statusBOINC, sizeof(BOINC_STATUS));

	// handle project prefs special - note dataBOINC.proj_pref would already have been freed
	if (dataBOINC.project_preferences) {
		strlcpy(strProjectPreferences, dataBOINC.project_preferences, SIZEOF_PROJECT_PREFERENCES);
		free(dataBOINC.project_preferences);
		dataBOINC.project_preferences = NULL;
	}

	memset(pshmem->strProjectPreferences, 0x00, SIZEOF_PROJECT_PREFERENCES);
	strlcpy(pshmem->strProjectPreferences, strProjectPreferences, SIZEOF_PROJECT_PREFERENCES);

	// now copy over, projectprefs is safely in strProjectPreferences
	// and we freed the memory from the dataBOINC.project_prefs
    memcpy(&pshmem->dataBOINC, &dataBOINC, sizeof(APP_INIT_DATA));

	// this "atomic" copy back should be safe
    memcpy(this, pshmem, sizeof(CQCNShMem));
    resetMinMax();
    delete pshmem;
    bTriggerLock = false;
}

void CQCNShMem::resetMinMax()
{
    //fmin[E_DX] = fmin[E_DY] = fmin[E_DZ] = fmin[E_DS] = std::numeric_limits<float>::max();
    //fmax[E_DX] = fmax[E_DY] = fmax[E_DZ] = fmax[E_DS] = std::numeric_limits<float>::min();
    fmin[E_DX] = fmin[E_DY] = fmin[E_DZ] = 10000.0;
    fmax[E_DX] = fmax[E_DY] = fmax[E_DZ] = -10000.0;
    fmin[E_DS] = fmax[E_DS] = 0.0; // sigma should be 0 min, set 0 max too;
}

void CQCNShMem::testMinMax(const float fVal, const e_maxmin eType)
{
   if (fVal < fmin[eType]) fmin[eType] = fVal;
   else if (fVal > fmax[eType]) fmax[eType] = fVal;
}

const double CQCNShMem::TimeError()
{
  return  ((t0active-t0check) / dt) * 100.0f;  // timing error relative to the (usually) 0.02 second dt window
}


