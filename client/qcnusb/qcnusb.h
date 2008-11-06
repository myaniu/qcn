/*
 *  qcnusb.h
 *  qcnusb
 *
 *  Created by Carl Christensen on 9/17/08.
 *  Copyright 2008 Stanford University School of Earth Sciences. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32  // the windows library should have select() & pthread functions
#include <winsock2.h>
#else
#include <sys/select.h>
#include <pthread.h>
#endif

#include <errno.h>
//#include <ctime.h>

#include "define.h"
#include "qcn_shmem_usb.h"
#include "qcn_util.h"
#include "shmem.h"  // boinc shmem stuff

#include "qcn_signal.h"

#include "csensor.h"

#ifdef _WIN32
#include "csensor_win_usb_jw.h"
#else
#ifdef __APPLE_CC__
#include "csensor_mac_usb_jw.h"
#endif
#endif

int main(int argc, char** argv);
bool pollUSB(); // detect & monitor the USB accelerometer if present
int mainLoop();
int cleanup();
void qcnusb_sigpipe(int iSignal);
void qcnusb_signal(int iSignal);

// basic signal handling a la boinc/lib/diagnostics.C
void qcn_catch_signal(int signal);
int qcn_install_signal_handlers(); 
void qcn_set_signal_handler(int sig, void(*handler)(int));

// process ID detection
bool IsPIDRunning(const long lPID);
bool check_qcn_running(const long lPID);


