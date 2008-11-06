#ifndef _CQCNSHMEMUSB_H_
#define _CQCNSHMEMUSB_H_
// Carl Christensen, copyright 2007 Stanford University
// this class will be instantiated in shared memory, for updating of sensor data and sharing with an external OpenGL graphics app
// NB:  this is a small shared mem region used for the USB stub "driver" program

#include "boinc_api.h"
#include "define.h"

struct CQCNUSBState
{
  public:
    // PID_USB is for the PID of the USB sensor, PID_QCN is for the PID of the BOINC QCN application
   CQCNUSBState() { alPID[PID_USB] = alPID[PID_QCN] = 0L; bStop = true; };
   long alPID[2];
   bool bStop;
};

/*
struct CQCNUSBType
{
  public:
   CQCNUSBType() { bSensorFound = false; eSensor = SENSOR_NOTFOUND; };
   bool bSensorFound;
   e_sensor eSensor;     // an enum of the sensor type
};
*/

struct CQCNUSBSensor
{
  public:
    CQCNUSBSensor();
    ~CQCNUSBSensor();
    void clear();
    void resetSampleClock();

    // two vars for synchronization between the multiple processes (i.e. the qcnusb "server" that does the USB access, and the BOINC/QCN app)
    //int readpos;   // if readpos >=8 it's safe to write to first 8 vars -- readpos only set by the BOINC QCN app (or test.cpp)
    //int writepos;  // if writepos >=8 it's safe to read the first 8 vars -- writepos only set by the qcnusb program

    // arrays for the timeseries data -- will be in shared memory to share between procs, i.e. main app & graphics app
   float  x0;
   float  y0;
   float  z0;
   double t0;
   double dt;

   double t0active; // for use by the sensor polling, this value is the real system time (unadjusted from server time)
   double t0check;  // used to see what the timing error is from constant polling of sensor, it's start time + # of dt's/lOffsets
   double tstart;   // start time

    // note the use ofgfor vars that can be read/written by other threads/procs
   bool bWriting;
   bool bReading;

    //struct qcn_usb_msg_queue msgq;  // preserves values between reads & writes

   long lSampleSize; // the sample size for this reading at lOffset
   unsigned long long ullSampleCount;  // used to compute a running average of sample count
   unsigned long long ullSampleTotal;  // used to compute a running average of sample count

   float fRealDT;
   int iNumReset;

   bool bSensorFound;
   e_sensor eSensor;     // an enum of the sensor type

   //double cpu_time;
   //double clock_time;
};

/*
// trick for easier multiprocess communication 
typedef struct ringbuffer {
    CQCNShMemUSB sm;
    int buflen;
    int readpos;
    int writepos;
} *ringbuffer;
 
#define BYTES_TO_READ(ringbuffer) (ringbuffer->writepos - \
    ringbuffer->readpos + \
    ((ringbuffer->readpos > ringbuffer->writepos) * \
        (ringbuffer->buflen)))
 
#define BYTES_TO_WRITE(ringbuffer) (ringbuffer->readpos - \
    ringbuffer->writepos + \
    ((ringbuffer->writepos >= ringbuffer->readpos) * \
        ringbuffer->buflen) - 1)

*/

#endif
