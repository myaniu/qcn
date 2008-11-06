/*
 *  csensor.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University.  All rights reserved.
 *
 * Implementation file for CSensor base classes
 * note it requires a reference to the sm shared memory datastructure (CQCNShMem)
 */

#include "csensor.h"
#include <math.h>

extern double dtime();

extern CQCNShMem* sm;
bool g_bStop = false;

CSensor::CSensor()
  : 
    m_iType(SENSOR_NOTFOUND), 
    m_port(-1),
    m_bSingleSampleDT(false),
    m_strSensor("")
{ 
}

CSensor::~CSensor()
{
    if (m_port>-1) closePort();
}

bool CSensor::getSingleSampleDT()
{
   return m_bSingleSampleDT;
}

void CSensor::setSingleSampleDT(const bool bSingle)
{
   m_bSingleSampleDT = bSingle;
}

const char* CSensor::getSensorStr() 
{
   return m_strSensor.c_str();
}

void CSensor::setSensorStr(const char* strIn)
{
    if (strIn)
       m_strSensor.assign(strIn);
    else
       m_strSensor.assign("");
}

void CSensor::setType(e_sensor esType)
{
   m_iType = esType;
}

void CSensor::setPort(const int iPort)
{
   m_port = iPort;
}

int CSensor::getPort() 
{
   return m_port;
}

void CSensor::closePort()
{
    fprintf(stdout, "Closing port...\n");
}

const e_sensor CSensor::getTypeEnum()
{
   return m_iType;
}

const char* CSensor::getTypeStr()
{
   switch (m_iType) {
     case SENSOR_MAC_PPC_TYPE1:
        return "PPC Mac Type 1";
        break;
     case SENSOR_MAC_PPC_TYPE2:
        return "PPC Mac Type 2";
        break;
     case SENSOR_MAC_PPC_TYPE3:
        return "PPC Mac Type 3";
        break;
     case SENSOR_MAC_INTEL:
        return "Intel Mac";
        break;
     case SENSOR_WIN_THINKPAD:
        return "Lenovo Thinkpad";
        break;
     case SENSOR_WIN_HP:
        return "HP Laptop";
        break;
     case SENSOR_USB:
        return "JoyWarrior 24F8 USB";
        break;
     default:
        return "Not Found";
   }
}

// this is the heart of qcn -- it gets called 50-500 times a second!
inline bool CSensor::mean_xyz(const bool bVerbose)
{
/* This subroutine finds the mean amplitude for x,y, & z of the sudden motion 
 * sensor in a window dt from time t0.
 */
   static long lLastSample = 10L;  // store last sample size, start at 10 so doesn't give less sleep below, but will if lastSample<3
   float x1,y1,z1;
   double dTimeDiff=0.0f;
   bool result = false;

   // set up pointers to array offset for ease in functions below
   float *px2, *py2, *pz2;
   double *pt2;

   if (g_bStop || !sm) throw EXCEPTION_SHUTDOWN;   // see if we're shutting down, if so throw an exception which gets caught in the sensor_thread

   px2 = (float*) &(sm->x0[sm->lOffset]);
   py2 = (float*) &(sm->y0[sm->lOffset]);
   pz2 = (float*) &(sm->z0[sm->lOffset]);
   pt2 = (double*) &(sm->t0[sm->lOffset]);
   sm->lSampleSize = 0L; 

   *px2 = *py2 = *pz2 = *pt2 = 0.0f;  // zero sample averages
 
   if (m_bSingleSampleDT) // just one sample, i.e. the hardware does the sampling a la the JoyWarrior USB sensor
      sm->lSampleSize = 1; 
		 
   // this will get executed at least once, then the time is checked to see if we have enough time left for more samples
   do {
       if (sm->lSampleSize < SAMPLE_SIZE) {  // note we only get a sample if sample size < 10
           x1 = y1 = z1 = 0.0f; 
	   result = read_xyz(x1, y1, z1);
	   *px2 += x1; 
           *py2 += y1; 
           *pz2 += z1; 
       	   if (!m_bSingleSampleDT) sm->lSampleSize++; // only increment if not a single sample sensor
        }  // done sample size stuff

       // dt is in seconds, want to slice it into 10 (SAMPLING_FREQUENCY), put into microseconds, so multiply by 100000
       // using usleep saves all the FP math every millisecond

       // sleep for dt seconds, this is where the CPU time gets used up, for dt/10 6% CPU, for dt/1000 30% CPU!
       // note the use of the "lLastSample" -- if we are getting low sample rates i.e. due to an overworked computer,
       // let's drop the sleep time dramatically and hope it can "catch up"
       //usleep((long) lLastSample < 3 ? DT_MICROSECOND_SAMPLE/100 : DT_MICROSECOND_SAMPLE);   

       usleep(DT_MICROSECOND_SAMPLE); // usually 2000, which is 2 ms or .002 seconds, 10 times finer than the .02 sec / 50 Hz sample rate
       sm->t0active = dtime(); // use the function in the util library (was used to set t0)
       dTimeDiff = sm->t0check - sm->t0active;  // t0check should be bigger than t0active by dt, when t0check = t0active we're done
   }
   while (dTimeDiff > 0.0f);

   //fprintf(stdout, "Sensor sampling info:  t0check=%f  t0active=%f  diff=%f  timeadj=%d  sample_size=%ld, dt=%f\n", 
   //   sm->t0check, sm->t0active, dTimeDiff, sm->iNumReset, sm->lSampleSize, sm->dt);
   //fprintf(stdout, "sensorout,%f,%f,%f,%d,%ld,%f\n",
   //   sm->t0check, sm->t0active, dTimeDiff, sm->iNumReset, sm->lSampleSize, sm->dt);
   //fflush(stdout);

   lLastSample = sm->lSampleSize;

   // store values i.e. divide by sample size
   *px2 /= (double) sm->lSampleSize; 
   *py2 /= (double) sm->lSampleSize; 
   *pz2 /= (double) sm->lSampleSize; 
   *pt2 = sm->t0active; // save the time into the array, this is the real clock time

   if (bVerbose) {
         fprintf(stdout, "At t0check=%f  t0active=%f :   x1=%f  y1=%f  z1=%f  sample_size=%ld  lOffset=%ld\n",
            sm->t0check, *pt2, *px2, *py2, *pz2, sm->lSampleSize, sm->lOffset);
   }

   // if active time is falling behind the checked (wall clock) time -- set equal, may have gone to sleep & woken up etc
   sm->t0check += sm->dt;   // t0check is the "ideal" time i.e. start time + the dt interval

   sm->ullSampleTotal += sm->lSampleSize;
   sm->ullSampleCount++;

   sm->fRealDT += fabs(sm->t0active - sm->t0check);

   if (fabs(dTimeDiff) > TIME_ERROR_SECONDS) { // if our times are different by a second, that's a big lag, so let's reset t0check to t0active
      if (bVerbose) {
         fprintf(stdout, "Timing error encountered t0check=%f  t0active=%f  diff=%f  timeadj=%d  sample_size=%ld, dt=%f, resetting...\n", 
            sm->t0check, sm->t0active, dTimeDiff, sm->iNumReset, sm->lSampleSize, sm->dt);
      }
#ifndef _DEBUG
      return false;   // if we're not debugging, this is a serious run-time problem, so reset time & counters & try again
#endif
   }

   return true;
}

