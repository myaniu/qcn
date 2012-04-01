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

using namespace std;

// HP specific stuff 
// function prototypes and ordinals of interest
// Note -- the HP DLL accesses a service running on the laptop --
// if this service isn't running the DLL function access will work but the sensor data values never change
//typedef int (__stdcall *HPImportFunction)(HPSensorData* psd);

//typedef __declspec( dllimport ) unsigned char (__stdcall *HPImportFindDevice)(void **);  // ordinal 3 
//typedef __declspec( dllimport ) unsigned long (__stdcall *HPImportEnabled)(void *, unsigned char *); // ordinal 6
//typedef __declspec( dllimport ) unsigned long (__stdcall *HPImportGetXYZ)(void *, unsigned short *, struct _OVERLAPPED *); // ordinal 5

// most of the implementation from Rafal Ostanek

typedef unsigned long (__stdcall *IsSoftwareEnabled)(void *, unsigned char *);
typedef unsigned long (__stdcall *GetRealTimeXYZ)(void *, unsigned short *, _OVERLAPPED *);

typedef unsigned char (__stdcall *stdcall_FindAccelerometerDevice)(void * *);
typedef unsigned char (__cdecl *cdecl_FindAccelerometerDevice)(void * *);

typedef stdcall_FindAccelerometerDevice FindAccelerometerDev; // depends on operating system and architecture

// end Rafal Ostanek


// this is the Windows implementation of the sensor - HP
class CSensorWinHP  : public CSensor
{
   private: 
#ifdef _WIN64
        static const bool x64 = true;
#else
        static const bool x64 = false;
#endif
        short x,y,z,coords[3];
        LPCTSTR source;
        bool started;
        HMODULE hLibrary;
        HANDLE hDevice;
        FindAccelerometerDev findAccelerometerDev;
        GetRealTimeXYZ getRealTimeXYZ;
        HANDLE Find();
        bool LoadLibrary();
        void showLastError(LPTSTR);

        const char m_cstrDLL[64];

   public:
      CSensorWinHP();
      virtual ~CSensorWinHP();

      virtual void closePort(); // closes the port if open
      virtual bool detect();   // this detects & initializes a sensor on a Mac G4/PPC or Intel laptop, sets m_iType to 0 if not found

      // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
      virtual bool read_xyz(float& x1, float& y1, float& z1);  

};

#endif  // _CSENSOR_WIN_LAPTOP_HP_H_
