#ifndef _CSENSOR_WIN_LAPTOP_HP_H_
#define _CSENSOR_WIN_LAPTOP_HP_H_

/*
 *  csensor-win-laptop-hp.h
 *  qcn
 *
 *  Created by Carl Christensen on 10/05/2008.
 *  Copyright 2007 Stanford University
 *
 * This file contains the declarations for the CSensor-derived Windows HP laptop accelerometer
 */

#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include "csensor.h"

#define _USE_DLL_

#ifndef _USE_DLL_
// the following is the opcode into the HP laptop accelerometer to return xyz
#define HP_XYZ_IO_CODE 0xCF50601A
#endif
// SetAccelerometerProperty code: 0xCF509FFC
// GetAccelerometerProperty code: 0xCF502000
// NotifyAccelAboutPower    code: 0xCF50A024

using namespace std;

// HP specific stuff 
// info from PE explorer on the HP accel dll
/* function prototypes and ordinals of interest
ord 3
unsigned char __stdcall FindAccelerometerDevice(void * *)

ord 5
unsigned long __stdcall GetRealTimeXYZ(void *,unsigned short *,struct _OVERLAPPED *)

ord 6
unsigned long __stdcall IsSoftwareEnabled(void *,unsigned char *)

unsigned long __stdcall SetAccelerometerProperty(void *,enum _ACCELEROMETER_PROPERTY_FLAGS,void *)
*/


// DLL function signature for HP DLL -- note the use of __stdcall so Windows does the cleanup (as opposed to __cdecl)
// Note -- the HP DLL accesses a service running on the laptop --
// if this service isn't running the DLL function access will work but the sensor data values never change
//typedef int (__stdcall *HPImportFunction)(HPSensorData* psd);

//typedef __declspec( dllimport ) unsigned char (__stdcall *HPImportFindDevice)(void **);  // ordinal 3 
//typedef __declspec( dllimport ) unsigned long (__stdcall *HPImportEnabled)(void *, unsigned char *); // ordinal 6
//typedef __declspec( dllimport ) unsigned long (__stdcall *HPImportGetXYZ)(void *, unsigned short *, struct _OVERLAPPED *); // ordinal 5

#ifdef _USE_DLL_
typedef unsigned char (__stdcall *HPImportFindDevice)(LPHANDLE);  // ordinal 3 
typedef unsigned long (__stdcall *HPImportSoftwareEnabled)(HANDLE, unsigned char *); // ordinal 6
typedef unsigned long (__stdcall *HPImportGetXYZ)(HANDLE, unsigned short *, LPOVERLAPPED); // ordinal 5
typedef unsigned long (__stdcall *HPImportGetProperty)(HANDLE, int, PDWORD); // ordinal 
typedef unsigned long (__stdcall *HPImportSetProperty)(HANDLE, int, PDWORD); // ordinal 
typedef unsigned long (__stdcall *HPImportCanSettingsChange)(HANDLE, unsigned char *);
typedef unsigned long (__stdcall *HPImportSessionChange)(HANDLE, unsigned short const *, void *);
typedef unsigned long (__stdcall *HPImportClearLogFile)(HANDLE);
typedef unsigned long (__stdcall *HPImportNotifyAccelerometerAboutPower)(HANDLE,unsigned long);
typedef void * (__stdcall *HPImportRegisterForAccelerometerDisabledEvent)(HWND,HANDLE);
typedef void * (__stdcall *HPImportRegisterForAccelerometerDiskCountChangeEvent)(HWND,HANDLE);
typedef void * (__stdcall *HPImportRegisterForAccelerometerEnabledEvent)(HWND,HANDLE);
typedef void * (__stdcall *HPImportRegisterForAccelerometerParameterChangeEvent)(HWND,HANDLE);
typedef void * (__stdcall *HPImportRegisterForAccelerometerShockEndEvent)(HWND,HANDLE);
typedef void * (__stdcall *HPImportRegisterForAccelerometerShockSignaledEvent)(HWND,HANDLE);
#endif

// this is the Windows implementation of the sensor - HP
class CSensorWinHP  : public CSensor
{
   private:   
      // private member vars
      static const char   m_cstrDLL[];   // for dll name
      HMODULE             m_WinDLLHandle;
      HANDLE              m_device;

#ifdef _USE_DLL_
        HPImportGetXYZ              m_FNGetXYZ;       
        HPImportSoftwareEnabled     m_FNSoftwareEnabled;   
        HPImportFindDevice  m_FNFindDevice;  
        HPImportGetProperty m_FNGetProperty;
        HPImportSetProperty m_FNSetProperty;
        HPImportCanSettingsChange  m_FNCanSettingsChange;
        HPImportSessionChange m_FNSessionChange;
        HPImportClearLogFile m_FNClearLogFile;
        HPImportNotifyAccelerometerAboutPower m_FNNotifyAboutPower;
        HPImportRegisterForAccelerometerDisabledEvent m_FNRegDisabledEvt;
        HPImportRegisterForAccelerometerDiskCountChangeEvent m_FNRegDiskCountChangeEvt;
        HPImportRegisterForAccelerometerEnabledEvent m_FNRegEnabledEvt;
        HPImportRegisterForAccelerometerParameterChangeEvent m_FNRegParamChangeEvt;
        HPImportRegisterForAccelerometerShockEndEvent m_FNRegShockEndEvt;
        HPImportRegisterForAccelerometerShockSignaledEvent m_FNRegShockSigEvt;
#else
      bool getDevice();  // gets the m_device directly without using the DLL
#endif
    // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
      virtual bool read_xyz(float& x1, float& y1, float& z1);  

   public:
      CSensorWinHP();
      virtual ~CSensorWinHP();

     virtual void closePort(); // closes the port if open
     virtual bool detect();   // this detects & initializes a sensor on a Mac G4/PPC or Intel laptop, sets m_iType to 0 if not found
     virtual const char* getTypeStr(int iType);  // sensor names
     virtual const char* getTypeStrShort();

};

#endif  // _CSENSOR_WIN_LAPTOP_HP_H_
