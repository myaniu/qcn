/*
 * csensor_hp.cpp
 *
 * author: Rafal Ostanek 
 * creation date: 18.09.2011
 *
 * Implementation of CSensorHP class.
 */
 
#include "define.h"
#include "csensor_hp.h"
#include <Strsafe.h>

CSensorHP::CSensorHP() : CSensor() {
	coords[0] = 0;
	coords[1] = 0;
	coords[2] = 0;
	started = FALSE;
	source = TEXT("accelerometerdll.dll");

	this->setType(SENSOR_WIN_HP);
	this->loadLibrary();
}

CSensorHP::~CSensorHP() {

	if (hLibrary != NULL) {
		FreeLibrary(hLibrary);
	}
}

bool CSensorHP::detect() {
	if (hLibrary == NULL) {
		printf("Library not loaded!\n");
		return false;
	} else if (findAccelerometerDev == NULL) {
		printf("Function findAccelerometerDevice not found!\n");
		return false;
	}
	
	findAccelerometerDev(&hDevice);

	return true;
}

bool CSensorHP::read_xyz(float& x1, float& y1, float& z1) {

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

BOOL CSensorHP::loadLibrary() {
	
	DWORD attribs = GetFileAttributes(source);
	showLastError(TEXT("GetFileAttributes"));
	printf("File attributes: %d\n", attribs);

	printf("LoadLibrary(%S)\n", source);
	
	hLibrary = LoadLibrary(source);
	showLastError(TEXT("LoadLibrary"));

	if (hLibrary == NULL) {
		printf("Loading DLL failed.");
		return FALSE;
	} else {
		printf("Loaded library AccDll: %x\n",hLibrary);
	}

	//isSoftwareEnabled = (IsSoftwareEnabled) GetProcAddress(hLibrary,"?IsSoftwareEnabled@@YAKPEAXPEAE@Z");
	//printf("isSoftwareEnabled = %x\n LastError %d\n",isSoftwareEnabled, GetLastError());

	if (x64) {
		getRealTimeXYZ = (GetRealTimeXYZ) GetProcAddress(hLibrary,"?GetRealTimeXYZ@@YAKPEAXPEAGPEAU_OVERLAPPED@@@Z"); // x64
	} else {
		getRealTimeXYZ = (GetRealTimeXYZ) GetProcAddress(hLibrary,"?GetRealTimeXYZ@@YGKPAXPAGPAU_OVERLAPPED@@@Z"); // x86
	}
	showLastError(TEXT("GetProcAddress(GetRealTimeXYZ...)"));

	if (getRealTimeXYZ == NULL) {
		return FALSE;
	}

	if (x64) {
		findAccelerometerDev = (FindAccelerometerDev) GetProcAddress(hLibrary,"?FindAccelerometerDevice@@YAEPEAPEAX@Z"); // x64
	} else {	
		findAccelerometerDev = (FindAccelerometerDev) GetProcAddress(hLibrary, "?FindAccelerometerDevice@@YGEPAPAX@Z"); // x86
	}
	showLastError(TEXT("GetProcAddress(FindAccelerometerDevice...)"));

	if (findAccelerometerDev == NULL) {
		return FALSE;
	}

	return TRUE;
}

void CSensorHP::showLastError(LPTSTR lpszFunction) {
	
	LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();
	if (dw==0) return;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    //MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
	printf("%S\n", lpDisplayBuf);
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
	SetLastError(0);
}