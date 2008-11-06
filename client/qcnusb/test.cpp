#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef _WIN32
#include <unistd.h>
#endif
#include <pthread.h>

#include "define.h"
#include "qcn_shmem_usb.h"
#include "qcn_util.h"
#include "shmem.h"  // boinc shmem stuff
#include "qcn_signal.h"

static CQCNUSBSensor* tsm = NULL;
static CQCNUSBState*  tsmState = NULL;

int fdPipe[2] = {-1, -1}; // first is PIPE_STATE to write state data, second is PIPE_SENSOR to read sensor data
FDSET_GROUP fdsWatch; // FDSET_GROUP is a struct taken from boinc/lib/network.h

bool writePipeState(const int iSeconds = 0, const int iMicroSeconds = 0)
{
   if (fdPipe[PIPE_STATE] == -1) return false;

   struct timeval tvTimeout; // timeout value for the select
   tvTimeout.tv_sec = iSeconds;
   tvTimeout.tv_usec = 0;

   FDSET_GROUP fdsCopy; // FDSET_GROUP is a struct taken from boinc/lib/network.h
   FD_COPY(&(fdsWatch.read_fds), &(fdsCopy.read_fds));
   FD_COPY(&(fdsWatch.write_fds), &(fdsCopy.write_fds));
   FD_COPY(&(fdsWatch.exc_fds), &(fdsCopy.exc_fds));
   fdsCopy.max_fd = fdsWatch.max_fd;

   // write the results to the sensor pipe, i.e. if sensor was found or not
   if (FD_ISSET(fdPipe[PIPE_STATE], &fdsWatch.write_fds)) {
     if (select(fdsCopy.max_fd+1, NULL, &(fdsCopy.write_fds), NULL,
       iSeconds > 0 || iMicroSeconds > 0 ? &tvTimeout : NULL) < 0) { // check the write fs (PIPE_STATE)
          fprintf(stderr, "writePipeState Level 0\n");
          return false;
     }
     // which file descriptors are ready to be read from?
     if (FD_ISSET(fdPipe[PIPE_STATE], &fdsCopy.write_fds)) {
         // file is ready to be written to
         // now write this sensor information
         int retval = write(fdPipe[PIPE_STATE], tsm, sizeof(CQCNUSBState));
         if (retval < 0) { // pipe error
            fprintf(stderr, "writePipeState Level 1\n");
            return false;
         }
         else if (!retval) { //pipe has been closed
            fprintf(stderr, "writePipeState Level 2\n");
            return false;
         }
         //else { // bytes were written to the pipe OK
         //}
     }
     else { // something wrong, just return
         fprintf(stderr, "writePipeState Level 3\n");
         return false;
     }
   }
   return true;  // must have written OK
}

bool readPipeSensor(const int iSeconds = 0, const int iMicroSeconds = 0)
{
   if (fdPipe[PIPE_SENSOR] == -1) return false;

   struct timeval tvTimeout; // timeout value for the select
   tvTimeout.tv_sec = iSeconds;
   tvTimeout.tv_usec = iMicroSeconds;

   FDSET_GROUP fdsCopy; // FDSET_GROUP is a struct taken from boinc/lib/network.h
   FD_COPY(&(fdsWatch.read_fds), &(fdsCopy.read_fds));
   FD_COPY(&(fdsWatch.write_fds), &(fdsCopy.write_fds));
   FD_COPY(&(fdsWatch.exc_fds), &(fdsCopy.exc_fds));
   fdsCopy.max_fd = fdsWatch.max_fd;

   // write the results to the sensor pipe, i.e. if sensor was found or not
   if (FD_ISSET(fdPipe[PIPE_SENSOR], &fdsWatch.read_fds)) {
     if (select(fdsCopy.max_fd+1, &(fdsCopy.read_fds), NULL, NULL,
       iSeconds > 0 || iMicroSeconds > 0 ? &tvTimeout : NULL) < 0) { // check the write fs (PIPE_STATE)
          fprintf(stderr, "readPipeSensor Level 0\n");
          return false;
     }

     // which file descriptors are ready to be read from?
     if (FD_ISSET(fdPipe[PIPE_SENSOR], &fdsCopy.read_fds)) {
        //file is ready to be read from
        int retval = read(fdPipe[PIPE_SENSOR], tsm, sizeof(CQCNUSBSensor));
        if (retval < 0) { // pipe error
           fprintf(stderr, "readPipeSensor Level 1\n");
           return false;
        }
        else if (!retval) { //pipe has been closed
           //remove the closed pipe from the set
           fprintf(stderr, "readPipeSensor Level 2\n");
           return false;
        }
     }
     else { // something wrong, just return
       fprintf(stderr, "readPipeSensor Level 3\n");
       return false;
     }
   }
   return true;  // must have read OK
}

int cleanup()
{
    fprintf(stdout, "Cleaning up...\n");
    if (tsmState) {
        tsmState->bStop = true;
        tsmState->alPID[PID_QCN] = 0L;
        try {
           writePipeState(1); 
        }
        catch(...) {
        }
        delete tsmState;
    }
    if (tsm) {
        delete tsm;
    }

    if (fdPipe[PIPE_STATE])  close(fdPipe[PIPE_STATE]);
    if (fdPipe[PIPE_SENSOR]) close(fdPipe[PIPE_SENSOR]);

    fdPipe[PIPE_STATE] = fdPipe[PIPE_SENSOR] = 0;
    tsm = NULL;
    tsmState = NULL;
    return 0;
}

void qcnusb_signal(int iSignal)
{
   fprintf(stderr, "Signal %d received, exiting...\n", iSignal);
   cleanup();
   _exit(EXIT_SIGNAL);
}

void qcnusb_sigpipe(int iSignal)
{
   fprintf(stderr, "SIGPIPE received, resetting\n");
   cleanup();
   _exit(EXIT_SIGNAL);
}

int main(int argc, char** argv)
{

#ifndef _DEBUG
    qcn_signal::InstallHandlers(qcnusb_signal);
    qcn_signal::InstallHandlerSIGPIPE(qcnusb_sigpipe);
#endif

    //fdPipe[PIPE_SENSOR] = open(QCN_USB_SENSOR, O_RDONLY | O_NONBLOCK);
    fdPipe[PIPE_SENSOR] = open(QCN_USB_SENSOR, O_RDONLY);
    if (fdPipe[PIPE_SENSOR] == -1) {
        fprintf(stderr, "Error %d opening sensor pipe %d, try later, or reboot!\n", errno, fdPipe[PIPE_SENSOR]);
        cleanup();
        return ERR_SHMEM;
    }
    tsm = new CQCNUSBSensor(); // create the tsm instance which will be used for writing to the pipe
    if (!tsm) {
        fprintf(stderr, "Failed to attach to shared memory segment for sensor data, try later, or reboot!\n");
        cleanup();
        return ERR_SHMEM;
    }
    tsmState = new CQCNUSBState(); // create the tsm instance which will be used for writing to the pipe
    if (!tsmState) {
        fprintf(stderr, "Failed to attach to shared memory segment for state data, try later, or reboot!\n");
        cleanup();
        return ERR_SHMEM;
    }

    while (fdPipe[PIPE_STATE] == -1) {
      fdPipe[PIPE_STATE]  = open(QCN_USB_STATE,  O_WRONLY | O_NONBLOCK);  
      usleep(MAIN_LOOP_SLEEP_MICROSECONDS);
    }

    // now setup fds'
    fdsWatch.zero();
    FD_SET(fdPipe[PIPE_SENSOR], &(fdsWatch.read_fds));
    FD_SET(fdPipe[PIPE_STATE],  &(fdsWatch.write_fds));
    fdsWatch.max_fd = fdPipe[PIPE_STATE] > fdPipe[PIPE_SENSOR] ? fdPipe[PIPE_STATE] : fdPipe[PIPE_SENSOR];

    // to start monitoring, set the process ID and set bStop to false
    tsmState->alPID[PID_QCN] = getpid();  // set the process ID of this routine
    tsmState->bStop = false;

    if (!writePipeState(0)) {
        fprintf(stderr, "The qcnusb service does not appear to be running, try a reboot!\n");
        cleanup();
        return ERR_SHMEM;
    }

    int i = 0;
    float x, y, z;
    double dTimeLast = 0.0f, dTimeCheck = 0.0f, dTimeActive = 0.0f; // need time vars to check against volatile shared mem vars

    while (!readPipeSensor() && i++<100) { //(iRead = read(fdPipe[PIPE_SENSOR], tsm, sizeof(CQCNUSBSensor))) <= 0 && i++ < 100) {
       usleep(100000); // this will return when something was read from the pipe or 10 seconds
    }

    //fprintf(stdout, "i=%d  bSensorFound=%s  eSensor=%d\n", i, tsm->bSensorFound ? "true" : "false", tsm->eSensor);
    if (i==100 || !tsm->bSensorFound || tsm->eSensor == SENSOR_NOTFOUND)  { // didn't get anything 
       fprintf(stderr, "No sensor found from qcnusb service\n");
       cleanup();
       return 1;
    }

    fprintf(stdout, "Sensor found of type %d\n", tsm->eSensor);

    i = 0;
    while(true) { 
       //if (i++ == 100) { cleanup(); usleep(10000000); _exit(2); }

       if (!readPipeSensor()) {
           fprintf(stderr, "Read error on sensor pipe from QCN client\n");
           cleanup();
           return 2;
       }
       /*if (dTimeCheck == tsm->t0check || tsm->t0check == 0.0f) { 
           continue;
       }*/

       dTimeCheck = tsm->t0check;

       x = tsm->x0;
       y = tsm->y0;
       z = tsm->z0;

       tsm->bReading = false;
       //tsm->readpos = 10;

       dTimeActive = tsm->t0active;
       dTimeLast = dTimeCheck;

       // we're not writing & detector found so check if the time changed
       //fprintf(stdout, "t0check=%f  t0active=%f  err=%f  x0=%f  y0=%f  z0=%f!\n", 
       //      tsm->t0check, tsm->t0active, (tsm->t0active-tsm->t0check)/(tsm->dt) * 100.0f, tsm->x0, tsm->y0, tsm->z0);
       fprintf(stdout, "t0check=%f  t0active=%f  err=%f  x0=%f  y0=%f  z0=%f!\n", 
           dTimeCheck, dTimeActive + tsm->dt, (dTimeActive + tsm->dt - dTimeCheck) / (tsm->dt) * 100.0f,
           x, y, z
       );

       i++;
       //if (i++ > 1000) break;
    }

    fprintf(stdout, "outside of mainloop\n");
    if (tsm) delete tsm;
    if (fdPipe[PIPE_STATE]) close(fdPipe[PIPE_STATE]);
    if (fdPipe[PIPE_SENSOR]) close(fdPipe[PIPE_SENSOR]);

    return 0;
}

