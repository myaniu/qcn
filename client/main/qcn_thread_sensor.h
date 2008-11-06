#ifndef _QCN_THREAD_SENSOR_H_
#define _QCN_THREAD_SENSOR_H_

// functions to be used for the sensor thread

#ifdef _WIN32
DWORD WINAPI QCNThreadSensor(LPVOID);
#else
extern void* QCNThreadSensor(void*);
/*
#ifdef __APPLE_CC__
   // init objective-c
   extern "C" void _objcInit();  
#endif
*/
#endif

#endif

