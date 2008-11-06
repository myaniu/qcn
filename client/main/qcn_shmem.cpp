#include "qcn_shmem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str_util.h"  // boinc strlcpy etc
#include "qcn_util.h"
#include "util.h" // boinc, for dtime()

void CTriggerInfo::clear()
{
   memset(this, 0x00, sizeof(CTriggerInfo));
   bSent = false;
   bDemo = false;
   bRemove = false;
}

CTriggerInfo::CTriggerInfo()
{
   clear();
}

CQCNShMem::CQCNShMem()
{
    clear(true);
}

CQCNShMem::~CQCNShMem()
{
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
        // take care of some special objects i.e. databoinc & vectTrigger
        vectTrigger.clear();
        memset(this, 0x00, sizeof(CQCNShMem));

        // start with some values that really shouldn't be 0 ever
        dt = DT;
        fSignificanceFilterCutoff = DEFAULT_SIGCUTOFF;
        iWindow = (int) (cfTimeWindow / DT);  // number of points in time window
#ifdef QCNLIVE
        bDemo = true; // set demo mode
#endif
        return;
    }

    // for a 'not totally clear' - probably easiest to just make a "new copy"
    CQCNShMem* pshmem = new CQCNShMem();

    // copy over important, persistent fields we want to preserve
    bTriggerLock = true;
    pshmem->bTriggerLock = true;  // rudimentary locking
    pshmem->dt = dt; // this is the delta-time between point readings, currently .02 for the Mac sensor
    pshmem->fSignificanceFilterCutoff = fSignificanceFilterCutoff;
    pshmem->iWindow = iWindow;
    pshmem->bReadOnly = bReadOnly;
    pshmem->bDemo = bDemo;
    pshmem->iNumTrigger = iNumTrigger;        // the total number of triggers for this workunit
    pshmem->iNumUpload = iNumUpload;         // the total number of uploads for this workunit
    pshmem->iNumReset = iNumReset;          // the number of timing resets this session has had (diags which can be trickled up)
    pshmem->dMyLatitude = dMyLatitude; 
    pshmem->dMyLongitude = dMyLongitude; 
    pshmem->dMyElevationMeter = dMyElevationMeter; 
    pshmem->iMyElevationFloor = iMyElevationFloor; 
    strcpy(pshmem->strMyStation, strMyStation);
    strcpy(pshmem->strPathTrigger, strPathTrigger);  // this is the path to trigger, doesn't change after startup
    memcpy(&pshmem->dataBOINC, &dataBOINC, sizeof(APP_INIT_DATA));  // useful BOINC user prefs, i.e. for graphics
    strcpy(pshmem->strProjectPreferences, strProjectPreferences); // need to copy this separately as dataBOINC.project_preferences is dynamic string
    // BOINC status values -- called regularly in main::update_sharedmem() to update
    pshmem->fraction_done = fraction_done;
    pshmem->update_time = update_time;
    pshmem->cpu_time = cpu_time;
    pshmem->clock_time = clock_time;
    memcpy(&pshmem->statusBOINC, &statusBOINC, sizeof(BOINC_STATUS));
    pshmem->dTimeOffset = dTimeOffset;  // the time offset between client & server, +/- in seconds difference from server
    pshmem->dTimeSync = dTimeSync;    // the (unadjusted client) time this element was retrieved
    pshmem->dTimeSyncRetry = dTimeSyncRetry; // retry time for the time sync thread

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


