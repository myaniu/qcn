#if 0
/*
 *  csensor_win_laptop_hp.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 10/05/2008.
 *  Copyright 2008 Stanford University.  All rights reserved.
 *
 * Implementation file for Windows HP laptop sensor classes
 * 
 * NB: much of the implementatoin done by Rafal Ostenak  rostanek@op.pl
 *     since HP seems to have changed their security model from my old version
 */

#include "csensor_win_laptop_hp.h"

// HP DLL to access, they also must be running the HP service
// this would have to be in the PATH
const char CSensorWinHP::m_cstrDLL[] = {"accelerometerdll.dll"};   


CSensorWinHP::Init()
{
        memset(m_coords, 0x00, sizeof(unsigned short) * 3);
        m_bStarted = false;
        m_hLibrary = NULL;
        m_hDevice = NULL;
        m_getRealTimeXYZ = NULL;
        m_findAccelerometerDev = NULL;
}

CSensorWinHP::CSensorWinHP()
  : CSensor()
{ 
   Init();
}

CSensorWinHP::~CSensorWinHP()
{
   closePort();
}

void CSensorWinHP::closePort()
{
   if (m_hLibrary) {
     ::FreeLibrary(m_hLibrary);
     m_hLibrary = NULL;
   }

   if (getPort() > -1) {
      setPort();
   }
   Init();
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

        m_hLibrary = ::LoadLibrary(m_cstrDLL);
        //showLastError(TEXT("LoadLibrary"));

        if (!m_hLibrary) {
         //       printf("Loading DLL failed.");
                return false;
        } 
        //else {
          //      printf("Loaded library AccDll: %x\n",m_hLibrary);
        //}

        //isSoftwareEnabled = (IsSoftwareEnabled) GetProcAddress(m_hLibrary,"?IsSoftwareEnabled@@YAKPEAXPEAE@Z");
        //printf("isSoftwareEnabled = %x\n LastError %d\n",isSoftwareEnabled, GetLastError());

#ifdef _WIN64
                m_getRealTimeXYZ = (GetRealTimeXYZ) GetProcAddress(m_hLibrary,"?GetRealTimeXYZ@@YAKPEAXPEAGPEAU_OVERLAPPED@@@Z"); // x64
#else
                m_getRealTimeXYZ = (GetRealTimeXYZ) GetProcAddress(m_hLibrary,"?GetRealTimeXYZ@@YGKPAXPAGPAU_OVERLAPPED@@@Z"); // x86
#endif
        //showLastError(TEXT("GetProcAddress(GetRealTimeXYZ...)"));

        if (!m_getRealTimeXYZ) {
            return false;
        }

#ifdef _WIN64
                m_findAccelerometerDev = (FindAccelerometerDev) GetProcAddress(m_hLibrary,"?FindAccelerometerDevice@@YAEPEAPEAX@Z"); // x64
#else
                m_findAccelerometerDev = (FindAccelerometerDev) GetProcAddress(m_hLibrary, "?FindAccelerometerDevice@@YGEPAPAX@Z"); // x86
#endif
        //showLastError(TEXT("GetProcAddress(FindAccelerometerDevice...)"));

        if (!m_findAccelerometerDev) {
                return false;
        }
        m_findAccelerometerDev(&m_hDevice);
        if (!m_hDevice) // NULL device
           return false;

        return true;
}

bool CSensorWinHP::read_xyz(float& x1, float& y1, float& z1)
{

    // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)

        if (!m_bStarted) {
                OVERLAPPED overlapped;
                overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                m_getRealTimeXYZ(m_hDevice, (unsigned short * )m_coords, &overlapped);

                ::ResetEvent(overlapped.hEvent);

                m_bStarted = TRUE;
        }
        x1 = m_coords[0];
        y1 = m_coords[1];
        z1 = m_coords[2];
        return true;
}


