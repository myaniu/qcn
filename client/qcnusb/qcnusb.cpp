/*
 *  qcnusb.cpp
 *  qcnusb
 *
 *  Created by Carl Christensen on 9/17/08.
 *  Copyright 2008 Stanford University School of Earth Sciences. All rights reserved.
 *
 */

/*
 Mac notes:
   This program is a "stub" for Mac's to do the detection & monitoring of external USB sensors under BOINC.
   This is required because due to BOINC "sandboxing" of accounts (i.e. BOINC uses a lower-privileged account than
   the logged in user), the QCN Mac application is unable to access the USB HID device (i.e. external accelerometer)
   and monitor via BOINC.  This does not happen with the "standalone" wxWidgets QCNLive GUI of course, as that is run as
   the local user with appropriate permissions.

   On a Mac, probably should poll to make sure the IOKit is ready & init before going into the main loop,
   although perhaps that's overkill since most likely BOINC on the Mac wouldn't have started up before these services?
   launchd (which will launch this at startup) seems to happen after the kernel is init so presumably IOKit is ready by then
   
   launchd info at:
     http://developer.apple.com/documentation/MacOSX/Conceptual/BPSystemStartup/Articles/LaunchOnDemandDaemons.html#//apple_ref/doc/uid/TP40001762-108425
 
   Suggested plist for Mac OS X is in qcn/client/qcnusb/install/edu.stanford.qcn.qcnusb.plist
   This plist should go in /Library/LaunchDaemons/
   The qcnusb exec should reside in /Library/QCN/

*/

#include "qcnusb.h"
#include <fcntl.h>

// note this is a different shared memory segment, it's a chopped down version that will just report the latest x/y/z readings etc
CQCNUSBState*  volatile smState = NULL;
CQCNUSBSensor* volatile sm      = NULL;
CSensor*       volatile psms    = NULL;

FDSET_GROUP fdsWatch; // FDSET_GROUP is a struct taken from boinc/lib/network.h
int fdPipe[2] = {-1,-1}; // two pipes, the first for state info, the second for data
#ifdef _WIN32
   HANDLE hPipe[2] = {NULL, NULL}; // windows handles for the Windows named pipe
#endif

bool writePipeSensor(const int iSeconds, const int iMicroSeconds = 0)
{
   if (fdPipe[PIPE_SENSOR] == -1) return false;

   struct timeval tvTimeout; // timeout value for the select
   tvTimeout.tv_sec = iSeconds;
   tvTimeout.tv_usec = iMicroSeconds;

   FDSET_GROUP fdsCopy; // FDSET_GROUP is a struct taken from boinc/lib/network.h
#ifdef _WIN32
   fdsCopy = fdsWatch;  // default copy constructor should be fine
#else
   FD_COPY(&(fdsWatch.read_fds), &(fdsCopy.read_fds));
   FD_COPY(&(fdsWatch.write_fds), &(fdsCopy.write_fds));
   FD_COPY(&(fdsWatch.exc_fds), &(fdsCopy.exc_fds));
   fdsCopy.max_fd = fdsWatch.max_fd;
#endif

   // write the results to the sensor pipe, i.e. if sensor was found or not
   if (FD_ISSET(fdPipe[PIPE_SENSOR], &fdsWatch.write_fds)) {
     if (select(fdsCopy.max_fd+1, NULL, &(fdsCopy.write_fds), NULL, 
       iSeconds > 0 || iMicroSeconds > 0 ? &tvTimeout : NULL) < 0) { // check the write fs (PIPE_SENSOR)
         return false;
     }
     // which file descriptors are ready to be read from?
     if (FD_ISSET(fdPipe[PIPE_SENSOR], &fdsCopy.write_fds)) {
         // file is ready to be written to
         // now write this sensor information
         int retval = write(fdPipe[PIPE_SENSOR], sm, sizeof(CQCNUSBSensor));
         if (retval < 0) { // pipe error
            //fprintf(stderr, "writePipeSensor Level 1\n");
            return false;
         }
         else if (!retval) { //pipe has been closed
            //fprintf(stderr, "writePipeSensor Level 2\n");
            return false;
         }
#ifdef _DEBUG
         else { // bytes were written to the pipe OK
            fprintf(stdout, "SUCCESS %f %f %f %f\n", sm->x0, sm->y0, sm->z0, sm->t0);
         }
#endif
      }
      else { // something wrong, just return
        //fprintf(stderr, "writePipeSensor Level 3\n");
        return false;
      }
   } 
   return true;  // must have written OK
}

bool readPipeState(const int iSeconds, const int iMicroSeconds = 0)
{
   if (fdPipe[PIPE_STATE] == -1) return false;

   struct timeval tvTimeout; // timeout value for the select
   tvTimeout.tv_sec = iSeconds;
   tvTimeout.tv_usec = iMicroSeconds;

   FDSET_GROUP fdsCopy; // FDSET_GROUP is a struct taken from boinc/lib/network.h
#ifdef _WIN32
   fdsCopy = fdsWatch;  // default copy constructor should be fine
#else
   FD_COPY(&(fdsWatch.read_fds), &(fdsCopy.read_fds));
   FD_COPY(&(fdsWatch.write_fds), &(fdsCopy.write_fds));
   FD_COPY(&(fdsWatch.exc_fds), &(fdsCopy.exc_fds));
   fdsCopy.max_fd = fdsWatch.max_fd;
#endif

   if (FD_ISSET(fdPipe[PIPE_STATE], &fdsWatch.read_fds)) {
     if (select(fdsCopy.max_fd+1, &(fdsCopy.read_fds), NULL, NULL, 
       iSeconds > 0 || iMicroSeconds > 0 ? &tvTimeout : NULL) < 0) { // check the read fs (PIPE_STATE)
        //fprintf(stderr, "readPipeState Level 0\n");
        return false;
     }

     // which file descriptors are ready to be read from?
     if (FD_ISSET(fdPipe[PIPE_STATE], &fdsCopy.read_fds)) {
        //file is ready to be read from
        int retval = read(fdPipe[PIPE_STATE], smState, sizeof(CQCNUSBState));
        if (retval < 0) { // pipe error
           //fprintf(stderr, "readPipeState Level 1\n");
           return false;
        }
        else if (!retval) { //pipe has been closed
           //fprintf(stderr, "readPipeState Level 2\n");
           return false;
        }
/*
        else {
           fprintf(stdout, "SUCCESS - readPipeState - bStop=%s  PID0 = %d  PID1 = %d\n", smState->bStop ? "true" : "false", smState->alPID[0], smState->alPID[1]);
        }
*/
     }
     else { // something wrong, just return
        //fprintf(stderr, "readPipeState Level 3\n");
        return false;
     }
   }
   return true;  // must have read OK
}

int cleanup()
{
    fprintf(stdout, "Cleaning up...\n");
    if (smState) { 
        smState->alPID[PID_QCN] = smState->alPID[PID_USB] = 0L;
        delete smState;
        smState = NULL;
    }
    if (sm) {
        if (fdPipe[PIPE_SENSOR] > -1) {
           sm->bSensorFound = false;
           sm->eSensor = SENSOR_NOTFOUND;
        }
        delete sm;
        sm = NULL;
    }
    //FD_CLR(fdPipe[PIPE_STATE], &fdsWatch.read_fds); 
    //FD_CLR(fdPipe[PIPE_SENSOR], &fdsWatch.write_fds); 
    if (fdPipe[PIPE_STATE] > -1)  close(fdPipe[PIPE_STATE]);
    if (fdPipe[PIPE_SENSOR] > -1) close(fdPipe[PIPE_SENSOR]);
    fdPipe[PIPE_STATE] = fdPipe[PIPE_SENSOR] = -1;

    if (! access(QCN_USB_SENSOR, ALL_ACCESS)) {
        unlink(QCN_USB_SENSOR);
    }
    if (! access(QCN_USB_STATE, ALL_ACCESS)) {
        unlink(QCN_USB_STATE);
    }

    if (psms) {
       psms->closePort();
       delete psms;  // delete this object, not found
    }
    sm = NULL;
    smState = NULL;
    psms = NULL;
    fflush(stdout);  // flush buffers since me be here due to an error
    fflush(stderr);
    return 0;
}

void qcnusb_signal(int iSignal)
{
   fprintf(stderr, "Signal %d received, exiting...\n", iSignal);
   cleanup();
   _exit(EXIT_SIGNAL);
}

/*
void qcnusb_sigpipe(int iSignal)
{
   //fprintf(stderr, "SIGPIPE received, resetting\n");
   if (smState) smState->bStop = true;
   if (sm) {
      sm->bSensorFound = false;
      sm->eSensor = SENSOR_NOTFOUND;
   }
}
*/

int main(int argc, char** argv)
{
    // first off see if we just want the version number 
    if (argc == 2 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
         fprintf(stdout, "%s\n", QCN_VERSION_STRING);
         return 0;
    }

    // first install signal handlers so this service can be shut down remotely
#ifndef _DEBUG
    qcn_signal::InstallHandlers(qcnusb_signal);
    //qcn_signal::InstallHandlerSIGPIPE(qcnusb_sigpipe);   // note now ignoring SIGPIPE via default bool in above InstallHandlers()
#endif

    // now create the smaller shared mem segment to emulate the "usual" sm variable (but this just holds the latest sensor reading)
    // nicked from boinc/graphics_util2.C boinc_graphics_make_shmem routine to use memory mapped files
    int retval = 0;
    errno = 0;
#ifdef _WIN32
    /*
    hPipe[PIPE_STATE] = ::CreateNamedPipe(
        QCN_USB_STATE,
        PIPE_ACCESS_INBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | WRITE_DAC,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
        1, sizeof(CQCNUSBState), sizeof(CQCNUSBState), 1000, NULL
    );
    if (hPipe[PIPE_STATE] == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error %d creating state pipe, try later, or reboot!\n", ::GetLastError());
        cleanup();
        return ERR_SHMEM; 
    }
    */
    hPipe[PIPE_SENSOR] = ::CreateNamedPipe(
        QCN_USB_SENSOR,
        PIPE_ACCESS_OUTBOUND | FILE_FLAG_FIRST_PIPE_INSTANCE | WRITE_DAC,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
        1, sizeof(CQCNUSBSensor), sizeof(CQCNUSBSensor), 1000, NULL
    );
    if (hPipe[PIPE_SENSOR] == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error %d creating sensor pipe, try later, or reboot!\n", ::GetLastError());
        cleanup();
        return ERR_SHMEM; 
    }
#else   // Mac or Linux fifo's
    retval = mkfifo(QCN_USB_SENSOR, ALL_ACCESS); // S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if (retval == -1 && errno != EEXIST) {
        fprintf(stderr, "Error %d %d creating sensor pipe, try later, or reboot!\n", retval, errno);
        cleanup();
        return ERR_SHMEM; 
    }
    retval = mkfifo(QCN_USB_STATE, ALL_ACCESS); // S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    if (retval == -1 && errno != EEXIST) {
        fprintf(stderr, "Error %d %d creating state pipe, try later, or reboot!\n", retval, errno);
        cleanup();
        return ERR_SHMEM;
    }
    chmod(QCN_USB_SENSOR, ALL_ACCESS);
    chmod(QCN_USB_STATE,  ALL_ACCESS);
#endif

    sm = new CQCNUSBSensor(); // create the sm instance which will be used for writing to the pipe
    if (!sm) {
        fprintf(stderr, "Failed to attach to shared memory segment, try later, or reboot!\n");
        cleanup();
        return ERR_SHMEM;
    }
    smState = new CQCNUSBState(); // create the sm instance which will be used for writing to the pipe
    if (!smState) {
        fprintf(stderr, "Failed to create state info memory segment, try later, or reboot!\n");
        cleanup();
        return ERR_SHMEM;
    }

    // setup some basic vars
    sm->dt = g_DT;
    sm->bSensorFound = false;
    sm->eSensor = SENSOR_NOTFOUND;

    smState->bStop = true;
    smState->alPID[PID_USB] = getpid();

#ifdef _WIN32
#else // Mac & Linux fifo open
    fdPipe[PIPE_STATE]  = open(QCN_USB_STATE,  O_RDONLY | O_NONBLOCK);
    if (fdPipe[PIPE_STATE] == -1) {
        fprintf(stderr, "Error %d opening state pipe %d, try later, or reboot!\n", errno, fdPipe[PIPE_STATE]);
        cleanup();
        return ERR_SHMEM;
    }
#endif

    fprintf(stdout, "QCN/BOINC USB Device Monitor version %s Started - Process ID # %ld\n", 
         QCN_VERSION_STRING, smState->alPID[PID_USB]);
    fflush(stdout);

    mainLoop();

    // this will never actually return of course, it's basically a local service that will stay in the above polling loop
    // and monitor requests from the BOINC QCN application to detect & monitor an external USB sensor
    // it can exit via Ctrl+C or SIGTERM etc of course, which is handled below
    cleanup();
    return 0;
}

int mainLoop()
{
   // if shared memory was opened, monitor the smState->bStop variable in a 
   // polling loop, basically just check every .2 seconds to see if was requested to start monitoring...

   // this loop we're just polling PIPE_STATE indefinitely for changes to the smState->bStop flag
   while (true) {  // outer while loop which we should usually be in after setup
#ifdef _WIN32
       // this waits until a client has connected (read) to the PIPE_SENSOR
       if (! ::ConnectNamedPipe(hPipe[PIPE_SENSOR], NULL)) {
            usleep(MAIN_LOOP_SLEEP_MICROSECONDS);
            continue;
       }
#else
       if (fdPipe[PIPE_SENSOR] == -1) {
          fdPipe[PIPE_SENSOR] = open(QCN_USB_SENSOR, O_WRONLY | O_NONBLOCK);
          if (fdPipe[PIPE_SENSOR] == -1) {
            //fprintf(stderr, "Error %d opening sensor pipe %d, try later, or reboot!\n", errno, fdPipe[PIPE_SENSOR]);
            usleep(MAIN_LOOP_SLEEP_MICROSECONDS);
            continue;
          }  // inner if PIPE_SENSOR==-1
       }  // outer if (ConnectNamedPipe or outer if PIPE_SENSOR==-1
#endif

#ifdef _WIN32
#else
       // now setup fds' since we must have obtained a valid PIPE_SENSOR
       fdsWatch.zero();
       FD_SET(fdPipe[PIPE_SENSOR], &(fdsWatch.write_fds));
       FD_SET(fdPipe[PIPE_STATE],  &(fdsWatch.read_fds));
       fdsWatch.max_fd = fdPipe[PIPE_STATE] > fdPipe[PIPE_SENSOR] ? fdPipe[PIPE_STATE] : fdPipe[PIPE_SENSOR];
#endif
       if (readPipeState(1) && ! smState->bStop) { // we've been requested to detect and monitor a USB accelerometer!
          pollUSB();
       } 
       usleep(MAIN_LOOP_SLEEP_MICROSECONDS);
   } // outer while
   return 0;
}

// detect the sensor & write values to shared memory
bool pollUSB()
{  
   // currently only one USB detector for Mac's, the JoyWarrios 24F8
   int iLoop = 0;
   bool bPipeErr = false;

#ifdef _DEBUG
       fprintf(stdout, "in detectUSB()\n");
#endif

   const int iMaxUSB = 1;
   //sm->writepos = 10;
   if (!qcn_signal::PIDRunning(smState->alPID[PID_QCN])) {
#ifdef _DEBUG
       fprintf(stderr, "qcnusb1: QCN application PID # %ld not found, exiting...\n", smState->alPID[PID_QCN]);
#endif
       smState->bStop = true;
       return false;
   }
   for (int i = 0; i < iMaxUSB; i++)  {
       switch(i) {
           case 0:
#ifdef _WIN32
              psms = (CSensor*) new CSensorWinUSBJW();
#else
#ifdef __APPLE_CC__
              psms = (CSensor*) new CSensorMacUSBJW();
#endif
#endif
              break;
       }

       // ok, instantiated a CSensor object, now see if this sensor is actually detected
       if (psms && psms->detect()) {
           sm->bSensorFound = true;
           sm->eSensor = psms->getTypeEnum();
           break;
       }
       else { // not found
           sm->eSensor = SENSOR_NOTFOUND;
           sm->bSensorFound = false;
           // if here we need to delete the pointer to try again in the loop (or just cleanup in general if nothing found)
           if (psms) {
              psms->closePort();
              delete psms;  // delete this object, not found i.e. detect() == false
           }
           psms = NULL;
           // go through loop again to find the next sensor
       }
   }

   if (!writePipeSensor(1) || !psms)  { 
        bPipeErr = true;
        goto PipeErr;
   }

   // if we made it here we have a sensor of type eSensor (psms->getTypeEnum())
   // so now just keep calling mean_xyz
   sm->resetSampleClock();

   while (!smState->bStop) { // breaks on detection error or qcn app isn't running
      if (++iLoop == 10) { // this checks every 10 mean_xyz's, or .2 seconds
            iLoop = 0;
            if (readPipeState(0, 10000) && smState->bStop) { 
                break;
            }
            if (!qcn_signal::PIDRunning(smState->alPID[PID_QCN])) {
#ifdef _DEBUG
                fprintf(stderr, "qcnusb2: QCN application PID # %ld not found, exiting...\n", smState->alPID[PID_QCN]);
#endif
                smState->bStop = true;
                break; // get out of while loop
            }
      }
      // get the sensor readings
      try {
         if (!psms->mean_xyz()) {  
            fprintf(stderr, "pollUSB(): mean_xyz() returned false\n");
            break;  // do mean_xyz so all the DT timing is taken care of, read_xyz would be too fast (up to 500Hz) too pass through?
         }
         // CMC - the following is causing massive numbers of resets, so probably isn't useful
         // set these flags to verify sensor is still valid, the writePipeSensor below will send it to the client process
         //sm->eSensor = psms->getTypeEnum();
         //sm->bSensorFound = true; 
      }
      catch(...) {
         fprintf(stderr, "pollUSB(): Exception in mean_xyz()\n");
         break;
      }

      // write the sensor data to the pipe
      if (!writePipeSensor(1)) {
         bPipeErr = true;
         goto PipeErr;  // error pipe lost
      }

#ifdef _DEBUG  
      fprintf(stdout, "t0check=%f  t0active=%f  err=%f  x0=%f  y0=%f  z0=%f!\n", 
           sm->t0check, sm->t0active + sm->dt, (sm->t0active + sm->dt - sm->t0check)/(sm->dt) * 100.0f, 
           sm->x0, sm->y0, sm->z0);
#endif
   }

   // OK, we broke out of the loop so most likely the QCN app has exited
   // now write this sensor type information if we didn't exit due to a pipe error (bPipeErr=true)

 PipeErr:
   // fprintf(stderr, "Error in writePipeSensor / pollUSB()\n");
   sm->eSensor = SENSOR_NOTFOUND;
   sm->bSensorFound = false;
   smState->bStop = true;
   if (psms) {
       psms->closePort(); 
       delete psms;  // delete this object, not found i.e. detect() == false
   }
   psms = NULL;
   if (!bPipeErr) { // have an OK pipe
      writePipeSensor(1);
   }
   return false;
}

