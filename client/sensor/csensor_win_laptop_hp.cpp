/*
 *  csensor_win_laptop_hp.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 10/05/2008.
 *  Copyright 2008 Stanford University.  All rights reserved.
 *
 * Implementation file for Windows HP laptop sensor classes
 * 
 * NB: much of the implementatoin done by Rafal Ostanek  rostanek@op.pl
 *     since HP seems to have changed their security model from my old version
 *
 * The HP sensor is an 8-bit 3D sensor, but seems to be very slow, ie values stay the same for a second or more,
 * although it does seem to pick up "deltas" for trigger detection
 *
 *
 */

#include "csensor_win_laptop_hp.h"

// HP DLL to access, they also must be running the HP service
// this would have to be in the PATH
const char* CSensorWinHP::m_cstrDLL = {"accelerometerdll.dll"};   


void CSensorWinHP::Init()
{
        memset(m_coords, 0x00, sizeof(unsigned short) * 3);
        memset(&m_overlapped, 0x00, sizeof(OVERLAPPED));
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
   if (m_hDevice) {
	   ::CloseHandle(m_hDevice);
	   m_hDevice = NULL;
   }
   if (m_overlapped.hEvent) {
	   ::CloseHandle(m_overlapped.hEvent);
	   m_overlapped.hEvent = NULL;
   }
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
   float x,y,z;
   if ( (bFound = this->LoadLibrary()) && read_xyz(x,y,z) ) { // this checks for the HP DLL existence and function pointers into the DLL 
      this->setType(SENSOR_WIN_HP);
      setSingleSampleDT(true);
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

        if (!m_overlapped.hEvent) {
			m_overlapped.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
            m_getRealTimeXYZ(m_hDevice, (unsigned short * )m_coords, &m_overlapped);
            ::ResetEvent(m_overlapped.hEvent);
        }

		// 8-bit -- range is 0 to 255
		x1 = (((float) m_coords[0] - 127.5f) / 63.75f) * EARTH_G;
		y1 = (((float) m_coords[1] - 127.5f) / 63.75f) * EARTH_G;
		z1 = (((float) m_coords[2] - 127.5f) / 63.75f) * EARTH_G;

#if 0 //def _DEBUG
		static float max[3] = {-9990.,-9990.,-99999.0};
		static float min[3] = {99990.,999990.,9999.0};
		float test[3] = {x1, y1, z1};
		for (int j = 0; j < 3; j++) {
			if (max[j] < test[j]) max[j] = test[j];
			if (min[j] > test[j]) min[j] = test[j];
		}
		fprintf(stdout, "x=%f  y=%f  z=%f   max=[%f, %f, %f]   min=[%f, %f, %f]\n",
			      test[0], test[1], test[2], max[0], max[1], max[2], min[0], min[1], min[2]);
#endif
        return true;
}


