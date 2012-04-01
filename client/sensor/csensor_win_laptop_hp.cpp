#if 0
/*
 *  csensor_win_laptop_hp.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 10/05/2008.
 *  Copyright 2008 Stanford University.  All rights reserved.
 *
 * Implementation file for Windows HP laptop sensor classes
 */

#include "csensor_win_laptop_hp.h"

// HP DLL to access, they also must be running the HP service
// this would have to be in the PATH
const char CSensorWinHP::m_cstrDLL[] = {"accelerometerdll.dll"};   

CSensorWinHP::CSensorWinHP()
  : CSensor()
{ 
        coords[0] = 0;
        coords[1] = 0;
        coords[2] = 0;
        started = false;
        hLibrary = NULL;
        hDevice = NULL;
        getRealTimeXYZ = NULL;
        findAccelerometerDev = NULL;
}

CSensorWinHP::~CSensorWinHP()
{
   closePort();
}

void CSensorWinHP::closePort()
{
   getRealTimeXYZ = NULL;
   findAccelerometerDev = NULL;
   hDevice = NULL;
   started = false;

   if (hLibrary) {
     ::FreeLibrary(hLibrary);
     hLibrary = NULL;
   }

   if (getPort() > -1) {
      setPort();
   }
}

bool CSensorWinHP::detect()
{
   bool bFound;
   if ( (bFound = this->LoadLibrary()) ) { // this checks for the HP DLL existence and function pointers into the DLL 
      this->setType(SENSOR_WIN_HP);
      setSingleSampleDT(false);
    }
    else {
      closePort(); // close handles if necessary
    }
    return bFound;
}

bool CSensorWinHP::LoadLibrary() 
{
        DWORD attribs = ::GetFileAttributes(m_cstrDLL);
        //showLastError(TEXT("GetFileAttributes"));
        //printf("File attributes: %d\n", attribs);

        //printf("LoadLibrary(%S)\n", m_cstrDLL);

        hLibrary = ::LoadLibrary(m_cstrDLL);
        //showLastError(TEXT("LoadLibrary"));

        if (!hLibrary) {
         //       printf("Loading DLL failed.");
                return false;
        } 
        //else {
          //      printf("Loaded library AccDll: %x\n",hLibrary);
        //}

        //isSoftwareEnabled = (IsSoftwareEnabled) GetProcAddress(hLibrary,"?IsSoftwareEnabled@@YAKPEAXPEAE@Z");
        //printf("isSoftwareEnabled = %x\n LastError %d\n",isSoftwareEnabled, GetLastError());

        if (x64) {
                getRealTimeXYZ = (GetRealTimeXYZ) GetProcAddress(hLibrary,"?GetRealTimeXYZ@@YAKPEAXPEAGPEAU_OVERLAPPED@@@Z"); // x64
        } else {
                getRealTimeXYZ = (GetRealTimeXYZ) GetProcAddress(hLibrary,"?GetRealTimeXYZ@@YGKPAXPAGPAU_OVERLAPPED@@@Z"); // x86
        }
        //showLastError(TEXT("GetProcAddress(GetRealTimeXYZ...)"));

        if (!getRealTimeXYZ) {
            return false;
        }

        if (x64) {
                findAccelerometerDev = (FindAccelerometerDev) GetProcAddress(hLibrary,"?FindAccelerometerDevice@@YAEPEAPEAX@Z"); // x64
        } else {
                findAccelerometerDev = (FindAccelerometerDev) GetProcAddress(hLibrary, "?FindAccelerometerDevice@@YGEPAPAX@Z"); // x86
        }
        //showLastError(TEXT("GetProcAddress(FindAccelerometerDevice...)"));

        if (!findAccelerometerDev) {
                return false;
        }
        findAccelerometerDev(&hDevice);
        if (!hDevice) // NULL device
           return false;

        return true;
}

bool CSensorWinHP::read_xyz(float& x1, float& y1, float& z1)
{

    // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)

        if (!started) {
                OVERLAPPED overlapped;
                overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                getRealTimeXYZ(hDevice, (unsigned short * )coords, &overlapped);

                ResetEvent(overlapped.hEvent);

                started = TRUE;
        }
        x1 = coords[0];
        y1 = coords[1];
        z1 = coords[2];
        return true;
}


