/*
 * csensor_hp.h
 *
 * author: Rafal Ostanek 
 * creation date: 18.09.2011
 *
 * Declaration of CSensorHP class.
 */

#ifndef _CSENSOR_HP_
#define _CSENSOR_HP_

#include "csensor.h"

typedef unsigned long (__stdcall *IsSoftwareEnabled)(void *, unsigned char *);
typedef unsigned long (__stdcall *GetRealTimeXYZ)(void *, unsigned short *, _OVERLAPPED *);

typedef unsigned char (__stdcall *stdcall_FindAccelerometerDevice)(void * *);
typedef unsigned char (__cdecl *cdecl_FindAccelerometerDevice)(void * *);

typedef stdcall_FindAccelerometerDevice FindAccelerometerDev; // depends on operating system and architecture

class CSensorHP : public CSensor {

private:
	static const BOOL x64 = FALSE;
	SHORT x,y,z,coords[3];
	LPCTSTR source;
	BOOL started;
	HMODULE hLibrary;
	HANDLE hDevice;
	FindAccelerometerDev findAccelerometerDev;
	GetRealTimeXYZ getRealTimeXYZ;
	HANDLE Find();
	BOOL loadLibrary();
	void showLastError(LPTSTR);

public:
	CSensorHP();
	~CSensorHP();
	bool detect();
	bool read_xyz(float &, float &, float &);
};

#endif