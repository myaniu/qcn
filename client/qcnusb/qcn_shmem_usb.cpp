#include "qcn_shmem_usb.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h" // boinc/lib/util.h -- for dtime()

CQCNUSBSensor::CQCNUSBSensor()
{
    clear();
}

CQCNUSBSensor::~CQCNUSBSensor()
{
}

void CQCNUSBSensor::clear()
{
   memset(this, 0x00, sizeof(CQCNUSBSensor));
   dt = DT;
   resetSampleClock();
}

// this resets the t0check & t0active, call right before you start accessing the sensor for mean_xyz
void CQCNUSBSensor::resetSampleClock()
{
    t0active = dtime(); // use the function in boinc/lib
    t0check = t0active + dt;
}

