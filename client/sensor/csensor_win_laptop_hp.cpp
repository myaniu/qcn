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

#ifdef _USE_DLL_
/*
HWND g_HWND = NULL;
HANDLE g_HEVENT = NULL;

BOOL CALLBACK FindMyChildHWND(          
    HWND chwnd,
    LPARAM clParam
)
{
    DWORD dwProc = 0;
    DWORD dwThread = ::GetWindowThreadProcessId(chwnd, &dwProc);
    if (dwThread == clParam || dwProc == ::GetCurrentProcessId()) {
//    if (dwThread == clParam) {
        g_HWND = chwnd;
        return FALSE; // found it, stop enumerating
    }
    return TRUE;    
}

BOOL CALLBACK FindMyThreadHWND( 
    HWND hwnd,
    LPARAM lParam
)
{
    DWORD dwProc = 0;
    DWORD dwThread = ::GetWindowThreadProcessId(hwnd, &dwProc);
    if (dwThread == lParam || dwProc == ::GetCurrentProcessId()) {
//    if (dwThread == lParam) {
        g_HWND = hwnd;
        return FALSE; // found it, stop enumerating
    }
    return TRUE; //::EnumChildWindows(hwnd, FindMyChildHWND, lParam);
}
*/
#else
/*
VOID CALLBACK read_xyz_completion_routine(
  DWORD dwErrorCode,
  DWORD dwNumberOfBytesTransfered,
  LPOVERLAPPED lpOverlapped)
{
    if (dwErrorCode) {
        fprintf(stdout, "completion retcode = %ld\n", dwErrorCode);
    }
}
*/
#endif

CSensorWinHP::CSensorWinHP()
  : CSensor()
#ifdef _USE_DLL_
     , m_device(INVALID_HANDLE_VALUE),
       m_WinDLLHandle(NULL), 
       m_FNGetXYZ(NULL),
       m_FNSoftwareEnabled(NULL),   
       m_FNFindDevice(NULL),
       m_FNGetProperty(NULL),
       m_FNSetProperty(NULL),
       m_FNCanSettingsChange(NULL),
       m_FNSessionChange(NULL),
       m_FNClearLogFile(NULL),
       m_FNNotifyAboutPower(NULL),
       m_FNRegDisabledEvt(NULL),
       m_FNRegDiskCountChangeEvt(NULL),
       m_FNRegEnabledEvt(NULL),
       m_FNRegParamChangeEvt(NULL),
       m_FNRegShockEndEvt(NULL),
       m_FNRegShockSigEvt(NULL)
#endif
{ 
}

CSensorWinHP::~CSensorWinHP()
{
  closePort();
}

void CSensorWinHP::closePort()
{
    if (m_device != INVALID_HANDLE_VALUE) {
        ::CloseHandle(m_device);
        m_device = INVALID_HANDLE_VALUE;
    }

    // close hp dll if needed
    if (m_WinDLLHandle) {
	    ::FreeLibrary(m_WinDLLHandle);
	    //::CloseHandle(m_WinDLLHandle);
	    m_WinDLLHandle = NULL;
    }
    if (getPort() > -1) {
        fprintf(stdout, "Port closed!\n");
        fflush(stdout);
        setPort();
    }
}

bool CSensorWinHP::detect()
{

//#ifndef _DEBUG   // always returns false for production Release (NDEBUG) builds
    return false;
//#endif

    // basically, just try to open the sensor.dll and read a value, if it fails, they don't have a HP
    setType();
    setPort();

    m_device = INVALID_HANDLE_VALUE; // initialize the device pointer/handle/whatever the hell it is...

#ifdef _USE_DLL_
    // access via the DLL file
	m_WinDLLHandle = ::LoadLibrary(m_cstrDLL);

    if (!m_WinDLLHandle) {
       return false;
    }
 
    // Get function pointers
    m_FNFindDevice = (HPImportFindDevice) ::GetProcAddress(m_WinDLLHandle, 
        "?FindAccelerometerDevice@@YGEPAPAX@Z");
    if (m_FNFindDevice == NULL) {
        closePort();  // frees the handles
        return false;
    }
    m_FNSoftwareEnabled = (HPImportSoftwareEnabled) ::GetProcAddress(m_WinDLLHandle, 
        "?IsSoftwareEnabled@@YGKPAXPAE@Z");
    if (m_FNSoftwareEnabled == NULL) {
        closePort();  // frees the handles
        return false;
    }
    m_FNGetXYZ = (HPImportGetXYZ) ::GetProcAddress(m_WinDLLHandle, 
        "?GetRealTimeXYZ@@YGKPAXPAGPAU_OVERLAPPED@@@Z");
    if (m_FNGetXYZ == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNGetProperty = (HPImportGetProperty) ::GetProcAddress(m_WinDLLHandle, 
        "?GetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z");
    if (m_FNGetProperty == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNSetProperty = (HPImportSetProperty) ::GetProcAddress(m_WinDLLHandle, 
        "?SetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z");
    if (m_FNSetProperty == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNCanSettingsChange = (HPImportCanSettingsChange) ::GetProcAddress(m_WinDLLHandle, 
        "?CanSettingsChange@@YGKPAXPAE@Z");
    if (m_FNCanSettingsChange == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNSessionChange = (HPImportSessionChange) ::GetProcAddress(m_WinDLLHandle, 
        "?SessionChange@@YGKPAXPBGE@Z");
    if (m_FNSessionChange == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNClearLogFile = (HPImportClearLogFile) ::GetProcAddress(m_WinDLLHandle, 
        "?ClearLogFile@@YGKPAX@Z");
    if (m_FNClearLogFile == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNNotifyAboutPower = (HPImportNotifyAccelerometerAboutPower) ::GetProcAddress(m_WinDLLHandle, 
        "?NotifyAccelerometerAboutPower@@YGKPAXK@Z");
    if (m_FNNotifyAboutPower == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNRegDisabledEvt = (HPImportRegisterForAccelerometerDisabledEvent) ::GetProcAddress(m_WinDLLHandle, 
        "?RegisterForAccelerometerDisabledEvent@@YGPAXPAUHWND__@@PAX@Z");
    if (m_FNRegDisabledEvt == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNRegDiskCountChangeEvt = (HPImportRegisterForAccelerometerDiskCountChangeEvent) ::GetProcAddress(m_WinDLLHandle, 
        "?RegisterForAccelerometerDiskCountChangeEvent@@YGPAXPAUHWND__@@PAX@Z");
    if (m_FNRegDiskCountChangeEvt == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNRegEnabledEvt = (HPImportRegisterForAccelerometerEnabledEvent) ::GetProcAddress(m_WinDLLHandle, 
        "?RegisterForAccelerometerEnabledEvent@@YGPAXPAUHWND__@@PAX@Z");
    if (m_FNRegEnabledEvt == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNRegParamChangeEvt = (HPImportRegisterForAccelerometerParameterChangeEvent) ::GetProcAddress(m_WinDLLHandle, 
        "?RegisterForAccelerometerParameterChangeEvent@@YGPAXPAUHWND__@@PAX@Z");
    if (m_FNRegParamChangeEvt == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNRegShockEndEvt = (HPImportRegisterForAccelerometerShockEndEvent) ::GetProcAddress(m_WinDLLHandle, 
        "?RegisterForAccelerometerShockEndEvent@@YGPAXPAUHWND__@@PAX@Z");
    if (m_FNRegShockEndEvt == NULL) {
        closePort();  // frees the handles
        return false;
    }

    m_FNRegShockSigEvt = (HPImportRegisterForAccelerometerShockSignaledEvent) ::GetProcAddress(m_WinDLLHandle, 
        "?RegisterForAccelerometerShockSignaledEvent@@YGPAXPAUHWND__@@PAX@Z");
    if (m_FNRegShockSigEvt == NULL) {
        closePort();  // frees the handles
        return false;
    }

    // OK all the DLL exports found, now try to use them!
    BYTE byteTest[8];
    memset(byteTest, 0x00, 8);
	DWORD dwError = m_FNFindDevice(&m_device);
    if (dwError == 1 && m_device != INVALID_HANDLE_VALUE) {
        dwError  = m_FNSoftwareEnabled(m_device, byteTest);
        if (!dwError && byteTest[0] == 0x01) { // success, try to read an xyz value
      /*
            g_HWND = NULL;
            DWORD dwPID = ::GetCurrentThreadId();
            ::EnumWindows(FindMyThreadHWND, dwPID);
            g_HEVENT = NULL;
            if (g_HWND) {
               g_HEVENT = m_FNRegEnabledEvt(g_HWND, m_device);
            }
       */
            // try to read a value, if fails, reset device as there is a problem
            float x,y,z;
            if (!read_xyz(x,y,z)) {
               ::CloseHandle(m_device);
                m_device = INVALID_HANDLE_VALUE;
            }
        }
        else { // failure, reset m_device
            ::CloseHandle(m_device);
            m_device = INVALID_HANDLE_VALUE;
        }
    }
    else {
        if (m_device != INVALID_HANDLE_VALUE) {
            ::CloseHandle(m_device);
        }
        m_device = INVALID_HANDLE_VALUE;
    }
#else
    if (! getDevice()) return false;  // direct access of accelerometer.sys
	try { // query the FindDevice & Enabled functions from the HP accelerometer DLL
        // OK, here we should have a valid device, try to read a value
        float x,y,z;
        if (! read_xyz(x,y,z)) {
            ::CloseHandle(m_device);
            m_device = INVALID_HANDLE_VALUE;
        }
    }
   	catch(...) {
        m_device = INVALID_HANDLE_VALUE; // failed miserably
	}
#endif
        
    if (m_device == INVALID_HANDLE_VALUE) { 
        closePort();
    }
    else {
        // we must have opened up the device & read a point OK
        setType(SENSOR_WIN_HP);
	    setPort((int) getTypeEnum());	
	    fprintf(stdout, "HP sensor detected.\n");
    }
    return (bool)(m_device != INVALID_HANDLE_VALUE);
}

bool CSensorWinHP::read_xyz(float& x1, float& y1, float& z1)
{
    // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)

    if (m_device == INVALID_HANDLE_VALUE) return false; // obviously we need a valid, open handle!

    bool bRetVal = false;
    DWORD dwError = 0;
    DWORD dwReturn = 0;
    BOOL BRet = FALSE;
    long lVal = 1000L;
    unsigned short usData[3] = {0,0,0}; // data struct the HP device driver uses
    unsigned short usNew[3] = {0,0,0};

    LPOVERLAPPED pOverlapped = new OVERLAPPED;
    memset(pOverlapped, 0x00, sizeof(OVERLAPPED));

    try { 
        /*
        OVERLAPPED myOverlapped;
        SECURITY_ATTRIBUTES secatt;
        secatt.bInheritHandle = TRUE;
        secatt.nLength = sizeof(SECURITY_ATTRIBUTES);
        secatt.lpSecurityDescriptor = NULL;
        myOverlapped.Offset = myOverlapped.OffsetHigh = 0;
        HANDLE hMutex = ::CreateMutex(NULL, FALSE, NULL);
        myOverlapped.hEvent = ::CreateEvent(&secatt, FALSE, FALSE, "hpaccelevt");
        if (!myOverlapped.hEvent) return false; // error - can't create event!
        */

#ifdef _USE_DLL_
        //dwError = SignalObjectAndWait(myOverlapped.hEvent, hMutex, 10000, TRUE);
        dwError = m_FNGetXYZ(m_device, usData, pOverlapped);
        if (dwError == ERROR_IO_PENDING) {
            dwError = ::WaitForSingleObject(pOverlapped->hEvent, 10000);
        }
#else
/*  from the accelerometerdll.dll disassembly for GetRealXYZ:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	[ebp+10h]
  		lea	eax,[ebp+10h]
  		push	eax
  		push	00000006h
  		push	[ebp+0Ch]
  		push	00000000h
  		push	00000000h
  		push	CF50601Ah
  		push	[ebp+08h]
  		call	[KERNEL32.dll!DeviceIoControl]
  		test	eax,eax
  		jnz	L00401790
        */
        /*
        SECURITY_ATTRIBUTES secatt;
        SECURITY_DESCRIPTOR secdesc;
        secatt.bInheritHandle = TRUE;
        secatt.nLength = sizeof(SECURITY_ATTRIBUTES);
        secatt.lpSecurityDescriptor = NULL;
        if (::InitializeSecurityDescriptor(&secdesc, SECURITY_DESCRIPTOR_REVISION)) {
            ::SetSecurityDescriptorDacl(&secdesc, TRUE, NULL, TRUE);
            ::SetSecurityDescriptorSacl(&secdesc, TRUE, NULL, TRUE);
            secatt.lpSecurityDescriptor = &secdesc;
        }   
        */

        // create an event for device synchronization
        pOverlapped->hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        BRet = ::DeviceIoControl(
            m_device,
            HP_XYZ_IO_CODE,
            NULL,
            0,
            usData, 
            6,
            &dwReturn,
            pOverlapped);

        dwError = ::GetLastError();
        if (dwError == ERROR_IO_PENDING) { // wait for it...
            dwError = ::WaitForSingleObject(pOverlapped->hEvent, 2000);
            //BRet = ::ReadFile(m_device, usNew, 6, &dwReturn, pOverlapped);
            //BRet = ::ReadFileEx(m_device, usNew, 6, pOverlapped, read_xyz_completion_routine);
            dwError = ::GetLastError();
        }
#endif
    }
	catch(...) {
		bRetVal = false;
	}
    if (pOverlapped) delete pOverlapped;
	return bRetVal;
}

#ifndef _USE_DLL_
bool CSensorWinHP::getDevice()
{
// have to create GUID bearing in mind that Windows/Intel box is little-endian
// so can't just cram the bytes in as one would normally do e.g.
// "reverse engineered" the GUID from looking at HP's c:\windows\system32\accelerometerdll.dll
// with PE Explorer

// typical way to set a GUID but wrong since ignores byte ordering from
// the raw data in the DLL 
// static const GUID GUID_HP_ACCELEROMETER = 
//   { 0x82662ADD, 0x5E73, 0x8E4E, { 0x8A, 0x59, 0xD9, 0xDC, 0xCF, 0x1E, 0xBE, 0xCE } };

   GUID GUID_HP_ACCELEROMETER;
   memset(&GUID_HP_ACCELEROMETER, 0x00, sizeof(GUID));

   // long Data1
   PBYTE pb = (PBYTE) &GUID_HP_ACCELEROMETER.Data1;
   *pb = 0x82;
   *(pb+1) = 0x66;
   *(pb+2) = 0x2A;
   *(pb+3) = 0xDD;

   // short Data2
   pb = (PBYTE) &GUID_HP_ACCELEROMETER.Data2;
   *pb = 0x5E;
   *(pb+1) = 0x73;

   // short Data3
   pb = (PBYTE) &GUID_HP_ACCELEROMETER.Data3;
   *pb = 0x8E;
   *(pb+1) = 0x4E;

   // char Data4 - since these are just a string of bytes no endian-ness to worry about
   GUID_HP_ACCELEROMETER.Data4[0] = 0x8A;
   GUID_HP_ACCELEROMETER.Data4[1] = 0x59;
   GUID_HP_ACCELEROMETER.Data4[2] = 0xD9;
   GUID_HP_ACCELEROMETER.Data4[3] = 0xDC;
   GUID_HP_ACCELEROMETER.Data4[4] = 0xCF;
   GUID_HP_ACCELEROMETER.Data4[5] = 0x1E;
   GUID_HP_ACCELEROMETER.Data4[6] = 0xBE;
   GUID_HP_ACCELEROMETER.Data4[7] = 0xCE;

   HDEVINFO hDevInfoSet;  // two handles old & new
   hDevInfoSet = ::SetupDiGetClassDevs(
       &GUID_HP_ACCELEROMETER,  // example guid: GUID_DEVINTERFACE_DISK
       NULL,
       NULL,
       DIGCF_DEVICEINTERFACE | DIGCF_PRESENT
   );
    if (hDevInfoSet == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Retrieve a context structure for a device interface of a device 
    // information set.
    BOOL bRet = TRUE;
    //DWORD dwIndex = 0;
    SP_DEVICE_INTERFACE_DATA devInterfaceData;
    ::ZeroMemory(&devInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
    devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    // there's only one laptop accelerometer (if any) so just need to enumerate once
    bRet = ::SetupDiEnumDeviceInterfaces(
        hDevInfoSet, // HDEVINFO DeviceInfoSet 
        NULL, // PSP_DEVINFO_DATA DeviceInfoData  
        &GUID_HP_ACCELEROMETER, // CONST GUID * InterfaceClassGuid 
        0,
        &devInterfaceData // PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData 
    );
    if (!bRet) {
        ::SetupDiDestroyDeviceInfoList(hDevInfoSet);
        return false;
    }

    // now get the device path we can use for CreateFile(), ReadFile() etc
    // since we're not in unicode it will never be more than _MAX_PATH
    const int ciLen = 1024; // 1024 bytes for string should be fine, we're not unicode
    DWORD dwDevDataSize = ciLen + sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    DWORD dwReqSize = 0; 
    PSP_DEVICE_INTERFACE_DETAIL_DATA pbBuffer = 
        (PSP_DEVICE_INTERFACE_DETAIL_DATA) ::LocalAlloc(LPTR, dwDevDataSize);
    if (!pbBuffer) { // memory alloc failed!
        ::SetupDiDestroyDeviceInfoList(hDevInfoSet);
        return false;
    }
    ::ZeroMemory(pbBuffer, dwDevDataSize);
    pbBuffer->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);;

    bRet = ::SetupDiGetDeviceInterfaceDetail(
        hDevInfoSet,
        &devInterfaceData,
        pbBuffer,
        dwDevDataSize,
        &dwReqSize,
        NULL
     );

    ::SetupDiDestroyDeviceInfoList(hDevInfoSet);

    // make sure we have a valid return value i.e. string length's etc
    if (!bRet || !dwReqSize || dwReqSize > dwDevDataSize 
      || strlen(pbBuffer->DevicePath) > ciLen) {
        ::LocalFree(pbBuffer);
        return false;
    }

    /* from accelerometerdll.dll call dissassembly
  		push	ebx
  		push	40000000h
  		push	00000003h
  		push	ebx
  		push	00000003h
  		push	C0000000h
  		lea	eax,[esi+04h]
  		push	eax
  		call	[KERNEL32.dll!CreateFileW] 
        */
    // OK, at this point we have the path name which we can use in a call to CreateFile
    m_device = ::CreateFile(
        pbBuffer->DevicePath, 
        GENERIC_READ | GENERIC_WRITE,   // 0x40000000
        FILE_SHARE_READ | FILE_SHARE_WRITE,  // * 3
        NULL,
        OPEN_EXISTING,  // * 3
        FILE_FLAG_OVERLAPPED, // * 0xC0000000
        NULL);

    ::LocalFree(pbBuffer); // don't forget to free the pbBuffer allocated above!
    return (bool)(m_device != INVALID_HANDLE_VALUE);
}
#endif // ndef _USE_DLL_

/* reverse-engineer the accelerometerdll.dll

//        GENERIC_READ | GENERIC_WRITE,  // * 0xC0000000 
//        FILE_FLAG_OVERLAPPED, // * 0x40000000

// Exports:

ordinal 1
unsigned long __stdcall CanSettingsChange(void *,unsigned char *)

ordinal 2
unsigned long __stdcall ClearLogFile(void *)

ordinal 3
unsigned char __stdcall FindAccelerometerDevice(void * *)

ordinal 4
unsigned long __stdcall GetAccelerometerProperty(void *,enum _ACCELEROMETER_PROPERTY_FLAGS,void *)

ordinal 5
unsigned long __stdcall GetRealTimeXYZ(void *,unsigned short *,struct _OVERLAPPED *)

ordinal 6
unsigned long __stdcall IsSoftwareEnabled(void *,unsigned char *)

ordinal 7
unsigned long __stdcall NotifyAccelerometerAboutPower(void *,unsigned long)

ordinal 8
void * __stdcall RegisterForAccelerometerDisabledEvent(HWND *,void *)

ordinal 9
void * __stdcall RegisterForAccelerometerDiskCountChangeEvent(HWND *,void *)

ordinal 10
void * __stdcall RegisterForAccelerometerEnabledEvent(HWND *,void *)

ordinal 11
void * __stdcall RegisterForAccelerometerParameterChangeEvent(HWND *,void *)

ordinal 12
void * __stdcall RegisterForAccelerometerShockEndEvent(HWND *,void *)

ordinal 13
void * __stdcall RegisterForAccelerometerShockSignaledEvent(HWND *,void *)

ordinal 14
unsigned long __stdcall SessionChange(void *,unsigned short const *,unsigned char)

ordinal 15
unsigned long __stdcall SetAccelerometerProperty(void *,enum _ACCELEROMETER_PROPERTY_FLAGS,void *)

// assembler:

;  Name: .text (Code Section)
;  Virtual Address:    00401000h  Virtual Size:    000017F6h
;  Pointer To RawData: 00000400h  Size Of RawData: 00001800h
;
 KERNEL32.dll!CreateFileW:
  		dd	??
 KERNEL32.dll!InterlockedExchange:
  		dd	??
 KERNEL32.dll!SetUnhandledExceptionFilter:
  		dd	??
 KERNEL32.dll!UnhandledExceptionFilter:
  		dd	??
 KERNEL32.dll!GetCurrentProcess:
  		dd	??
 KERNEL32.dll!TerminateProcess:
  		dd	??
 KERNEL32.dll!GetSystemTimeAsFileTime:
  		dd	??
 KERNEL32.dll!GetCurrentProcessId:
  		dd	??
 KERNEL32.dll!GetCurrentThreadId:
  		dd	??
 KERNEL32.dll!GetTickCount:
  		dd	??
 KERNEL32.dll!QueryPerformanceCounter:
  		dd	??
 KERNEL32.dll!InterlockedCompareExchange:
  		dd	??
 KERNEL32.dll!DeviceIoControl:
  		dd	??
 KERNEL32.dll!GetLastError:
  		dd	??
 KERNEL32.dll!Sleep:
  		dd	??
  		dd	00000000
 SETUPAPI.dll!SetupDiGetClassDevsW:
  		dd	??
 SETUPAPI.dll!SetupDiGetDeviceInterfaceDetailW:
  		dd	??
 SETUPAPI.dll!SetupDiDestroyDeviceInfoList:
  		dd	??
 SETUPAPI.dll!SetupDiEnumDeviceInterfaces:
  		dd	??
  		dd	00000000
 USER32.dll!RegisterDeviceNotificationW:
  		dd	??
  		dd	00000000
 msvcrt.dll!_except_handler4_common:
  		dd	??
 msvcrt.dll!_adjust_fdiv:
  		dd	??
 msvcrt.dll!_amsg_exit:
  		dd	??
 msvcrt.dll!_initterm:
  		dd	??
 msvcrt.dll!_XcptFilter:
  		dd	??
 msvcrt.dll!memset:
  		dd	??
 msvcrt.dll!malloc:
  		dd	??
 msvcrt.dll!free:
  		dd	??
  		dd	00000000
 L00401080:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00401084:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00401088:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401899
 L00401090:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
  		dd	00000000h
  		dd	45BBF382h
  		dw	0000h
  		dw	0000h
  		dd	00000002h
  		dd	0000006Ah
  		dd	00001180h
  		dd	00000580h
 L004010BC:
 		db	82h;   '''
 		db	66h;   'f'
 		db	2Ah;   '*'
 		db	DDh;   'ù'
 		db	5Eh;   '^'
 		db	73h;   's'
 		db	8Eh;   '?'
 		db	4Eh;   'N'
 		db	8Ah;   '?'
 		db	59h;   'Y'
 		db	D9h;   'ô'
 		db	DCh;   'ú'
 		db	CFh;   'è'
 		db	1Eh;
 		db	BEh;   '?'
 		db	CEh;   'é'
 L004010CC:
 		db	0Bh;
 		db	77h;   'w'
 		db	75h;   'u'
 		db	6Eh;   'n'
 		db	09h;
 		db	3Dh;   '='
 		db	72h;   'r'
 		db	4Eh;   'N'
 		db	93h;   '"'
 		db	BEh;   '?'
 		db	DFh;   'ü'
 		db	D6h;   'ñ'
 		db	25h;   '%'
 		db	A2h;   '˜'
 		db	72h;   'r'
 		db	92h;   '''
 L004010DC:
 		db	57h;   'W'
 		db	F5h;   'Â'
 		db	A1h;   'ˆ'
 		db	6Dh;   'm'
 		db	A4h;   '˝'
 		db	7Dh;   '}'
 		db	A1h;   'ˆ'
 		db	47h;   'G'
 		db	B6h;
 		db	3Bh;   ';'
 		db	ADh;   '-'
 		db	41h;   'A'
 		db	60h;   '`'
 		db	43h;   'C'
 		db	DAh;   'ö'
 		db	6Dh;   'm'
 L004010EC:
 		db	65h;   'e'
 		db	1Ch;
 		db	0Ch;
 		db	3Eh;   '>'
 		db	B0h;   '¯'
 		db	78h;   'x'
 		db	66h;   'f'
 		db	42h;   'B'
 		db	86h;   '≈'
 		db	98h;   '?'
 		db	00h;
 		db	DFh;   'ü'
 		db	D3h;   'ì'
 		db	75h;   'u'
 		db	FAh;   'Í'
 		db	59h;   'Y'
 L004010FC:
 		db	6Ah;   'j'
 		db	00h;
 		db	C1h;   'Å'
 		db	40h;   '@'
 		db	54h;   'T'
 		db	E2h;   '¢'
 		db	36h;   '6'
 		db	48h;   'H'
 		db	BFh;   'ı'
 		db	10h;
 		db	32h;   '2'
 		db	07h;
 		db	14h;
 		db	33h;   '3'
 		db	9Dh;   '?'
 		db	B3h;   'i'
 L0040110C:
 		db	5Fh;   '_'
 		db	73h;   's'
 		db	86h;   '≈'
 		db	68h;   'h'
 		db	BBh;   '>'
 		db	2Ah;   '*'
 		db	09h;
 		db	4Dh;   'M'
 		db	9Ah;   '?'
 		db	2Fh;   '/'
 		db	C9h;   'â'
 		db	2Bh;   '+'
 		db	D7h;   'ó'
 		db	A2h;   '˜'
 		db	CFh;   'è'
 		db	68h;   'h'
 L0040111C:
 		db	F5h;   'Â'
 		db	03h;
 		db	C3h;   'É'
 		db	98h;   '?'
 		db	ACh;   'ø'
 		db	EEh;   'Æ'
 		db	B8h;   'Ò'
 		db	46h;   'F'
 		db	A3h;   '?'
 		db	D6h;   'ñ'
 		db	EAh;   '™'
 		db	2Dh;   '-'
 		db	48h;   'H'
 		db	73h;   's'
 		db	98h;   '?'
 		db	9Fh;   '?'
 L0040112C:
 		dd	L00403020
 		dd	L00403070
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	48h;   'H'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00403000
 		dd	L004011F0
 		db	01h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	52h;   'R'
 		db	53h;   'S'
 		db	44h;   'D'
 		db	53h;   'S'
 		db	3Dh;   '='
 		db	6Dh;   'm'
 		db	4Eh;   'N'
 		db	F7h;   'Á'
 		db	CCh;   'å'
 		db	D0h;   'ê'
 		db	6Fh;   'o'
 		db	41h;   'A'
 		db	A7h;
 		db	FBh;   'Î'
 		db	FAh;   'Í'
 		db	BBh;   '>'
 		db	F7h;   'Á'
 		db	40h;   '@'
 		db	B7h;   '˙'
 		db	74h;   't'
 		db	01h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	63h;   'c'
 		db	3Ah;   ':'
 		db	5Ch;   '\'
 		db	70h;   'p'
 		db	72h;   'r'
 		db	6Fh;   'o'
 		db	67h;   'g'
 		db	5Ch;   '\'
 		db	76h;   'v'
 		db	65h;   'e'
 		db	6Eh;   'n'
 		db	64h;   'd'
 		db	6Fh;   'o'
 		db	72h;   'r'
 		db	73h;   's'
 		db	5Ch;   '\'
 		db	68h;   'h'
 		db	70h;   'p'
 		db	6Dh;   'm'
 		db	64h;   'd'
 		db	70h;   'p'
 		db	73h;   's'
 		db	72h;   'r'
 		db	76h;   'v'
 		db	69h;   'i'
 		db	73h;   's'
 		db	74h;   't'
 		db	61h;   'a'
 		db	5Ch;   '\'
 		db	73h;   's'
 		db	6Fh;   'o'
 		db	75h;   'u'
 		db	72h;   'r'
 		db	63h;   'c'
 		db	65h;   'e'
 		db	5Ch;   '\'
 		db	73h;   's'
 		db	72h;   'r'
 		db	63h;   'c'
 		db	5Ch;   '\'
 		db	6Ch;   'l'
 		db	69h;   'i'
 		db	62h;   'b'
 		db	5Ch;   '\'
 		db	66h;   'f'
 		db	72h;   'r'
 		db	65h;   'e'
 		db	5Fh;   '_'
 		db	77h;   'w'
 		db	6Ch;   'l'
 		db	68h;   'h'
 		db	5Fh;   '_'
 		db	78h;   'x'
 		db	38h;   '8'
 		db	36h;   '6'
 		db	5Ch;   '\'
 		db	69h;   'i'
 		db	33h;   '3'
 		db	38h;   '8'
 		db	36h;   '6'
 		db	5Ch;   '\'
 		db	61h;   'a'
 		db	63h;   'c'
 		db	63h;   'c'
 		db	65h;   'e'
 		db	6Ch;   'l'
 		db	65h;   'e'
 		db	72h;   'r'
 		db	6Fh;   'o'
 		db	6Dh;   'm'
 		db	65h;   'e'
 		db	74h;   't'
 		db	65h;   'e'
 		db	72h;   'r'
 		db	64h;   'd'
 		db	6Ch;   'l'
 		db	6Ch;   'l'
 		db	2Eh;   '.'
 		db	70h;   'p'
 		db	64h;   'd'
 		db	62h;   'b'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L004011F0:
 		db	8Eh;   '?'
 		db	1Eh;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 ?FindAccelerometerDevice@@YGEPAPAX@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000002Ch
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	eax,[ebp+08h]
  		push	ebx
  		push	esi
  		push	edi
  		push	00000012h
  		xor	ebx,ebx
  		push	ebx
  		push	ebx
  		mov	esi,L004010BC
  		push	esi
  		mov	[ebp-2Ch],eax
  		call	[SETUPAPI.dll!SetupDiGetClassDevsW]
  		mov	edi,eax
  		cmp	edi,FFFFFFFFh
  		mov	[ebp-24h],edi
  		jz 	L00401302
  		lea	eax,[ebp-20h]
  		push	eax
  		push	ebx
  		push	esi
  		push	ebx
  		push	edi
  		mov	dword ptr [ebp-20h],0000001Ch
  		call	[SETUPAPI.dll!SetupDiEnumDeviceInterfaces]
  		test	eax,eax
  		jz 	L004012F3
  		push	ebx
  		lea	eax,[ebp-28h]
  		push	eax
  		push	ebx
  		push	ebx
  		lea	eax,[ebp-20h]
  		push	eax
  		push	edi
  		mov	edi,[SETUPAPI.dll!SetupDiGetDeviceInterfaceDetailW]
  		call	edi
  		test	eax,eax
  		jnz	L00401280
  		call	[KERNEL32.dll!GetLastError]
  		cmp	eax,0000007Ah
  		jnz	L004012F9
 L00401280:
  		push	[ebp-28h]
  		call	[msvcrt.dll!malloc]
  		mov	esi,eax
  		cmp	esi,ebx
  		pop	ecx
  		jz 	L004012F9
  		push	ebx
  		lea	eax,[ebp-28h]
  		push	eax
  		mov	dword ptr [esi],00000006h
  		push	[ebp-28h]
  		lea	eax,[ebp-20h]
  		push	esi
  		push	eax
  		push	[ebp-24h]
  		call	edi
  		test	eax,eax
  		jnz	L004012C1
 L004012AC:
  		push	[ebp-24h]
  		call	[SETUPAPI.dll!SetupDiDestroyDeviceInfoList]
  		push	esi
  		call	[msvcrt.dll!free]
  		pop	ecx
  		mov	al,bl
  		jmp	L00401304
 L004012C1:
  		push	ebx
  		push	40000000h
  		push	00000003h
  		push	ebx
  		push	00000003h
  		push	C0000000h
  		lea	eax,[esi+04h]
  		push	eax
  		call	[KERNEL32.dll!CreateFileW]
  		mov	ecx,[ebp-2Ch]
  		cmp	ecx,FFFFFFFFh
  		mov	[ecx],eax
  		jnz	L004012EF
  		push	esi
  		call	[msvcrt.dll!free]
  		pop	ecx
  		jmp	L004012F9
 L004012EF:
  		mov	bl,01h
  		jmp	L004012AC
 L004012F3:
  		call	[KERNEL32.dll!GetLastError]
 L004012F9:
  		push	[ebp-24h]
  		call	[SETUPAPI.dll!SetupDiDestroyDeviceInfoList]
 L00401302:
  		xor	al,al
 L00401304:
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L00401CD0
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 ?RegisterForAccelerometerShockSignaledEvent@@YGPAXPAUHWND__@@PAX@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000030h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	ebx
  		mov	ebx,[ebp+08h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		push	edi
  		push	0000002Ch
  		pop	edi
  		push	edi
  		lea	eax,[ebp-30h]
  		push	00000000h
  		push	eax
  		call	jmp_msvcrt.dll!memset
  		mov	[ebp-30h],edi
  		mov	[ebp-24h],esi
  		mov	dword ptr [ebp-2Ch],00000006h
  		mov	esi,L004010FC
  		lea	edi,[ebp-1Ch]
  		movsd
  		movsd
  		add	esp,0000000Ch
  		push	00000000h
  		movsd
  		lea	eax,[ebp-30h]
  		push	eax
  		push	ebx
  		movsd
  		call	[USER32.dll!RegisterDeviceNotificationW]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L00401CD0
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 ?RegisterForAccelerometerShockEndEvent@@YGPAXPAUHWND__@@PAX@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000030h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	ebx
  		mov	ebx,[ebp+08h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		push	edi
  		push	0000002Ch
  		pop	edi
  		push	edi
  		lea	eax,[ebp-30h]
  		push	00000000h
  		push	eax
  		call	jmp_msvcrt.dll!memset
  		mov	[ebp-30h],edi
  		mov	[ebp-24h],esi
  		mov	dword ptr [ebp-2Ch],00000006h
  		mov	esi,L0040110C
  		lea	edi,[ebp-1Ch]
  		movsd
  		movsd
  		add	esp,0000000Ch
  		push	00000000h
  		movsd
  		lea	eax,[ebp-30h]
  		push	eax
  		push	ebx
  		movsd
  		call	[USER32.dll!RegisterDeviceNotificationW]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L00401CD0
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	4
 ?RegisterForAccelerometerEnabledEvent@@YGPAXPAUHWND__@@PAX@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000030h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	ebx
  		mov	ebx,[ebp+08h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		push	edi
  		push	0000002Ch
  		pop	edi
  		push	edi
  		lea	eax,[ebp-30h]
  		push	00000000h
  		push	eax
  		call	jmp_msvcrt.dll!memset
  		mov	[ebp-30h],edi
  		mov	[ebp-24h],esi
  		mov	dword ptr [ebp-2Ch],00000006h
  		mov	esi,L004010CC
  		lea	edi,[ebp-1Ch]
  		movsd
  		movsd
  		add	esp,0000000Ch
  		push	00000000h
  		movsd
  		lea	eax,[ebp-30h]
  		push	eax
  		push	ebx
  		movsd
  		call	[USER32.dll!RegisterDeviceNotificationW]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L00401CD0
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 ?RegisterForAccelerometerDisabledEvent@@YGPAXPAUHWND__@@PAX@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000030h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	ebx
  		mov	ebx,[ebp+08h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		push	edi
  		push	0000002Ch
  		pop	edi
  		push	edi
  		lea	eax,[ebp-30h]
  		push	00000000h
  		push	eax
  		call	jmp_msvcrt.dll!memset
  		mov	[ebp-30h],edi
  		mov	[ebp-24h],esi
  		mov	dword ptr [ebp-2Ch],00000006h
  		mov	esi,L004010DC
  		lea	edi,[ebp-1Ch]
  		movsd
  		movsd
  		add	esp,0000000Ch
  		push	00000000h
  		movsd
  		lea	eax,[ebp-30h]
  		push	eax
  		push	ebx
  		movsd
  		call	[USER32.dll!RegisterDeviceNotificationW]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L00401CD0
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 ?RegisterForAccelerometerParameterChangeEvent@@YGPAXPAUHWND__@@PAX@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000030h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	ebx
  		mov	ebx,[ebp+08h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		push	edi
  		push	0000002Ch
  		pop	edi
  		push	edi
  		lea	eax,[ebp-30h]
  		push	00000000h
  		push	eax
  		call	jmp_msvcrt.dll!memset
  		mov	[ebp-30h],edi
  		mov	[ebp-24h],esi
  		mov	dword ptr [ebp-2Ch],00000006h
  		mov	esi,L004010EC
  		lea	edi,[ebp-1Ch]
  		movsd
  		movsd
  		add	esp,0000000Ch
  		push	00000000h
  		movsd
  		lea	eax,[ebp-30h]
  		push	eax
  		push	ebx
  		movsd
  		call	[USER32.dll!RegisterDeviceNotificationW]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L00401CD0
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 ?RegisterForAccelerometerDiskCountChangeEvent@@YGPAXPAUHWND__@@PAX@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000030h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	ebx
  		mov	ebx,[ebp+08h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		push	edi
  		push	0000002Ch
  		pop	edi
  		push	edi
  		lea	eax,[ebp-30h]
  		push	00000000h
  		push	eax
  		call	jmp_msvcrt.dll!memset
  		mov	[ebp-30h],edi
  		mov	[ebp-24h],esi
  		mov	dword ptr [ebp-2Ch],00000006h
  		mov	esi,L0040111C
  		lea	edi,[ebp-1Ch]
  		movsd
  		movsd
  		add	esp,0000000Ch
  		push	00000000h
  		movsd
  		lea	eax,[ebp-30h]
  		push	eax
  		push	ebx
  		movsd
  		call	[USER32.dll!RegisterDeviceNotificationW]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L00401CD0
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	8
 ?SetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000040h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	eax,[ebp+0Ch]
  		cmp	eax,00000002h
  		mov	edx,[ebp+08h]
  		mov	ecx,[ebp+10h]
  		mov	[ebp-3Ch],eax
  		jz 	L004015C9
  		cmp	eax,00000003h
  		jle	L004015C4
  		cmp	eax,00000005h
  		jg 	L004015C4
  		mov	eax,[ecx]
  		mov	[ebp-38h],eax
  		jmp	L004015CE
 L004015C4:
  		push	00000057h
  		pop	eax
  		jmp	L004015F7
 L004015C9:
  		mov	al,[ecx]
  		mov	[ebp-38h],al
 L004015CE:
  		xor	eax,eax
  		push	eax
  		lea	ecx,[ebp-40h]
  		push	ecx
  		push	eax
  		push	eax
  		push	00000038h
  		lea	eax,[ebp-3Ch]
  		push	eax
  		push	CF509FFCh
  		push	edx
  		call	[KERNEL32.dll!DeviceIoControl]
  		test	eax,eax
  		jnz	L004015F5
  		call	[KERNEL32.dll!GetLastError]
  		jmp	L004015F7
 L004015F5:
  		xor	eax,eax
 L004015F7:
  		mov	ecx,[ebp-04h]
  		xor	ecx,ebp
  		call	SUB_L00401CD0
  		leave
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 ?GetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000040h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	eax,[ebp+0Ch]
  		cmp	eax,00000002h
  		mov	ecx,[ebp+08h]
  		push	edi
  		mov	edi,[ebp+10h]
  		mov	[ebp-3Ch],eax
  		jz 	L0040163D
  		cmp	eax,00000003h
  		jle	L00401638
  		cmp	eax,00000007h
  		jle	L0040163D
 L00401638:
  		push	00000057h
  		pop	eax
  		jmp	L00401687
 L0040163D:
  		push	00000000h
  		lea	eax,[ebp-40h]
  		push	eax
  		push	00000038h
  		lea	eax,[ebp-3Ch]
  		push	eax
  		push	00000038h
  		push	eax
  		push	CF502000h
  		push	ecx
  		call	[KERNEL32.dll!DeviceIoControl]
  		test	eax,eax
  		jnz	L00401664
  		call	[KERNEL32.dll!GetLastError]
  		jmp	L00401687
 L00401664:
  		mov	eax,[ebp-3Ch]
  		cmp	eax,00000002h
  		jz 	L00401680
  		cmp	eax,00000003h
  		jle	L00401638
  		cmp	eax,00000005h
  		jle	L004016A2
  		cmp	eax,00000006h
  		jz 	L00401696
  		cmp	eax,00000007h
  		jnz	L00401638
 L00401680:
  		mov	al,[ebp-38h]
  		mov	[edi],al
 L00401685:
  		xor	eax,eax
 L00401687:
  		mov	ecx,[ebp-04h]
  		xor	ecx,ebp
  		pop	edi
  		call	SUB_L00401CD0
  		leave
  		retn	000Ch
;------------------------------------------------------------------------------
 L00401696:
  		push	esi
  		push	0000000Dh
  		pop	ecx
  		lea	esi,[ebp-38h]
  		rep movsd
  		pop	esi
  		jmp	L00401638
 L004016A2:
  		mov	eax,[ebp-38h]
  		mov	[edi],eax
  		jmp	L00401685
  		Align	2
 ?ClearLogFile@@YGKPAX@Z:
  		xor	eax,eax
  		inc	eax
  		retn	0004h
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 ?NotifyAccelerometerAboutPower@@YGKPAXK@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		xor	eax,eax
  		push	eax
  		lea	ecx,[ebp+08h]
  		push	ecx
  		push	eax
  		push	eax
  		push	00000004h
  		lea	eax,[ebp+0Ch]
  		push	eax
  		push	CF50A024h
  		push	[ebp+08h]
  		call	[KERNEL32.dll!DeviceIoControl]
  		test	eax,eax
  		jnz	L004016E7
  		call	[KERNEL32.dll!GetLastError]
  		jmp	L004016E9
 L004016E7:
  		xor	eax,eax
 L004016E9:
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 ?SessionChange@@YGKPAXPBGE@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		xor	ebx,ebx
  		cmp	[ebp+10h],bl
  		push	esi
  		mov	esi,CF50A028h
  		jnz	L00401712
  		add	esi,00000004h
  		cmp	[ebp+0Ch],ebx
  		jz 	L00401731
  		push	00000057h
  		pop	eax
  		jmp	L00401756
 L00401712:
  		cmp	[ebp+0Ch],ebx
  		jz 	L00401731
  		mov	eax,[ebp+0Ch]
  		lea	edx,[eax+02h]
 L0040171D:
  		mov	cx,[eax]
  		inc	eax
  		inc	eax
  		cmp	cx,bx
  		jnz	L0040171D
  		sub	eax,edx
  		sar	eax,1
  		lea	eax,[eax+eax+02h]
  		jmp	L00401733
 L00401731:
  		xor	eax,eax
 L00401733:
  		push	ebx
  		lea	ecx,[ebp+10h]
  		push	ecx
  		push	ebx
  		push	ebx
  		push	eax
  		push	[ebp+0Ch]
  		push	esi
  		push	[ebp+08h]
  		call	[KERNEL32.dll!DeviceIoControl]
  		test	eax,eax
  		jnz	L00401754
  		call	[KERNEL32.dll!GetLastError]
  		jmp	L00401756
 L00401754:
  		xor	eax,eax
 L00401756:
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 ?GetRealTimeXYZ@@YGKPAXPAGPAU_OVERLAPPED@@@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	[ebp+10h]
  		lea	eax,[ebp+10h]
  		push	eax
  		push	00000006h
  		push	[ebp+0Ch]
  		push	00000000h
  		push	00000000h
  		push	CF50601Ah
  		push	[ebp+08h]
  		call	[KERNEL32.dll!DeviceIoControl]
  		test	eax,eax
  		jnz	L00401790
  		call	[KERNEL32.dll!GetLastError]
  		jmp	L00401795
 L00401790:
  		mov	eax,000003E5h
 L00401795:
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 ?IsSoftwareEnabled@@YGKPAXPAE@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000040h
  		mov	eax,[L00403000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	eax,[ebp+08h]
  		push	esi
  		push	edi
  		mov	edi,[ebp+0Ch]
  		push	00000002h
  		pop	esi
  		push	00000000h
  		lea	ecx,[ebp-40h]
  		push	ecx
  		push	00000038h
  		lea	ecx,[ebp-3Ch]
  		push	ecx
  		push	00000038h
  		push	ecx
  		push	CF502000h
  		push	eax
  		mov	[ebp-3Ch],esi
  		call	[KERNEL32.dll!DeviceIoControl]
  		test	eax,eax
  		jnz	L004017E5
  		call	[KERNEL32.dll!GetLastError]
  		jmp	L00401807
 L004017E5:
  		mov	eax,[ebp-3Ch]
  		cmp	eax,esi
  		jz 	L00401800
  		cmp	eax,00000003h
  		jle	L0040181F
  		cmp	eax,00000005h
  		jle	L00401824
  		cmp	eax,00000006h
  		jz 	L00401817
  		cmp	eax,00000007h
  		jnz	L0040181F
 L00401800:
  		mov	al,[ebp-38h]
  		mov	[edi],al
 L00401805:
  		xor	eax,eax
 L00401807:
  		mov	ecx,[ebp-04h]
  		pop	edi
  		xor	ecx,ebp
  		pop	esi
  		call	SUB_L00401CD0
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
 L00401817:
  		push	0000000Dh
  		pop	ecx
  		lea	esi,[ebp-38h]
  		rep movsd
 L0040181F:
  		push	00000057h
  		pop	eax
  		jmp	L00401807
 L00401824:
  		mov	eax,[ebp-38h]
  		mov	[edi],eax
  		jmp	L00401805
  		Align	8
 ?CanSettingsChange@@YGKPAXPAE@Z:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	[ebp+0Ch]
  		push	00000007h
  		push	[ebp+08h]
  		call	?GetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 SUB_L0040184B:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	eax,E06D7363h
  		cmp	[ebp+08h],eax
  		jnz	L00401867
  		push	[ebp+0Ch]
  		push	eax
  		call	jmp_msvcrt.dll!_XcptFilter
  		pop	ecx
  		pop	ecx
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
 L00401867:
  		xor	eax,eax
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00401870:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	esi
  		mov	esi,[ebp+08h]
  		xor	eax,eax
  		jmp	L0040188C
 L0040187D:
  		test	eax,eax
  		jnz	L00401891
  		mov	ecx,[esi]
  		test	ecx,ecx
  		jz 	L00401889
  		call	ecx
 L00401889:
  		add	esi,00000004h
 L0040188C:
  		cmp	esi,[ebp+0Ch]
  		jc 	L0040187D
 L00401891:
  		pop	esi
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 L00401899:
  		push	00000080h
  		call	[msvcrt.dll!malloc]
  		test	eax,eax
  		pop	ecx
  		mov	[L0040334C],eax
  		mov	[L00403348],eax
  		jnz	L004018B5
  		inc	eax
  		retn
;------------------------------------------------------------------------------
 L004018B5:
  		and	dword ptr [eax],00000000h
  		xor	eax,eax
  		retn
;------------------------------------------------------------------------------
  		Align	8
 SUB_L004018C0:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		xor	eax,eax
  		cmp	[ebp+0Ch],eax
  		jnz	L004018DA
  		cmp	[L00403018],eax
  		jle	L00401910
  		dec	[L00403018]
 L004018DA:
  		cmp	dword ptr [ebp+0Ch],00000001h
  		mov	ecx,[msvcrt.dll!_adjust_fdiv]
  		mov	ecx,[ecx]
  		push	ebx
  		push	esi
  		push	edi
  		mov	[L0040333C],ecx
  		jnz	L004019C9
  		mov	ecx,fs:[00000018h]
  		mov	ebx,[ecx+04h]
  		mov	edi,[KERNEL32.dll!InterlockedCompareExchange]
  		mov	[ebp+0Ch],eax
  		push	eax
  		mov	esi,L00403344
  		jmp	L00401928
 L00401910:
  		xor	eax,eax
  		jmp	L00401A5D
 L00401917:
  		cmp	eax,ebx
  		jz 	L00401932
  		push	000003E8h
  		call	[KERNEL32.dll!Sleep]
  		push	00000000h
 L00401928:
  		push	ebx
  		push	esi
  		call	edi
  		test	eax,eax
  		jnz	L00401917
  		jmp	L00401939
 L00401932:
  		mov	dword ptr [ebp+0Ch],00000001h
 L00401939:
  		mov	eax,[L00403340]
  		test	eax,eax
  		push	00000002h
  		pop	edi
  		jz 	L0040194E
  		push	0000001Fh
  		call	jmp_msvcrt.dll!_amsg_exit
  		jmp	L0040198A
 L0040194E:
  		push	L00401090
  		push	L00401088
  		mov	dword ptr [L00403340],00000001h
  		call	SUB_L00401870
  		test	eax,eax
  		pop	ecx
  		pop	ecx
  		jz 	L00401974
  		xor	eax,eax
  		jmp	L00401A5A
 L00401974:
  		push	L00401084
  		push	L00401080
  		call	jmp_msvcrt.dll!_initterm
  		pop	ecx
  		mov	[L00403340],edi
 L0040198A:
  		xor	ebx,ebx
  		cmp	[ebp+0Ch],ebx
  		pop	ecx
  		jnz	L0040199A
  		push	ebx
  		push	esi
  		call	[KERNEL32.dll!InterlockedExchange]
 L0040199A:
  		cmp	[L00403354],ebx
  		jz 	L004019BE
  		push	L00403354
  		call	SUB_L00401D9B
  		test	eax,eax
  		pop	ecx
  		jz 	L004019BE
  		push	[ebp+10h]
  		push	edi
  		push	[ebp+08h]
  		call	[L00403354]
 L004019BE:
  		inc	[L00403018]
  		jmp	L00401A57
 L004019C9:
  		cmp	[ebp+0Ch],eax
  		jnz	L00401A57
  		mov	edi,[KERNEL32.dll!InterlockedCompareExchange]
  		push	eax
  		mov	esi,L00403344
  		jmp	L004019ED
 L004019E0:
  		push	000003E8h
  		call	[KERNEL32.dll!Sleep]
  		push	00000000h
 L004019ED:
  		push	00000001h
  		push	esi
  		call	edi
  		test	eax,eax
  		jnz	L004019E0
  		mov	eax,[L00403340]
  		cmp	eax,00000002h
  		jz 	L00401A0A
  		push	0000001Fh
  		call	jmp_msvcrt.dll!_amsg_exit
  		pop	ecx
  		jmp	L00401A57
 L00401A0A:
  		mov	ebx,[L0040334C]
  		test	ebx,ebx
  		jz 	L00401A44
  		mov	edi,[L00403348]
  		add	edi,FFFFFFFCh
  		jmp	L00401A2A
 L00401A1F:
  		mov	eax,[edi]
  		test	eax,eax
  		jz 	L00401A27
  		call	eax
 L00401A27:
  		sub	edi,00000004h
 L00401A2A:
  		cmp	edi,ebx
  		jnc	L00401A1F
  		push	ebx
  		call	[msvcrt.dll!free]
  		and	dword ptr [L00403348],00000000h
  		and	dword ptr [L0040334C],00000000h
  		pop	ecx
 L00401A44:
  		push	00000000h
  		push	esi
  		mov	dword ptr [L00403340],00000000h
  		call	[KERNEL32.dll!InterlockedExchange]
 L00401A57:
  		xor	eax,eax
  		inc	eax
 L00401A5A:
  		pop	edi
  		pop	esi
  		pop	ebx
 L00401A5D:
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 L00401A66:
  		push	0000002Ch
  		push	L00402028
  		call	SUB_L00401E30
  		mov	ecx,[ebp+0Ch]
  		xor	edx,edx
  		inc	edx
  		mov	[ebp-1Ch],edx
  		xor	esi,esi
  		mov	[ebp-04h],esi
  		mov	[L00403008],ecx
  		cmp	ecx,esi
  		jnz	L00401A95
  		cmp	[L00403018],esi
  		jnz	L00401A95
  		mov	[ebp-1Ch],esi
 L00401A95:
  		cmp	ecx,edx
  		jz 	L00401AA2
  		cmp	ecx,00000002h
  		jnz	L00401B29
 L00401AA2:
  		mov	eax,[L00403350]
  		cmp	eax,esi
  		jz 	L00401ADB
  		mov	[ebp-04h],edx
  		push	[ebp+10h]
  		push	ecx
  		push	[ebp+08h]
  		call	eax
  		mov	[ebp-1Ch],eax
  		jmp	L00401AD8
 L00401ABC:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-20h],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401AD0:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
  		mov	[ebp-1Ch],esi
 L00401AD8:
  		mov	[ebp-04h],esi
 L00401ADB:
  		cmp	[ebp-1Ch],esi
  		jz 	L00401C8E
  		mov	dword ptr [ebp-04h],00000002h
  		push	[ebp+10h]
  		push	[ebp+0Ch]
  		push	[ebp+08h]
  		call	SUB_L004018C0
  		mov	[ebp-1Ch],eax
  		jmp	L00401B1A
 L00401AFE:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-24h],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401B12:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
  		mov	[ebp-1Ch],esi
 L00401B1A:
  		mov	[ebp-04h],esi
  		cmp	[ebp-1Ch],esi
  		jz 	L00401C8E
  		mov	ecx,[ebp+0Ch]
 L00401B29:
  		mov	dword ptr [ebp-04h],00000003h
  		push	[ebp+10h]
  		push	ecx
  		push	[ebp+08h]
  		call	SUB_L00401E23
  		mov	[ebp-1Ch],eax
  		jmp	L00401B5D
 L00401B41:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-28h],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401B55:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
  		mov	[ebp-1Ch],esi
 L00401B5D:
  		mov	[ebp-04h],esi
  		cmp	dword ptr [ebp+0Ch],00000001h
  		jnz	L00401C06
  		cmp	[ebp-1Ch],esi
  		jnz	L00401C06
  		mov	dword ptr [ebp-04h],00000004h
  		push	esi
  		push	esi
  		push	[ebp+08h]
  		call	SUB_L00401E23
  		jmp	L00401B9F
 L00401B86:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-2Ch],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401B9A:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
 L00401B9F:
  		mov	[ebp-04h],esi
  		mov	dword ptr [ebp-04h],00000005h
  		push	esi
  		push	esi
  		push	[ebp+08h]
  		call	SUB_L004018C0
  		jmp	L00401BCE
 L00401BB5:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-30h],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401BC9:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
 L00401BCE:
  		mov	[ebp-04h],esi
  		mov	eax,[L00403350]
  		cmp	eax,esi
  		jz 	L00401C06
  		mov	dword ptr [ebp-04h],00000006h
  		push	esi
  		push	esi
  		push	[ebp+08h]
  		call	eax
  		jmp	L00401C03
 L00401BEA:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-34h],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401BFE:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
 L00401C03:
  		mov	[ebp-04h],esi
 L00401C06:
  		cmp	[ebp+0Ch],esi
  		jz 	L00401C11
  		cmp	dword ptr [ebp+0Ch],00000003h
  		jnz	L00401C8E
 L00401C11:
  		mov	dword ptr [ebp-04h],00000007h
  		push	[ebp+10h]
  		push	[ebp+0Ch]
  		push	[ebp+08h]
  		call	SUB_L004018C0
  		mov	[ebp-1Ch],eax
  		jmp	L00401C47
 L00401C2B:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-38h],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401C3F:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
  		mov	[ebp-1Ch],esi
 L00401C47:
  		mov	[ebp-04h],esi
  		cmp	[ebp-1Ch],esi
  		jz 	L00401C8E
  		mov	eax,[L00403350]
  		cmp	eax,esi
  		jz 	L00401C8E
  		mov	dword ptr [ebp-04h],00000008h
  		push	[ebp+10h]
  		push	[ebp+0Ch]
  		push	[ebp+08h]
  		call	eax
  		mov	[ebp-1Ch],eax
  		jmp	L00401C8B
 L00401C6F:
  		mov	eax,[ebp-14h]
  		mov	ecx,[eax]
  		mov	ecx,[ecx]
  		mov	[ebp-3Ch],ecx
  		push	eax
  		push	ecx
  		call	SUB_L0040184B
  		pop	ecx
  		pop	ecx
  		retn
;------------------------------------------------------------------------------
 L00401C83:
  		mov	esp,[ebp-18h]
  		xor	esi,esi
  		mov	[ebp-1Ch],esi
 L00401C8B:
  		mov	[ebp-04h],esi
 L00401C8E:
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		call	SUB_L00401CA5
  		mov	eax,[ebp-1Ch]
  		call	SUB_L00401E75
  		retn	000Ch
;------------------------------------------------------------------------------
 SUB_L00401CA5:
  		mov	dword ptr [L00403008],FFFFFFFFh
  		retn
;------------------------------------------------------------------------------
  		Align	8
;------------------------------------------------------------------------------
 EntryPoint:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		cmp	dword ptr [ebp+0Ch],00000001h
  		jnz	L00401CC5
  		call	SUB_L00401EB8
 L00401CC5:
  		pop	ebp
  		jmp	L00401A66
  		Align	8
 SUB_L00401CD0:
  		cmp	ecx,[L00403000]
  		jnz	L00401CE2
  		test	ecx,FFFF0000h
  		jnz	L00401CE2
 		db	F3h;   '„'
 		db	C3h;   'É'
 L00401CE2:
  		jmp	L00401F36
  		Align	4
 jmp_msvcrt.dll!memset:
  		jmp	[msvcrt.dll!memset]
  		Align	8
 jmp_msvcrt.dll!_XcptFilter:
  		jmp	[msvcrt.dll!_XcptFilter]
  		Align	32
 SUB_L00401D10:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	ecx,[ebp+08h]
  		cmp	word ptr [ecx],5A4Dh
  		jz 	L00401D23
 L00401D1F:
  		xor	eax,eax
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
 L00401D23:
  		mov	eax,[ecx+3Ch]
  		add	eax,ecx
  		cmp	dword ptr [eax],00004550h
  		jnz	L00401D1F
  		xor	ecx,ecx
  		cmp	word ptr [eax+18h],010Bh
  		setz 	cl
  		mov	eax,ecx
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
  		Align	32
 SUB_L00401D50:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+08h]
  		mov	ecx,[eax+3Ch]
  		add	ecx,eax
  		movzx	eax,[ecx+14h]
  		push	ebx
  		push	esi
  		movzx	esi,[ecx+06h]
  		xor	edx,edx
  		test	esi,esi
  		push	edi
  		lea	eax,[eax+ecx+18h]
  		jbe	L00401D8F
  		mov	edi,[ebp+0Ch]
 L00401D75:
  		mov	ecx,[eax+0Ch]
  		cmp	edi,ecx
  		jc 	L00401D85
  		mov	ebx,[eax+08h]
  		add	ebx,ecx
  		cmp	edi,ebx
  		jc 	L00401D91
 L00401D85:
  		add	edx,00000001h
  		add	eax,00000028h
  		cmp	edx,esi
  		jc 	L00401D75
 L00401D8F:
  		xor	eax,eax
 L00401D91:
  		pop	edi
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 SUB_L00401D9B:
  		push	00000008h
  		push	L004020A8
  		call	SUB_L00401E30
  		and	dword ptr [ebp-04h],00000000h
  		mov	edx,L00400000
  		push	edx
  		call	SUB_L00401D10
  		pop	ecx
  		test	eax,eax
  		jz 	L00401DF8
  		mov	eax,[ebp+08h]
  		sub	eax,edx
  		push	eax
  		push	edx
  		call	SUB_L00401D50
  		pop	ecx
  		pop	ecx
  		test	eax,eax
  		jz 	L00401DF8
  		mov	eax,[eax+24h]
  		shr	eax,1Fh
  		not	eax
  		and	eax,00000001h
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		jmp	L00401E01
 L00401DE1:
  		mov	eax,[ebp-14h]
  		mov	eax,[eax]
  		mov	eax,[eax]
  		xor	ecx,ecx
  		cmp	eax,C0000005h
  		setz 	cl
  		mov	eax,ecx
  		retn
;------------------------------------------------------------------------------
 L00401DF5:
  		mov	esp,[ebp-18h]
 L00401DF8:
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		xor	eax,eax
 L00401E01:
  		call	SUB_L00401E75
  		retn
;------------------------------------------------------------------------------
  		Align	4
 jmp_msvcrt.dll!_initterm:
  		jmp	[msvcrt.dll!_initterm]
  		Align	8
 jmp_msvcrt.dll!_amsg_exit:
  		jmp	[msvcrt.dll!_amsg_exit]
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 SUB_L00401E23:
  		xor	eax,eax
  		inc	eax
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00401E30:
  		push	L00401E8E
  		push	fs:[00000000h]
  		mov	eax,[esp+10h]
  		mov	[esp+10h],ebp
  		lea	ebp,[esp+10h]
  		sub	esp,eax
  		push	ebx
  		push	esi
  		push	edi
  		mov	eax,[L00403000]
  		xor	[ebp-04h],eax
  		xor	eax,ebp
  		push	eax
  		mov	[ebp-18h],esp
  		push	[ebp-08h]
  		mov	eax,[ebp-04h]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		mov	[ebp-08h],eax
  		lea	eax,[ebp-10h]
  		mov	fs:[00000000h],eax
  		retn
;------------------------------------------------------------------------------
 SUB_L00401E75:
  		mov	ecx,[ebp-10h]
  		mov	fs:[00000000h],ecx
  		pop	ecx
  		pop	edi
  		pop	edi
  		pop	esi
  		pop	ebx
  		mov	esp,ebp
  		pop	ebp
  		push	ecx
  		retn
;------------------------------------------------------------------------------
  		Align	2
 L00401E8E:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		push	[ebp+14h]
  		push	[ebp+10h]
  		push	[ebp+0Ch]
  		push	[ebp+08h]
  		push	SUB_L00401CD0
  		push	L00403000
  		call	jmp_msvcrt.dll!_except_handler4_common
  		add	esp,00000018h
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00401EB8:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000010h
  		mov	eax,[L00403000]
  		and	dword ptr [ebp-08h],00000000h
  		and	dword ptr [ebp-04h],00000000h
  		push	edi
  		mov	edi,0000BB40h
  		cmp	eax,edi
  		jnz	L00401F26
  		push	esi
  		lea	eax,[ebp-08h]
  		push	eax
  		call	[KERNEL32.dll!GetSystemTimeAsFileTime]
  		mov	esi,[ebp-04h]
  		xor	esi,[ebp-08h]
  		call	[KERNEL32.dll!GetCurrentProcessId]
  		xor	esi,eax
  		call	[KERNEL32.dll!GetCurrentThreadId]
  		xor	esi,eax
  		call	[KERNEL32.dll!GetTickCount]
  		xor	esi,eax
  		lea	eax,[ebp-10h]
  		push	eax
  		call	[KERNEL32.dll!QueryPerformanceCounter]
  		mov	eax,[ebp-0Ch]
  		xor	eax,[ebp-10h]
  		xor	eax,esi
  		and	eax,0000FFFFh
  		cmp	eax,edi
  		pop	esi
  		jnz	L00401F21
  		mov	eax,0000BB41h
 L00401F21:
  		mov	[L00403000],eax
 L00401F26:
  		not	eax
  		mov	[L00403004],eax
  		pop	edi
  		leave
  		retn
;------------------------------------------------------------------------------
  		Align	2
 L00401F36:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000328h
  		mov	[L00403120],eax
  		mov	[L0040311C],ecx
  		mov	[L00403118],edx
  		mov	[L00403114],ebx
  		mov	[L00403110],esi
  		mov	[L0040310C],edi
  		mov	[L00403138],ss
  		mov	[L0040312C],cs
  		mov	[L00403108],ds
  		mov	[L00403104],es
  		mov	[L00403100],fs
  		mov	[L004030FC],gs
  		pushfd
  		pop	[L00403130]
  		mov	eax,[ebp+00h]
  		mov	[L00403124],eax
  		mov	eax,[ebp+04h]
  		mov	[L00403128],eax
  		lea	eax,[ebp+08h]
  		mov	[L00403134],eax
  		mov	eax,[ebp-00000320h]
  		mov	dword ptr [L00403070],00010001h
  		mov	eax,[L00403128]
  		mov	[L0040302C],eax
  		mov	dword ptr [L00403020],C0000409h
  		mov	dword ptr [L00403024],00000001h
  		mov	eax,[L00403000]
  		mov	[ebp-00000328h],eax
  		mov	eax,[L00403004]
  		mov	[ebp-00000324h],eax
  		push	00000000h
  		call	[KERNEL32.dll!SetUnhandledExceptionFilter]
  		push	L0040112C
  		call	[KERNEL32.dll!UnhandledExceptionFilter]
  		push	C0000409h
  		call	[KERNEL32.dll!GetCurrentProcess]
  		push	eax
  		call	[KERNEL32.dll!TerminateProcess]
  		leave
  		retn
;------------------------------------------------------------------------------
  		Align	2
 jmp_msvcrt.dll!_except_handler4_common:
  		jmp	[msvcrt.dll!_except_handler4_common]
  		Align	8
 L00402028:
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	B4h;   '?'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	SUB_L00401CA5
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401ABC
 		dd	L00401AD0
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401AFE
 		dd	L00401B12
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401B41
 		dd	L00401B55
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401B86
 		dd	L00401B9A
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401BB5
 		dd	L00401BC9
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401BEA
 		dd	L00401BFE
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401C2B
 		dd	L00401C3F
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00401C6F
 		dd	L00401C83
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L004020A8:
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	D8h;   'ò'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		dd	L00401DE1
 		dd	L00401DF5
  		dd	00002184h
  		dd	00000000h
  		dd	00000000h
  		dd	000021FCh
  		dd	0000105Ch
  		dd	00002168h
  		dd	00000000h
  		dd	00000000h
  		dd	0000229Ch
  		dd	00001040h
  		dd	00002128h
  		dd	00000000h
  		dd	00000000h
  		dd	000023E8h
  		dd	00001000h
  		dd	0000217Ch
  		dd	00000000h
  		dd	00000000h
  		dd	00002414h
  		dd	00001054h
  		dd	00000000h
  		dd	00000000h
  		dd	00000000h
  		dd	00000000h
  		dd	00000000h
  		dd	000022AAh
  		dd	000022DAh
  		dd	000023CAh
  		dd	000023AEh
  		dd	0000239Ah
  		dd	00002386h
  		dd	0000236Ch
  		dd	00002356h
  		dd	00002340h
  		dd	00002330h
  		dd	00002316h
  		dd	000022F8h
  		dd	000022C8h
  		dd	000022B8h
  		dd	000022F0h
  		dd	00000000h
  		dd	00002284h
  		dd	00002242h
  		dd	00002222h
  		dd	00002266h
  		dd	00000000h
  		dd	000023F6h
  		dd	00000000h
  		dd	00002208h
  		dd	000021ECh
  		dd	000021DEh
  		dd	000021D2h
  		dd	000021C4h
  		dd	000021BAh
  		dd	000021B0h
  		dd	000021A8h
  		dd	00000000h
  		dw	04A2h
  		db	'free',0
  		db	00h
  		dw	04DAh
  		db	'malloc',0
  		db	00h
  		dw	04EAh
  		db	'memset',0
  		db	00h
  		dw	006Ah
  		db	'_XcptFilter',0
  		dw	01D1h
  		db	'_initterm',0
  		dw	0101h
  		db	'_amsg_exit',0
  		db	00h
  		dw	00F5h
  		db	'_adjust_fdiv',0
  		db	00h
  		db	'msvcrt.dll',0
  		db	00h
  		dw	0159h
  		db	'_except_handler4_common',0
  		dw	013Dh
  		db	'SetupDiDestroyDeviceInfoList',0
  		db	00h
  		dw	016Ch
  		db	'SetupDiGetDeviceInterfaceDetailW',0
  		db	00h
  		dw	0141h
  		db	'SetupDiEnumDeviceInterfaces',0
  		dw	0154h
  		db	'SetupDiGetClassDevsW',0
  		db	00h
  		db	'SETUPAPI.dll',0
  		db	00h
  		dw	007Fh
  		db	'CreateFileW',0
  		dw	01E6h
  		db	'GetLastError',0
  		db	00h
  		dw	00CAh
  		db	'DeviceIoControl',0
  		dw	02BAh
  		db	'InterlockedExchange',0
  		dw	041Ch
  		db	'Sleep',0
  		dw	02B7h
  		db	'InterlockedCompareExchange',0
  		db	00h
  		dw	034Fh
  		db	'QueryPerformanceCounter',0
  		dw	0266h
  		db	'GetTickCount',0
  		db	00h
  		dw	01ADh
  		db	'GetCurrentThreadId',0
  		db	00h
  		dw	01AAh
  		db	'GetCurrentProcessId',0
  		dw	024Fh
  		db	'GetSystemTimeAsFileTime',0
  		dw	0428h
  		db	'TerminateProcess',0
  		db	00h
  		dw	01A9h
  		db	'GetCurrentProcess',0
  		dw	0439h
  		db	'UnhandledExceptionFilter',0
  		db	00h
  		dw	0410h
  		db	'SetUnhandledExceptionFilter',0
  		db	'KERNEL32.dll',0
  		db	00h
  		dw	0238h
  		db	'RegisterDeviceNotificationW',0
  		db	'USER32.dll',0
  		db	00h
  		dd	00000000h
  		dd	45BBF382h
  		dw	0000h
  		dw	0000h
  		dd	000024DEh
  		dd	00000001h
  		dd	0000000Fh
  		dd	0000000Fh
  		dd	00002448h
  		dd	00002484h
  		dd	000024C0h
  		dd	00001830h
  		dd	000016AEh
  		dd	00001205h
  		dd	0000160Ah
  		dd	00001761h
  		dd	0000179Eh
  		dd	000016B9h
  		dd	00001455h
  		dd	00001527h
  		dd	000013ECh
  		dd	000014BEh
  		dd	00001383h
  		dd	0000131Ah
  		dd	000016F2h
  		dd	00001590h
  		dd	000024F3h
  		dd	00002513h
  		dd	0000252Bh
  		dd	00002550h
  		dd	00002596h
  		dd	000025C3h
  		dd	000025E3h
  		dd	0000260Dh
  		dd	0000264Bh
  		dd	00002690h
  		dd	000026CDh
  		dd	00002712h
  		dd	00002750h
  		dd	00002793h
  		dd	000027B0h
  		dw	0000h
  		dw	0001h
  		dw	0002h
  		dw	0003h
  		dw	0004h
  		dw	0005h
  		dw	0006h
  		dw	0007h
  		dw	0008h
  		dw	0009h
  		dw	000Ah
  		dw	000Bh
  		dw	000Ch
  		dw	000Dh
  		dw	000Eh
  		db	'accelerometerDLL.dll',0
  		db	'?CanSettingsChange@@YGKPAXPAE@Z',0
  		db	'?ClearLogFile@@YGKPAX@Z',0
  		db	'?FindAccelerometerDevice@@YGEPAPAX@Z',0
  		db	'?GetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z',0
  		db	'?GetRealTimeXYZ@@YGKPAXPAGPAU_OVERLAPPED@@@Z',0
  		db	'?IsSoftwareEnabled@@YGKPAXPAE@Z',0
  		db	'?NotifyAccelerometerAboutPower@@YGKPAXK@Z',0
  		db	'?RegisterForAccelerometerDisabledEvent@@YGPAXPAUHWND__@@PAX@Z',0
  		db	'?RegisterForAccelerometerDiskCountChangeEvent@@YGPAXPAUHWND__@@PAX@Z',0
  		db	'?RegisterForAccelerometerEnabledEvent@@YGPAXPAUHWND__@@PAX@Z',0
  		db	'?RegisterForAccelerometerParameterChangeEvent@@YGPAXPAUHWND__@@PAX@Z',0
  		db	'?RegisterForAccelerometerShockEndEvent@@YGPAXPAUHWND__@@PAX@Z',0
  		db	'?RegisterForAccelerometerShockSignaledEvent@@YGPAXPAUHWND__@@PAX@Z',0
  		db	'?SessionChange@@YGKPAXPBGE@Z',0
  		db	'?SetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z',0
;------------------------------------------------------------------------------
 		0000000Ah DUP (??)
;
;
;------------------------------------------------------------------------------
;  Name: .data (Data Section)
;  Virtual Address:    00403000h  Virtual Size:    00000358h
;  Pointer To RawData: 00001C00h  Size Of RawData: 00000200h
;
 L00403000:
  		dd	0000BB40h
 L00403004:
  		dd	FFFF44BFh
 L00403008:
  		dd	FFFFFFFFh
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403018:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403020:
  		dd	00000000h
 L00403024:
  		dd	00000000h
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L0040302C:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403070:
  		dd	00000000h
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L004030FC:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403100:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403104:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403108:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L0040310C:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403110:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403114:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403118:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L0040311C:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403120:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403124:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403128:
  		dd	00000000h
 L0040312C:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403130:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403134:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00403138:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
;
;
;------------------------------------------------------------------------------
; Exports
;
 	Index: 1	Name: ?CanSettingsChange@@YGKPAXPAE@Z
 	Index: 2	Name: ?ClearLogFile@@YGKPAX@Z
 	Index: 3	Name: ?FindAccelerometerDevice@@YGEPAPAX@Z
 	Index: 4	Name: ?GetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z
 	Index: 5	Name: ?GetRealTimeXYZ@@YGKPAXPAGPAU_OVERLAPPED@@@Z
 	Index: 6	Name: ?IsSoftwareEnabled@@YGKPAXPAE@Z
 	Index: 7	Name: ?NotifyAccelerometerAboutPower@@YGKPAXK@Z
 	Index: 8	Name: ?RegisterForAccelerometerDisabledEvent@@YGPAXPAUHWND__@@PAX@Z
 	Index: 9	Name: ?RegisterForAccelerometerDiskCountChangeEvent@@YGPAXPAUHWND__@@PAX@Z
 	Index: 10	Name: ?RegisterForAccelerometerEnabledEvent@@YGPAXPAUHWND__@@PAX@Z
 	Index: 11	Name: ?RegisterForAccelerometerParameterChangeEvent@@YGPAXPAUHWND__@@PAX@Z
 	Index: 12	Name: ?RegisterForAccelerometerShockEndEvent@@YGPAXPAUHWND__@@PAX@Z
 	Index: 13	Name: ?RegisterForAccelerometerShockSignaledEvent@@YGPAXPAUHWND__@@PAX@Z
 	Index: 14	Name: ?SessionChange@@YGKPAXPBGE@Z
 	Index: 15	Name: ?SetAccelerometerProperty@@YGKPAXW4_ACCELEROMETER_PROPERTY_FLAGS@@0@Z
;
;------------------------------------------------------------------------------
; Imports from msvcrt.dll
;
 	extrn _except_handler4_common
 	extrn _adjust_fdiv
 	extrn _amsg_exit
 	extrn _initterm
 	extrn _XcptFilter
 	extrn memset
 	extrn malloc
 	extrn free
;
; Imports from SETUPAPI.dll
;
 	extrn SetupDiGetClassDevsW
 	extrn SetupDiGetDeviceInterfaceDetailW
 	extrn SetupDiDestroyDeviceInfoList
 	extrn SetupDiEnumDeviceInterfaces
;
; Imports from KERNEL32.dll
;
 	extrn CreateFileW
 	extrn InterlockedExchange
 	extrn SetUnhandledExceptionFilter
 	extrn UnhandledExceptionFilter
 	extrn GetCurrentProcess
 	extrn TerminateProcess
 	extrn GetSystemTimeAsFileTime
 	extrn GetCurrentProcessId
 	extrn GetCurrentThreadId
 	extrn GetTickCount
 	extrn QueryPerformanceCounter
 	extrn InterlockedCompareExchange
 	extrn DeviceIoControl
 	extrn GetLastError
 	extrn Sleep
;
; Imports from USER32.dll
;
 	extrn RegisterDeviceNotificationW
;
;------------------------------------------------------------------------------


/////////////////////////////////////

*/



/*  accelerometer.sys

;  Virtual Address:    00011000h  Virtual Size:    0000464Ch
;  Pointer To RawData: 00000400h  Size Of RawData: 00004800h
;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00011006:
  		mov	eax,[L0001708C]
  		test	eax,eax
  		push	esi
  		mov	esi,[ntoskrnl.exe!ExFreePoolWithTag]
  		jz 	L0001101B
  		push	00000000h
  		push	eax
  		call	esi
 L0001101B:
  		mov	eax,[L00017094]
  		test	eax,eax
  		jz 	L00011029
  		push	00000000h
  		push	eax
  		call	esi
 L00011029:
  		mov	eax,[L0001709C]
  		test	eax,eax
  		jz 	L00011037
  		push	00000000h
  		push	eax
  		call	esi
 L00011037:
  		mov	eax,[L000170A4]
  		test	eax,eax
  		jz 	L00011045
  		push	00000000h
  		push	eax
  		call	esi
 L00011045:
  		pop	esi
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L0001104E:
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 L00011056:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000034h
  		push	ebx
  		push	SWC00014E08__Device_Accelerometer
  		lea	eax,[ebp-14h]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		lea	eax,[ebp-08h]
  		push	eax
  		xor	ebx,ebx
  		push	ebx
  		push	00000100h
  		push	0000CF50h
  		lea	eax,[ebp-14h]
  		push	eax
  		push	00000248h
  		push	[ebp+08h]
  		call	[ntoskrnl.exe!IoCreateDevice]
  		cmp	eax,ebx
  		jl 	L00011505
  		mov	eax,[ebp-08h]
  		push	esi
  		mov	esi,[eax+28h]
  		push	edi
  		lea	edi,[esi+0Ch]
  		push	edi
  		push	ebx
  		push	L0001619C
  		push	[ebp+0Ch]
  		mov	[esi],eax
  		call	[ntoskrnl.exe!IoRegisterDeviceInterface]
  		cmp	eax,ebx
  		jge	L000110CC
  		mov	esi,eax
 L000110BC:
  		push	[ebp-08h]
  		call	[ntoskrnl.exe!IoDeleteDevice]
  		mov	eax,esi
  		jmp	L00011503
 L000110CC:
  		mov	eax,[ebp+0Ch]
  		push	eax
  		mov	[esi+04h],eax
  		push	[ebp-08h]
  		call	[ntoskrnl.exe!IoAttachDeviceToDeviceStack]
  		cmp	eax,ebx
  		mov	[esi+08h],eax
  		jnz	L000110FB
  		mov	eax,[ebp-08h]
  		and	dword ptr [eax+1Ch],FFFFFF7Fh
  		push	edi
  		call	[ntoskrnl.exe!RtlFreeUnicodeString]
  		mov	esi,C0000001h
  		jmp	L000110BC
 L000110FB:
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	SWC00014DF8_Enabled
  		mov	edi,L00017098
  		push	edi
  		mov	dword ptr [ebp-04h],00000001h
  		call	SUB_L000129C2
  		xor	eax,eax
  		cmp	[ebp-04h],bl
  		setnz	al
  		cmp	al,bl
  		setz 	al
  		mov	[esi+41h],al
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	L00014DCA
  		push	edi
  		mov	dword ptr [ebp-04h],00004E20h
  		call	SUB_L000129C2
  		mov	eax,[ebp-04h]
  		mov	[esi+000000C4h],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	L00014D9A
  		push	edi
  		mov	dword ptr [ebp-04h],000007D0h
  		call	SUB_L000129C2
  		mov	eax,[ebp-04h]
  		mov	[esi+000000C8h],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	SWC00014D68_ClearInterruptInSoftware
  		push	edi
  		mov	dword ptr [ebp-04h],00000001h
  		call	SUB_L000129C2
  		mov	al,[ebp-04h]
  		mov	[esi+000001FCh],al
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	SWC00014D3C_CreateErrorLogEntries
  		push	edi
  		mov	[ebp-04h],ebx
  		call	SUB_L000129C2
  		cmp	[ebp-04h],bl
  		setnz	al
  		mov	[esi+000001FDh],al
  		mov	[L000170B8],al
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	SWC00014D20_ErrorLogLimit
  		push	edi
  		mov	dword ptr [ebp-04h],0000000Ah
  		call	SUB_L000129C2
  		movzx	eax,[ebp-04h]
  		mov	[esi+00000200h],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	L00014CEE
  		push	edi
  		mov	dword ptr [ebp-04h],00000001h
  		call	SUB_L000129C2
  		xor	eax,eax
  		cmp	[ebp-04h],bl
  		setnz	al
  		mov	[esi+00000134h],al
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	L00014CC6
  		push	edi
  		mov	dword ptr [ebp-04h],00000001h
  		call	SUB_L000129C2
  		xor	eax,eax
  		cmp	[ebp-04h],bl
  		setnz	al
  		mov	[esi+00000135h],al
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	L00014C8E
  		push	edi
  		mov	[ebp-04h],ebx
  		call	SUB_L000129C2
  		cmp	[ebp-04h],ebx
  		jz 	L0001124D
  		push	FFFFFFFFh
  		push	FFFFD8F0h
  		push	ebx
  		push	[ebp-04h]
  		call	jmp_ntoskrnl.exe!_allmul
  		mov	[esi+38h],eax
  		mov	[esi+3Ch],edx
 L0001124D:
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	L00014C6A
  		push	L000170A0
  		mov	[ebp-04h],ebx
  		call	SUB_L000129C2
  		cmp	[ebp-04h],ebx
  		jz 	L00011276
  		cmp	[ebp-04h],bl
  		setnz	al
  		mov	[esi+00000101h],al
 L00011276:
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000004h
  		push	SWC00014C4C_ShocksDetected
  		push	L00017090
  		mov	[ebp-04h],ebx
  		call	SUB_L000129C2
  		mov	eax,[ebp-04h]
  		mov	edi,[ntoskrnl.exe!KeInitializeEvent]
  		push	ebx
  		mov	[esi+00000244h],eax
  		push	00000001h
  		lea	eax,[esi+5Ch]
  		push	eax
  		call	edi
  		push	ebx
  		push	00000001h
  		lea	eax,[esi+44h]
  		push	eax
  		call	edi
  		push	ebx
  		push	00000001h
  		lea	eax,[esi+00000124h]
  		push	eax
  		mov	[esi+6Ch],bl
  		mov	[esi+6Dh],bl
  		mov	[esi+00000082h],bl
  		mov	[esi+00000083h],bl
  		call	edi
  		push	ebx
  		push	00000001h
  		lea	eax,[esi+00000104h]
  		push	eax
  		call	edi
  		push	ebx
  		push	00000001h
  		lea	eax,[esi+00000114h]
  		push	eax
  		call	edi
  		push	ebx
  		lea	eax,[esi+00000140h]
  		push	ebx
  		push	eax
  		call	edi
  		push	ebx
  		lea	eax,[esi+00000150h]
  		push	ebx
  		push	eax
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000164h]
  		push	eax
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000174h]
  		push	eax
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+000001C8h]
  		push	eax
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+000001D8h]
  		push	eax
  		call	edi
  		push	ebx
  		push	00000001h
  		lea	eax,[esi+000001E8h]
  		push	eax
  		call	edi
  		mov	dword ptr [esi+00000190h],00000001h
  		mov	byte ptr [esi+30h],01h
  		mov	byte ptr [esi+32h],01h
  		mov	dword ptr [ebp-34h],00000018h
  		mov	[ebp-30h],ebx
  		mov	dword ptr [ebp-28h],00000200h
  		mov	[ebp-2Ch],ebx
  		push	esi
  		push	L00013F3A
  		push	ebx
  		push	ebx
  		lea	eax,[ebp-34h]
  		push	eax
  		push	001FFFFFh
  		lea	eax,[esi+0000013Ch]
  		push	eax
  		mov	[ebp-24h],ebx
  		mov	[ebp-20h],ebx
  		call	[ntoskrnl.exe!PsCreateSystemThread]
  		cmp	eax,ebx
  		mov	[ebp+0Ch],eax
  		jge	L000113A9
  		lea	eax,[esi+0Ch]
  		push	eax
  		call	[ntoskrnl.exe!RtlFreeUnicodeString]
  		mov	eax,[ebp-08h]
  		and	dword ptr [eax+1Ch],FFFFFF7Fh
 L00011398:
  		push	[esi+08h]
  		call	[ntoskrnl.exe!IoDetachDevice]
  		mov	esi,[ebp+0Ch]
  		jmp	L000110BC
 L000113A9:
  		push	esi
  		push	L000140BA
  		push	ebx
  		push	ebx
  		lea	eax,[ebp-34h]
  		push	eax
  		push	001FFFFFh
  		lea	eax,[esi+000001C4h]
  		push	eax
  		call	[ntoskrnl.exe!PsCreateSystemThread]
  		cmp	eax,ebx
  		mov	[ebp+0Ch],eax
  		jge	L000113FB
  		lea	eax,[esi+0Ch]
  		push	eax
  		call	[ntoskrnl.exe!RtlFreeUnicodeString]
  		push	00000001h
  		push	ebx
  		lea	eax,[esi+00000140h]
  		push	eax
  		call	[ntoskrnl.exe!KeSetEvent]
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000150h]
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForSingleObject]
  		jmp	L00011398
 L000113FB:
  		cmp	[esi+000001FDh],bl
  		jz 	L00011438
  		push	esi
  		push	L000146A2
  		push	ebx
  		push	ebx
  		lea	eax,[ebp-34h]
  		push	eax
  		push	001FFFFFh
  		lea	eax,[esi+00000160h]
  		push	eax
  		call	[ntoskrnl.exe!PsCreateSystemThread]
  		test	eax,eax
  		jge	L0001142D
  		mov	[esi+000001FDh],bl
  		jmp	L00011438
 L0001142D:
  		push	SSZ00014C3C_Log_File_Opened
  		call	SUB_L00013A0E
  		pop	ecx
 L00011438:
  		push	ebx
  		lea	eax,[esi+000000D8h]
  		push	eax
  		call	[ntoskrnl.exe!KeInitializeTimerEx]
  		push	00000001h
  		lea	eax,[esi+00000204h]
  		push	eax
  		mov	byte ptr [esi+000000C0h],01h
  		mov	word ptr [esi+000000CEh],0001h
  		mov	dword ptr [esi+000000D0h],00000080h
  		call	[ntoskrnl.exe!KeInitializeMutex]
  		lea	eax,[esi+00000224h]
  		push	ebx
  		mov	[eax+04h],eax
  		mov	[eax],eax
  		push	00000001h
  		lea	eax,[esi+0000022Ch]
  		push	eax
  		call	edi
  		push	L00014C12
  		lea	eax,[ebp-1Ch]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		xor	edi,edi
  		lea	eax,[ebp-1Ch]
  		inc	edi
  		push	edi
  		mov	[ebp-2Ch],eax
  		push	ebx
  		lea	eax,[ebp-34h]
  		push	eax
  		lea	eax,[ebp-0Ch]
  		push	eax
  		mov	dword ptr [ebp-34h],00000018h
  		mov	[ebp-30h],ebx
  		mov	dword ptr [ebp-28h],00000050h
  		mov	[ebp-24h],ebx
  		mov	[ebp-20h],ebx
  		call	[ntoskrnl.exe!ExCreateCallback]
  		test	eax,eax
  		jl 	L000114DE
  		push	esi
  		push	L00014404
  		push	[ebp-0Ch]
  		call	[ntoskrnl.exe!ExRegisterCallback]
  		mov	[esi+000001F8h],eax
 L000114DE:
  		mov	eax,edi
  		push	eax
  		mov	[L000170A8],esi
  		push	edi
  		mov	[esi+18h],edi
  		mov	[esi+14h],edi
  		push	[ebp-08h]
  		call	[ntoskrnl.exe!PoSetPowerState]
  		mov	eax,[ebp-08h]
  		and	dword ptr [eax+1Ch],FFFFFF7Fh
  		xor	eax,eax
 L00011503:
  		pop	edi
  		pop	esi
 L00011505:
  		pop	ebx
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	8
 L00011510:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+0Ch]
  		push	eax
  		mov	eax,[eax]
  		push	00000000h
  		add	eax,00000074h
  		push	eax
  		call	[ntoskrnl.exe!KeInsertQueueDpc]
  		mov	al,01h
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 L00011532:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+0Ch]
  		sub	esp,0000000Ch
  		push	ebx
  		push	esi
  		mov	esi,[eax+28h]
  		lea	edx,[ebp-0Ch]
  		mov	ecx,L000170B4
  		call	[ntoskrnl.exe!KeAcquireInStackQueuedSpinLockAtDpcLevel]
  		xor	ebx,ebx
  		cmp	[esi+41h],bl
  		jnz	L00011686
  		cmp	[esi+30h],bl
  		jnz	L00011686
  		cmp	[esi+31h],bl
  		jnz	L00011686
  		inc	[esi+00000244h]
  		cmp	[esi+000000D4h],bl
  		push	edi
  		jnz	L0001161A
  		mov	edi,[ntoskrnl.exe!KeSetEvent]
  		push	ebx
  		push	ebx
  		lea	eax,[esi+5Ch]
  		push	eax
  		mov	byte ptr [esi+40h],01h
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000104h]
  		push	eax
  		call	edi
  		push	[esi+00000244h]
  		push	SSZ00014EA6_Shock_Event_Signaled__d
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
  		push	esi
  		call	SUB_L00012C8A
  		cmp	[esi+000000C0h],bl
  		push	FFFFFFFFh
  		push	FFFFD8F0h
  		mov	byte ptr [esi+000000D4h],01h
  		push	ebx
  		jz 	L000115D4
  		push	[esi+000000C8h]
  		jmp	L000115DA
 L000115D4:
  		push	[esi+000000C4h]
 L000115DA:
  		call	jmp_ntoskrnl.exe!_allmul
  		mov	[esi+000000B8h],eax
  		lea	eax,[esi+00000098h]
  		push	eax
  		push	edx
  		push	[esi+000000B8h]
  		lea	eax,[esi+000000D8h]
  		push	eax
  		mov	[esi+000000BCh],edx
  		call	[ntoskrnl.exe!KeSetTimer]
  		push	[esi+00000244h]
  		push	SSZ00014E88_Shock_Event_Timer_Started__d
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
  		jmp	L0001167A
 L0001161A:
  		lea	edi,[esi+000000D8h]
  		push	edi
  		call	[ntoskrnl.exe!KeCancelTimer]
  		test	al,al
  		jz 	L00011652
  		lea	eax,[esi+00000098h]
  		push	eax
  		push	[esi+000000BCh]
  		push	[esi+000000B8h]
  		push	edi
  		call	[ntoskrnl.exe!KeSetTimer]
  		push	[esi+00000244h]
  		push	SSZ00014E5A_Timer_Running__Shock_Event_Timer
  		jmp	L00011664
 L00011652:
  		push	[esi+00000244h]
  		mov	byte ptr [esi+00000100h],01h
  		push	SSZ00014E34_Shock_Event_Timer_Already_Runnin
 L00011664:
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
  		push	ebx
  		push	ebx
  		add	esi,00000124h
  		push	esi
  		call	[ntoskrnl.exe!KeSetEvent]
 L0001167A:
  		lea	ecx,[ebp-0Ch]
  		call	[ntoskrnl.exe!KeReleaseInStackQueuedSpinLockFromDpcLevel]
  		pop	edi
  		jmp	L0001169E
 L00011686:
  		push	ebx
  		push	ebx
  		add	esi,00000124h
  		push	esi
  		call	[ntoskrnl.exe!KeSetEvent]
  		lea	ecx,[ebp-0Ch]
  		call	[ntoskrnl.exe!KeReleaseInStackQueuedSpinLockFromDpcLevel]
 L0001169E:
  		pop	esi
  		pop	ebx
  		leave
  		retn	0010h
;------------------------------------------------------------------------------
  		Align	2
 L000116AA:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000000Ch
  		push	ebx
  		push	esi
  		push	edi
  		lea	edx,[ebp-0Ch]
  		mov	ecx,L000170B4
  		call	[ntoskrnl.exe!KeAcquireInStackQueuedSpinLockAtDpcLevel]
  		mov	esi,[ebp+0Ch]
  		xor	ebx,ebx
  		cmp	[esi+00000100h],bl
  		jnz	L00011714
  		mov	edi,[ntoskrnl.exe!KeSetEvent]
  		push	ebx
  		push	ebx
  		lea	eax,[esi+44h]
  		push	eax
  		mov	[esi+40h],bl
  		mov	[esi+000000D4h],bl
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000114h]
  		push	eax
  		call	edi
  		push	ebx
  		push	00000001h
  		push	L000170CC
  		call	edi
  		push	[esi+00000244h]
  		push	SSZ00014EE0_Shock_End_Event_Signaled__d
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
  		push	esi
  		call	SUB_L00012CEE
 L00011714:
  		push	ebx
  		lea	edi,[esi+000000D8h]
  		push	edi
  		call	[ntoskrnl.exe!KeInitializeTimerEx]
  		cmp	[esi+00000100h],bl
  		jz 	L00011786
  		cmp	[esi+000000C0h],bl
  		push	FFFFFFFFh
  		push	FFFFD8F0h
  		push	ebx
  		jz 	L00011742
  		push	[esi+000000C8h]
  		jmp	L00011748
 L00011742:
  		push	[esi+000000C4h]
 L00011748:
  		call	jmp_ntoskrnl.exe!_allmul
  		mov	[esi+000000B8h],eax
  		lea	eax,[esi+00000098h]
  		push	eax
  		push	edx
  		push	[esi+000000B8h]
  		mov	[esi+000000BCh],edx
  		push	edi
  		call	[ntoskrnl.exe!KeSetTimer]
  		push	[esi+00000244h]
  		mov	[esi+00000100h],bl
  		push	SSZ00014EBE_Shock_Event_Duration_Extended__d
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
 L00011786:
  		lea	ecx,[ebp-0Ch]
  		call	[ntoskrnl.exe!KeReleaseInStackQueuedSpinLockFromDpcLevel]
  		pop	edi
  		pop	esi
  		pop	ebx
  		leave
  		retn	0010h
;------------------------------------------------------------------------------
  		Align	4
 SUB_L0001179C:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	esi
  		mov	esi,[ebp+08h]
  		cmp	byte ptr [esi+33h],00h
  		jz 	L000117B3
  		push	[esi]
  		call	[ntoskrnl.exe!IoUnregisterShutdownNotification]
 L000117B3:
  		mov	eax,[esi+1Ch]
  		test	eax,eax
  		jz 	L000117C9
  		push	eax
  		call	[ntoskrnl.exe!IoDisconnectInterrupt]
  		and	dword ptr [esi+1Ch],00000000h
  		mov	byte ptr [esi+20h],00h
 L000117C9:
  		pop	esi
  		pop	ebp
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	4
 SUB_L000117D4:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		push	esi
  		mov	esi,[ebp+08h]
  		push	edi
  		xor	edi,edi
  		push	edi
  		push	edi
  		lea	eax,[esi+00000140h]
  		push	eax
  		call	[ntoskrnl.exe!KeSetEvent]
  		cmp	[esi+0000013Ch],edi
  		mov	ebx,[ntoskrnl.exe!KeWaitForSingleObject]
  		jz 	L00011818
  		push	edi
  		push	edi
  		push	edi
  		push	edi
  		lea	eax,[esi+00000150h]
  		push	eax
  		call	ebx
  		lea	eax,[esi+0000013Ch]
  		push	eax
  		call	[ntoskrnl.exe!ZwClose]
 L00011818:
  		push	edi
  		push	edi
  		lea	eax,[esi+000001C8h]
  		push	eax
  		call	[ntoskrnl.exe!KeSetEvent]
  		cmp	[esi+000001C4h],edi
  		jz 	L00011849
  		push	edi
  		push	edi
  		push	edi
  		push	edi
  		lea	eax,[esi+000001D8h]
  		push	eax
  		call	ebx
  		lea	eax,[esi+000001C4h]
  		push	eax
  		call	[ntoskrnl.exe!ZwClose]
 L00011849:
  		push	edi
  		push	edi
  		lea	eax,[esi+00000164h]
  		push	eax
  		call	[ntoskrnl.exe!KeSetEvent]
  		cmp	[esi+00000160h],edi
  		jz 	L0001187A
  		push	edi
  		push	edi
  		push	edi
  		push	edi
  		lea	eax,[esi+00000174h]
  		push	eax
  		call	ebx
  		lea	eax,[esi+00000160h]
  		push	eax
  		call	[ntoskrnl.exe!ZwClose]
 L0001187A:
  		mov	esi,[esi+000001F8h]
  		cmp	esi,edi
  		jz 	L0001188B
  		push	esi
  		call	[ntoskrnl.exe!ExUnregisterCallback]
 L0001188B:
  		pop	edi
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00011898:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000010h
  		push	ebx
  		push	esi
  		push	edi
  		push	00000000h
  		push	00000000h
  		lea	eax,[ebp-10h]
  		push	eax
  		call	[ntoskrnl.exe!KeInitializeEvent]
  		mov	ebx,[ebp+0Ch]
  		mov	esi,[ebx+60h]
  		lea	eax,[esi-24h]
  		push	00000007h
  		mov	edi,eax
  		pop	ecx
  		rep movsd
  		mov	byte ptr [eax+03h],00h
  		mov	eax,[ebx+60h]
  		sub	eax,00000024h
  		lea	ecx,[ebp-10h]
  		mov	[eax+20h],ecx
  		mov	ecx,[ebp+08h]
  		mov	edx,ebx
  		mov	dword ptr [eax+1Ch],L00012D52
  		mov	byte ptr [eax+03h],E0h
  		call	[ntoskrnl.exe!IofCallDriver]
  		cmp	eax,00000103h
  		jnz	L00011901
  		xor	eax,eax
  		push	eax
  		push	eax
  		push	eax
  		push	eax
  		lea	eax,[ebp-10h]
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForSingleObject]
  		mov	eax,[ebx+18h]
 L00011901:
  		pop	edi
  		pop	esi
  		pop	ebx
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L0001190E:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000002Ch
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	ecx,[ebp+0Ch]
  		mov	eax,[ecx+04h]
  		mov	ecx,[ecx+08h]
  		push	ebx
  		xor	ebx,ebx
  		cmp	eax,ebx
  		push	esi
  		mov	esi,[ebp+08h]
  		jz 	L000119C7
  		cmp	ecx,ebx
  		jz 	L000119C7
  		cmp	[eax],ebx
  		jnz	L0001194E
  		mov	eax,C000009Ah
  		jmp	L00011AC7
 L0001194E:
  		cmp	[eax+10h],ebx
  		push	edi
  		mov	[ebp-2Ch],ebx
  		jbe	L0001199B
  		lea	edi,[eax+14h]
  		add	ecx,0000001Ch
 L0001195D:
  		movzx	edx,[edi]
  		dec	edx
  		dec	edx
  		jnz	L0001198A
  		cmp	[esi+20h],bl
  		jnz	L000119C7
  		cmp	byte ptr [ecx-08h],02h
  		jnz	L000119C7
  		mov	dl,[ecx-04h]
  		mov	[esi+20h],dl
  		mov	edx,[ecx]
  		mov	[esi+24h],edx
  		mov	edx,[ecx+04h]
  		mov	[esi+28h],edx
  		movzx	edx,[ecx-06h]
  		and	edx,00000001h
  		mov	[esi+2Ch],edx
 L0001198A:
  		inc	[ebp-2Ch]
  		mov	edx,[ebp-2Ch]
  		add	ecx,00000010h
  		add	edi,00000010h
  		cmp	edx,[eax+10h]
  		jc 	L0001195D
 L0001199B:
  		cmp	[esi+20h],bl
  		jz 	L000119C7
  		push	00000020h
  		lea	eax,[ebp-24h]
  		push	eax
  		push	ebx
  		push	ebx
  		push	00000005h
  		call	[ntoskrnl.exe!ZwPowerInformation]
  		test	eax,eax
  		jl 	L000119EA
  		cmp	[ebp-24h],bl
  		jz 	L000119D6
  		mov	[esi+000000CCh],bx
  		push	SSZ00014F7E_Query_Power_State_On_AC_
  		jmp	L000119E4
 L000119C7:
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		push	00010001h
  		call	[ntoskrnl.exe!KeBugCheckEx]
 L000119D6:
  		mov	word ptr [esi+000000CCh],0001h
  		push	SSZ00014F64_Query_Power_State_On_DC_
 L000119E4:
  		call	SUB_L00013A0E
  		pop	ecx
 L000119EA:
  		push	00000001h
  		lea	eax,[ebp-25h]
  		push	eax
  		push	ebx
  		push	00000005h
  		push	esi
  		call	SUB_L00012D72
  		test	eax,eax
  		jl 	L00011A0B
  		cmp	[ebp-25h],bl
  		jnz	L00011A16
  		mov	[esi+000000CEh],bx
  		jmp	L00011A16
 L00011A0B:
  		push	SSZ00014F2E_Query_Lid_State_from_BIOS_failed
  		call	SUB_L00013A0E
  		pop	ecx
 L00011A16:
  		cmp	[esi+000000CEh],bx
  		jnz	L00011A30
  		push	SSZ00014EFC_Initial_Lid_State_is_CLOSED__Usi
  		call	SUB_L00013A0E
  		pop	ecx
  		mov	byte ptr [esi+42h],01h
  		jmp	L00011A36
 L00011A30:
  		push	esi
  		call	SUB_L00013C06
 L00011A36:
  		lea	eax,[esi+000000CCh]
  		push	eax
  		push	esi
  		call	SUB_L00013BA6
  		mov	eax,[esi]
  		mov	edi,[ntoskrnl.exe!KeInitializeDpc]
  		push	eax
  		push	L00011532
  		add	eax,00000074h
  		push	eax
  		call	edi
  		mov	eax,[esi]
  		mov	ebx,[ntoskrnl.exe!KeSetImportanceDpc]
  		push	00000002h
  		add	eax,00000074h
  		push	eax
  		call	ebx
  		push	esi
  		lea	eax,[esi+00000098h]
  		push	L000116AA
  		push	eax
  		call	edi
  		push	00000002h
  		lea	eax,[esi+00000098h]
  		push	eax
  		call	ebx
  		movzx	eax,[esi+20h]
  		push	00000000h
  		push	[esi+28h]
  		push	00000001h
  		push	[esi+2Ch]
  		push	eax
  		push	eax
  		push	[esi+24h]
  		lea	eax,[esi+1Ch]
  		push	00000000h
  		push	esi
  		push	L00011510
  		push	eax
  		call	[ntoskrnl.exe!IoConnectInterrupt]
  		mov	edi,eax
  		test	edi,edi
  		jl 	L00011ABA
  		push	[esi]
  		call	[ntoskrnl.exe!IoRegisterShutdownNotification]
  		mov	edi,eax
  		test	edi,edi
  		jge	L00011AC4
 L00011ABA:
  		push	esi
  		call	SUB_L0001179C
  		mov	eax,edi
  		jmp	L00011AC6
 L00011AC4:
  		xor	eax,eax
 L00011AC6:
  		pop	edi
 L00011AC7:
  		mov	ecx,[ebp-04h]
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L000147FB
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	4
 L00011ADC:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		push	ecx
  		mov	ecx,[ebp+08h]
  		push	ebx
  		push	esi
  		mov	esi,[ecx+28h]
  		mov	ecx,[esi+08h]
  		push	edi
  		mov	edi,[ebp+0Ch]
  		mov	eax,[edi+60h]
  		mov	[ebp-04h],eax
  		movzx	eax,[eax+01h]
  		xor	ebx,ebx
  		cmp	eax,00000004h
  		mov	[ebp+0Ch],ecx
  		jg 	L00011BF7
  		jz 	L00011BE7
  		sub	eax,ebx
  		jz 	L00011B90
  		dec	eax
  		jz 	L00011C2A
  		dec	eax
  		jz 	L00011B2F
  		dec	eax
  		jnz	L00011C37
 L00011B24:
  		push	esi
  		call	SUB_L00013384
  		jmp	L00011C37
 L00011B2F:
  		lea	eax,[esi+0Ch]
  		push	ebx
  		push	eax
  		mov	[L000170A8],ebx
  		call	[ntoskrnl.exe!IoSetDeviceInterfaceState]
  		lea	eax,[esi+0Ch]
  		push	eax
  		call	[ntoskrnl.exe!RtlFreeUnicodeString]
  		push	esi
  		call	SUB_L0001179C
  		push	esi
  		call	SUB_L000117D4
  		push	edi
  		push	[esi+08h]
  		call	SUB_L00011898
  		push	[esi+08h]
  		mov	[ebp+0Ch],eax
  		call	[ntoskrnl.exe!IoDetachDevice]
  		push	[esi]
  		call	[ntoskrnl.exe!IoDeleteDevice]
  		mov	[esi+08h],ebx
  		mov	esi,[ebp+0Ch]
  		xor	dl,dl
  		mov	ecx,edi
  		mov	[edi+18h],esi
  		mov	[edi+1Ch],ebx
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		mov	eax,esi
  		jmp	L00011C49
 L00011B90:
  		push	edi
  		push	ecx
  		call	SUB_L00011898
  		cmp	eax,ebx
  		mov	[ebp+0Ch],eax
  		jl 	L00011BCF
  		push	[ebp-04h]
  		push	esi
  		call	SUB_L0001190E
  		cmp	eax,ebx
  		mov	[ebp+0Ch],eax
  		jl 	L00011BCF
  		push	00000001h
  		lea	eax,[esi+0Ch]
  		push	eax
  		call	[ntoskrnl.exe!IoSetDeviceInterfaceState]
  		cmp	[esi+41h],bl
  		push	esi
  		jz 	L00011BC7
  		call	SUB_L00012B5E
  		jmp	L00011BCC
 L00011BC7:
  		call	SUB_L00012AFA
 L00011BCC:
  		mov	[ebp+0Ch],ebx
 L00011BCF:
  		mov	eax,[ebp+0Ch]
  		xor	dl,dl
  		mov	ecx,edi
  		mov	[edi+18h],eax
  		mov	[edi+1Ch],ebx
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		mov	eax,[ebp+0Ch]
  		jmp	L00011C49
 L00011BE7:
  		push	esi
  		call	SUB_L0001179C
  		xor	ebx,ebx
  		mov	[edi+18h],ebx
  		mov	[edi+1Ch],ebx
  		jmp	L00011C37
 L00011BF7:
  		sub	eax,00000005h
  		jz 	L00011C2A
  		dec	eax
  		jz 	L00011B24
  		sub	eax,00000011h
  		jnz	L00011C37
  		xor	ebx,ebx
  		push	ebx
  		push	esi
  		call	SUB_L00013C4E
  		push	ebx
  		lea	eax,[esi+0Ch]
  		push	eax
  		mov	[L000170A8],ebx
  		call	[ntoskrnl.exe!IoSetDeviceInterfaceState]
  		push	esi
  		call	SUB_L0001179C
  		jmp	L00011C37
 L00011C2A:
  		push	ebx
  		push	esi
  		mov	[edi+18h],ebx
  		mov	[edi+1Ch],ebx
  		call	SUB_L00013C4E
 L00011C37:
  		mov	ecx,[ebp+0Ch]
  		inc	[edi+23h]
  		add	dword ptr [edi+60h],00000024h
  		mov	edx,edi
  		call	[ntoskrnl.exe!IofCallDriver]
 L00011C49:
  		pop	edi
  		pop	esi
  		pop	ebx
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 L00011C56:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		mov	ecx,[ebp+0Ch]
  		and	dword ptr [ecx+18h],00000000h
  		and	dword ptr [ecx+1Ch],00000000h
  		xor	dl,dl
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		xor	eax,eax
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 L00011C7A:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+08h]
  		sub	esp,0000000Ch
  		push	ebx
  		mov	ebx,[eax+28h]
  		push	esi
  		push	edi
  		mov	edi,[ebp+0Ch]
  		mov	eax,[edi+60h]
  		mov	ecx,[eax+0Ch]
  		xor	edx,edx
  		cmp	[edi+20h],dl
  		jz 	L00011CB8
  		mov	eax,C0000010h
  		mov	[edi+18h],eax
  		mov	esi,eax
 L00011CA5:
  		xor	dl,dl
  		mov	ecx,edi
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		pop	edi
  		mov	eax,esi
  		pop	esi
  		pop	ebx
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
 L00011CB8:
  		cmp	ecx,CF50A004h
  		mov	[edi+1Ch],edx
  		jnz	L00011D1D
  		mov	esi,[edi+0Ch]
  		cmp	esi,edx
  		mov	[ebp+08h],edx
  		jz 	L00011D1D
  		cmp	dword ptr [eax+08h],00000018h
  		jnz	L00011D1D
  		lea	edx,[ebp-0Ch]
  		mov	ecx,L000170B4
  		call	[HAL.dll!KeAcquireInStackQueuedSpinLock]
  		mov	eax,[L000170B0]
  		mov	dword ptr [esi],L000170AC
  		mov	[esi+04h],eax
  		mov	[eax],esi
  		mov	[L000170B0],esi
  		add	ebx,00000040h
  		lea	ecx,[ebp-0Ch]
  		mov	dword ptr [esi+08h],L0001297C
  		mov	dword ptr [esi+0Ch],SUB_L00013A0E
  		mov	dword ptr [esi+10h],L0001336E
  		mov	[esi+14h],ebx
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
  		jmp	L00011D24
 L00011D1D:
  		mov	dword ptr [ebp+08h],C000000Dh
 L00011D24:
  		mov	esi,[ebp+08h]
  		mov	[edi+18h],esi
  		jmp	L00011CA5
  		Align	4
 L00011D34:
 		db	6Ah;   'j'
 		db	10h;
 		db	68h;   'h'
 		dd	L000162B8
  		call	SUB_L00014860
  		mov	eax,[ebp+28h]
  		mov	ecx,[eax+28h]
  		cmp	dword ptr [ebp+20h],CF50A024h
  		jnz	L00011DE3
  		mov	eax,[ebp+10h]
  		xor	edi,edi
  		cmp	eax,edi
  		jz 	L00011DD3
  		cmp	dword ptr [ebp+14h],00000004h
  		jc 	L00011DD3
  		mov	[ebp-04h],edi
  		mov	esi,[eax]
  		mov	[ebp-20h],esi
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		cmp	esi,00000012h
  		ja 	CASE_00011DF0_PROC0002
  		movzx	eax,[esi+CASE_00011DFC]
  		jmp	[CASE_PROCTABLE_00011DF0+eax*4]
 CASE_00011DF0_PROC0000:
  		push	edi
  		push	ecx
  		call	SUB_L00013C4E
  		push	esi
  		push	SSZ00014FC0_FastIOCTL_rcvd__IgnoreInterrupts
  		jmp	L00011D9F
 CASE_00011DF0_PROC0001:
  		push	ecx
  		call	SUB_L00013384
  		push	esi
  		push	SSZ00014F98_FastIOCTL_rcvd__No_IgnoreInterru
 L00011D9F:
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
 CASE_00011DF0_PROC0002:
  		mov	eax,[ebp+24h]
  		mov	[eax],edi
  		jmp	L00011DDC
 L00011DAD:
  		mov	eax,[ebp-14h]
  		mov	eax,[eax]
  		mov	eax,[eax]
  		mov	[ebp-1Ch],eax
  		xor	eax,eax
  		inc	eax
  		retn
;------------------------------------------------------------------------------
 L00011DBB:
  		mov	esp,[ebp-18h]
  		mov	eax,[ebp+24h]
  		mov	ecx,[ebp-1Ch]
  		mov	[eax],ecx
  		and	dword ptr [eax+04h],00000000h
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		jmp	L00011DDF
 L00011DD3:
  		mov	eax,[ebp+24h]
  		mov	dword ptr [eax],C000000Dh
 L00011DDC:
  		mov	[eax+04h],edi
 L00011DDF:
  		mov	al,01h
  		jmp	L00011DE5
 L00011DE3:
  		xor	al,al
 L00011DE5:
  		call	SUB_L000148A5
  		retn	0024h
;------------------------------------------------------------------------------
  		Align	4
 CASE_PROCTABLE_00011DF0:
 		dd	CASE_00011DF0_PROC0000
 		dd	CASE_00011DF0_PROC0001
 		dd	CASE_00011DF0_PROC0002
 CASE_00011DFC:
  		db	00h, 00h, 01h, 01h, 00h, 00h, 01h, 01h, 01h, 00h, 02h, 02h, 02h, 02h, 02h, 02h
  		db	02h, 02h, 01h
  		Align	4
 L00011E14:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000001Ch
  		or	dword ptr [ebp-10h],FFFFFFFFh
  		or	dword ptr [ebp-18h],FFFFFFFFh
  		push	ebx
  		push	esi
  		push	edi
  		push	00000010h
  		mov	dword ptr [ebp-14h],FFE17B80h
  		mov	dword ptr [ebp-1Ch],FF676980h
  		call	jmp_ntoskrnl.exe!KeGetCurrentThread
  		push	eax
  		call	[ntoskrnl.exe!KeSetPriorityThread]
  		mov	esi,[ebp+08h]
  		xor	ebx,ebx
 L00011E48:
  		lea	eax,[ebp-14h]
  		push	eax
  		push	ebx
  		push	ebx
  		call	[ntoskrnl.exe!KeDelayExecutionThread]
  		xor	edi,edi
  		inc	edi
  		push	edi
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000028h
  		push	00000002h
  		push	esi
  		call	SUB_L00012D72
  		movzx	eax,[ebp-04h]
  		push	edi
  		mov	[ebp+08h],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	00000029h
  		push	00000002h
  		push	esi
  		call	SUB_L00012D72
  		xor	eax,eax
  		mov	ah,[ebp-04h]
  		push	edi
  		or	[ebp+08h],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	0000002Ah
  		push	00000002h
  		push	esi
  		call	SUB_L00012D72
  		movzx	eax,[ebp-04h]
  		push	edi
  		mov	[ebp-08h],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	0000002Bh
  		push	00000002h
  		push	esi
  		call	SUB_L00012D72
  		xor	eax,eax
  		mov	ah,[ebp-04h]
  		push	edi
  		or	[ebp-08h],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	0000002Ch
  		push	00000002h
  		push	esi
  		call	SUB_L00012D72
  		movzx	eax,[ebp-04h]
  		push	edi
  		mov	[ebp-0Ch],eax
  		lea	eax,[ebp-04h]
  		push	eax
  		push	0000002Dh
  		push	00000002h
  		push	esi
  		call	SUB_L00012D72
  		xor	eax,eax
  		mov	ah,[ebp-04h]
  		or	[ebp-0Ch],eax
  		lea	eax,[ebp-1Ch]
  		push	eax
  		push	ebx
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000204h]
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForSingleObject]
  		test	eax,eax
  		jz 	L00011F07
  		cmp	[esi+00000240h],bl
  		jz 	L00011E48
  		jmp	L00011FB9
 L00011F07:
  		lea	eax,[esi+00000224h]
  		mov	edi,[eax]
  		jmp	L00011F7E
 L00011F11:
  		lea	ecx,[edi-58h]
  		cmp	[ecx+24h],bl
  		jz 	L00011F43
  		mov	eax,[edi]
  		mov	edi,[edi+04h]
  		mov	[edi],eax
  		mov	[eax+04h],edi
  		xor	dl,dl
  		mov	dword ptr [ecx+18h],C0000120h
  		mov	[ecx+1Ch],ebx
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		lea	eax,[esi+00000224h]
  		cmp	[eax],eax
  		jz 	L00011F95
  		mov	edi,[eax]
  		jmp	L00011F76
 L00011F43:
  		mov	ecx,[ecx+04h]
  		test	byte ptr [ecx+06h],05h
  		jz 	L00011F51
  		mov	eax,[ecx+0Ch]
  		jmp	L00011F5F
 L00011F51:
  		push	00000010h
  		push	ebx
  		push	ebx
  		push	00000001h
  		push	ebx
  		push	ecx
  		call	[ntoskrnl.exe!MmMapLockedPagesSpecifyCache]
 L00011F5F:
  		mov	cx,[ebp+08h]
  		mov	[eax],cx
  		mov	cx,[ebp-08h]
  		mov	[eax+02h],cx
  		mov	cx,[ebp-0Ch]
  		mov	[eax+04h],cx
 L00011F76:
  		mov	edi,[edi]
  		lea	eax,[esi+00000224h]
 L00011F7E:
  		cmp	edi,eax
  		jnz	L00011F11
  		push	ebx
  		lea	eax,[esi+00000204h]
  		push	eax
  		call	[ntoskrnl.exe!KeReleaseMutex]
  		jmp	L00011E48
 L00011F95:
  		lea	edi,[esi+0000023Ch]
  		push	[edi]
  		call	[ntoskrnl.exe!ZwClose]
  		push	ebx
  		lea	eax,[esi+00000204h]
  		push	eax
  		mov	[edi],ebx
  		mov	[esi+00000241h],bl
  		call	[ntoskrnl.exe!KeReleaseMutex]
 L00011FB9:
  		push	ebx
  		push	ebx
  		add	esi,0000022Ch
  		push	esi
  		call	[ntoskrnl.exe!KeSetEvent]
  		pop	edi
  		pop	esi
  		pop	ebx
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	4
 L00011FD4:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000004Ch
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	eax,[ebp+08h]
  		push	ebx
  		mov	ebx,[eax+28h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		mov	ecx,[esi+60h]
  		mov	eax,[ecx+0Ch]
  		push	edi
  		xor	edi,edi
  		cmp	eax,CF502000h
  		mov	[ebp-38h],esi
  		mov	[ebp-2Ch],ecx
  		mov	[esi+1Ch],edi
  		jz 	L00012483
  		cmp	eax,CF50601Ah
  		jz 	L0001237B
  		cmp	eax,CF509FFCh
  		jz 	L000121EA
  		cmp	eax,CF50A014h
  		jz 	L00012137
  		cmp	eax,CF50A024h
  		jz 	L000120D7
  		mov	ecx,CF50A028h
  		cmp	eax,ecx
  		jz 	L00012050
  		cmp	eax,CF50A02Ch
  		jnz	L00012634
  		cmp	eax,ecx
 L00012050:
  		lea	eax,[ebp-44h]
  		push	eax
  		setz 	[ebp-28h]
  		call	[ntoskrnl.exe!SeCaptureSubjectContext]
  		push	00000007h
  		xor	eax,eax
  		inc	eax
  		pop	ecx
  		push	eax
  		mov	[ebp-18h],eax
  		mov	[ebp-14h],eax
  		lea	eax,[ebp-44h]
  		push	eax
  		lea	eax,[ebp-18h]
  		xor	edx,edx
  		push	eax
  		mov	[ebp-10h],ecx
  		mov	[ebp-0Ch],edx
  		mov	[ebp-08h],edi
  		call	[ntoskrnl.exe!SePrivilegeCheck]
  		test	al,al
  		jnz	L00012091
  		mov	dword ptr [ebp-20h],C0000022h
  		jmp	L000120B9
 L00012091:
  		xor	ecx,ecx
  		cmp	[ebp-28h],cl
  		jz 	L000120C8
  		mov	ecx,[esi+0Ch]
  		cmp	ecx,edi
  		jz 	L000120B2
  		mov	eax,[ebp-2Ch]
  		mov	eax,[eax+08h]
  		cmp	eax,edi
  		jz 	L000120B2
  		shr	eax,1
  		cmp	[ecx+eax*2-02h],di
  		jz 	L000120C8
 L000120B2:
  		mov	dword ptr [ebp-20h],C000000Dh
 L000120B9:
  		lea	eax,[ebp-44h]
  		push	eax
  		call	[ntoskrnl.exe!SeReleaseSubjectContext]
  		jmp	L0001263B
 L000120C8:
  		push	[ebp-28h]
  		push	ecx
  		push	ebx
  		call	SUB_L00013D90
  		mov	[ebp-20h],eax
  		jmp	L000120B9
 L000120D7:
  		mov	eax,[esi+0Ch]
  		cmp	eax,edi
  		mov	[ebp-2Ch],eax
  		jz 	L00012634
  		cmp	dword ptr [ecx+08h],00000004h
  		jc 	L00012634
  		mov	eax,[eax]
  		cmp	eax,00000012h
  		ja 	CASE_00012660_PROC0002
  		movzx	eax,[eax+CASE_0001266C]
  		jmp	[CASE_PROCTABLE_00012660+eax*4]
 CASE_00012660_PROC0000:
  		push	edi
  		push	ebx
  		call	SUB_L00013C4E
  		mov	eax,[ebp-2Ch]
  		push	[eax]
  		push	SSZ0001509A_IOCTL_rcvd__IgnoreInterrupts__d
  		jmp	L0001212B
 CASE_00012660_PROC0001:
  		push	ebx
  		call	SUB_L00013384
  		mov	eax,[ebp-2Ch]
  		push	[eax]
  		push	SSZ00015076_IOCTL_rcvd__No_IgnoreInterrupts_
 L0001212B:
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
  		jmp	CASE_00012660_PROC0002
 L00012137:
  		mov	edi,[esi+0Ch]
  		test	edi,edi
  		jz 	L00012634
  		mov	eax,ecx
  		mov	edx,[eax+08h]
  		cmp	edx,0000000Ch
  		jc 	L00012634
  		mov	ecx,[edi+04h]
  		lea	eax,[ecx+0Bh]
  		cmp	edx,eax
  		jc 	L00012634
  		mov	edx,[ebp-2Ch]
  		cmp	[edx+04h],eax
  		jc 	L00012634
  		cmp	byte ptr [edi],00h
  		mov	al,[edi+01h]
  		mov	[ebp-31h],al
  		jz 	L00012193
  		cmp	ecx,00000001h
  		jnz	L00012634
  		mov	al,[edi+08h]
  		mov	[ebp-21h],al
  		push	ecx
  		lea	eax,[ebp-21h]
  		push	eax
  		movzx	ax,[edi+01h]
  		push	eax
  		push	00000003h
  		jmp	L000121A0
 L00012193:
  		movzx	ax,al
  		push	00000001h
  		lea	ecx,[ebp-21h]
  		push	ecx
  		push	eax
  		push	00000002h
 L000121A0:
  		push	ebx
  		call	SUB_L00012D72
  		test	eax,eax
  		mov	[ebp-20h],eax
  		jl 	L0001263B
  		cmp	byte ptr [edi],00h
  		jnz	L0001263B
  		mov	eax,[ebp-2Ch]
  		push	[eax+04h]
  		push	00000000h
  		push	edi
  		call	jmp_ntoskrnl.exe!memset
  		mov	al,[ebp-31h]
  		mov	[edi+01h],al
  		mov	dword ptr [edi+04h],00000001h
  		mov	al,[ebp-21h]
  		mov	[edi+08h],al
  		add	esp,0000000Ch
  		mov	dword ptr [esi+1Ch],0000000Ch
  		jmp	L0001263B
 L000121EA:
  		mov	eax,[esi+0Ch]
  		cmp	eax,edi
  		jz 	L00012634
  		push	00000038h
  		pop	edx
  		cmp	[ecx+08h],edx
  		jc 	L00012634
  		mov	ecx,[eax]
  		dec	ecx
  		dec	ecx
  		jz 	L00012256
  		dec	ecx
  		dec	ecx
  		jz 	L00012229
  		dec	ecx
  		jnz	L00012634
  		mov	eax,[eax+04h]
  		lea	ecx,[ebx+000000C8h]
  		mov	[ecx],eax
  		mov	[esi+1Ch],edx
  		push	[ecx]
  		push	L00014D9A
  		jmp	L0001223E
 L00012229:
  		mov	eax,[eax+04h]
  		lea	ecx,[ebx+000000C4h]
  		mov	[ecx],eax
  		mov	[esi+1Ch],edx
  		push	[ecx]
  		push	L00014DCA
 L0001223E:
  		push	L00017098
  		call	SUB_L00012A90
  		push	ebx
  		mov	[ebp-20h],edi
  		call	SUB_L00012BC2
  		jmp	L0001263B
 L00012256:
  		cmp	byte ptr [eax+04h],00h
  		push	ebx
  		setz 	al
  		test	al,al
  		mov	[ebx+41h],al
  		jz 	L0001226C
  		call	SUB_L00012B5E
  		jmp	L00012271
 L0001226C:
  		call	SUB_L00012AFA
 L00012271:
  		cmp	byte ptr [ebx+00000101h],00h
  		jz 	L00012353
  		lea	eax,[ebp-28h]
  		push	eax
  		push	00000004h
  		push	L0001505A
  		push	L000170A0
  		mov	[ebp-28h],edi
  		call	SUB_L000129C2
  		lea	eax,[ebp-4Ch]
  		push	eax
  		call	[ntoskrnl.exe!RtlFormatCurrentUserKeyPath]
  		cmp	eax,edi
  		mov	[ebp-20h],eax
  		jl 	L0001263B
  		xor	eax,eax
  		lea	edi,[ebp-3Ch]
  		stosd
  		stosd
  		movzx	eax,[ebp-4Ch]
  		push	75736572h
  		add	eax,00000078h
  		push	eax
  		push	00000001h
  		call	[ntoskrnl.exe!ExAllocatePoolWithTag]
  		test	eax,eax
  		mov	[ebp-38h],eax
  		jnz	L000122DD
  		lea	eax,[ebp-4Ch]
  		push	eax
  		call	[ntoskrnl.exe!RtlFreeUnicodeString]
  		jmp	L000123AD
 L000122DD:
  		mov	eax,[ebp-4Ch]
  		add	eax,00000078h
  		mov	[ebp-3Ah],ax
  		push	SWC00014FE4__Software_Hewlett_Packard_HP_Mob
  		lea	eax,[ebp-30h]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		mov	edi,[ntoskrnl.exe!RtlAppendUnicodeStringToString]
  		lea	eax,[ebp-4Ch]
  		push	eax
  		lea	eax,[ebp-3Ch]
  		push	eax
  		call	edi
  		lea	eax,[ebp-30h]
  		push	eax
  		lea	eax,[ebp-3Ch]
  		push	eax
  		call	edi
  		lea	eax,[ebp-4Ch]
  		push	eax
  		call	[ntoskrnl.exe!RtlFreeUnicodeString]
  		xor	eax,eax
  		cmp	[ebx+41h],al
  		push	00000001h
  		setz 	al
  		push	eax
  		push	SWC00014DF8_Enabled
  		lea	eax,[ebp-3Ch]
  		push	eax
  		call	SUB_L000134B4
  		push	00000001h
  		push	[ebp-28h]
  		lea	eax,[ebp-3Ch]
  		push	L0001505A
  		push	eax
  		call	SUB_L000134B4
  		push	00000000h
  		push	[ebp-38h]
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
  		jmp	L0001236B
 L00012353:
  		xor	eax,eax
  		cmp	[ebx+41h],al
  		setz 	al
  		push	eax
  		push	SWC00014DF8_Enabled
  		push	L00017098
  		call	SUB_L00012A90
 L0001236B:
  		and	dword ptr [ebp-20h],00000000h
  		mov	dword ptr [esi+1Ch],00000038h
  		jmp	L0001263B
 L0001237B:
  		mov	eax,[esi+04h]
  		cmp	eax,edi
  		jz 	L00012634
  		cmp	dword ptr [ecx+04h],00000006h
  		jc 	L00012634
  		test	byte ptr [eax+06h],05h
  		jz 	L0001239B
  		mov	eax,[eax+0Ch]
  		jmp	L000123A9
 L0001239B:
  		push	00000010h
  		push	edi
  		push	edi
  		push	00000001h
  		push	edi
  		push	eax
  		call	[ntoskrnl.exe!MmMapLockedPagesSpecifyCache]
 L000123A9:
  		cmp	eax,edi
  		jnz	L000123B9
 L000123AD:
  		mov	dword ptr [ebp-20h],C000009Ah
  		jmp	L0001263B
 L000123B9:
  		push	edi
  		push	edi
  		push	edi
  		lea	eax,[ebx+00000204h]
  		push	edi
  		push	eax
  		mov	[ebp-2Ch],eax
  		call	[ntoskrnl.exe!KeWaitForSingleObject]
  		cmp	byte ptr [ebx+00000241h],00h
  		jz 	L0001240A
  		push	edi
  		push	[ebp-2Ch]
 L000123DA:
  		mov	eax,[esi+60h]
  		or	byte ptr [eax+03h],01h
  		mov	eax,[ebx+00000228h]
  		add	esi,00000058h
  		add	ebx,00000224h
  		mov	[esi],ebx
  		mov	[esi+04h],eax
  		mov	[eax],esi
  		mov	[ebx+04h],esi
  		call	[ntoskrnl.exe!KeReleaseMutex]
  		mov	eax,00000103h
  		jmp	L0001264D
 L0001240A:
  		mov	eax,[ebx+0000023Ch]
  		cmp	eax,edi
  		jz 	L00012421
  		push	eax
  		call	[ntoskrnl.exe!ZwClose]
  		mov	[ebx+0000023Ch],edi
 L00012421:
  		push	edi
  		push	00000001h
  		lea	eax,[ebx+0000022Ch]
  		push	eax
  		call	[ntoskrnl.exe!KeInitializeEvent]
  		push	ebx
  		push	L00011E14
  		push	edi
  		push	edi
  		lea	eax,[ebp-1Ch]
  		push	eax
  		push	001FFFFFh
  		lea	eax,[ebx+0000023Ch]
  		push	eax
  		mov	dword ptr [ebp-1Ch],00000018h
  		mov	[ebp-18h],edi
  		mov	dword ptr [ebp-10h],00000200h
  		mov	[ebp-14h],edi
  		mov	[ebp-0Ch],edi
  		mov	[ebp-08h],edi
  		call	[ntoskrnl.exe!PsCreateSystemThread]
  		cmp	eax,edi
  		push	edi
  		push	[ebp-2Ch]
  		mov	[ebp-20h],eax
  		jge	L000123DA
  		call	[ntoskrnl.exe!KeReleaseMutex]
  		jmp	L0001263B
 L00012483:
  		mov	eax,[esi+0Ch]
  		cmp	eax,edi
  		mov	[ebp-2Ch],eax
  		jz 	L00012634
  		push	00000038h
  		pop	edx
  		cmp	[ecx+08h],edx
  		jc 	L00012634
  		cmp	[ecx+04h],edx
  		jc 	L00012634
  		mov	ecx,[eax]
  		dec	ecx
  		dec	ecx
  		jz 	L00012622
  		dec	ecx
  		dec	ecx
  		jz 	L00012617
  		dec	ecx
  		jz 	L0001260F
  		dec	ecx
  		jz 	L00012539
  		dec	ecx
  		jnz	L00012634
  		lea	eax,[ebp-44h]
  		push	eax
  		call	[ntoskrnl.exe!SeCaptureSubjectContext]
  		lea	eax,[ebp-44h]
  		push	eax
  		call	[ntoskrnl.exe!SeLockSubjectContext]
  		cmp	byte ptr [ebx+00000101h],00h
  		jz 	L000124F7
  		lea	eax,[ebp-44h]
  		push	eax
  		call	SUB_L000136D8
  		mov	ecx,[ebp-2Ch]
  		mov	[ecx+04h],al
  		jmp	L00012520
 L000124F7:
  		mov	eax,[ebp-44h]
  		cmp	eax,edi
  		jnz	L00012505
  		mov	eax,[ebp-3Ch]
  		cmp	eax,edi
  		jz 	L00012519
 L00012505:
  		push	eax
  		call	[ntoskrnl.exe!SeTokenIsAdmin]
  		test	al,al
  		jz 	L00012519
  		mov	eax,[ebp-2Ch]
  		mov	byte ptr [eax+04h],01h
  		jmp	L00012520
 L00012519:
  		mov	eax,[ebp-2Ch]
  		mov	byte ptr [eax+04h],00h
 L00012520:
  		lea	eax,[ebp-44h]
  		push	eax
  		mov	dword ptr [esi+1Ch],00000038h
  		mov	[ebp-20h],edi
  		call	[ntoskrnl.exe!SeUnlockSubjectContext]
  		jmp	L000120B9
 L00012539:
  		push	00000014h
  		lea	eax,[ebp-18h]
  		push	eax
  		push	edi
  		push	00000006h
  		push	ebx
  		mov	[ebp-48h],edi
  		mov	dword ptr [ebp-28h],00000001h
  		call	SUB_L00012D72
  		cmp	eax,edi
  		mov	[ebp-20h],eax
  		jl 	L0001263B
  		mov	esi,[ntoskrnl.exe!RtlCompareMemory]
  		push	00000014h
  		lea	eax,[ebp-18h]
  		push	eax
  		lea	eax,[ebx+6Eh]
  		push	eax
  		call	esi
  		cmp	eax,00000014h
  		jnz	L00012580
  		mov	[ebp-48h],edi
  		mov	dword ptr [ebp-28h],00000001h
  		jmp	L0001259E
 L00012580:
  		push	00000014h
  		lea	eax,[ebp-18h]
  		push	eax
  		lea	eax,[ebx+00000084h]
  		push	eax
  		call	esi
  		cmp	eax,00000014h
  		jnz	L0001259E
  		mov	dword ptr [ebp-48h],00000001h
  		mov	[ebp-28h],edi
 L0001259E:
  		mov	ecx,[ebx+54h]
  		mov	eax,[ebp-2Ch]
  		movzx	edi,[ebp-48h]
  		mov	[eax+04h],ecx
  		mov	ecx,[ebx+58h]
  		mov	[eax+08h],ecx
  		mov	ecx,edi
  		imul	ecx,16h
  		lea	esi,[ecx+ebx]
  		mov	cl,[esi+6Dh]
  		mov	[eax+0Dh],cl
  		mov	cl,[esi+6Ch]
  		lea	esi,[edi+05h]
  		imul	esi,16h
  		mov	[eax+0Ch],cl
  		add	esi,ebx
  		push	00000005h
  		pop	ecx
  		lea	edi,[eax+0Eh]
  		rep movsd
  		movzx	edi,[ebp-28h]
  		mov	ecx,edi
  		imul	ecx,16h
  		lea	esi,[ecx+ebx]
  		mov	cl,[esi+6Dh]
  		mov	[eax+23h],cl
  		mov	cl,[esi+6Ch]
  		lea	esi,[edi+05h]
  		imul	esi,16h
  		mov	[eax+22h],cl
  		add	esi,ebx
  		and	dword ptr [ebp-20h],00000000h
  		push	00000005h
  		lea	edi,[eax+24h]
  		mov	eax,[ebp-38h]
  		pop	ecx
  		rep movsd
  		mov	dword ptr [eax+1Ch],00000038h
  		mov	esi,eax
  		jmp	L0001263B
 L0001260F:
  		mov	ecx,[ebx+000000C8h]
  		jmp	L0001261D
 L00012617:
  		mov	ecx,[ebx+000000C4h]
 L0001261D:
  		mov	[eax+04h],ecx
  		jmp	L0001262C
 L00012622:
  		cmp	byte ptr [ebx+41h],00h
  		setz 	cl
  		mov	[eax+04h],cl
 L0001262C:
  		mov	[esi+1Ch],edx
 CASE_00012660_PROC0002:
  		mov	[ebp-20h],edi
  		jmp	L0001263B
 L00012634:
  		mov	dword ptr [ebp-20h],C000000Dh
 L0001263B:
  		mov	edi,[ebp-20h]
  		xor	dl,dl
  		mov	ecx,esi
  		mov	[esi+18h],edi
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		mov	eax,edi
 L0001264D:
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L000147FB
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	4
 CASE_PROCTABLE_00012660:
 		dd	CASE_00012660_PROC0000
 		dd	CASE_00012660_PROC0001
 		dd	CASE_00012660_PROC0002
 CASE_0001266C:
  		db	00h, 00h, 01h, 01h, 00h, 00h, 01h, 01h, 01h, 00h, 02h, 02h, 02h, 02h, 02h, 02h
  		db	02h, 02h, 01h
  		Align	4
 L00012684:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		movzx	edx,[ebp+0Ch]
  		mov	ecx,[ebp+14h]
  		push	esi
  		mov	esi,[ecx+00000138h]
  		mov	eax,[esi+60h]
  		dec	edx
  		dec	edx
  		jz 	L000126B8
  		dec	edx
  		jnz	L000126B3
  		mov	eax,[ebp+18h]
  		mov	eax,[eax]
  		xor	dl,dl
  		mov	ecx,esi
  		mov	[esi+18h],eax
  		call	[ntoskrnl.exe!IofCompleteRequest]
 L000126B3:
  		pop	esi
  		pop	ebp
  		retn	0014h
;------------------------------------------------------------------------------
 L000126B8:
  		push	edi
  		mov	edi,[ebp+18h]
  		xor	edx,edx
  		cmp	[edi],edx
  		jl 	L000126EE
  		mov	eax,[eax+0Ch]
  		mov	[ecx+14h],eax
  		dec	eax
  		jnz	L000126DC
  		push	ecx
  		call	SUB_L00013384
  		push	SSZ000150BA_System_Powering_Up__Not_Ignoring
  		call	SUB_L00013A0E
  		pop	ecx
 L000126DC:
  		mov	eax,[edi]
  		xor	dl,dl
  		mov	ecx,esi
  		mov	[esi+18h],eax
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		pop	edi
  		jmp	L000126B3
 L000126EE:
  		push	edx
  		push	edx
  		push	edx
  		push	edx
  		push	00010001h
  		call	[ntoskrnl.exe!KeBugCheckEx]
  		Align	4
 L00012704:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		push	ecx
  		mov	edx,[ebp+0Ch]
  		push	ebx
  		xor	ebx,ebx
  		cmp	[edx+18h],ebx
  		jge	L0001271C
  		xor	eax,eax
  		jmp	L000127C2
 L0001271C:
  		mov	eax,[ebp+08h]
  		mov	ecx,[edx+60h]
  		push	esi
  		mov	esi,[eax+28h]
  		mov	eax,[ecx+08h]
  		push	edi
  		mov	edi,[ecx+0Ch]
  		movzx	ecx,[ecx+01h]
  		dec	ecx
  		dec	ecx
  		jz 	L00012777
  		dec	ecx
  		jnz	L000127BE
  		cmp	eax,ebx
  		jnz	L00012768
  		cmp	edi,[esi+14h]
  		jle	L00012768
  		push	00000004h
  		pop	eax
  		push	ebx
  		push	esi
  		push	L00012684
  		push	eax
  		push	00000003h
 L00012752:
  		push	[esi+08h]
  		mov	[esi+00000138h],edx
  		call	[ntoskrnl.exe!PoRequestPowerIrp]
  		mov	eax,C0000016h
  		jmp	L000127C0
 L00012768:
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		push	00010001h
  		call	[ntoskrnl.exe!KeBugCheckEx]
 L00012777:
  		sub	eax,ebx
  		jz 	L000127A1
  		dec	eax
  		jnz	L000127BE
  		cmp	[edx+21h],bl
  		jz 	L0001278A
  		mov	edx,[edx+60h]
  		or	byte ptr [edx+03h],01h
 L0001278A:
  		cmp	[esi+18h],edi
  		jle	L00012768
  		push	edi
  		push	00000001h
  		push	[esi]
  		call	[ntoskrnl.exe!PoSetPowerState]
  		mov	[esi+18h],edi
  		xor	eax,eax
  		jmp	L000127C0
 L000127A1:
  		mov	eax,[esi+14h]
  		cmp	edi,eax
  		jle	L000127AD
  		push	00000004h
  		pop	edi
  		jmp	L000127B2
 L000127AD:
  		jge	L000127B2
  		xor	edi,edi
  		inc	edi
 L000127B2:
  		push	ebx
  		push	esi
  		push	L00012684
  		push	edi
  		push	00000002h
  		jmp	L00012752
 L000127BE:
  		mov	eax,ebx
 L000127C0:
  		pop	edi
  		pop	esi
 L000127C2:
  		pop	ebx
  		leave
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	4
 L000127CC:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+08h]
  		push	ebx
  		mov	ebx,[ebp+0Ch]
  		push	esi
  		mov	esi,[eax+28h]
  		mov	eax,[ebx+60h]
  		mov	ecx,[eax+08h]
  		push	edi
  		mov	edi,[eax+0Ch]
  		movzx	eax,[eax+01h]
  		dec	eax
  		dec	eax
  		mov	[ebp+08h],esi
  		jz 	L00012818
  		dec	eax
  		jnz	L000127FF
  		and	[ebx+18h],eax
  		sub	ecx,eax
  		jz 	L00012889
 L000127FF:
  		inc	[ebx+23h]
  		add	dword ptr [ebx+60h],00000024h
  		mov	ecx,[esi+08h]
 L00012809:
  		mov	edx,ebx
  		call	[ntoskrnl.exe!IofCallDriver]
 L00012811:
  		pop	edi
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
 L00012818:
  		xor	edx,edx
  		sub	ecx,edx
  		mov	[ebx+18h],edx
  		jz 	L00012871
  		dec	ecx
  		jnz	L000127FF
  		mov	eax,[esi+18h]
  		cmp	eax,edi
  		jge	L0001283B
  		push	edi
  		push	00000001h
  		push	[esi]
  		call	[ntoskrnl.exe!PoSetPowerState]
  		mov	[esi+18h],edi
  		jmp	L000127FF
 L0001283B:
  		jz 	L000127FF
  		push	esi
  		call	SUB_L00012BC2
  		mov	esi,[ebx+60h]
  		lea	eax,[esi-24h]
  		mov	edi,eax
  		push	00000007h
  		pop	ecx
  		rep movsd
  		mov	byte ptr [eax+03h],00h
  		mov	eax,[ebx+60h]
  		sub	eax,00000024h
  		and	dword ptr [eax+20h],00000000h
  		mov	dword ptr [eax+1Ch],L00012704
  		mov	byte ptr [eax+03h],E0h
  		mov	eax,[ebp+08h]
  		mov	ecx,[eax+08h]
  		jmp	L00012809
 L00012871:
  		mov	eax,[esi+14h]
  		cmp	eax,edi
  		jge	L00012881
  		push	edx
  		push	esi
  		call	SUB_L00013C4E
  		jmp	L00012889
 L00012881:
  		jle	L00012889
  		push	esi
  		call	SUB_L00012BC2
 L00012889:
  		mov	eax,[ebx+60h]
  		or	byte ptr [eax+03h],01h
  		mov	esi,[ebx+60h]
  		lea	eax,[esi-24h]
  		mov	edi,eax
  		push	00000007h
  		pop	ecx
  		rep movsd
  		mov	byte ptr [eax+03h],00h
  		mov	eax,[ebx+60h]
  		and	dword ptr [eax-04h],00000000h
  		sub	eax,00000024h
  		mov	dword ptr [eax+1Ch],L00012704
  		mov	byte ptr [eax+03h],E0h
  		mov	eax,[ebp+08h]
  		mov	ecx,[eax+08h]
  		mov	edx,ebx
  		call	[ntoskrnl.exe!IofCallDriver]
  		mov	eax,00000103h
  		jmp	L00012811
  		Align	4
 SUB_L000128D4:
  		mov	edi,edi
  		push	esi
  		mov	esi,eax
  		xor	eax,eax
  		test	edx,edx
  		mov	ecx,edx
  		jz 	L000128ED
 L000128E1:
  		cmp	[esi],al
  		jz 	L000128E9
  		inc	esi
  		dec	edx
  		jnz	L000128E1
 L000128E9:
  		test	edx,edx
  		jnz	L000128F2
 L000128ED:
  		mov	eax,C000000Dh
 L000128F2:
  		test	edi,edi
  		pop	esi
  		jz 	L00012903
  		test	eax,eax
  		jl 	L00012900
  		sub	ecx,edx
  		mov	[edi],ecx
  		retn
;------------------------------------------------------------------------------
 L00012900:
  		and	dword ptr [edi],00000000h
 L00012903:
  		retn
;------------------------------------------------------------------------------
  		Align	2
 SUB_L0001290A:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		push	esi
  		push	[ebp+10h]
  		mov	esi,eax
  		push	[ebp+0Ch]
  		xor	ebx,ebx
  		dec	esi
  		push	esi
  		push	edi
  		call	[ntoskrnl.exe!_vsnprintf]
  		add	esp,00000010h
  		test	eax,eax
  		jl 	L00012936
  		cmp	eax,esi
  		ja 	L00012936
  		jnz	L00012940
  		mov	[esi+edi],bl
  		jmp	L0001293E
 L00012936:
  		mov	[esi+edi],bl
  		mov	ebx,80000005h
 L0001293E:
  		mov	eax,esi
 L00012940:
  		mov	ecx,[ebp+08h]
  		test	ecx,ecx
  		jz 	L00012949
  		mov	[ecx],eax
 L00012949:
  		pop	esi
  		mov	eax,ebx
  		pop	ebx
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012956:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ecx
  		push	ecx
  		rdtsc
  		mov	[ebp-08h],eax
  		mov	[ebp-04h],edx
  		mov	ecx,[ebp-08h]
  		mov	eax,[ebp+08h]
  		mov	[eax],ecx
  		mov	ecx,[ebp-04h]
  		mov	[eax+04h],ecx
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	4
 L0001297C:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000000Ch
  		lea	edx,[ebp-0Ch]
  		mov	ecx,L000170B4
  		call	[HAL.dll!KeAcquireInStackQueuedSpinLock]
  		mov	eax,[ebp+08h]
  		mov	ecx,[eax]
  		mov	edx,[eax+04h]
  		mov	[edx],ecx
  		mov	[ecx+04h],edx
  		xor	ecx,ecx
  		mov	[eax+08h],ecx
  		mov	[eax+0Ch],ecx
  		mov	[eax+10h],ecx
  		mov	[eax+14h],ecx
  		lea	ecx,[ebp-0Ch]
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
  		xor	eax,eax
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L000129C2:
  		push	00000044h
  		push	L000162D8
  		call	SUB_L00014860
  		mov	ebx,[ebp+14h]
  		xor	edi,edi
  		cmp	ebx,edi
  		jz 	L00012A82
  		push	6D656D41h
  		mov	esi,[ebp+08h]
  		movzx	eax,[esi]
  		inc	eax
  		inc	eax
  		push	eax
  		push	edi
  		call	[ntoskrnl.exe!ExAllocatePoolWithTag]
  		mov	[ebp+14h],eax
  		cmp	eax,edi
  		jz 	L00012A82
  		push	00000038h
  		push	edi
  		lea	eax,[ebp-54h]
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		inc	eax
  		inc	eax
  		push	eax
  		push	edi
  		push	[ebp+14h]
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		push	eax
  		push	[esi+04h]
  		push	[ebp+14h]
  		call	jmp_ntoskrnl.exe!memcpy
  		add	esp,00000024h
  		mov	dword ptr [ebp-50h],00000020h
  		mov	eax,[ebp+0Ch]
  		mov	[ebp-4Ch],eax
  		mov	[ebp-48h],ebx
  		mov	eax,[ebp+10h]
  		mov	[ebp-44h],eax
  		mov	[ebp-04h],edi
  		push	edi
  		push	edi
  		lea	eax,[ebp-54h]
  		push	eax
  		push	[ebp+14h]
  		push	80000000h
  		call	[ntoskrnl.exe!RtlQueryRegistryValues]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		jmp	L00012A78
 L00012A5E:
  		mov	eax,[ebp-14h]
  		mov	eax,[eax]
  		mov	eax,[eax]
  		mov	[ebp-1Ch],eax
  		xor	eax,eax
  		inc	eax
  		retn
;------------------------------------------------------------------------------
 L00012A6C:
  		mov	esp,[ebp-18h]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		xor	edi,edi
 L00012A78:
  		push	edi
  		push	[ebp+14h]
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
 L00012A82:
  		call	SUB_L000148A5
  		retn	0010h
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00012A90:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	esi
  		mov	esi,[ebp+08h]
  		movzx	eax,[esi]
  		push	edi
  		inc	eax
  		push	6D656D41h
  		inc	eax
  		push	eax
  		push	00000000h
  		call	[ntoskrnl.exe!ExAllocatePoolWithTag]
  		mov	edi,eax
  		test	edi,edi
  		jz 	L00012AEE
  		movzx	eax,[esi]
  		inc	eax
  		inc	eax
  		push	eax
  		push	00000000h
  		push	edi
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		push	eax
  		push	[esi+04h]
  		push	edi
  		call	jmp_ntoskrnl.exe!memcpy
  		add	esp,00000018h
  		push	00000004h
  		lea	eax,[ebp+10h]
  		push	eax
  		push	00000004h
  		push	[ebp+0Ch]
  		push	edi
  		push	00000000h
  		call	[ntoskrnl.exe!RtlWriteRegistryValue]
  		push	00000000h
  		push	edi
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
 L00012AEE:
  		pop	edi
  		pop	esi
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012AFA:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000024h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	esi
  		push	edi
  		mov	edx,[ebp+08h]
  		push	00000008h
  		pop	ecx
  		xor	eax,eax
  		lea	edi,[ebp-24h]
  		rep stosd
  		or	dword ptr [ebp-0Ch],FFFFFFFFh
  		mov	[ebp-10h],eax
  		mov	word ptr [ebp-24h],0001h
  		mov	word ptr [ebp-22h],0020h
  		mov	esi,L000161AC
  		lea	edi,[ebp-20h]
  		movsd
  		push	eax
  		movsd
  		push	eax
  		lea	eax,[ebp-24h]
  		movsd
  		push	eax
  		push	[edx+04h]
  		movsd
  		call	[ntoskrnl.exe!IoReportTargetDeviceChangeAsynchronous]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		xor	ecx,ebp
  		pop	esi
  		call	SUB_L000147FB
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012B5E:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000024h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	esi
  		push	edi
  		mov	edx,[ebp+08h]
  		push	00000008h
  		pop	ecx
  		xor	eax,eax
  		lea	edi,[ebp-24h]
  		rep stosd
  		or	dword ptr [ebp-0Ch],FFFFFFFFh
  		mov	[ebp-10h],eax
  		mov	word ptr [ebp-24h],0001h
  		mov	word ptr [ebp-22h],0020h
  		mov	esi,L000161BC
  		lea	edi,[ebp-20h]
  		movsd
  		push	eax
  		movsd
  		push	eax
  		lea	eax,[ebp-24h]
  		movsd
  		push	eax
  		push	[edx+04h]
  		movsd
  		call	[ntoskrnl.exe!IoReportTargetDeviceChangeAsynchronous]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		xor	ecx,ebp
  		pop	esi
  		call	SUB_L000147FB
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012BC2:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000024h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	esi
  		push	edi
  		mov	edx,[ebp+08h]
  		push	00000008h
  		pop	ecx
  		xor	eax,eax
  		lea	edi,[ebp-24h]
  		rep stosd
  		or	dword ptr [ebp-0Ch],FFFFFFFFh
  		mov	[ebp-10h],eax
  		mov	word ptr [ebp-24h],0001h
  		mov	word ptr [ebp-22h],0020h
  		mov	esi,L000161CC
  		lea	edi,[ebp-20h]
  		movsd
  		push	eax
  		movsd
  		push	eax
  		lea	eax,[ebp-24h]
  		movsd
  		push	eax
  		push	[edx+04h]
  		movsd
  		call	[ntoskrnl.exe!IoReportTargetDeviceChangeAsynchronous]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		xor	ecx,ebp
  		pop	esi
  		call	SUB_L000147FB
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012C26:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000024h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	esi
  		push	edi
  		mov	edx,[ebp+08h]
  		push	00000008h
  		pop	ecx
  		xor	eax,eax
  		lea	edi,[ebp-24h]
  		rep stosd
  		or	dword ptr [ebp-0Ch],FFFFFFFFh
  		mov	[ebp-10h],eax
  		mov	word ptr [ebp-24h],0001h
  		mov	word ptr [ebp-22h],0020h
  		mov	esi,L000161FC
  		lea	edi,[ebp-20h]
  		movsd
  		push	eax
  		movsd
  		push	eax
  		lea	eax,[ebp-24h]
  		movsd
  		push	eax
  		push	[edx+04h]
  		movsd
  		call	[ntoskrnl.exe!IoReportTargetDeviceChangeAsynchronous]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		xor	ecx,ebp
  		pop	esi
  		call	SUB_L000147FB
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012C8A:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000024h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	esi
  		push	edi
  		mov	edx,[ebp+08h]
  		push	00000008h
  		pop	ecx
  		xor	eax,eax
  		lea	edi,[ebp-24h]
  		rep stosd
  		or	dword ptr [ebp-0Ch],FFFFFFFFh
  		mov	[ebp-10h],eax
  		mov	word ptr [ebp-24h],0001h
  		mov	word ptr [ebp-22h],0020h
  		mov	esi,L000161DC
  		lea	edi,[ebp-20h]
  		movsd
  		push	eax
  		movsd
  		push	eax
  		lea	eax,[ebp-24h]
  		movsd
  		push	eax
  		push	[edx+04h]
  		movsd
  		call	[ntoskrnl.exe!IoReportTargetDeviceChangeAsynchronous]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		xor	ecx,ebp
  		pop	esi
  		call	SUB_L000147FB
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012CEE:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000024h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	esi
  		push	edi
  		mov	edx,[ebp+08h]
  		push	00000008h
  		pop	ecx
  		xor	eax,eax
  		lea	edi,[ebp-24h]
  		rep stosd
  		or	dword ptr [ebp-0Ch],FFFFFFFFh
  		mov	[ebp-10h],eax
  		mov	word ptr [ebp-24h],0001h
  		mov	word ptr [ebp-22h],0020h
  		mov	esi,L000161EC
  		lea	edi,[ebp-20h]
  		movsd
  		push	eax
  		movsd
  		push	eax
  		lea	eax,[ebp-24h]
  		movsd
  		push	eax
  		push	[edx+04h]
  		movsd
  		call	[ntoskrnl.exe!IoReportTargetDeviceChangeAsynchronous]
  		mov	ecx,[ebp-04h]
  		pop	edi
  		xor	ecx,ebp
  		pop	esi
  		call	SUB_L000147FB
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 L00012D52:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		push	00000000h
  		push	00000000h
  		push	[ebp+10h]
  		call	[ntoskrnl.exe!KeSetEvent]
  		mov	eax,C0000016h
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00012D72:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000058h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	eax,[ebp+08h]
  		push	ebx
  		push	esi
  		mov	esi,[ebp+14h]
  		push	edi
  		xor	ebx,ebx
  		cmp	esi,ebx
  		push	00000008h
  		mov	[ebp-48h],eax
  		mov	[ebp-44h],esi
  		pop	edi
  		jnz	L00012DA6
  		mov	eax,C00000F2h
  		jmp	L00013000
 L00012DA6:
  		mov	eax,[ebp+0Ch]
  		dec	eax
  		jz 	L00012EB6
  		dec	eax
  		jz 	L00012E84
  		dec	eax
  		jz 	L00012E35
  		dec	eax
  		jz 	L00012E03
  		dec	eax
  		jz 	L00012DEE
  		dec	eax
  		jz 	L00012DCD
 L00012DC3:
  		mov	eax,C0000010h
  		jmp	L00013000
 L00012DCD:
  		push	0000003Ch
  		lea	eax,[ebp-40h]
  		push	ebx
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		mov	byte ptr [ebp-3Ch],41h
  		mov	byte ptr [ebp-3Bh],44h
  		mov	byte ptr [ebp-3Ah],53h
  		mov	byte ptr [ebp-39h],4Eh
  		jmp	L00012ED2
 L00012DEE:
  		push	0000003Ch
  		lea	eax,[ebp-40h]
  		push	ebx
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		mov	byte ptr [ebp-3Ah],49h
  		jmp	L00012E94
 L00012E03:
  		push	0000003Ch
  		lea	eax,[ebp-40h]
  		push	ebx
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		xor	eax,eax
  		add	esp,0000000Ch
  		inc	eax
  		mov	[ebp-34h],eax
  		mov	[ebp-2Eh],ax
  		movzx	eax,[esi]
  		mov	dword ptr [ebp-40h],43696541h
  		mov	byte ptr [ebp-3Ah],45h
  		mov	[ebp-30h],bx
  		mov	[ebp-2Ch],eax
  		push	00000020h
  		jmp	L00012EA7
 L00012E35:
  		push	0000003Ch
  		pop	edi
  		push	edi
  		lea	eax,[ebp-40h]
  		push	ebx
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[ebp+10h]
  		mov	[ebp-2Ch],eax
  		movzx	eax,[esi]
  		mov	dword ptr [ebp-40h],43696541h
  		mov	byte ptr [ebp-3Ch],41h
  		mov	byte ptr [ebp-3Bh],4Ch
  		mov	byte ptr [ebp-3Ah],57h
  		mov	byte ptr [ebp-39h],52h
  		mov	dword ptr [ebp-34h],00000002h
  		mov	[ebp-30h],bx
  		mov	word ptr [ebp-2Eh],0001h
  		mov	[ebp-28h],bx
  		mov	word ptr [ebp-26h],0001h
  		mov	[ebp-24h],eax
  		jmp	L00012ED9
 L00012E84:
  		push	0000003Ch
  		lea	eax,[ebp-40h]
  		push	ebx
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		mov	byte ptr [ebp-3Ah],52h
 L00012E94:
  		movzx	eax,[ebp+10h]
  		add	esp,0000000Ch
  		mov	dword ptr [ebp-40h],49696541h
  		mov	[ebp-38h],eax
  		push	0000000Ch
 L00012EA7:
  		mov	byte ptr [ebp-3Ch],41h
  		mov	byte ptr [ebp-3Bh],4Ch
  		mov	byte ptr [ebp-39h],44h
  		pop	edi
  		jmp	L00012EDC
 L00012EB6:
  		push	0000003Ch
  		lea	eax,[ebp-40h]
  		push	ebx
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		mov	byte ptr [ebp-3Ch],43h
  		mov	byte ptr [ebp-3Bh],4Ch
  		mov	byte ptr [ebp-3Ah],52h
  		mov	byte ptr [ebp-39h],49h
 L00012ED2:
  		mov	dword ptr [ebp-40h],42696541h
 L00012ED9:
  		add	esp,0000000Ch
 L00012EDC:
  		push	ebx
  		push	ebx
  		lea	eax,[ebp-58h]
  		push	eax
  		call	[ntoskrnl.exe!KeInitializeEvent]
  		mov	eax,[ebp-48h]
  		mov	eax,[eax+08h]
  		movzx	eax,[eax+30h]
  		push	ebx
  		push	eax
  		call	[ntoskrnl.exe!IoAllocateIrp]
  		mov	esi,eax
  		mov	[esi+20h],bl
  		mov	[esi+08h],ebx
  		lea	eax,[ebp-40h]
  		mov	[esi+0Ch],eax
  		mov	eax,[esi+60h]
  		sub	eax,00000024h
  		mov	byte ptr [eax],0Eh
  		mov	[eax+01h],bl
  		mov	dword ptr [eax+0Ch],0032C004h
  		mov	[eax+08h],edi
  		mov	dword ptr [eax+04h],0000003Ch
  		mov	eax,[esi+60h]
  		sub	eax,00000024h
  		lea	ecx,[ebp-58h]
  		mov	dword ptr [eax+1Ch],L00012D52
  		mov	[eax+20h],ecx
  		mov	byte ptr [eax+03h],E0h
  		mov	eax,[ebp-48h]
  		mov	ecx,[eax+08h]
  		mov	edx,esi
  		call	[ntoskrnl.exe!IofCallDriver]
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		lea	eax,[ebp-58h]
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForSingleObject]
  		mov	edi,[esi+18h]
  		cmp	edi,ebx
  		jl 	L00012F6F
  		push	[esi+1Ch]
  		lea	eax,[ebp-40h]
  		push	eax
  		push	eax
  		call	jmp_ntoskrnl.exe!memcpy
  		add	esp,0000000Ch
 L00012F6F:
  		push	esi
  		call	[ntoskrnl.exe!IoFreeIrp]
  		cmp	edi,ebx
  		jl 	L00012DC3
  		mov	eax,[ebp+0Ch]
  		xor	ecx,ecx
  		inc	ecx
  		cmp	eax,ecx
  		jz 	L00012FE1
  		cmp	eax,00000002h
  		jz 	L00012FE1
  		jle	L00012DC3
  		cmp	eax,00000004h
  		jle	L00012FFE
  		cmp	eax,00000005h
  		jz 	L00012FE1
  		cmp	eax,00000006h
  		jnz	L00012DC3
  		cmp	dword ptr [ebp-40h],426F6541h
  		jnz	L00012FFE
  		mov	si,[ebp-32h]
  		cmp	si,cx
  		jc 	L00012FFE
  		cmp	word ptr [ebp-34h],0002h
  		jnz	L00012FFE
 L00012FBF:
  		cmp	[ebp+18h],si
  		movzx	eax,[ebp+18h]
  		jc 	L00012FCC
  		movzx	eax,si
 L00012FCC:
  		cmp	bx,ax
  		jnc	L00012FFE
  		mov	edx,[ebp-44h]
  		movzx	eax,bx
  		mov	cl,[ebp+eax-30h]
  		mov	[eax+edx],cl
  		inc	ebx
  		jmp	L00012FBF
 L00012FE1:
  		cmp	dword ptr [ebp-40h],426F6541h
  		jnz	L00012FFE
  		cmp	[ebp-32h],cx
  		jc 	L00012FFE
  		cmp	[ebp-34h],bx
  		jnz	L00012FFE
  		mov	ecx,[ebp-44h]
  		mov	al,[ebp-30h]
  		mov	[ecx],al
 L00012FFE:
  		mov	eax,edi
 L00013000:
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L000147FB
  		leave
  		retn	0014h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00013016:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000240h
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		mov	eax,[ebp+08h]
  		push	ebx
  		push	esi
  		push	edi
  		push	00000200h
  		mov	[ebp-00000238h],eax
  		xor	ebx,ebx
  		lea	eax,[ebp-00000204h]
  		push	ebx
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		add	esp,0000000Ch
  		push	0000000Fh
  		pop	ecx
  		push	ebx
  		push	ebx
  		push	00000021h
  		push	00000003h
  		push	00000001h
  		lea	eax,[ebp-00000204h]
  		mov	[ebp-00000208h],eax
  		push	00000080h
  		lea	eax,[ebp-0000020Ch]
  		mov	[ebp-00000224h],eax
  		push	ebx
  		lea	eax,[ebp-00000234h]
  		push	eax
  		lea	eax,[ebp-0000022Ch]
  		push	eax
  		push	40000000h
  		lea	eax,[ebp-00000214h]
  		mov	esi,L0001512A
  		lea	edi,[ebp-00000204h]
  		push	eax
  		rep movsd
  		mov	word ptr [ebp-0000020Ch],003Ah
  		mov	word ptr [ebp-0000020Ah],003Ah
  		mov	dword ptr [ebp-0000022Ch],00000018h
  		mov	[ebp-00000228h],ebx
  		mov	dword ptr [ebp-00000220h],00000240h
  		mov	[ebp-0000021Ch],ebx
  		mov	[ebp-00000218h],ebx
  		call	[ntoskrnl.exe!ZwCreateFile]
  		cmp	eax,ebx
  		mov	[ebp-00000210h],eax
  		jl 	L0001330A
  		push	[ebp-00000214h]
  		call	[ntoskrnl.exe!ZwClose]
  		movzx	edi,[ebp-0000020Ch]
  		add	word ptr [ebp-0000020Ch],001Ch
  		mov	ax,[ebp-0000020Ch]
  		push	00000007h
  		pop	ecx
  		push	ebx
  		push	ebx
  		push	00000021h
  		push	00000003h
  		push	00000001h
  		mov	[ebp-0000020Ah],ax
  		push	00000080h
  		lea	eax,[ebp-0000020Ch]
  		mov	[ebp-00000224h],eax
  		push	ebx
  		lea	eax,[ebp-00000234h]
  		push	eax
  		lea	eax,[ebp-0000022Ch]
  		push	eax
  		shr	edi,1
  		push	40000000h
  		lea	eax,[ebp-00000214h]
  		lea	edi,[ebp+edi*2-00000204h]
  		mov	esi,SWC0001510C__Accelerometer
  		push	eax
  		rep movsd
  		mov	dword ptr [ebp-0000022Ch],00000018h
  		mov	[ebp-00000228h],ebx
  		mov	dword ptr [ebp-00000220h],00000240h
  		mov	[ebp-0000021Ch],ebx
  		mov	[ebp-00000218h],ebx
  		call	[ntoskrnl.exe!ZwCreateFile]
  		cmp	eax,ebx
  		mov	[ebp-00000210h],eax
  		jl 	L0001330A
  		push	[ebp-00000214h]
  		call	[ntoskrnl.exe!ZwClose]
  		movzx	edi,[ebp-0000020Ch]
  		add	word ptr [ebp-0000020Ch],0024h
  		mov	ax,[ebp-0000020Ch]
  		push	00000009h
  		pop	ecx
  		push	ebx
  		push	ebx
  		push	00000062h
  		push	00000003h
  		push	00000003h
  		mov	[ebp-0000020Ah],ax
  		push	00000080h
  		lea	eax,[ebp-0000020Ch]
  		push	ebx
  		mov	[ebp-00000224h],eax
  		shr	edi,1
  		lea	eax,[ebp-00000234h]
  		push	eax
  		lea	edi,[ebp+edi*2-00000204h]
  		mov	esi,L000150E6
  		rep movsd
  		mov	esi,[ebp-00000238h]
  		lea	eax,[ebp-0000022Ch]
  		push	eax
  		push	40000000h
  		lea	edi,[esi+00000184h]
  		push	edi
  		mov	dword ptr [ebp-0000022Ch],00000018h
  		mov	[ebp-00000228h],ebx
  		mov	dword ptr [ebp-00000220h],00000240h
  		mov	[ebp-0000021Ch],ebx
  		mov	[ebp-00000218h],ebx
  		call	[ntoskrnl.exe!ZwCreateFile]
  		cmp	eax,ebx
  		mov	[ebp-00000210h],eax
  		jl 	L0001330A
  		push	00000005h
  		push	00000018h
  		lea	eax,[esi+00000198h]
  		push	eax
  		lea	eax,[ebp-00000234h]
  		push	eax
  		push	[edi]
  		call	[ntoskrnl.exe!ZwQueryInformationFile]
  		mov	eax,[esi+00000200h]
  		mov	ecx,[esi+000001A4h]
  		shl	eax,14h
  		cmp	ecx,ebx
  		jl 	L000132AE
  		jg 	L0001327D
  		mov	ecx,[esi+000001A0h]
  		cmp	ecx,eax
  		jc 	L000132AE
 L0001327D:
  		push	00000014h
  		push	00000008h
  		lea	eax,[ebp-00000240h]
  		push	eax
  		lea	eax,[ebp-00000234h]
  		push	eax
  		push	edi
  		mov	[ebp-00000240h],ebx
  		mov	[ebp-0000023Ch],ebx
  		call	[ntoskrnl.exe!ZwSetInformationFile]
  		mov	[esi+000001A0h],ebx
  		mov	[esi+000001A4h],ebx
 L000132AE:
  		mov	ecx,[esi+000001A0h]
  		mov	edx,[esi+000001A4h]
  		mov	eax,ecx
  		or	eax,edx
  		jz 	L000132DF
  		push	0000000Eh
  		lea	eax,[esi+000001B8h]
  		push	00000008h
  		push	eax
  		mov	[eax],ecx
  		mov	[eax+04h],edx
  		lea	eax,[ebp-00000234h]
  		push	eax
  		push	[edi]
  		call	[ntoskrnl.exe!ZwSetInformationFile]
 L000132DF:
  		push	ebx
  		lea	eax,[esi+000001B0h]
  		push	eax
  		push	ebx
  		push	ebx
  		push	ebx
  		push	[edi]
  		call	[ntoskrnl.exe!ObReferenceObjectByHandle]
  		push	[esi+000001B0h]
  		mov	[ebp-00000210h],eax
  		call	[ntoskrnl.exe!IoGetRelatedDeviceObject]
  		mov	[esi+000001B4h],eax
 L0001330A:
  		mov	ecx,[ebp-04h]
  		mov	eax,[ebp-00000210h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L000147FB
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 L00013326:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		mov	ebx,[ebp+0Ch]
  		push	esi
  		push	edi
  		mov	edi,[ebp+10h]
  		push	[ebx+04h]
  		mov	esi,[edi+0Ch]
  		call	[ntoskrnl.exe!IoFreeMdl]
  		push	00000000h
  		push	edi
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
  		push	ebx
  		call	[ntoskrnl.exe!IoFreeIrp]
  		add	esi,000001C0h
  		or	eax,FFFFFFFFh
  		lock
  		xadd	[esi],eax
  		pop	edi
  		pop	esi
  		mov	eax,C0000016h
  		pop	ebx
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 L0001336E:
  		mov	eax,[L000170A8]
  		test	eax,eax
  		jz 	L0001337D
  		push	eax
  		call	SUB_L00012C26
 L0001337D:
  		retn
;------------------------------------------------------------------------------
  		Align	4
 SUB_L00013384:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+08h]
  		cmp	byte ptr [eax+31h],00h
  		mov	byte ptr [eax+30h],00h
  		jnz	L0001339C
  		push	eax
  		call	SUB_L00012CEE
 L0001339C:
  		pop	ebp
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L000133A6:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000010h
  		push	ebx
  		push	esi
  		push	edi
  		push	00000000h
  		push	00000000h
  		lea	eax,[ebp-10h]
  		push	eax
  		call	[ntoskrnl.exe!KeInitializeEvent]
  		mov	ebx,[ebp+0Ch]
  		mov	esi,[ebx+60h]
  		lea	eax,[esi-24h]
  		push	00000007h
  		mov	edi,eax
  		pop	ecx
  		rep movsd
  		mov	byte ptr [eax+03h],00h
  		mov	eax,[ebx+60h]
  		sub	eax,00000024h
  		lea	ecx,[ebp-10h]
  		mov	[eax+20h],ecx
  		mov	ecx,[ebp+08h]
  		mov	edx,ebx
  		mov	dword ptr [eax+1Ch],L00012D52
  		mov	byte ptr [eax+03h],E0h
  		call	[ntoskrnl.exe!IofCallDriver]
  		cmp	eax,00000103h
  		jnz	L0001340F
  		xor	eax,eax
  		push	eax
  		push	eax
  		push	eax
  		push	eax
  		lea	eax,[ebp-10h]
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForSingleObject]
  		mov	eax,[ebx+18h]
 L0001340F:
  		pop	edi
  		test	eax,eax
  		pop	esi
  		setge	al
  		pop	ebx
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00013420:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		push	esi
  		push	edi
  		mov	edi,[ebp+08h]
  		mov	eax,edi
  		xor	esi,esi
  		lea	ecx,[eax+02h]
 L00013432:
  		mov	dx,[eax]
  		inc	eax
  		inc	eax
  		test	dx,dx
  		jnz	L00013432
  		sub	eax,ecx
  		sar	eax,1
  		cmp	word ptr [edi],005Ch
  		mov	[ebp+08h],eax
  		jnz	L00013450
  		xor	esi,esi
  		inc	esi
  		cmp	eax,esi
  		jc 	L0001349F
 L00013450:
  		mov	ebx,[ntoskrnl.exe!RtlCreateRegistryKey]
  		cmp	esi,eax
 L00013458:
  		jnc	L00013496
 L0001345A:
  		cmp	word ptr [edi+esi*2],005Ch
  		jz 	L00013466
  		inc	esi
  		cmp	esi,eax
  		jc 	L0001345A
 L00013466:
  		cmp	esi,eax
  		jnc	L00013496
  		and	word ptr [edi+esi*2],0000h
  		push	edi
  		push	00000000h
  		call	[ntoskrnl.exe!RtlCheckRegistryKey]
  		test	eax,eax
  		jge	L0001348D
  		push	edi
  		push	00000000h
  		call	ebx
  		test	eax,eax
  		mov	word ptr [edi+esi*2],005Ch
  		jl 	L000134A1
  		jmp	L00013493
 L0001348D:
  		mov	word ptr [edi+esi*2],005Ch
 L00013493:
  		inc	esi
  		jmp	L00013498
 L00013496:
  		jz 	L000134A8
 L00013498:
  		mov	eax,[ebp+08h]
  		cmp	esi,eax
  		jbe	L00013458
 L0001349F:
  		xor	eax,eax
 L000134A1:
  		pop	edi
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn	0004h
;------------------------------------------------------------------------------
 L000134A8:
  		push	edi
  		push	00000000h
  		call	ebx
  		jmp	L000134A1
  		Align	4
 SUB_L000134B4:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		push	esi
  		mov	esi,[ebp+08h]
  		movzx	eax,[esi]
  		push	edi
  		inc	eax
  		push	6D656D41h
  		inc	eax
  		push	eax
  		xor	ebx,ebx
  		push	ebx
  		call	[ntoskrnl.exe!ExAllocatePoolWithTag]
  		mov	edi,eax
  		cmp	edi,ebx
  		jz 	L00013538
  		movzx	eax,[esi]
  		inc	eax
  		inc	eax
  		push	eax
  		push	ebx
  		push	edi
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		push	eax
  		push	[esi+04h]
  		push	edi
  		call	jmp_ntoskrnl.exe!memcpy
  		mov	esi,[ntoskrnl.exe!RtlWriteRegistryValue]
  		add	esp,00000018h
  		push	00000004h
  		lea	eax,[ebp+10h]
  		push	eax
  		push	00000004h
  		push	[ebp+0Ch]
  		push	edi
  		push	ebx
  		call	esi
  		cmp	eax,C0000034h
  		jnz	L00013530
  		cmp	[ebp+14h],bl
  		jz 	L00013530
  		push	edi
  		call	SUB_L00013420
  		test	eax,eax
  		jl 	L00013530
  		push	00000004h
  		lea	eax,[ebp+10h]
  		push	eax
  		push	00000004h
  		push	[ebp+0Ch]
  		push	edi
  		push	ebx
  		call	esi
 L00013530:
  		push	ebx
  		push	edi
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
 L00013538:
  		pop	edi
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn	0010h
;------------------------------------------------------------------------------
  		Align	4
 SUB_L00013544:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,000000ACh
  		mov	eax,[L00017000]
  		xor	eax,ebp
  		mov	[ebp-04h],eax
  		push	ebx
  		mov	ebx,[ebp+10h]
  		push	esi
  		mov	esi,[ebp+08h]
  		xor	eax,eax
  		cmp	ebx,eax
  		push	edi
  		mov	edi,[ebp+0Ch]
  		jz 	L00013629
  		mov	[ebp-0000009Ch],eax
  		mov	[ebp-00000090h],eax
  		mov	[ebp-0000008Ch],eax
  		lea	eax,[ebp-000000A0h]
  		push	eax
  		push	80000000h
  		lea	eax,[ebp-00000084h]
  		push	eax
  		mov	dword ptr [ebp-000000A0h],00000018h
  		mov	dword ptr [ebp-00000094h],00000240h
  		mov	[ebp-00000098h],esi
  		call	[ntoskrnl.exe!ZwOpenKey]
  		push	eax
  		push	edi
  		push	esi
  		push	SSZ00015166_RegistryReadCurrentUserValue_rea
  		push	00000003h
  		push	0000004Dh
  		mov	[ebp-00000088h],eax
  		call	[ntoskrnl.exe!DbgPrintEx]
  		add	esp,00000018h
  		cmp	dword ptr [ebp-00000088h],00000000h
  		jl 	L00013629
  		push	edi
  		lea	eax,[ebp-000000ACh]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		lea	eax,[ebp-000000A4h]
  		push	eax
  		push	0000007Ch
  		lea	eax,[ebp-80h]
  		push	eax
  		push	00000001h
  		lea	eax,[ebp-000000ACh]
  		push	eax
  		push	[ebp-00000084h]
  		call	[ntoskrnl.exe!ZwQueryValueKey]
  		test	eax,eax
  		push	[ebp-00000084h]
  		jge	L0001361A
  		call	[ntoskrnl.exe!ZwClose]
  		jmp	L00013629
 L0001361A:
  		call	[ntoskrnl.exe!ZwClose]
  		mov	eax,[ebp-78h]
  		mov	eax,[ebp+eax-80h]
  		mov	[ebx],eax
 L00013629:
  		mov	ecx,[ebp-04h]
  		pop	edi
  		pop	esi
  		xor	ecx,ebp
  		pop	ebx
  		call	SUB_L000147FB
  		leave
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00013640:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000001Ch
  		mov	eax,[ebp+08h]
  		push	esi
  		mov	[ebp-14h],eax
  		lea	eax,[ebp-1Ch]
  		push	eax
  		xor	esi,esi
  		push	80000000h
  		lea	eax,[ebp+08h]
  		push	eax
  		mov	[ebp-04h],esi
  		mov	dword ptr [ebp-1Ch],00000018h
  		mov	[ebp-18h],esi
  		mov	dword ptr [ebp-10h],00000240h
  		mov	[ebp-0Ch],esi
  		mov	[ebp-08h],esi
  		call	[ntoskrnl.exe!ZwOpenKey]
  		cmp	eax,esi
  		jl 	L000136CD
  		push	edi
  		push	esi
  		lea	eax,[ebp-04h]
  		push	eax
  		push	esi
  		push	esi
  		push	esi
  		push	[ebp+08h]
  		call	[ntoskrnl.exe!ObReferenceObjectByHandle]
  		mov	edi,eax
  		cmp	edi,esi
  		jge	L000136A7
  		push	[ebp+08h]
  		call	[ntoskrnl.exe!ZwClose]
  		mov	eax,edi
  		jmp	L000136CC
 L000136A7:
  		push	[ebp+10h]
  		push	[ebp+0Ch]
  		push	[ebp-04h]
  		call	[ntoskrnl.exe!ObGetObjectSecurity]
  		mov	ecx,[ebp-04h]
  		mov	esi,eax
  		call	[ntoskrnl.exe!ObfDereferenceObject]
  		push	[ebp+08h]
  		call	[ntoskrnl.exe!ZwClose]
  		mov	eax,esi
 L000136CC:
  		pop	edi
 L000136CD:
  		pop	esi
  		leave
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	8
 SUB_L000136D8:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000014h
  		push	ebx
  		push	esi
  		mov	esi,[ebp+08h]
  		mov	eax,[esi]
  		xor	ebx,ebx
  		cmp	eax,ebx
  		mov	[ebp-04h],ebx
  		mov	[ebp-08h],bl
  		mov	[ebp-0Ch],ebx
  		jnz	L000136FD
  		mov	eax,[esi+08h]
  		cmp	eax,ebx
  		jz 	L0001370C
 L000136FD:
  		push	eax
  		call	[ntoskrnl.exe!SeTokenIsAdmin]
  		test	al,al
  		jz 	L0001370C
  		mov	al,01h
  		jmp	L0001376B
 L0001370C:
  		push	L000151AE
  		lea	eax,[ebp-14h]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		lea	eax,[ebp-08h]
  		push	eax
  		lea	eax,[ebp-04h]
  		push	eax
  		lea	eax,[ebp-14h]
  		push	eax
  		call	SUB_L00013640
  		cmp	eax,ebx
  		mov	[ebp+08h],eax
  		jge	L00013737
  		xor	al,al
  		jmp	L0001376B
 L00013737:
  		lea	eax,[ebp+08h]
  		push	eax
  		lea	eax,[ebp-0Ch]
  		push	eax
  		push	00000001h
  		call	[ntoskrnl.exe!IoGetFileObjectGenericMapping]
  		push	eax
  		push	ebx
  		push	ebx
  		push	00020000h
  		push	00000001h
  		push	esi
  		push	[ebp-04h]
  		call	[ntoskrnl.exe!SeAccessCheck]
  		push	[ebp-08h]
  		mov	bl,al
  		push	[ebp-04h]
  		call	[ntoskrnl.exe!ObReleaseObjectSecurity]
  		mov	al,bl
 L0001376B:
  		pop	esi
  		pop	ebx
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00013776:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		xor	eax,eax
  		cmp	[ebp+0Ch],eax
  		jz 	L0001378B
  		cmp	dword ptr [ebp+0Ch],7FFFFFFFh
  		jbe	L00013790
 L0001378B:
  		mov	eax,C000000Dh
 L00013790:
  		test	eax,eax
  		jl 	L000137A9
  		mov	eax,[ebp+0Ch]
  		push	edi
  		push	[ebp+14h]
  		mov	edi,[ebp+08h]
  		push	[ebp+10h]
  		push	00000000h
  		call	SUB_L0001290A
  		pop	edi
 L000137A9:
  		pop	ebp
  		retn	0010h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L000137B2:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		xor	eax,eax
  		cmp	[ebp+0Ch],eax
  		jz 	L000137C7
  		cmp	dword ptr [ebp+0Ch],7FFFFFFFh
  		jbe	L000137CC
 L000137C7:
  		mov	eax,C000000Dh
 L000137CC:
  		test	eax,eax
  		jl 	L000137E6
  		push	edi
  		mov	edi,[ebp+08h]
  		lea	eax,[ebp+14h]
  		push	eax
  		push	[ebp+10h]
  		mov	eax,[ebp+0Ch]
  		push	00000000h
  		call	SUB_L0001290A
  		pop	edi
 L000137E6:
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
  		Align	2
 SUB_L000137EE:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+08h]
  		test	eax,eax
  		push	edi
  		mov	edi,[ebp+10h]
  		jz 	L00013810
  		mov	edx,[ebp+0Ch]
  		cmp	edx,7FFFFFFFh
  		ja 	L00013810
  		call	SUB_L000128D4
  		jmp	L00013815
 L00013810:
  		mov	eax,C000000Dh
 L00013815:
  		test	eax,eax
  		jge	L00013820
  		test	edi,edi
  		jz 	L00013820
  		and	dword ptr [edi],00000000h
 L00013820:
  		pop	edi
  		pop	ebp
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 SUB_L0001382A:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000010h
  		push	ebx
  		mov	ebx,[ebp+1Ch]
  		movzx	eax,[ebx+30h]
  		inc	al
  		push	esi
  		push	00000000h
  		push	eax
  		call	[ntoskrnl.exe!IoAllocateIrp]
  		mov	esi,eax
  		test	esi,esi
  		jnz	L0001385D
  		mov	eax,[ebp+18h]
  		and	[eax+04h],esi
  		mov	dword ptr [eax],C000009Ah
  		jmp	L000138DE
 L0001385D:
  		mov	eax,[ebp+0Ch]
  		mov	[esi+0Ch],eax
  		lea	eax,[ebp-10h]
  		mov	[esi+2Ch],eax
  		mov	eax,[ebp+18h]
  		push	edi
  		mov	[esi+28h],eax
  		call	jmp_ntoskrnl.exe!KeGetCurrentThread
  		mov	edi,[ebp+08h]
  		push	00000000h
  		mov	[esi+50h],eax
  		push	00000001h
  		lea	eax,[ebp-10h]
  		push	eax
  		mov	[esi+64h],edi
  		mov	byte ptr [esi+20h],00h
  		mov	dword ptr [esi+08h],00000004h
  		call	[ntoskrnl.exe!KeInitializeEvent]
  		add	dword ptr [esi+60h],FFFFFFDCh
  		mov	eax,[esi+60h]
  		dec	[esi+23h]
  		mov	ecx,[ebp+10h]
  		mov	[eax+04h],ecx
  		mov	ecx,[ebp+14h]
  		push	esi
  		push	ebx
  		mov	byte ptr [eax],06h
  		mov	[eax+14h],ebx
  		mov	[eax+18h],edi
  		mov	[eax+08h],ecx
  		call	SUB_L000133A6
  		test	al,al
  		pop	edi
  		jnz	L000138C9
  		mov	dword ptr [esi+18h],C0000001h
 L000138C9:
  		mov	ecx,[esi+18h]
  		mov	eax,[ebp+18h]
  		mov	[eax],ecx
  		mov	ecx,[esi+1Ch]
  		push	esi
  		mov	[eax+04h],ecx
  		call	[ntoskrnl.exe!IoFreeIrp]
 L000138DE:
  		pop	esi
  		pop	ebx
  		leave
  		retn	0018h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L000138EA:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		push	esi
  		push	edi
  		mov	edi,[ebp+08h]
  		mov	eax,[edi+000001B4h]
  		movzx	eax,[eax+30h]
  		xor	ebx,ebx
  		push	ebx
  		push	eax
  		call	[ntoskrnl.exe!IoAllocateIrp]
  		cmp	eax,ebx
  		mov	esi,[ebp+0Ch]
  		mov	[esi+10h],eax
  		jnz	L0001391D
 L00013913:
  		mov	eax,C000009Ah
  		jmp	L00013A02
 L0001391D:
  		mov	[eax+08h],ebx
  		mov	eax,[esi+10h]
  		mov	[eax+2Ch],ebx
  		mov	ecx,[esi+10h]
  		lea	eax,[esi+14h]
  		mov	[ecx+28h],eax
  		call	jmp_ntoskrnl.exe!KeGetCurrentThread
  		mov	ecx,[esi+10h]
  		mov	[ecx+50h],eax
  		mov	eax,[esi+10h]
  		mov	ecx,[edi+000001B0h]
  		mov	[eax+64h],ecx
  		mov	eax,[esi+10h]
  		push	ebx
  		mov	[eax+20h],bl
  		mov	eax,[esi+10h]
  		push	ebx
  		push	ebx
  		mov	[eax+0Ch],ebx
  		push	[esi+08h]
  		lea	eax,[esi+1Ch]
  		push	eax
  		call	[ntoskrnl.exe!IoAllocateMdl]
  		mov	ecx,[esi+10h]
  		mov	[ecx+04h],eax
  		mov	eax,[esi+10h]
  		mov	ecx,[eax+04h]
  		cmp	ecx,ebx
  		jnz	L0001397E
  		push	eax
  		call	[ntoskrnl.exe!IoFreeIrp]
  		mov	[esi+10h],ebx
  		jmp	L00013913
 L0001397E:
  		push	ecx
  		call	[ntoskrnl.exe!MmBuildMdlForNonPagedPool]
  		mov	eax,[esi+10h]
  		mov	ecx,[eax+04h]
  		mov	edx,[ecx+18h]
  		add	edx,[ecx+10h]
  		mov	[eax+3Ch],edx
  		mov	eax,[esi+10h]
  		mov	eax,[eax+60h]
  		sub	eax,00000024h
  		mov	byte ptr [eax],04h
  		mov	[eax+01h],bl
  		mov	ecx,[edi+000001B4h]
  		mov	[eax+14h],ecx
  		mov	ecx,[edi+000001B0h]
  		mov	[eax+18h],ecx
  		mov	ecx,[esi+08h]
  		mov	[eax+04h],ecx
  		mov	ecx,[ebp+10h]
  		mov	[eax+0Ch],ecx
  		mov	ecx,[ebp+14h]
  		mov	[eax+10h],ecx
  		xor	ecx,ecx
  		mov	[eax+08h],ebx
  		lea	eax,[edi+000001C0h]
  		inc	ecx
  		lock
  		xadd	[eax],ecx
  		mov	eax,[esi+10h]
  		mov	[esi+0Ch],edi
  		mov	eax,[eax+60h]
  		sub	eax,00000024h
  		mov	dword ptr [eax+1Ch],L00013326
  		mov	[eax+20h],esi
  		mov	byte ptr [eax+03h],E0h
  		mov	edx,[esi+10h]
  		mov	ecx,[edi+000001B4h]
  		call	[ntoskrnl.exe!IofCallDriver]
  		xor	eax,eax
 L00013A02:
  		pop	edi
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn	0010h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00013A0E:
  		push	00000078h
  		push	L000162F8
  		call	SUB_L00014A28
  		xor	ebx,ebx
  		mov	[ebp-54h],ebx
  		cmp	[L000170B8],bl
  		jz 	L00013B9B
  		lea	edx,[ebp-78h]
  		mov	ecx,L000170E4
  		call	[HAL.dll!KeAcquireInStackQueuedSpinLock]
  		push	676F6C61h
  		mov	edi,0000011Ch
  		push	edi
  		push	ebx
  		call	[ntoskrnl.exe!ExAllocatePoolWithTag]
  		mov	esi,eax
  		mov	[ebp-58h],esi
  		cmp	esi,ebx
  		jz 	L00013B92
  		push	edi
  		push	ebx
  		push	esi
  		call	jmp_ntoskrnl.exe!memset
  		add	esp,0000000Ch
  		cmp	[ebp+08h],ebx
  		jz 	L00013B17
  		push	00000032h
  		push	ebx
  		lea	eax,[ebp-50h]
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		add	esp,0000000Ch
  		mov	[ebp-04h],ebx
  		lea	eax,[ebp+0Ch]
  		push	eax
  		push	[ebp+08h]
  		push	00000031h
  		lea	eax,[ebp-50h]
  		push	eax
  		call	SUB_L00013776
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		cmp	[ebp+08h],ebx
  		jz 	L00013B17
  		lea	eax,[ebp-00000088h]
  		push	eax
  		call	[ntoskrnl.exe!KeQuerySystemTime]
  		lea	eax,[ebp-80h]
  		push	eax
  		lea	eax,[ebp-00000088h]
  		push	eax
  		call	[ntoskrnl.exe!ExSystemTimeToLocalTime]
  		lea	eax,[ebp-6Ch]
  		push	eax
  		lea	eax,[ebp-80h]
  		push	eax
  		call	[ntoskrnl.exe!RtlTimeToTimeFields]
  		lea	eax,[ebp-5Ch]
  		push	eax
  		call	SUB_L00012956
  		lea	eax,[ebp-50h]
  		push	eax
  		push	[ebp-58h]
  		push	[ebp-5Ch]
  		movsx	eax,[ebp-60h]
  		push	eax
  		movsx	eax,[ebp-62h]
  		push	eax
  		movsx	eax,[ebp-64h]
  		push	eax
  		movsx	eax,[ebp-66h]
  		push	eax
  		movsx	eax,[ebp-68h]
  		push	eax
  		movsx	eax,[ebp-6Ah]
  		push	eax
  		movsx	eax,[ebp-6Ch]
  		push	eax
  		push	SSZ00015244__04_4d__02_2d__02_2d__02_2d__02_
  		push	00000100h
  		lea	eax,[esi+1Ch]
  		push	eax
  		call	SUB_L000137B2
  		add	esp,00000034h
  		mov	[ebp-54h],eax
 L00013B17:
  		cmp	[ebp-54h],ebx
  		jl 	L00013B8A
  		cmp	[ebp+08h],ebx
  		jz 	L00013B59
  		lea	eax,[esi+08h]
  		push	eax
  		push	00000100h
  		lea	eax,[esi+1Ch]
  		push	eax
  		call	SUB_L000137EE
  		jmp	L00013B5C
 L00013B35:
  		xor	eax,eax
  		inc	eax
  		retn
;------------------------------------------------------------------------------
 L00013B39:
  		mov	esp,[ebp-18h]
  		push	00000000h
  		push	[ebp-58h]
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
  		lea	ecx,[ebp-78h]
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		jmp	L00013B9B
 L00013B59:
  		mov	[esi+08h],ebx
 L00013B5C:
  		mov	eax,[L000170E0]
  		mov	dword ptr [esi],L000170DC
  		mov	[esi+04h],eax
  		mov	[eax],esi
  		mov	[L000170E0],esi
  		lea	ecx,[ebp-78h]
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
  		push	ebx
  		push	ebx
  		push	L000170BC
  		call	[ntoskrnl.exe!KeSetEvent]
  		jmp	L00013B9B
 L00013B8A:
  		push	ebx
  		push	esi
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
 L00013B92:
  		lea	ecx,[ebp-78h]
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
 L00013B9B:
  		call	SUB_L00014A70
  		retn
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00013BA6:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+0Ch]
  		mov	eax,[eax]
  		sub	eax,00000000h
  		jz 	L00013BE8
  		dec	eax
  		jz 	L00013BD7
  		sub	eax,0000FFFFh
  		jz 	L00013BD0
  		dec	eax
  		jz 	L00013BC9
  		push	SSZ000152E2_Unknown_State
  		jmp	L00013BF7
 L00013BC9:
  		push	SSZ000152CA_Lid_Open_On_DC___Short
  		jmp	L00013BED
 L00013BD0:
  		push	SSZ000152B2_Lid_Open_On_AC___Short
  		jmp	L00013BED
 L00013BD7:
  		mov	eax,[ebp+08h]
  		mov	byte ptr [eax+000000C0h],00h
  		push	SSZ0001529A_Lid_Closed_On_DC___Long
  		jmp	L00013BF7
 L00013BE8:
  		push	SSZ00015280_Lid_Closed_On_AC___Short
 L00013BED:
  		mov	eax,[ebp+08h]
  		mov	byte ptr [eax+000000C0h],01h
 L00013BF7:
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00013C06:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+08h]
  		xor	cl,cl
  		cmp	[eax+00000135h],cl
  		jz 	L00013C23
  		mov	byte ptr [eax+42h],01h
  		push	SSZ00015358_UseStandardModeOnly___TRUE__usin
  		jmp	L00013C3E
 L00013C23:
  		cmp	[eax+00000134h],cl
  		jz 	L00013C36
  		mov	byte ptr [eax+42h],01h
  		push	SSZ00015322_DisableFastParkOnLidOpen___TRUE_
  		jmp	L00013C3E
 L00013C36:
  		mov	[eax+42h],cl
  		push	SSZ000152F0_DisableFastParkOnLidOpen___FALSE
 L00013C3E:
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ebp
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 SUB_L00013C4E:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000010h
  		push	ebx
  		xor	ebx,ebx
  		push	esi
  		mov	[ebp-01h],bl
  		call	[HAL.dll!KeGetCurrentIrql]
  		mov	cl,02h
  		cmp	al,cl
  		jz 	L00013C76
  		call	[HAL.dll!KfRaiseIrql]
  		mov	[ebp-02h],al
  		mov	byte ptr [ebp-01h],01h
 L00013C76:
  		lea	edx,[ebp-10h]
  		mov	ecx,L000170B4
  		call	[ntoskrnl.exe!KeAcquireInStackQueuedSpinLockAtDpcLevel]
  		mov	esi,[ebp+08h]
  		mov	al,[esi+30h]
  		cmp	al,bl
  		jnz	L00013D43
  		cmp	[esi+31h],bl
  		jnz	L00013D43
  		cmp	[ebp+0Ch],bl
  		jz 	L00013CAE
  		push	00000001h
  		push	ebx
  		mov	byte ptr [esi+31h],01h
  		push	SSZ00015446_Critical_Battery_Ignoring_Interr
  		jmp	L00013CBA
 L00013CAE:
  		push	ebx
  		push	00000001h
  		mov	byte ptr [esi+30h],01h
  		push	SSZ0001541C_Ignoring_Interrupts____II____x__
 L00013CBA:
  		call	SUB_L00013A0E
  		add	esp,0000000Ch
  		push	edi
  		lea	edi,[esi+000000D4h]
  		cmp	[edi],bl
  		mov	[esi+00000100h],bl
  		jz 	L00013D37
  		lea	eax,[esi+000000D8h]
  		push	eax
  		mov	[ebp+0Ch],eax
  		call	[ntoskrnl.exe!KeCancelTimer]
  		test	al,al
  		jz 	L00013D2C
  		push	ebx
  		push	ebx
  		lea	eax,[esi+44h]
  		mov	[edi],bl
  		mov	edi,[ntoskrnl.exe!KeSetEvent]
  		push	eax
  		mov	[esi+40h],bl
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000114h]
  		push	eax
  		call	edi
  		push	ebx
  		push	00000001h
  		push	L000170CC
  		call	edi
  		push	SSZ000153E0_Shock_End_Event_Signaled__Hibera
  		call	SUB_L00013A0E
  		pop	ecx
  		push	esi
  		call	SUB_L00012CEE
  		push	ebx
  		push	[ebp+0Ch]
  		call	[ntoskrnl.exe!KeInitializeTimerEx]
  		jmp	L00013D37
 L00013D2C:
  		push	SSZ000153BA_IgnoreInterrupts_cannot_cancel_t
  		call	SUB_L00013A0E
  		pop	ecx
 L00013D37:
  		lea	ecx,[ebp-10h]
  		call	[ntoskrnl.exe!KeReleaseInStackQueuedSpinLockFromDpcLevel]
  		pop	edi
  		jmp	L00013D76
 L00013D43:
  		cmp	[ebp+0Ch],bl
  		jz 	L00013D4E
  		mov	byte ptr [esi+31h],01h
  		jmp	L00013D56
 L00013D4E:
  		cmp	al,bl
  		jnz	L00013D56
  		mov	byte ptr [esi+30h],01h
 L00013D56:
  		lea	ecx,[ebp-10h]
  		call	[ntoskrnl.exe!KeReleaseInStackQueuedSpinLockFromDpcLevel]
  		movzx	eax,[esi+31h]
  		push	eax
  		movzx	eax,[esi+30h]
  		push	eax
  		push	SSZ00015388_Already_Ignoring_Interrupts____I
  		call	SUB_L00013A0E
  		add	esp,0000000Ch
 L00013D76:
  		cmp	[ebp-01h],bl
  		jz 	L00013D84
  		mov	cl,[ebp-02h]
  		call	[HAL.dll!KfLowerIrql]
 L00013D84:
  		pop	esi
  		pop	ebx
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00013D90:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000018h
  		push	ebx
  		push	esi
  		mov	esi,[ebp+08h]
  		push	edi
  		xor	edi,edi
  		cmp	byte ptr [esi+00000101h],00h
  		mov	[ebp-08h],edi
  		jz 	L00013F2C
  		cmp	byte ptr [ebp+10h],00h
  		jz 	L00013EF7
  		cmp	[ebp+0Ch],edi
  		mov	[ebp-04h],edi
  		jnz	L00013DCC
  		mov	eax,C000000Dh
  		jmp	L00013F2E
 L00013DCC:
  		lea	eax,[ebp+10h]
  		push	eax
  		push	00000004h
  		push	SWC00014DF8_Enabled
  		push	L00017098
  		call	SUB_L000129C2
  		xor	eax,eax
  		cmp	[ebp+10h],al
  		mov	ebx,L0001505A
  		setnz	al
  		mov	[ebp+10h],eax
  		lea	eax,[ebp-08h]
  		push	eax
  		push	00000004h
  		push	ebx
  		push	L000170A0
  		call	SUB_L000129C2
  		mov	eax,[ebp+0Ch]
  		lea	ecx,[eax+02h]
 L00013E08:
  		mov	dx,[eax]
  		inc	eax
  		inc	eax
  		cmp	dx,di
  		jnz	L00013E08
  		sub	eax,ecx
  		sar	eax,1
  		push	79654B61h
  		lea	eax,[eax+eax+00000096h]
  		push	eax
  		push	00000001h
  		call	[ntoskrnl.exe!ExAllocatePoolWithTag]
  		cmp	eax,edi
  		mov	[ebp-0Ch],eax
  		jnz	L00013E3C
  		mov	eax,C000009Ah
  		jmp	L00013F2E
 L00013E3C:
  		mov	eax,[ebp+0Ch]
  		mov	[ebp-10h],di
  		lea	edx,[eax+02h]
 L00013E46:
  		mov	cx,[eax]
  		inc	eax
  		inc	eax
  		cmp	cx,di
  		jnz	L00013E46
  		mov	esi,[ntoskrnl.exe!RtlInitUnicodeString]
  		sub	eax,edx
  		sar	eax,1
  		lea	eax,[eax+eax+00000096h]
  		mov	[ebp-0Eh],ax
  		push	L00015482
  		lea	eax,[ebp-18h]
  		push	eax
  		call	esi
  		mov	edi,[ntoskrnl.exe!RtlAppendUnicodeStringToString]
  		lea	eax,[ebp-18h]
  		push	eax
  		lea	eax,[ebp-10h]
  		push	eax
  		call	edi
  		push	[ebp+0Ch]
  		lea	eax,[ebp-18h]
  		push	eax
  		call	esi
  		lea	eax,[ebp-18h]
  		push	eax
  		lea	eax,[ebp-10h]
  		push	eax
  		call	edi
  		push	SWC00014FE4__Software_Hewlett_Packard_HP_Mob
  		lea	eax,[ebp-18h]
  		push	eax
  		call	esi
  		lea	eax,[ebp-18h]
  		push	eax
  		lea	eax,[ebp-10h]
  		push	eax
  		call	edi
  		lea	eax,[ebp-04h]
  		push	eax
  		push	ebx
  		lea	eax,[ebp-10h]
  		push	eax
  		call	SUB_L00013544
  		mov	eax,[ebp-04h]
  		cmp	eax,[ebp-08h]
  		jnz	L00013EDB
  		lea	eax,[ebp+10h]
  		push	eax
  		push	SWC00014DF8_Enabled
  		lea	eax,[ebp-10h]
  		push	eax
  		call	SUB_L00013544
  		xor	eax,eax
  		cmp	[ebp+10h],al
  		setnz	al
  		mov	[ebp+10h],eax
 L00013EDB:
  		push	00000000h
  		push	[ebp-0Ch]
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
  		cmp	byte ptr [ebp+10h],00h
  		mov	ecx,[ebp+08h]
  		setz 	al
  		mov	[ecx+41h],al
  		mov	esi,ecx
  		jmp	L00013F26
 L00013EF7:
  		lea	eax,[ebp+10h]
  		push	eax
  		push	00000004h
  		push	SWC00014DF8_Enabled
  		push	L00017098
  		mov	dword ptr [ebp+10h],00000001h
  		call	SUB_L000129C2
  		xor	eax,eax
  		cmp	[ebp+10h],al
  		setnz	al
  		test	al,al
  		mov	[ebp+10h],eax
  		setz 	al
  		mov	[esi+41h],al
 L00013F26:
  		push	esi
  		call	SUB_L00012BC2
 L00013F2C:
  		xor	eax,eax
 L00013F2E:
  		pop	edi
  		pop	esi
  		pop	ebx
  		leave
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 L00013F3A:
  		push	00000084h
  		push	L00016318
  		call	SUB_L00014860
  		mov	esi,[ebp+08h]
  		mov	[ebp-20h],esi
  		push	0000001Eh
  		call	jmp_ntoskrnl.exe!KeGetCurrentThread
  		push	eax
  		call	[ntoskrnl.exe!KeSetPriorityThread]
  		push	00000060h
  		xor	ebx,ebx
  		push	ebx
  		lea	eax,[ebp-00000094h]
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		add	esp,0000000Ch
  		lea	eax,[esi+00000140h]
  		mov	[ebp-34h],eax
  		lea	eax,[esi+00000104h]
  		mov	[ebp-30h],eax
  		lea	eax,[esi+00000124h]
  		mov	[ebp-2Ch],eax
  		lea	eax,[esi+00000114h]
  		mov	[ebp-28h],eax
  		mov	[ebp-04h],ebx
 L00013F98:
  		lea	eax,[ebp-00000094h]
  		push	eax
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		xor	edi,edi
  		inc	edi
  		push	edi
  		lea	eax,[ebp-34h]
  		push	eax
  		push	00000004h
  		pop	eax
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForMultipleObjects]
  		sub	eax,ebx
  		jz 	L00014077
  		dec	eax
  		jz 	L00013FE8
  		dec	eax
  		jz 	L00014017
  		dec	eax
  		jnz	L00013F98
  		mov	[ebp+0Bh],bl
  		push	edi
  		lea	eax,[ebp+0Bh]
  		push	eax
  		push	ebx
  		push	00000004h
  		push	esi
  		call	SUB_L00012D72
  		push	ebx
  		push	edi
  		lea	eax,[esi+000001E8h]
  		push	eax
  		call	[ntoskrnl.exe!KeSetEvent]
  		jmp	L00013F98
 L00013FE8:
  		mov	byte ptr [ebp-19h],01h
  		xor	eax,eax
  		lea	ecx,[esi+00000190h]
  		xchg	eax,[ecx]
  		push	edi
  		lea	eax,[ebp-19h]
  		push	eax
  		push	ebx
  		push	00000004h
  		push	esi
  		call	SUB_L00012D72
  		lea	eax,[esi+38h]
  		mov	ecx,[eax]
  		or	ecx,[eax+04h]
  		jz 	L00014017
  		push	eax
  		push	ebx
  		push	ebx
  		call	[ntoskrnl.exe!KeDelayExecutionThread]
 L00014017:
  		push	edi
  		lea	eax,[ebp-1Ah]
  		push	eax
  		push	ebx
  		push	edi
  		push	esi
  		call	SUB_L00012D72
  		cmp	eax,ebx
  		jl 	L00013F98
  		cmp	[ebp-1Ah],bl
  		jz 	L00014052
  		cmp	[L000170E8],bl
  		jnz	L00013F98
  		push	edi
  		push	esi
  		call	SUB_L00013C4E
  		mov	byte ptr [L000170E8],01h
  		push	SSZ000154BE_Battery_Critical_
  		jmp	L0001406C
 L00014052:
  		cmp	[L000170E8],bl
  		jz 	L00013F98
  		mov	[L000170E8],bl
  		mov	[esi+31h],bl
  		push	SSZ000154A2_Battery_No_longer_Critical_
 L0001406C:
  		call	SUB_L00013A0E
  		pop	ecx
  		jmp	L00013F98
 L00014077:
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		jmp	L0001409D
 L00014080:
  		mov	eax,[ebp-14h]
  		mov	eax,[eax]
  		mov	eax,[eax]
  		mov	[ebp-24h],eax
  		xor	eax,eax
  		inc	eax
  		retn
;------------------------------------------------------------------------------
 L0001408E:
  		mov	esp,[ebp-18h]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		xor	ebx,ebx
  		mov	esi,[ebp-20h]
 L0001409D:
  		push	ebx
  		push	ebx
  		add	esi,00000150h
  		push	esi
  		call	[ntoskrnl.exe!KeSetEvent]
  		call	SUB_L000148A5
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	2
 L000140BA:
  		push	00000074h
  		push	L00016338
  		call	SUB_L00014A28
  		mov	esi,[ebp+08h]
  		mov	[ebp-50h],esi
  		xor	ebx,ebx
  		mov	[ebp-58h],ebx
  		mov	[ebp-54h],ebx
  		mov	dword ptr [ebp-60h],FFD9DA60h
  		or	dword ptr [ebp-5Ch],FFFFFFFFh
  		push	SWC000154D0__EFSInitEvent
  		lea	eax,[ebp-80h]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		mov	dword ptr [ebp-78h],00000018h
  		mov	[ebp-74h],ebx
  		mov	[ebp-6Ch],ebx
  		lea	eax,[ebp-80h]
  		mov	[ebp-70h],eax
  		mov	[ebp-68h],ebx
  		mov	[ebp-64h],ebx
 L00014107:
  		lea	eax,[ebp-78h]
  		push	eax
  		push	00000001h
  		lea	eax,[ebp-4Ch]
  		push	eax
  		call	[ntoskrnl.exe!ZwOpenEvent]
  		cmp	eax,ebx
  		jge	L0001414D
  		lea	eax,[ebp-58h]
  		push	eax
  		push	ebx
  		push	ebx
  		push	ebx
  		lea	eax,[esi+000001C8h]
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForSingleObject]
  		mov	edi,eax
  		cmp	edi,ebx
  		jz 	L0001426A
  		lea	eax,[ebp-60h]
  		push	eax
  		push	ebx
  		push	ebx
  		call	[ntoskrnl.exe!KeDelayExecutionThread]
  		cmp	edi,00000102h
  		jz 	L00014107
 L0001414D:
  		push	ebx
  		lea	eax,[ebp-48h]
  		push	eax
  		push	ebx
  		mov	eax,[ntoskrnl.exe!ExEventObjectType]
  		push	[eax]
  		xor	edi,edi
  		inc	edi
  		push	edi
  		push	[ebp-4Ch]
  		call	[ntoskrnl.exe!ObReferenceObjectByHandle]
  		cmp	eax,ebx
  		jl 	L000141F3
  		lea	eax,[esi+000001C8h]
  		mov	[ebp-44h],eax
  		mov	eax,[ebp-48h]
  		mov	[ebp-40h],eax
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		push	edi
  		lea	eax,[ebp-44h]
  		push	eax
  		push	00000002h
  		call	[ntoskrnl.exe!KeWaitForMultipleObjects]
  		sub	eax,ebx
  		jz 	L0001426A
  		dec	eax
  		jnz	L000141EA
  		push	00000020h
  		lea	eax,[ebp-3Ch]
  		push	eax
  		push	ebx
  		push	ebx
  		push	00000005h
  		call	[ntoskrnl.exe!ZwPowerInformation]
  		cmp	eax,ebx
  		jl 	L000141D4
  		cmp	[ebp-3Ch],bl
  		jz 	L000141C2
  		mov	[esi+000000CCh],bx
  		push	SSZ00014F7E_Query_Power_State_On_AC_
  		jmp	L000141CE
 L000141C2:
  		mov	[esi+000000CCh],di
  		push	SSZ00014F64_Query_Power_State_On_DC_
 L000141CE:
  		call	SUB_L00013A0E
  		pop	ecx
 L000141D4:
  		lea	eax,[esi+000000CCh]
  		push	eax
  		push	esi
  		call	SUB_L00013BA6
  		push	esi
  		call	SUB_L00013384
  		mov	[esi+32h],bl
 L000141EA:
  		mov	ecx,[ebp-48h]
  		call	[ntoskrnl.exe!ObfDereferenceObject]
 L000141F3:
  		push	[ebp-4Ch]
  		call	[ntoskrnl.exe!ZwClose]
  		lea	eax,[esi+000001C8h]
  		mov	[ebp-44h],eax
  		add	esi,000001E8h
  		mov	[ebp-40h],esi
  		mov	[ebp-04h],ebx
 L00014211:
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		push	ebx
  		push	edi
  		lea	eax,[ebp-44h]
  		push	eax
  		push	00000002h
  		call	[ntoskrnl.exe!KeWaitForMultipleObjects]
  		sub	eax,ebx
  		jz 	L00014244
  		dec	eax
  		jnz	L00014211
  		mov	eax,[ebp-50h]
  		push	[eax+00000244h]
  		push	SWC00014C4C_ShocksDetected
  		push	L00017090
  		call	SUB_L00012A90
  		jmp	L00014211
 L00014244:
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		jmp	L0001426A
 L0001424D:
  		mov	eax,[ebp-14h]
  		mov	eax,[eax]
  		mov	eax,[eax]
  		mov	[ebp-00000084h],eax
  		xor	eax,eax
  		inc	eax
  		retn
;------------------------------------------------------------------------------
 L0001425E:
  		mov	esp,[ebp-18h]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		xor	ebx,ebx
 L0001426A:
  		push	ebx
  		push	ebx
  		mov	eax,[ebp-50h]
  		add	eax,000001D8h
  		push	eax
  		call	[ntoskrnl.exe!KeSetEvent]
  		call	SUB_L00014A70
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	8
 SUB_L00014288:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		sub	esp,0000001Ch
  		push	ebx
  		push	edi
  		lea	edx,[ebp-1Ch]
  		mov	ecx,L000170E4
  		call	[HAL.dll!KeAcquireInStackQueuedSpinLock]
  		mov	edi,[L000170DC]
  		mov	ebx,L000170DC
  		cmp	edi,ebx
  		jz 	L000143EF
  		push	esi
  		mov	esi,[ebp+08h]
 L000142B7:
  		mov	eax,[edi]
  		mov	[L000170DC],eax
  		mov	[eax+04h],ebx
  		cmp	byte ptr [esi+40h],00h
  		jnz	L000143DB
  		lea	ecx,[ebp-1Ch]
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
  		cmp	dword ptr [esi+00000184h],00000000h
  		jz 	L000143D0
  		mov	edx,[edi+08h]
  		mov	eax,[esi+00000200h]
  		xor	ecx,ecx
  		add	edx,[esi+000001A0h]
  		adc	ecx,[esi+000001A4h]
  		xor	ebx,ebx
  		shl	eax,14h
  		cmp	ecx,ebx
  		jg 	L00014312
  		jl 	L00014307
  		cmp	edx,eax
  		jnc	L00014312
 L00014307:
  		mov	eax,[edi+08h]
  		xor	ebx,ebx
  		cmp	eax,ebx
  		jnz	L00014368
  		jmp	L00014314
 L00014312:
  		xor	ebx,ebx
 L00014314:
  		push	[esi+000001B4h]
  		lea	eax,[ebp-10h]
  		push	eax
  		push	00000014h
  		push	00000008h
  		lea	eax,[ebp-08h]
  		push	eax
  		push	[esi+000001B0h]
  		mov	[ebp-08h],ebx
  		mov	[ebp-04h],ebx
  		call	SUB_L0001382A
  		push	[esi+000001B4h]
  		lea	ecx,[ebp-10h]
  		push	ecx
  		push	0000000Eh
  		lea	eax,[esi+000001B8h]
  		push	00000008h
  		push	eax
  		push	[esi+000001B0h]
  		mov	[esi+000001A0h],ebx
  		mov	[esi+000001A4h],ebx
  		mov	[eax],ebx
  		mov	[eax+04h],ebx
  		call	SUB_L0001382A
 L00014368:
  		mov	ebx,[edi+08h]
  		test	ebx,ebx
  		jz 	L00014384
  		push	[esi+000001BCh]
  		push	[esi+000001B8h]
  		push	edi
  		push	esi
  		call	SUB_L000138EA
  		jmp	L00014386
 L00014384:
  		xor	eax,eax
 L00014386:
  		test	eax,eax
  		jl 	L000143A4
  		add	[esi+000001A0h],ebx
  		push	00000000h
  		pop	ecx
  		adc	[esi+000001A4h],ecx
  		lea	eax,[esi+000001B8h]
  		add	[eax],ebx
  		adc	[eax+04h],ecx
 L000143A4:
  		mov	ebx,L000170DC
 L000143A9:
  		lea	edx,[ebp-1Ch]
  		mov	ecx,L000170E4
  		call	[HAL.dll!KeAcquireInStackQueuedSpinLock]
  		cmp	dword ptr [esi+00000190h],00000000h
  		jz 	L000143EE
  		mov	edi,[L000170DC]
  		cmp	edi,ebx
  		jnz	L000142B7
  		jmp	L000143EE
 L000143D0:
  		push	00000000h
  		push	edi
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
  		jmp	L000143A9
 L000143DB:
  		mov	eax,[L000170DC]
  		mov	[edi],eax
  		mov	[edi+04h],ebx
  		mov	[eax+04h],edi
  		mov	[L000170DC],edi
 L000143EE:
  		pop	esi
 L000143EF:
  		lea	ecx,[ebp-1Ch]
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
  		pop	edi
  		pop	ebx
  		leave
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	4
 L00014404:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000014h
  		push	ebx
  		push	esi
  		mov	esi,[ebp+08h]
  		mov	ax,[esi+000000CCh]
  		mov	[ebp-08h],ax
  		mov	eax,[ebp+0Ch]
  		xor	ebx,ebx
  		dec	eax
  		push	edi
  		mov	di,[esi+000000CEh]
  		mov	[ebp-02h],bl
  		mov	[ebp-01h],bl
  		mov	[ebp-06h],di
  		jz 	L00014582
  		dec	eax
  		dec	eax
  		jz 	L000144D8
  		dec	eax
  		jnz	L00014666
  		push	00000001h
  		lea	eax,[ebp+0Bh]
  		push	eax
  		push	ebx
  		push	00000005h
  		push	esi
  		call	SUB_L00012D72
  		test	eax,eax
  		jge	L0001446F
  		mov	eax,[ebp+10h]
  		push	eax
  		push	SSZ0001561C_ALID_method_failed__using_callba
  		mov	[ebp+0Bh],al
  		call	SUB_L00013A0E
  		pop	ecx
  		pop	ecx
 L0001446F:
  		cmp	byte ptr [ebp+0Bh],01h
  		jnz	L0001447A
  		cmp	di,bx
  		jz 	L0001448D
 L0001447A:
  		cmp	[ebp+0Bh],bl
  		jnz	L00014666
  		cmp	di,0001h
  		jnz	L00014666
 L0001448D:
  		movzx	eax,[ebp+0Bh]
  		push	eax
  		push	[ebp+10h]
  		mov	byte ptr [ebp-01h],01h
  		push	SSZ00015602_Lid_change___p__really__d
  		call	SUB_L00013A0E
  		add	esp,0000000Ch
  		cmp	byte ptr [ebp+0Bh],01h
  		jnz	L000144BD
 L000144AC:
  		push	esi
  		call	SUB_L00013C06
  		mov	word ptr [ebp-06h],0001h
  		jmp	L000145AC
 L000144BD:
  		push	[ebp+10h]
  		push	SSZ000155DE_Lid_Closed__using_Standard_Mode_
 L000144C5:
  		call	SUB_L00013A0E
  		pop	ecx
  		mov	byte ptr [esi+42h],01h
  		mov	[ebp-06h],bx
  		jmp	L000145AB
 L000144D8:
  		cmp	[ebp+10h],ebx
  		jz 	L0001456B
  		cmp	[esi+31h],bl
  		jz 	L000144EF
  		push	esi
  		mov	[esi+31h],bl
  		call	SUB_L00012CEE
 L000144EF:
  		push	SSZ000155C0_Reenter_S0___Check_Lid_State
  		call	SUB_L00013A0E
  		pop	ecx
  		push	00000001h
  		lea	eax,[ebp+0Bh]
  		push	eax
  		push	ebx
  		push	00000005h
  		push	esi
  		call	SUB_L00012D72
  		test	eax,eax
  		jl 	L0001455B
  		cmp	byte ptr [ebp+0Bh],01h
  		jnz	L00014518
  		cmp	di,bx
  		jz 	L0001452B
 L00014518:
  		cmp	[ebp+0Bh],bl
  		jnz	L00014666
  		cmp	di,0001h
  		jnz	L00014666
 L0001452B:
  		movzx	eax,[ebp+0Bh]
  		push	eax
  		push	[ebp+10h]
  		mov	byte ptr [ebp-01h],01h
  		push	SSZ000155A2_S0_Lid_change___p__really__d
  		call	SUB_L00013A0E
  		add	esp,0000000Ch
  		cmp	byte ptr [ebp+0Bh],01h
  		jz 	L000144AC
  		push	[ebp+10h]
  		push	SSZ0001557C_S0_Lid_Closed__using_Standard_Mo
  		jmp	L000144C5
 L0001455B:
  		push	SSZ00014F2E_Query_Lid_State_from_BIOS_failed
  		call	SUB_L00013A0E
  		pop	ecx
  		jmp	L00014666
 L0001456B:
  		push	SSZ00015570_Leaving_S0
  		call	SUB_L00013A0E
  		pop	ecx
  		push	ebx
  		push	esi
  		call	SUB_L00013C4E
  		jmp	L00014666
 L00014582:
  		cmp	[ebp+10h],ebx
  		jz 	L00014597
  		push	SSZ00015562_Running_on_AC
  		call	SUB_L00013A0E
  		mov	[ebp-08h],bx
  		jmp	L000145A7
 L00014597:
  		push	SSZ0001554E_Running_on_Battery
  		call	SUB_L00013A0E
  		mov	word ptr [ebp-08h],0001h
 L000145A7:
  		mov	byte ptr [ebp-02h],01h
 L000145AB:
  		pop	ecx
 L000145AC:
  		lea	edx,[ebp-14h]
  		mov	ecx,L000170B4
  		call	[HAL.dll!KeAcquireInStackQueuedSpinLock]
  		cmp	[esi+000000D4h],bl
  		jz 	L0001463F
  		push	SSZ00015532_State_Change_Timer_Running
  		call	SUB_L00013A0E
  		pop	ecx
  		lea	eax,[esi+000000D8h]
  		push	eax
  		call	[ntoskrnl.exe!KeCancelTimer]
  		test	al,al
  		jz 	L00014637
  		mov	edi,[ntoskrnl.exe!KeSetEvent]
  		push	ebx
  		push	ebx
  		lea	eax,[esi+44h]
  		push	eax
  		mov	[esi+40h],bl
  		mov	[esi+000000D4h],bl
  		call	edi
  		push	ebx
  		push	ebx
  		lea	eax,[esi+00000114h]
  		push	eax
  		call	edi
  		push	ebx
  		push	00000001h
  		push	L000170CC
  		call	edi
  		movzx	eax,[ebp-01h]
  		push	eax
  		movzx	eax,[ebp-02h]
  		push	eax
  		push	SSZ0001550C_Shock_End_Event_Signaled_AC__d_L
  		call	SUB_L00013A0E
  		add	esp,0000000Ch
  		push	esi
  		call	SUB_L00012CEE
  		push	ebx
  		lea	eax,[esi+000000D8h]
  		push	eax
  		call	[ntoskrnl.exe!KeInitializeTimerEx]
  		jmp	L0001464A
 L00014637:
  		mov	[esi+00000100h],bl
  		jmp	L0001464A
 L0001463F:
  		push	SSZ000154EC_State_Change_Timer_Not_Running
  		call	SUB_L00013A0E
  		pop	ecx
 L0001464A:
  		lea	eax,[ebp-08h]
  		push	eax
  		push	esi
  		call	SUB_L00013BA6
  		mov	eax,[ebp-08h]
  		lea	ecx,[ebp-14h]
  		mov	[esi+000000CCh],eax
  		call	[HAL.dll!KeReleaseInStackQueuedSpinLock]
 L00014666:
  		pop	edi
  		pop	esi
  		pop	ebx
  		leave
  		retn	000Ch
;------------------------------------------------------------------------------
  		Align	2
 L00014672:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[ebp+08h]
  		push	00000000h
  		push	[eax+28h]
  		call	SUB_L00013C4E
  		mov	ecx,[ebp+0Ch]
  		and	dword ptr [ecx+18h],00000000h
  		and	dword ptr [ecx+1Ch],00000000h
  		xor	dl,dl
  		call	[ntoskrnl.exe!IofCompleteRequest]
  		xor	eax,eax
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	2
 L000146A2:
  		push	00000028h
  		push	L00016358
  		call	SUB_L00014860
  		mov	edi,[ebp+08h]
  		mov	[ebp+08h],edi
  		mov	dword ptr [ebp-24h],EE1E5D00h
  		or	dword ptr [ebp-20h],FFFFFFFFh
  		lea	eax,[ebp-24h]
  		push	eax
  		xor	esi,esi
  		push	esi
  		push	esi
  		call	[ntoskrnl.exe!KeDelayExecutionThread]
  		mov	dword ptr [ebp-2Ch],FFD9DA60h
  		or	dword ptr [ebp-28h],FFFFFFFFh
  		push	00000014h
  		call	jmp_ntoskrnl.exe!KeGetCurrentThread
  		push	eax
  		call	[ntoskrnl.exe!KeSetPriorityThread]
  		mov	[ebp-04h],esi
  		cmp	[edi+00000184h],esi
  		jnz	L000146F7
  		push	edi
  		call	SUB_L00013016
 L000146F7:
  		lea	eax,[edi+00000164h]
  		mov	[ebp-38h],eax
  		mov	dword ptr [ebp-34h],L000170CC
  		mov	dword ptr [ebp-30h],L000170BC
 L0001470E:
  		push	esi
  		push	esi
  		push	esi
  		push	esi
  		push	esi
  		push	00000001h
  		lea	eax,[ebp-38h]
  		push	eax
  		push	00000003h
  		pop	eax
  		push	eax
  		call	[ntoskrnl.exe!KeWaitForMultipleObjects]
  		sub	eax,esi
  		jz 	L0001474A
  		dec	eax
  		jz 	L0001472F
  		dec	eax
  		jnz	L0001470E
  		jmp	L0001473A
 L0001472F:
  		xor	eax,eax
  		inc	eax
  		lea	ecx,[edi+00000190h]
  		xchg	eax,[ecx]
 L0001473A:
  		cmp	[edi+00000190h],esi
  		jz 	L0001470E
  		push	edi
  		call	SUB_L00014288
  		jmp	L0001470E
 L0001474A:
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		jmp	L00014770
 L00014753:
  		mov	eax,[ebp-14h]
  		mov	eax,[eax]
  		mov	eax,[eax]
  		mov	[ebp-1Ch],eax
  		xor	eax,eax
  		inc	eax
  		retn
;------------------------------------------------------------------------------
 L00014761:
  		mov	esp,[ebp-18h]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		xor	esi,esi
  		mov	edi,[ebp+08h]
 L00014770:
  		cmp	[edi+00000190h],esi
  		jz 	L0001477E
  		push	edi
  		call	SUB_L00014288
 L0001477E:
  		lea	ebx,[edi+000001C0h]
  		jmp	L00014792
 L00014786:
  		lea	eax,[ebp-2Ch]
  		push	eax
  		push	esi
  		push	esi
  		call	[ntoskrnl.exe!KeDelayExecutionThread]
 L00014792:
  		cmp	[ebx],esi
  		jnz	L00014786
  		mov	eax,[edi+00000184h]
  		cmp	eax,esi
  		jz 	L000147A7
  		push	eax
  		call	[ntoskrnl.exe!ZwClose]
 L000147A7:
  		mov	ecx,[edi+000001B0h]
  		cmp	ecx,esi
  		jz 	L000147B7
  		call	[ntoskrnl.exe!ObfDereferenceObject]
 L000147B7:
  		push	esi
  		push	esi
  		add	edi,00000174h
  		push	edi
  		call	[ntoskrnl.exe!KeSetEvent]
  		call	SUB_L000148A5
  		retn	0004h
;------------------------------------------------------------------------------
  		Align	4
 L000147D4:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		mov	edx,[ebp+0Ch]
  		mov	eax,[ebp+08h]
  		mov	eax,[eax+28h]
  		inc	[edx+23h]
  		add	dword ptr [edx+60h],00000024h
  		mov	ecx,[eax+08h]
  		call	[ntoskrnl.exe!IofCallDriver]
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 SUB_L000147FB:
  		cmp	ecx,[L00017000]
  		jnz	L00014805
 		db	F3h;   '„'
 		db	C3h;   'É'
 L00014805:
  		jmp	L0001480F
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 L0001480F:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ecx
  		mov	[ebp-04h],ecx
  		push	00000000h
  		push	[L00017004]
  		push	[L00017000]
  		push	[ebp-04h]
  		push	000000F7h
  		call	[ntoskrnl.exe!KeBugCheckEx]
  		int3
  		int3
  		int3
  		int3
  		int3
  		int3
 jmp_ntoskrnl.exe!memcpy:
  		jmp	[ntoskrnl.exe!memcpy]
  		Align	2
 jmp_ntoskrnl.exe!memset:
  		jmp	[ntoskrnl.exe!memset]
  		Align	2
 jmp_ntoskrnl.exe!_allmul:
  		jmp	[ntoskrnl.exe!_allmul]
  		Align	16
 SUB_L00014860:
  		push	L000148C0
  		push	fs:[00000000h]
  		mov	eax,[esp+10h]
  		mov	[esp+10h],ebp
  		lea	ebp,[esp+10h]
  		sub	esp,eax
  		push	ebx
  		push	esi
  		push	edi
  		mov	eax,[L00017000]
  		xor	[ebp-04h],eax
  		xor	eax,ebp
  		push	eax
  		mov	[ebp-18h],esp
  		push	[ebp-08h]
  		mov	eax,[ebp-04h]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		mov	[ebp-08h],eax
  		lea	eax,[ebp-10h]
  		mov	fs:[00000000h],eax
  		retn
;------------------------------------------------------------------------------
 SUB_L000148A5:
  		mov	ecx,[ebp-10h]
  		mov	fs:[00000000h],ecx
  		pop	ecx
  		pop	edi
  		pop	edi
  		pop	esi
  		pop	ebx
  		mov	esp,ebp
  		pop	ebp
  		push	ecx
  		retn
;------------------------------------------------------------------------------
  		Align	8
 L000148C0:
 		db	8Bh;   '<'
 		db	FFh;   'Ô'
  		push	ebp
  		mov	ebp,esp
  		sub	esp,00000014h
  		push	ebx
  		mov	ebx,[ebp+0Ch]
  		push	esi
  		mov	esi,[ebx+08h]
  		xor	esi,[L00017000]
  		push	edi
  		mov	eax,[esi]
  		cmp	eax,FFFFFFFEh
  		mov	byte ptr [ebp-01h],00h
  		mov	dword ptr [ebp-08h],00000001h
  		lea	edi,[ebx+10h]
  		jz 	L000148F9
  		mov	ecx,[esi+04h]
  		add	ecx,edi
  		xor	ecx,[eax+edi]
  		call	SUB_L000147FB
 L000148F9:
  		mov	ecx,[esi+0Ch]
  		mov	eax,[esi+08h]
  		add	ecx,edi
  		xor	ecx,[eax+edi]
  		call	SUB_L000147FB
  		mov	eax,[ebp+08h]
  		test	byte ptr [eax+04h],66h
  		jnz	L000149F8
  		mov	ecx,[ebp+10h]
  		lea	edx,[ebp-14h]
  		mov	[ebx-04h],edx
  		mov	ebx,[ebx+0Ch]
  		cmp	ebx,FFFFFFFEh
  		mov	[ebp-14h],eax
  		mov	[ebp-10h],ecx
  		jz 	L0001498C
  		lea	ecx,[ecx+00h]
 L00014930:
  		lea	eax,[ebx+ebx*2]
  		mov	ecx,[esi+eax*4+14h]
  		test	ecx,ecx
  		lea	eax,[esi+eax*4+10h]
  		mov	[ebp-0Ch],eax
  		mov	eax,[eax]
  		mov	[ebp+08h],eax
  		jz 	L0001495B
  		mov	edx,edi
  		call	SUB_L00014B76
  		test	eax,eax
  		mov	byte ptr [ebp-01h],01h
  		jl 	L00014996
  		jg 	L0001499F
  		mov	eax,[ebp+08h]
 L0001495B:
  		cmp	eax,FFFFFFFEh
  		mov	ebx,eax
  		jnz	L00014930
  		cmp	byte ptr [ebp-01h],00h
  		jz 	L0001498C
 L00014968:
  		mov	eax,[esi]
  		cmp	eax,FFFFFFFEh
  		jz 	L0001497C
  		mov	ecx,[esi+04h]
  		add	ecx,edi
  		xor	ecx,[eax+edi]
  		call	SUB_L000147FB
 L0001497C:
  		mov	ecx,[esi+0Ch]
  		mov	edx,[esi+08h]
  		add	ecx,edi
  		xor	ecx,[edx+edi]
  		call	SUB_L000147FB
 L0001498C:
  		mov	eax,[ebp-08h]
  		pop	edi
  		pop	esi
  		pop	ebx
  		mov	esp,ebp
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
 L00014996:
  		mov	dword ptr [ebp-08h],00000000h
  		jmp	L00014968
 L0001499F:
  		mov	ecx,[ebp+0Ch]
  		call	SUB_L00014BA6
  		mov	eax,[ebp+0Ch]
  		cmp	[eax+0Ch],ebx
  		jz 	L000149C1
  		push	L00017000
  		push	edi
  		mov	edx,ebx
  		mov	ecx,eax
  		call	SUB_L00014BC0
  		mov	eax,[ebp+0Ch]
 L000149C1:
  		mov	ecx,[ebp+08h]
  		mov	[eax+0Ch],ecx
  		mov	eax,[esi]
  		cmp	eax,FFFFFFFEh
  		jz 	L000149DB
  		mov	ecx,[esi+04h]
  		add	ecx,edi
  		xor	ecx,[eax+edi]
  		call	SUB_L000147FB
 L000149DB:
  		mov	ecx,[esi+0Ch]
  		mov	edx,[esi+08h]
  		add	ecx,edi
  		xor	ecx,[edx+edi]
  		call	SUB_L000147FB
  		mov	eax,[ebp-0Ch]
  		mov	ecx,[eax+08h]
  		mov	edx,edi
  		jmp	L00014B8D
 L000149F8:
  		mov	edx,FFFFFFFEh
  		cmp	[ebx+0Ch],edx
  		jz 	L0001498C
  		push	L00017000
  		push	edi
  		mov	ecx,ebx
  		call	SUB_L00014BC0
  		jmp	L00014968
  		Align	2
 jmp_ntoskrnl.exe!KeGetCurrentThread:
  		jmp	[ntoskrnl.exe!KeGetCurrentThread]
  		Align	8
 SUB_L00014A28:
  		push	L000148C0
  		push	fs:[00000000h]
  		mov	eax,[esp+10h]
  		mov	[esp+10h],ebp
  		lea	ebp,[esp+10h]
  		sub	esp,eax
  		push	ebx
  		push	esi
  		push	edi
  		mov	eax,[L00017000]
  		xor	[ebp-04h],eax
  		xor	eax,ebp
  		mov	[ebp-1Ch],eax
  		push	eax
  		mov	[ebp-18h],esp
  		push	[ebp-08h]
  		mov	eax,[ebp-04h]
  		mov	dword ptr [ebp-04h],FFFFFFFEh
  		mov	[ebp-08h],eax
  		lea	eax,[ebp-10h]
  		mov	fs:[00000000h],eax
  		retn
;------------------------------------------------------------------------------
 SUB_L00014A70:
  		mov	ecx,[ebp-1Ch]
  		xor	ecx,ebp
  		call	SUB_L000147FB
  		jmp	SUB_L000148A5
  		Align	4
 SUB_L00014A84:
  		push	ebx
  		push	esi
  		push	edi
  		mov	edx,[esp+10h]
  		mov	eax,[esp+14h]
  		mov	ecx,[esp+18h]
  		push	ebp
  		push	edx
  		push	eax
  		push	ecx
  		push	ecx
  		push	L00014B14
  		push	fs:[00000000h]
  		mov	eax,[L00017000]
  		xor	eax,esp
  		mov	[esp+08h],eax
  		mov	fs:[00000000h],esp
 L00014AB6:
  		mov	eax,[esp+30h]
  		mov	ebx,[eax+08h]
  		mov	ecx,[esp+2Ch]
  		xor	ebx,[ecx]
  		mov	esi,[eax+0Ch]
  		cmp	esi,FFFFFFFEh
  		jz 	L00014B06
  		mov	edx,[esp+34h]
  		cmp	edx,FFFFFFFEh
  		jz 	L00014AD8
  		cmp	esi,edx
  		jbe	L00014B06
 L00014AD8:
  		lea	esi,[esi+esi*2]
  		lea	ebx,[ebx+esi*4+10h]
  		mov	ecx,[ebx]
  		mov	[eax+0Ch],ecx
  		cmp	dword ptr [ebx+04h],00000000h
  		jnz	L00014AB6
  		push	00000101h
  		mov	eax,[ebx+08h]
  		call	SUB_L00014BE5
  		mov	ecx,00000001h
  		mov	eax,[ebx+08h]
  		call	SUB_L00014C04
  		jmp	L00014AB6
 L00014B06:
  		pop	fs:[00000000h]
  		add	esp,00000018h
  		pop	edi
  		pop	esi
  		pop	ebx
  		retn
;------------------------------------------------------------------------------
 L00014B14:
  		mov	ecx,[esp+04h]
  		test	dword ptr [ecx+04h],00000006h
  		mov	eax,00000001h
  		jz 	L00014B59
  		mov	eax,[esp+08h]
  		mov	ecx,[eax+08h]
  		xor	ecx,eax
  		call	SUB_L000147FB
  		push	ebp
  		mov	ebp,[eax+18h]
  		push	[eax+0Ch]
  		push	[eax+10h]
  		push	[eax+14h]
  		call	SUB_L00014A84
  		add	esp,0000000Ch
  		pop	ebp
  		mov	eax,[esp+08h]
  		mov	edx,[esp+10h]
  		mov	[edx],eax
  		mov	eax,00000003h
 L00014B59:
  		retn
;------------------------------------------------------------------------------
  		push	ebp
  		mov	ecx,[esp+08h]
  		mov	ebp,[ecx]
  		push	[ecx+1Ch]
  		push	[ecx+18h]
  		push	[ecx+28h]
  		call	SUB_L00014A84
  		add	esp,0000000Ch
  		pop	ebp
  		retn	0004h
;------------------------------------------------------------------------------
 SUB_L00014B76:
  		push	ebp
  		push	esi
  		push	edi
  		push	ebx
  		mov	ebp,edx
  		xor	eax,eax
  		xor	ebx,ebx
  		xor	edx,edx
  		xor	esi,esi
  		xor	edi,edi
  		call	ecx
  		pop	ebx
  		pop	edi
  		pop	esi
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
 L00014B8D:
  		mov	ebp,edx
  		mov	esi,ecx
  		mov	eax,ecx
  		push	00000001h
  		call	SUB_L00014BE5
  		xor	eax,eax
  		xor	ebx,ebx
  		xor	ecx,ecx
  		xor	edx,edx
  		xor	edi,edi
  		jmp	esi
 SUB_L00014BA6:
  		push	ebp
  		mov	ebp,esp
  		push	ebx
  		push	esi
  		push	edi
  		push	00000000h
  		push	00000000h
  		push	L00014BBB
  		push	ecx
  		call	jmp_ntoskrnl.exe!RtlUnwind
 L00014BBB:
  		pop	edi
  		pop	esi
  		pop	ebx
  		pop	ebp
  		retn
;------------------------------------------------------------------------------
 SUB_L00014BC0:
  		push	ebp
  		mov	ebp,[esp+08h]
  		push	edx
  		push	ecx
  		push	[esp+14h]
  		call	SUB_L00014A84
  		add	esp,0000000Ch
  		pop	ebp
  		retn	0008h
;------------------------------------------------------------------------------
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
 		db	CCh;   'å'
  		push	ebx
  		push	ecx
  		mov	ebx,L00017008
  		jmp	L00014BF0
 SUB_L00014BE5:
  		push	ebx
  		push	ecx
  		mov	ebx,L00017008
  		mov	ecx,[esp+0Ch]
 L00014BF0:
  		mov	[ebx+08h],ecx
  		mov	[ebx+04h],eax
  		mov	[ebx+0Ch],ebp
  		push	ebp
  		push	ecx
  		push	eax
  		pop	eax
  		pop	ecx
  		pop	ebp
  		pop	ecx
  		pop	ebx
  		retn	0004h
;------------------------------------------------------------------------------
 SUB_L00014C04:
  		call	eax
  		retn
;------------------------------------------------------------------------------
  		Align	4
 jmp_ntoskrnl.exe!RtlUnwind:
  		jmp	[ntoskrnl.exe!RtlUnwind]
 L00014C12:
 		db	5Ch;   '\'
 		db	00h;
 		db	43h;   'C'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	62h;   'b'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	6Bh;   'k'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	50h;   'P'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	77h;   'w'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	00h;
 		db	00h;
 SSZ00014C3C_Log_File_Opened:
  		db	'Log File Opened',0
 SWC00014C4C_ShocksDetected:
  		unicode	'ShocksDetected',0000h
 L00014C6A:
 		db	53h;   'S'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	43h;   'C'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	43h;   'C'
 		db	00h;
 		db	68h;   'h'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	00h;
 		db	00h;
 L00014C8E:
 		db	44h;   'D'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	79h;   'y'
 		db	00h;
 		db	43h;   'C'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	49h;   'I'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	75h;   'u'
 		db	00h;
 		db	70h;   'p'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	49h;   'I'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	76h;   'v'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	00h;
 		db	00h;
 L00014CC6:
 		db	55h;   'U'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	64h;   'd'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	64h;   'd'
 		db	00h;
 		db	4Dh;   'M'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	64h;   'd'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	4Fh;   'O'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	79h;   'y'
 		db	00h;
 		db	00h;
 		db	00h;
 L00014CEE:
 		db	44h;   'D'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	62h;   'b'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	46h;   'F'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	50h;   'P'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	6Bh;   'k'
 		db	00h;
 		db	4Fh;   'O'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	4Ch;   'L'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	64h;   'd'
 		db	00h;
 		db	4Fh;   'O'
 		db	00h;
 		db	70h;   'p'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	00h;
 		db	00h;
 SWC00014D20_ErrorLogLimit:
  		unicode	'ErrorLogLimit',0000h
 SWC00014D3C_CreateErrorLogEntries:
  		unicode	'CreateErrorLogEntries',0000h
 SWC00014D68_ClearInterruptInSoftware:
  		unicode	'ClearInterruptInSoftware',0000h
 L00014D9A:
 		db	53h;   'S'
 		db	00h;
 		db	68h;   'h'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	6Bh;   'k'
 		db	00h;
 		db	45h;   'E'
 		db	00h;
 		db	76h;   'v'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	44h;   'D'
 		db	00h;
 		db	75h;   'u'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	68h;   'h'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	00h;
 		db	00h;
 L00014DCA:
 		db	53h;   'S'
 		db	00h;
 		db	68h;   'h'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	6Bh;   'k'
 		db	00h;
 		db	45h;   'E'
 		db	00h;
 		db	76h;   'v'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	44h;   'D'
 		db	00h;
 		db	75h;   'u'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	4Ch;   'L'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	00h;
 		db	00h;
 SWC00014DF8_Enabled:
  		unicode	'Enabled',0000h
 SWC00014E08__Device_Accelerometer:
  		unicode	'\Device\Accelerometer',0000h
 SSZ00014E34_Shock_Event_Timer_Already_Runnin:
  		db	'Shock Event Timer Already Running %d',0
  		Align	2
 SSZ00014E5A_Timer_Running__Shock_Event_Timer:
  		db	'Timer Running, Shock Event Timer Restarted %d',0
 SSZ00014E88_Shock_Event_Timer_Started__d:
  		db	'Shock Event Timer Started %d',0
  		Align	2
 SSZ00014EA6_Shock_Event_Signaled__d:
  		db	'Shock Event Signaled %d',0
 SSZ00014EBE_Shock_Event_Duration_Extended__d:
  		db	'Shock Event Duration Extended %d',0
  		Align	4
 SSZ00014EE0_Shock_End_Event_Signaled__d:
  		db	'Shock End Event Signaled %d',0
 SSZ00014EFC_Initial_Lid_State_is_CLOSED__Usi:
  		db	'Initial Lid State is CLOSED. Using Standard Mode',0
  		Align	2
 SSZ00014F2E_Query_Lid_State_from_BIOS_failed:
  		db	'Query Lid State from BIOS failed.  Assuming LID open.',0
 SSZ00014F64_Query_Power_State_On_DC_:
  		db	'Query Power State On DC.',0
  		Align	2
 SSZ00014F7E_Query_Power_State_On_AC_:
  		db	'Query Power State On AC.',0
  		Align	4
 SSZ00014F98_FastIOCTL_rcvd__No_IgnoreInterru:
  		db	'FastIOCTL rcvd: No IgnoreInterrupts %d',0
  		Align	4
 SSZ00014FC0_FastIOCTL_rcvd__IgnoreInterrupts:
  		db	'FastIOCTL rcvd: IgnoreInterrupts %d',0
 SWC00014FE4__Software_Hewlett_Packard_HP_Mob:
  		unicode	'\Software\Hewlett-Packard\HP Mobile Data Protection System',0000h
 L0001505A:
 		db	49h;   'I'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	49h;   'I'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	64h;   'd'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	78h;   'x'
 		db	00h;
 		db	00h;
 		db	00h;
 SSZ00015076_IOCTL_rcvd__No_IgnoreInterrupts_:
  		db	'IOCTL rcvd: No IgnoreInterrupts %d',0
  		Align	2
 SSZ0001509A_IOCTL_rcvd__IgnoreInterrupts__d:
  		db	'IOCTL rcvd: IgnoreInterrupts %d',0
 SSZ000150BA_System_Powering_Up__Not_Ignoring:
  		db	'System Powering Up, Not Ignoring Interrupts',0
 L000150E6:
 		db	5Ch;   '\'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Dh;   'm'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	2Eh;   '.'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	00h;
 		db	00h;
 SWC0001510C__Accelerometer:
  		unicode	'\Accelerometer',0000h
 L0001512A:
 		db	5Ch;   '\'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	79h;   'y'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Dh;   'm'
 		db	00h;
 		db	52h;   'R'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	79h;   'y'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Dh;   'm'
 		db	00h;
 		db	33h;   '3'
 		db	00h;
 		db	32h;   '2'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	4Ch;   'L'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	66h;   'f'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	00h;
 		db	00h;
 SSZ00015166_RegistryReadCurrentUserValue_rea:
  		db	'RegistryReadCurrentUserValue reading key %wZ for value %ls status = %x',0Ah,0
 L000151AE:
 		db	5Ch;   '\'
 		db	00h;
 		db	52h;   'R'
 		db	00h;
 		db	45h;   'E'
 		db	00h;
 		db	47h;   'G'
 		db	00h;
 		db	49h;   'I'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	54h;   'T'
 		db	00h;
 		db	52h;   'R'
 		db	00h;
 		db	59h;   'Y'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	4Dh;   'M'
 		db	00h;
 		db	41h;   'A'
 		db	00h;
 		db	43h;   'C'
 		db	00h;
 		db	48h;   'H'
 		db	00h;
 		db	49h;   'I'
 		db	00h;
 		db	4Eh;   'N'
 		db	00h;
 		db	45h;   'E'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	79h;   'y'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Dh;   'm'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	43h;   'C'
 		db	00h;
 		db	75h;   'u'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	43h;   'C'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	76h;   'v'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	41h;   'A'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	6Ch;   'l'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	6Fh;   'o'
 		db	00h;
 		db	6Dh;   'm'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	00h;
 		db	00h;
 SSZ00015244__04_4d__02_2d__02_2d__02_2d__02_:
  		db	'%04.4d.%02.2d.%02.2d-%02.2d:%02.2d:%02.2d.%03.3d,%I64d,%s.',0Ah,0
 SSZ00015280_Lid_Closed_On_AC___Short:
  		db	'Lid Closed-On AC - Short',0
  		Align	2
 SSZ0001529A_Lid_Closed_On_DC___Long:
  		db	'Lid Closed-On DC - Long',0
 SSZ000152B2_Lid_Open_On_AC___Short:
  		db	'Lid Open-On AC - Short',0
  		Align	2
 SSZ000152CA_Lid_Open_On_DC___Short:
  		db	'Lid Open-On DC - Short',0
  		Align	2
 SSZ000152E2_Unknown_State:
  		db	'Unknown State',0
 SSZ000152F0_DisableFastParkOnLidOpen___FALSE:
  		db	'DisableFastParkOnLidOpen = FALSE, using Fast Mode',0
 SSZ00015322_DisableFastParkOnLidOpen___TRUE_:
  		db	'DisableFastParkOnLidOpen = TRUE, using Standard Mode',0
  		Align	4
 SSZ00015358_UseStandardModeOnly___TRUE__usin:
  		db	'UseStandardModeOnly = TRUE, using Standard Mode',0
 SSZ00015388_Already_Ignoring_Interrupts____I:
  		db	'Already Ignoring Interrupts... II = %x, IICB = %x',0
 SSZ000153BA_IgnoreInterrupts_cannot_cancel_t:
  		db	'IgnoreInterrupts cannot cancel timer',0
  		Align	4
 SSZ000153E0_Shock_End_Event_Signaled__Hibera:
  		db	'Shock End Event Signaled, Hiberation, Suspend, or Shutdown',0
  		Align	4
 SSZ0001541C_Ignoring_Interrupts____II____x__:
  		db	'Ignoring Interrupts... II = %x, IICB = %x',0
 SSZ00015446_Critical_Battery_Ignoring_Interr:
  		db	'Critical Battery Ignoring Interrupts... II = %x, IICB = %x',0
  		Align	2
 L00015482:
 		db	5Ch;   '\'
 		db	00h;
 		db	52h;   'R'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	79h;   'y'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	55h;   'U'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	5Ch;   '\'
 		db	00h;
 		db	00h;
 		db	00h;
 SSZ000154A2_Battery_No_longer_Critical_:
  		db	'Battery No longer Critical.',0
 SSZ000154BE_Battery_Critical_:
  		db	'Battery Critical.',0
 SWC000154D0__EFSInitEvent:
  		unicode	'\EFSInitEvent',0000h
 SSZ000154EC_State_Change_Timer_Not_Running:
  		db	'State Change Timer Not Running',0
  		Align	4
 SSZ0001550C_Shock_End_Event_Signaled_AC__d_L:
  		db	'Shock End Event Signaled AC=%d,LID=%d',0
 SSZ00015532_State_Change_Timer_Running:
  		db	'State Change Timer Running',0
  		Align	2
 SSZ0001554E_Running_on_Battery:
  		db	'Running on Battery',0
  		Align	2
 SSZ00015562_Running_on_AC:
  		db	'Running on AC',0
 SSZ00015570_Leaving_S0:
  		db	'Leaving S0',0
  		Align	4
 SSZ0001557C_S0_Lid_Closed__using_Standard_Mo:
  		db	'S0 Lid Closed, using Standard Mode %p',0
 SSZ000155A2_S0_Lid_change___p__really__d:
  		db	'S0 Lid change: %p, really %d',0
  		Align	4
 SSZ000155C0_Reenter_S0___Check_Lid_State:
  		db	'Reenter S0 - Check Lid State',0
  		Align	2
 SSZ000155DE_Lid_Closed__using_Standard_Mode_:
  		db	'Lid Closed, using Standard Mode %p',0
  		Align	2
 SSZ00015602_Lid_change___p__really__d:
  		db	'Lid change: %p, really %d',0
 SSZ0001561C_ALID_method_failed__using_callba:
  		db	'ALID method failed, using callback lid state %p',0
;------------------------------------------------------------------------------
 		000001B4h DUP (??)
;
;
;------------------------------------------------------------------------------
;  Name: .rdata (Data Section)
;  Virtual Address:    00016000h  Virtual Size:    00000374h
;  Pointer To RawData: 00004C00h  Size Of RawData: 00000400h
;
 HAL.dll!KfRaiseIrql:
  		dd	??
 HAL.dll!KfLowerIrql:
  		dd	??
 HAL.dll!KeAcquireInStackQueuedSpinLock:
  		dd	??
 HAL.dll!KeReleaseInStackQueuedSpinLock:
  		dd	??
 HAL.dll!KeGetCurrentIrql:
  		dd	??
  		dd	00000000
 ntoskrnl.exe!memset:
  		dd	??
 ntoskrnl.exe!PoSetPowerState:
  		dd	??
 ntoskrnl.exe!ExRegisterCallback:
  		dd	??
 ntoskrnl.exe!ExCreateCallback:
  		dd	??
 ntoskrnl.exe!KeInitializeMutex:
  		dd	??
 ntoskrnl.exe!KeInitializeTimerEx:
  		dd	??
 ntoskrnl.exe!KeWaitForSingleObject:
  		dd	??
 ntoskrnl.exe!KeSetEvent:
  		dd	??
 ntoskrnl.exe!IoDetachDevice:
  		dd	??
 ntoskrnl.exe!PsCreateSystemThread:
  		dd	??
 ntoskrnl.exe!RtlFreeUnicodeString:
  		dd	??
 ntoskrnl.exe!IoAttachDeviceToDeviceStack:
  		dd	??
 ntoskrnl.exe!IoDeleteDevice:
  		dd	??
 ntoskrnl.exe!IoRegisterDeviceInterface:
  		dd	??
 ntoskrnl.exe!IoCreateDevice:
  		dd	??
 ntoskrnl.exe!_allmul:
  		dd	??
 ntoskrnl.exe!KeInsertQueueDpc:
  		dd	??
 ntoskrnl.exe!KeReleaseInStackQueuedSpinLockFromDpcLevel:
  		dd	??
 ntoskrnl.exe!KeCancelTimer:
  		dd	??
 ntoskrnl.exe!KeSetTimer:
  		dd	??
 ntoskrnl.exe!KeAcquireInStackQueuedSpinLockAtDpcLevel:
  		dd	??
 ntoskrnl.exe!IoDisconnectInterrupt:
  		dd	??
 ntoskrnl.exe!IoUnregisterShutdownNotification:
  		dd	??
 ntoskrnl.exe!ExUnregisterCallback:
  		dd	??
 ntoskrnl.exe!ZwClose:
  		dd	??
 ntoskrnl.exe!IofCallDriver:
  		dd	??
 ntoskrnl.exe!IoRegisterShutdownNotification:
  		dd	??
 ntoskrnl.exe!IoConnectInterrupt:
  		dd	??
 ntoskrnl.exe!KeSetImportanceDpc:
  		dd	??
 ntoskrnl.exe!KeInitializeDpc:
  		dd	??
 ntoskrnl.exe!KeBugCheckEx:
  		dd	??
 ntoskrnl.exe!ZwPowerInformation:
  		dd	??
 ntoskrnl.exe!IofCompleteRequest:
  		dd	??
 ntoskrnl.exe!IoSetDeviceInterfaceState:
  		dd	??
 ntoskrnl.exe!KeReleaseMutex:
  		dd	??
 ntoskrnl.exe!MmMapLockedPagesSpecifyCache:
  		dd	??
 ntoskrnl.exe!KeDelayExecutionThread:
  		dd	??
 ntoskrnl.exe!KeSetPriorityThread:
  		dd	??
 ntoskrnl.exe!KeGetCurrentThread:
  		dd	??
 ntoskrnl.exe!RtlCompareMemory:
  		dd	??
 ntoskrnl.exe!memcpy:
  		dd	??
 ntoskrnl.exe!SeTokenIsAdmin:
  		dd	??
 ntoskrnl.exe!SeLockSubjectContext:
  		dd	??
 ntoskrnl.exe!RtlFormatCurrentUserKeyPath:
  		dd	??
 ntoskrnl.exe!SeReleaseSubjectContext:
  		dd	??
 ntoskrnl.exe!SePrivilegeCheck:
  		dd	??
 ntoskrnl.exe!SeCaptureSubjectContext:
  		dd	??
 ntoskrnl.exe!PoRequestPowerIrp:
  		dd	??
 ntoskrnl.exe!_vsnprintf:
  		dd	??
 ntoskrnl.exe!RtlQueryRegistryValues:
  		dd	??
 ntoskrnl.exe!RtlWriteRegistryValue:
  		dd	??
 ntoskrnl.exe!IoReportTargetDeviceChangeAsynchronous:
  		dd	??
 ntoskrnl.exe!IoFreeIrp:
  		dd	??
 ntoskrnl.exe!IoAllocateIrp:
  		dd	??
 ntoskrnl.exe!IoGetRelatedDeviceObject:
  		dd	??
 ntoskrnl.exe!ObReferenceObjectByHandle:
  		dd	??
 ntoskrnl.exe!ZwSetInformationFile:
  		dd	??
 ntoskrnl.exe!ZwQueryInformationFile:
  		dd	??
 ntoskrnl.exe!ZwCreateFile:
  		dd	??
 ntoskrnl.exe!IoFreeMdl:
  		dd	??
 ntoskrnl.exe!RtlCheckRegistryKey:
  		dd	??
 ntoskrnl.exe!RtlCreateRegistryKey:
  		dd	??
 ntoskrnl.exe!ZwQueryValueKey:
  		dd	??
 ntoskrnl.exe!DbgPrintEx:
  		dd	??
 ntoskrnl.exe!ZwOpenKey:
  		dd	??
 ntoskrnl.exe!ObfDereferenceObject:
  		dd	??
 ntoskrnl.exe!ObGetObjectSecurity:
  		dd	??
 ntoskrnl.exe!ObReleaseObjectSecurity:
  		dd	??
 ntoskrnl.exe!SeAccessCheck:
  		dd	??
 ntoskrnl.exe!IoGetFileObjectGenericMapping:
  		dd	??
 ntoskrnl.exe!MmBuildMdlForNonPagedPool:
  		dd	??
 ntoskrnl.exe!IoAllocateMdl:
  		dd	??
 ntoskrnl.exe!RtlTimeToTimeFields:
  		dd	??
 ntoskrnl.exe!ExSystemTimeToLocalTime:
  		dd	??
 ntoskrnl.exe!KeQuerySystemTime:
  		dd	??
 ntoskrnl.exe!KeWaitForMultipleObjects:
  		dd	??
 ntoskrnl.exe!ExEventObjectType:
  		dd	??
 ntoskrnl.exe!ZwOpenEvent:
  		dd	??
 ntoskrnl.exe!KeTickCount:
  		dd	??
 ntoskrnl.exe!RtlUnwind:
  		dd	??
 ntoskrnl.exe!ExAllocatePoolWithTag:
  		dd	??
 ntoskrnl.exe!RtlInitUnicodeString:
  		dd	??
 ntoskrnl.exe!RtlAppendUnicodeStringToString:
  		dd	??
 ntoskrnl.exe!KeInitializeEvent:
  		dd	??
 ntoskrnl.exe!SeUnlockSubjectContext:
  		dd	??
 ntoskrnl.exe!ExFreePoolWithTag:
  		dd	??
  		dd	00000000
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
  		dd	00000000h
  		dd	459EA8E7h
  		dw	0000h
  		dw	0000h
  		dd	00000002h
  		dd	00000058h
  		dd	00006258h
  		dd	00004E58h
 L0001619C:
 		db	82h;   '''
 		db	66h;   'f'
 		db	2Ah;   '*'
 		db	DDh;   'ù'
 		db	5Eh;   '^'
 		db	73h;   's'
 		db	8Eh;   '?'
 		db	4Eh;   'N'
 		db	8Ah;   '?'
 		db	59h;   'Y'
 		db	D9h;   'ô'
 		db	DCh;   'ú'
 		db	CFh;   'è'
 		db	1Eh;
 		db	BEh;   '?'
 		db	CEh;   'é'
 L000161AC:
 		db	0Bh;
 		db	77h;   'w'
 		db	75h;   'u'
 		db	6Eh;   'n'
 		db	09h;
 		db	3Dh;   '='
 		db	72h;   'r'
 		db	4Eh;   'N'
 		db	93h;   '"'
 		db	BEh;   '?'
 		db	DFh;   'ü'
 		db	D6h;   'ñ'
 		db	25h;   '%'
 		db	A2h;   '˜'
 		db	72h;   'r'
 		db	92h;   '''
 L000161BC:
 		db	57h;   'W'
 		db	F5h;   'Â'
 		db	A1h;   'ˆ'
 		db	6Dh;   'm'
 		db	A4h;   '˝'
 		db	7Dh;   '}'
 		db	A1h;   'ˆ'
 		db	47h;   'G'
 		db	B6h;
 		db	3Bh;   ';'
 		db	ADh;   '-'
 		db	41h;   'A'
 		db	60h;   '`'
 		db	43h;   'C'
 		db	DAh;   'ö'
 		db	6Dh;   'm'
 L000161CC:
 		db	65h;   'e'
 		db	1Ch;
 		db	0Ch;
 		db	3Eh;   '>'
 		db	B0h;   '¯'
 		db	78h;   'x'
 		db	66h;   'f'
 		db	42h;   'B'
 		db	86h;   '≈'
 		db	98h;   '?'
 		db	00h;
 		db	DFh;   'ü'
 		db	D3h;   'ì'
 		db	75h;   'u'
 		db	FAh;   'Í'
 		db	59h;   'Y'
 L000161DC:
 		db	6Ah;   'j'
 		db	00h;
 		db	C1h;   'Å'
 		db	40h;   '@'
 		db	54h;   'T'
 		db	E2h;   '¢'
 		db	36h;   '6'
 		db	48h;   'H'
 		db	BFh;   'ı'
 		db	10h;
 		db	32h;   '2'
 		db	07h;
 		db	14h;
 		db	33h;   '3'
 		db	9Dh;   '?'
 		db	B3h;   'i'
 L000161EC:
 		db	5Fh;   '_'
 		db	73h;   's'
 		db	86h;   '≈'
 		db	68h;   'h'
 		db	BBh;   '>'
 		db	2Ah;   '*'
 		db	09h;
 		db	4Dh;   'M'
 		db	9Ah;   '?'
 		db	2Fh;   '/'
 		db	C9h;   'â'
 		db	2Bh;   '+'
 		db	D7h;   'ó'
 		db	A2h;   '˜'
 		db	CFh;   'è'
 		db	68h;   'h'
 L000161FC:
 		db	F5h;   'Â'
 		db	03h;
 		db	C3h;   'É'
 		db	98h;   '?'
 		db	ACh;   'ø'
 		db	EEh;   'Æ'
 		db	B8h;   'Ò'
 		db	46h;   'F'
 		db	A3h;   '?'
 		db	D6h;   'ñ'
 		db	EAh;   '™'
 		db	2Dh;   '-'
 		db	48h;   'H'
 		db	73h;   's'
 		db	98h;   '?'
 		db	9Fh;   '?'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	48h;   'H'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		dd	L00017000
 		dd	L000162B0
 		db	02h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	52h;   'R'
 		db	53h;   'S'
 		db	44h;   'D'
 		db	53h;   'S'
 		db	54h;   'T'
 		db	3Bh;   ';'
 		db	50h;   'P'
 		db	B8h;   'Ò'
 		db	75h;   'u'
 		db	8Bh;   '<'
 		db	49h;   'I'
 		db	47h;   'G'
 		db	B5h;   'Á'
 		db	87h;   'ÿ'
 		db	E3h;   '£'
 		db	68h;   'h'
 		db	15h;
 		db	7Eh;   '~'
 		db	3Bh;   ';'
 		db	ABh;   '<'
 		db	01h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	67h;   'g'
 		db	3Ah;   ':'
 		db	5Ch;   '\'
 		db	70h;   'p'
 		db	72h;   'r'
 		db	6Fh;   'o'
 		db	6Ah;   'j'
 		db	65h;   'e'
 		db	63h;   'c'
 		db	74h;   't'
 		db	73h;   's'
 		db	5Ch;   '\'
 		db	68h;   'h'
 		db	70h;   'p'
 		db	72h;   'r'
 		db	6Fh;   'o'
 		db	62h;   'b'
 		db	6Fh;   'o'
 		db	74h;   't'
 		db	73h;   's'
 		db	5Ch;   '\'
 		db	73h;   's'
 		db	72h;   'r'
 		db	63h;   'c'
 		db	5Ch;   '\'
 		db	6Ch;   'l'
 		db	69h;   'i'
 		db	62h;   'b'
 		db	5Ch;   '\'
 		db	66h;   'f'
 		db	72h;   'r'
 		db	65h;   'e'
 		db	5Fh;   '_'
 		db	77h;   'w'
 		db	6Ch;   'l'
 		db	68h;   'h'
 		db	5Fh;   '_'
 		db	78h;   'x'
 		db	38h;   '8'
 		db	36h;   '6'
 		db	5Ch;   '\'
 		db	69h;   'i'
 		db	33h;   '3'
 		db	38h;   '8'
 		db	36h;   '6'
 		db	5Ch;   '\'
 		db	61h;   'a'
 		db	63h;   'c'
 		db	63h;   'c'
 		db	65h;   'e'
 		db	6Ch;   'l'
 		db	65h;   'e'
 		db	72h;   'r'
 		db	6Fh;   'o'
 		db	6Dh;   'm'
 		db	65h;   'e'
 		db	74h;   't'
 		db	65h;   'e'
 		db	72h;   'r'
 		db	2Eh;   '.'
 		db	70h;   'p'
 		db	64h;   'd'
 		db	62h;   'b'
 		db	00h;
 L000162B0:
 		db	C0h;   'Ä'
 		db	48h;   'H'
 		db	00h;
 		db	00h;
 		db	14h;
 		db	4Bh;   'K'
 		db	00h;
 		db	00h;
 L000162B8:
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	D0h;   'ê'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		dd	L00011DAD
 		dd	L00011DBB
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000162D8:
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	9Ch;   '?'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		dd	L00012A5E
 		dd	L00012A6C
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000162F8:
 		db	E4h;   '§'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	68h;   'h'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		dd	L00013B35
 		dd	L00013B39
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00016318:
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	5Ch;   '\'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		dd	L00014080
 		dd	L0001408E
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00016338:
 		db	E4h;   '§'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	6Ch;   'l'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		dd	L0001424D
 		dd	L0001425E
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00016358:
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	B8h;   'Ò'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	FEh;   'Ó'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		db	FFh;   'Ô'
 		dd	L00014753
 		dd	L00014761
;------------------------------------------------------------------------------
 		0000008Ch DUP (??)
;
;
;------------------------------------------------------------------------------
;  Name: .data
;  Virtual Address:    00017000h  Virtual Size:    000000E9h
;  Pointer To RawData: 00005000h  Size Of RawData: 00000200h
;
 L00017000:
  		dd	BB40E64Eh
 L00017004:
 		db	B1h;   '+'
 		db	19h;
 		db	BFh;   'ı'
 		db	44h;   'D'
 L00017008:
 		db	20h;   ' '
 		db	05h;
 		db	93h;   '"'
 		db	19h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00017018:
  		dd	00000000h
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00017040:
  		dd	00000000h
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00017088:
 		db	00h;
 		db	00h;
 L0001708A:
 		db	00h;
 		db	00h;
 L0001708C:
  		dd	00000000h
 L00017090:
 		db	00h;
 		db	00h;
 L00017092:
 		db	00h;
 		db	00h;
 L00017094:
  		dd	00000000h
 L00017098:
 		db	00h;
 		db	00h;
 L0001709A:
 		db	00h;
 		db	00h;
 L0001709C:
  		dd	00000000h
 L000170A0:
 		db	00h;
 		db	00h;
 L000170A2:
 		db	00h;
 		db	00h;
 L000170A4:
  		dd	00000000h
 L000170A8:
  		dd	00000000h
 L000170AC:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000170B0:
  		dd	00000000h
 L000170B4:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000170B8:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000170BC:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000170CC:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000170DC:
  		dd	00000000h
 L000170E0:
  		dd	00000000h
 L000170E4:
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L000170E8:
 		db	00h;
;------------------------------------------------------------------------------
 		00000117h DUP (??)
;
;
;------------------------------------------------------------------------------
;  Name: INIT
;  Virtual Address:    00018000h  Virtual Size:    00000D20h
;  Pointer To RawData: 00005200h  Size Of RawData: 00000E00h
;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 		db	00h;
 L00018006:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		push	ecx
  		push	ecx
  		push	ebx
  		mov	ebx,[ebp+08h]
  		push	esi
  		mov	esi,[ebp+0Ch]
  		push	edi
  		mov	eax,L00011C56
  		mov	[ebx+38h],eax
  		mov	[ebx+40h],eax
  		mov	eax,[ebx+18h]
  		mov	dword ptr [ebx+70h],L00011FD4
  		mov	dword ptr [ebx+74h],L00011C7A
  		mov	dword ptr [ebx+000000A4h],L00011ADC
  		mov	dword ptr [ebx+00000090h],L000127CC
  		mov	dword ptr [ebx+00000094h],L000147D4
  		mov	dword ptr [ebx+78h],L00014672
  		mov	dword ptr [eax+04h],L00011056
  		mov	dword ptr [ebx+34h],L00011006
  		xor	eax,eax
  		mov	edi,L00017088
  		stosd
  		stosd
  		movzx	eax,[esi+02h]
  		mov	edi,[ntoskrnl.exe!ExAllocatePoolWithTag]
  		push	5052534Fh
  		push	eax
  		push	00000001h
  		call	edi
  		test	eax,eax
  		mov	[L0001708C],eax
  		jnz	L00018095
 L0001808B:
  		mov	eax,C000009Ah
  		jmp	L000182EC
 L00018095:
  		movzx	ecx,[esi+02h]
  		push	ecx
  		push	00000000h
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		push	eax
  		push	[esi+04h]
  		push	[L0001708C]
  		call	jmp_ntoskrnl.exe!memcpy
  		mov	ax,[esi]
  		mov	[L00017088],ax
  		mov	ax,[esi+02h]
  		mov	[L0001708A],ax
  		movzx	eax,[esi+02h]
  		add	esp,00000018h
  		push	5052534Fh
  		add	eax,00000018h
  		push	eax
  		push	00000001h
  		call	edi
  		test	eax,eax
  		mov	[L0001709C],eax
  		jnz	L000180F3
  		push	eax
  		push	[L0001708C]
  		call	[ntoskrnl.exe!ExFreePoolWithTag]
  		jmp	L0001808B
 L000180F3:
  		movzx	ecx,[esi+02h]
  		add	ecx,00000018h
  		push	ecx
  		push	00000000h
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		push	eax
  		push	[esi+04h]
  		push	[L0001709C]
  		call	jmp_ntoskrnl.exe!memcpy
  		mov	ax,[esi]
  		mov	[L00017098],ax
  		mov	ax,[esi+02h]
  		add	ax,0018h
  		add	esp,00000018h
  		mov	[L0001709A],ax
  		push	L00018366
  		lea	eax,[ebp-08h]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		lea	eax,[ebp-08h]
  		push	eax
  		push	L00017098
  		call	[ntoskrnl.exe!RtlAppendUnicodeStringToString]
  		movzx	eax,[esi+02h]
  		push	5052534Fh
  		add	eax,00000014h
  		push	eax
  		push	00000001h
  		call	edi
  		test	eax,eax
  		mov	[L000170A4],eax
  		jnz	L00018185
  		mov	esi,[ntoskrnl.exe!ExFreePoolWithTag]
  		push	eax
  		push	[L0001708C]
  		call	esi
  		push	00000000h
  		push	[L0001709C]
 L0001817E:
  		call	esi
  		jmp	L0001808B
 L00018185:
  		movzx	ecx,[esi+02h]
  		add	ecx,00000014h
  		push	ecx
  		push	00000000h
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		push	eax
  		push	[esi+04h]
  		push	[L000170A4]
  		call	jmp_ntoskrnl.exe!memcpy
  		mov	ax,[esi]
  		mov	[L000170A0],ax
  		mov	ax,[esi+02h]
  		add	ax,0014h
  		add	esp,00000018h
  		mov	[L000170A2],ax
  		push	L00018352
  		lea	eax,[ebp-08h]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		lea	eax,[ebp-08h]
  		push	eax
  		push	L000170A0
  		call	[ntoskrnl.exe!RtlAppendUnicodeStringToString]
  		movzx	eax,[esi+02h]
  		push	5052534Fh
  		add	eax,00000018h
  		push	eax
  		push	00000001h
  		call	edi
  		xor	edi,edi
  		cmp	eax,edi
  		mov	[L00017094],eax
  		jnz	L0001821F
  		mov	esi,[ntoskrnl.exe!ExFreePoolWithTag]
  		push	edi
  		push	[L0001708C]
  		call	esi
  		push	edi
  		push	[L0001709C]
  		call	esi
  		push	edi
  		push	[L000170A4]
  		jmp	L0001817E
 L0001821F:
  		movzx	ecx,[esi+02h]
  		add	ecx,00000018h
  		push	ecx
  		push	edi
  		push	eax
  		call	jmp_ntoskrnl.exe!memset
  		movzx	eax,[esi]
  		push	eax
  		push	[esi+04h]
  		push	[L00017094]
  		call	jmp_ntoskrnl.exe!memcpy
  		mov	ax,[esi]
  		mov	[L00017090],ax
  		mov	ax,[esi+02h]
  		add	ax,0018h
  		add	esp,00000018h
  		mov	[L00017092],ax
  		push	L0001833A
  		lea	eax,[ebp-08h]
  		push	eax
  		call	[ntoskrnl.exe!RtlInitUnicodeString]
  		lea	eax,[ebp-08h]
  		push	eax
  		push	L00017090
  		call	[ntoskrnl.exe!RtlAppendUnicodeStringToString]
  		mov	esi,[ntoskrnl.exe!KeInitializeEvent]
  		push	edi
  		push	00000001h
  		push	L000170BC
  		call	esi
  		push	edi
  		push	00000001h
  		push	L000170CC
  		call	esi
  		mov	eax,L000170DC
  		push	00000070h
  		mov	[L000170E0],eax
  		mov	[L000170DC],eax
  		mov	eax,L000170AC
  		push	edi
  		mov	esi,L00017018
  		push	esi
  		mov	[L000170E4],edi
  		mov	[L000170B0],eax
  		mov	[L000170AC],eax
  		mov	[L000170B4],edi
  		call	jmp_ntoskrnl.exe!memset
  		add	esp,0000000Ch
  		mov	dword ptr [L00017018],00000070h
  		mov	dword ptr [L00017040],L00011D34
  		push	ebx
  		mov	[ebx+28h],esi
  		call	SUB_L0001104E
  		xor	eax,eax
 L000182EC:
  		pop	edi
  		pop	esi
  		pop	ebx
  		leave
  		retn	0008h
;------------------------------------------------------------------------------
  		Align	8
;------------------------------------------------------------------------------
 EntryPoint:
  		mov	edi,edi
  		push	ebp
  		mov	ebp,esp
  		mov	eax,[L00017000]
  		test	eax,eax
  		mov	ecx,BB40E64Eh
  		jz 	L0001830F
  		cmp	eax,ecx
  		jnz	L0001832D
 L0001830F:
  		mov	edx,[ntoskrnl.exe!KeTickCount]
  		mov	eax,L00017000
  		shr	eax,08h
  		xor	eax,[edx]
  		mov	[L00017000],eax
  		jnz	L0001832D
  		mov	eax,ecx
  		mov	[L00017000],eax
 L0001832D:
  		not	eax
  		mov	[L00017004],eax
  		pop	ebp
  		jmp	L00018006
 L0001833A:
 		db	5Ch;   '\'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	63h;   'c'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	00h;
 		db	00h;
 L00018352:
 		db	5Ch;   '\'
 		db	00h;
 		db	53h;   'S'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	69h;   'i'
 		db	00h;
 		db	6Eh;   'n'
 		db	00h;
 		db	67h;   'g'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	00h;
 		db	00h;
 L00018366:
 		db	5Ch;   '\'
 		db	00h;
 		db	50h;   'P'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	61h;   'a'
 		db	00h;
 		db	6Dh;   'm'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	74h;   't'
 		db	00h;
 		db	65h;   'e'
 		db	00h;
 		db	72h;   'r'
 		db	00h;
 		db	73h;   's'
 		db	00h;
 		db	00h;
 		db	00h;
 		db	CCh;   'å'
 		db	CCh;   'å'
  		dd	000083D4h
  		dd	00000000h
  		dd	00000000h
  		dd	00008C8Ah
  		dd	00006018h
  		dd	000083BCh
  		dd	00000000h
  		dd	00000000h
  		dd	00008D18h
  		dd	00006000h
  		dd	00000000h
  		dd	00000000h
  		dd	00000000h
  		dd	00000000h
  		dd	00000000h
  		dd	00008CF6h
  		dd	00008CE8h
  		dd	00008CC6h
  		dd	00008CA4h
  		dd	00008D04h
  		dd	00000000h
  		dd	000085B4h
  		dd	000085BEh
  		dd	000085D0h
  		dd	000085E6h
  		dd	000085FAh
  		dd	0000860Eh
  		dd	00008624h
  		dd	0000863Ch
  		dd	0000864Ah
  		dd	0000865Ch
  		dd	00008674h
  		dd	0000868Ch
  		dd	000086AAh
  		dd	000086BCh
  		dd	000086D8h
  		dd	000086EAh
  		dd	000086F4h
  		dd	00008708h
  		dd	00008736h
  		dd	00008746h
  		dd	00008754h
  		dd	00008780h
  		dd	00008798h
  		dd	000087BCh
  		dd	000087D4h
  		dd	000087DEh
  		dd	000087EEh
  		dd	00008810h
  		dd	00008826h
  		dd	0000883Ch
  		dd	0000884Eh
  		dd	0000885Eh
  		dd	00008874h
  		dd	0000888Ah
  		dd	000088A6h
  		dd	000088B8h
  		dd	000088D8h
  		dd	000088F2h
  		dd	00008908h
  		dd	0000891Eh
  		dd	000085AAh
  		dd	0000894Ch
  		dd	0000895Eh
  		dd	00008976h
  		dd	00008994h
  		dd	000089AEh
  		dd	000089C2h
  		dd	000089DCh
  		dd	000089F0h
  		dd	000089FEh
  		dd	00008A18h
  		dd	00008A30h
  		dd	00008A5Ah
  		dd	00008A66h
  		dd	00008A76h
  		dd	00008A92h
  		dd	00008AAEh
  		dd	00008AC6h
  		dd	00008AE0h
  		dd	00008AF0h
  		dd	00008AFCh
  		dd	00008B12h
  		dd	00008B2Ah
  		dd	00008B3Ch
  		dd	00008B4Ah
  		dd	00008B56h
  		dd	00008B6Eh
  		dd	00008B84h
  		dd	00008B9Eh
  		dd	00008BAEh
  		dd	00008BCEh
  		dd	00008BEAh
  		dd	00008BFAh
  		dd	00008C10h
  		dd	00008C2Ah
  		dd	00008C3Eh
  		dd	00008C5Ah
  		dd	00008C6Eh
  		dd	00008C7Ch
  		dd	00008C98h
  		dd	00008592h
  		dd	0000857Ah
  		dd	00008558h
  		dd	00008544h
  		dd	00008932h
  		dd	00008530h
  		dd	00000000h
  		dw	007Dh
  		db	'ExFreePoolWithTag',0
  		dw	02C4h
  		db	'KeInitializeEvent',0
  		dw	04C5h
  		db	'RtlAppendUnicodeStringToString',0
  		db	00h
  		dw	0546h
  		db	'RtlInitUnicodeString',0
  		db	00h
  		dw	0067h
  		db	'ExAllocatePoolWithTag',0
  		dw	076Eh
  		db	'memcpy',0
  		db	00h
  		dw	0770h
  		db	'memset',0
  		db	00h
  		dw	0441h
  		db	'PoSetPowerState',0
  		dw	00ABh
  		db	'ExRegisterCallback',0
  		db	00h
  		dw	006Ah
  		db	'ExCreateCallback',0
  		db	00h
  		dw	02C8h
  		db	'KeInitializeMutex',0
  		dw	02CEh
  		db	'KeInitializeTimerEx',0
  		dw	0330h
  		db	'KeWaitForSingleObject',0
  		dw	0313h
  		db	'KeSetEvent',0
  		db	00h
  		dw	01D8h
  		db	'IoDetachDevice',0
  		db	00h
  		dw	0452h
  		db	'PsCreateSystemThread',0
  		db	00h
  		dw	0527h
  		db	'RtlFreeUnicodeString',0
  		db	00h
  		dw	01A9h
  		db	'IoAttachDeviceToDeviceStack',0
  		dw	01D5h
  		db	'IoDeleteDevice',0
  		db	00h
  		dw	022Ch
  		db	'IoRegisterDeviceInterface',0
  		dw	01C1h
  		db	'IoCreateDevice',0
  		db	00h
  		dw	073Bh
  		db	'_allmul',0
  		dw	02D4h
  		db	'KeInsertQueueDpc',0
  		db	00h
  		dw	02F9h
  		db	'KeReleaseInStackQueuedSpinLockFromDpcLevel',0
  		db	00h
  		dw	02A0h
  		db	'KeCancelTimer',0
  		dw	031Eh
  		db	'KeSetTimer',0
  		db	00h
  		dw	0293h
  		db	'KeAcquireInStackQueuedSpinLockAtDpcLevel',0
  		db	00h
  		dw	01DCh
  		db	'IoDisconnectInterrupt',0
  		dw	0266h
  		db	'IoUnregisterShutdownNotification',0
  		db	00h
  		dw	00C0h
  		db	'ExUnregisterCallback',0
  		db	00h
  		dw	0699h
  		db	'ZwClose',0
  		dw	0281h
  		db	'IofCallDriver',0
  		dw	0232h
  		db	'IoRegisterShutdownNotification',0
  		db	00h
  		dw	01BDh
  		db	'IoConnectInterrupt',0
  		db	00h
  		dw	0316h
  		db	'KeSetImportanceDpc',0
  		db	00h
  		dw	02C3h
  		db	'KeInitializeDpc',0
  		dw	029Fh
  		db	'KeBugCheckEx',0
  		db	00h
  		dw	06E1h
  		db	'ZwPowerInformation',0
  		db	00h
  		dw	0282h
  		db	'IofCompleteRequest',0
  		db	00h
  		dw	0243h
  		db	'IoSetDeviceInterfaceState',0
  		dw	02FCh
  		db	'KeReleaseMutex',0
  		db	00h
  		dw	0383h
  		db	'MmMapLockedPagesSpecifyCache',0
  		db	00h
  		dw	02A3h
  		db	'KeDelayExecutionThread',0
  		db	00h
  		dw	0318h
  		db	'KeSetPriorityThread',0
  		dw	02B3h
  		db	'KeGetCurrentThread',0
  		db	00h
  		dw	04D6h
  		db	'RtlCompareMemory',0
  		db	00h
  		dw	063Ch
  		db	'SeUnlockSubjectContext',0
  		db	00h
  		dw	0637h
  		db	'SeTokenIsAdmin',0
  		db	00h
  		dw	061Ch
  		db	'SeLockSubjectContext',0
  		db	00h
  		dw	0521h
  		db	'RtlFormatCurrentUserKeyPath',0
  		dw	062Ch
  		db	'SeReleaseSubjectContext',0
  		dw	0623h
  		db	'SePrivilegeCheck',0
  		db	00h
  		dw	0607h
  		db	'SeCaptureSubjectContext',0
  		dw	043Dh
  		db	'PoRequestPowerIrp',0
  		dw	0759h
  		db	'_vsnprintf',0
  		db	00h
  		dw	05A0h
  		db	'RtlQueryRegistryValues',0
  		db	00h
  		dw	05F2h
  		db	'RtlWriteRegistryValue',0
  		dw	023Dh
  		db	'IoReportTargetDeviceChangeAsynchronous',0
  		db	00h
  		dw	01E8h
  		db	'IoFreeIrp',0
  		dw	019Fh
  		db	'IoAllocateIrp',0
  		dw	0204h
  		db	'IoGetRelatedDeviceObject',0
  		db	00h
  		dw	0423h
  		db	'ObReferenceObjectByHandle',0
  		dw	071Eh
  		db	'ZwSetInformationFile',0
  		db	00h
  		dw	06F1h
  		db	'ZwQueryInformationFile',0
  		db	00h
  		dw	06A1h
  		db	'ZwCreateFile',0
  		db	00h
  		dw	01E9h
  		db	'IoFreeMdl',0
  		dw	04CFh
  		db	'RtlCheckRegistryKey',0
  		dw	04EAh
  		db	'RtlCreateRegistryKey',0
  		db	00h
  		dw	0701h
  		db	'ZwQueryValueKey',0
  		dw	003Dh
  		db	'DbgPrintEx',0
  		db	00h
  		dw	06D3h
  		db	'ZwOpenKey',0
  		dw	042Bh
  		db	'ObfDereferenceObject',0
  		db	00h
  		dw	0419h
  		db	'ObGetObjectSecurity',0
  		dw	0427h
  		db	'ObReleaseObjectSecurity',0
  		dw	05F9h
  		db	'SeAccessCheck',0
  		dw	01FEh
  		db	'IoGetFileObjectGenericMapping',0
  		dw	0360h
  		db	'MmBuildMdlForNonPagedPool',0
  		dw	01A0h
  		db	'IoAllocateMdl',0
  		dw	05C9h
  		db	'RtlTimeToTimeFields',0
  		dw	00BFh
  		db	'ExSystemTimeToLocalTime',0
  		dw	02E9h
  		db	'KeQuerySystemTime',0
  		dw	032Eh
  		db	'KeWaitForMultipleObjects',0
  		db	00h
  		dw	0076h
  		db	'ExEventObjectType',0
  		dw	06D0h
  		db	'ZwOpenEvent',0
  		dw	0327h
  		db	'KeTickCount',0
  		db	'ntoskrnl.exe',0
  		db	00h
  		dw	05DFh
  		db	'RtlUnwind',0
  		dw	0052h
  		db	'KeReleaseInStackQueuedSpinLock',0
  		db	00h
  		dw	0045h
  		db	'KeAcquireInStackQueuedSpinLock',0
  		db	00h
  		dw	0059h
  		db	'KfLowerIrql',0
  		dw	005Ah
  		db	'KfRaiseIrql',0
  		dw	004Ch
  		db	'KeGetCurrentIrql',0
  		db	00h
  		db	'HAL.dll',0
;------------------------------------------------------------------------------
 		000000E0h DUP (??)
;
;
;------------------------------------------------------------------------------
; Imports from ntoskrnl.exe
;
 	extrn memset
 	extrn PoSetPowerState
 	extrn ExRegisterCallback
 	extrn ExCreateCallback
 	extrn KeInitializeMutex
 	extrn KeInitializeTimerEx
 	extrn KeWaitForSingleObject
 	extrn KeSetEvent
 	extrn IoDetachDevice
 	extrn PsCreateSystemThread
 	extrn RtlFreeUnicodeString
 	extrn IoAttachDeviceToDeviceStack
 	extrn IoDeleteDevice
 	extrn IoRegisterDeviceInterface
 	extrn IoCreateDevice
 	extrn _allmul
 	extrn KeInsertQueueDpc
 	extrn KeReleaseInStackQueuedSpinLockFromDpcLevel
 	extrn KeCancelTimer
 	extrn KeSetTimer
 	extrn KeAcquireInStackQueuedSpinLockAtDpcLevel
 	extrn IoDisconnectInterrupt
 	extrn IoUnregisterShutdownNotification
 	extrn ExUnregisterCallback
 	extrn ZwClose
 	extrn IofCallDriver
 	extrn IoRegisterShutdownNotification
 	extrn IoConnectInterrupt
 	extrn KeSetImportanceDpc
 	extrn KeInitializeDpc
 	extrn KeBugCheckEx
 	extrn ZwPowerInformation
 	extrn IofCompleteRequest
 	extrn IoSetDeviceInterfaceState
 	extrn KeReleaseMutex
 	extrn MmMapLockedPagesSpecifyCache
 	extrn KeDelayExecutionThread
 	extrn KeSetPriorityThread
 	extrn KeGetCurrentThread
 	extrn RtlCompareMemory
 	extrn memcpy
 	extrn SeTokenIsAdmin
 	extrn SeLockSubjectContext
 	extrn RtlFormatCurrentUserKeyPath
 	extrn SeReleaseSubjectContext
 	extrn SePrivilegeCheck
 	extrn SeCaptureSubjectContext
 	extrn PoRequestPowerIrp
 	extrn _vsnprintf
 	extrn RtlQueryRegistryValues
 	extrn RtlWriteRegistryValue
 	extrn IoReportTargetDeviceChangeAsynchronous
 	extrn IoFreeIrp
 	extrn IoAllocateIrp
 	extrn IoGetRelatedDeviceObject
 	extrn ObReferenceObjectByHandle
 	extrn ZwSetInformationFile
 	extrn ZwQueryInformationFile
 	extrn ZwCreateFile
 	extrn IoFreeMdl
 	extrn RtlCheckRegistryKey
 	extrn RtlCreateRegistryKey
 	extrn ZwQueryValueKey
 	extrn DbgPrintEx
 	extrn ZwOpenKey
 	extrn ObfDereferenceObject
 	extrn ObGetObjectSecurity
 	extrn ObReleaseObjectSecurity
 	extrn SeAccessCheck
 	extrn IoGetFileObjectGenericMapping
 	extrn MmBuildMdlForNonPagedPool
 	extrn IoAllocateMdl
 	extrn RtlTimeToTimeFields
 	extrn ExSystemTimeToLocalTime
 	extrn KeQuerySystemTime
 	extrn KeWaitForMultipleObjects
 	extrn ExEventObjectType
 	extrn ZwOpenEvent
 	extrn KeTickCount
 	extrn RtlUnwind
 	extrn ExAllocatePoolWithTag
 	extrn RtlInitUnicodeString
 	extrn RtlAppendUnicodeStringToString
 	extrn KeInitializeEvent
 	extrn SeUnlockSubjectContext
 	extrn ExFreePoolWithTag
;
; Imports from HAL.dll
;
 	extrn KfRaiseIrql
 	extrn KfLowerIrql
 	extrn KeAcquireInStackQueuedSpinLock
 	extrn KeReleaseInStackQueuedSpinLock
 	extrn KeGetCurrentIrql
;
;------------------------------------------------------------------------------

*/


/*

HP accel info:

system32/drivers/accelerometer.sys
hpdskflt.sys
hpservice.exe
HP -- build by WinDDK

HP Mobile Data Protection Sensor
IRQ 23

%SystemRoot%\System32\drivers\hpdskflt.sys

acpi/hpq0004/3&21436425&0

ACPI\HPQ0004

{4d36e97d-e325-11ce-bfc1-08002be10318}\0048
{4d36e97d-e325-11ce-bfc1-08002be10318}\0048

service:  Accelerometer

Enumerator:  ACPI

HP Mobile Data Protection Sensor

DevNode Status:
0180000A
DN_DRIVER_LOADED
DN_STARTED
DN_NT_ENUMERATOR
DN_NT_DRIVER



;
; ACCELEROMETER.INF file
;
; Installs the HP ACCELEROMETER driver
;
; Copyright 2005-2006 Hewlett-Packard Development Company, L.P.
;

[Version]
signature="$WINDOWS NT$"
Class=System
ClassGuid = {4d36e97d-e325-11ce-bfc1-08002be10318}
Provider=%Mfg%
CatalogFile.ntx86=hpqaccel.cat
DriverVer=09/26/2005,01.00.00.04

[SourceDisksNames]
1=%DiskId%

[SourceDisksFiles]
accelerometer.sys = 1
hpdskflt.sys=1


[DestinationDirs]
SYS.CopyList=10,system32\drivers

[Manufacturer]
%Mfg%=HP

[HP]
%DeviceDesc% = HPAccelerometerDriverInstall, ACPI\HPQ0004

[HPAccelerometerDriverInstall]
CopyFiles=SYS.CopyList

[SYS.CopyList]
Accelerometer.sys
hpdskflt.sys

[HPAccelerometerDriverInstall.Services]
AddService = Accelerometer,%SPSVCINST_ASSOCSERVICE%,HPAccelerometerDriver_Service_Inst
AddService = hpdskflt, , hpdskflt.Service.Install, hpdskflt.ErrorLog.Install

[HPAccelerometerDriver_Service_Inst]
ServiceType   = %SERVICE_KERNEL_DRIVER%
StartType     = %SERVICE_DEMAND_START%
ErrorControl  = %SERVICE_ERROR_NORMAL%
LoadOrderGroup = "Base"
ServiceBinary = %12%\Accelerometer.sys 
AddReg = HPAccelerometerDriver.NT.Services.AddReg

[hpdskflt.Service.Install]
DisplayName    = %service_desc%
ServiceType    = 1
StartType      = 0
ErrorControl   = 1
ServiceBinary  = %12%\hpdskflt.sys
LoadOrderGroup = "PnP Filter"

[hpdskflt.ErrorLog.Install]
AddReg = hpdskflt

[hpdskflt]
HKR,,EventMessageFile,%REG_EXPAND_SZ%,"%%SystemRoot%%\System32\drivers\hpdskflt.sys"
HKR,,TypesSupported,%REG_DWORD%,7

[HPAccelerometerDriver.NT.Services.AddReg]
HKR, Parameters, ShockEventDurationLong, 0x10001, 20000
HKR, Parameters, ShockEventDurationShort, 0x10001, 2000
HKR, Parameters, ClearInterruptInSoftware, 0x10001, 1
HKR, Parameters, CreateErrorLogEntries, 0x10001, 0
HKR, Parameters, ErrorLogLimit, 0x10001, 10
HKR, Parameters, Enabled, 0x10001, 1
HKR, Parameters, DisableFastParkOnLidOpen, 0x10001, 1
HKR, Parameters, UseStandardModeOnly, 0x10001, 1
HKR, Statistics, ShocksDetected, 0x10001, 0
HKLM,Software\Microsoft\Windows\CurrentVersion\Control Panel\cpls,Accelerometer,0x00000,accelerometercp.cpl
HKLM,Software\Microsoft\Windows\CurrentVersion\Control Panel\Extended Properties\{305CA226-D286-468e-B848-2B2E8E697B74} 2,%%SystemRoot%%\System32\accelerometercp.cpl,0x10001,0x00000002
HKLM, System\CurrentControlSet\Control\Class\{4d36e967-e325-11ce-bfc1-08002be10318}, LowerFilters, 0x00010008, hpdskflt



[Strings]
Mfg = "HP Hewlett-Packard Corporation"
DeviceDesc = "HP Mobile Data Protection Sensor"
service_desc = "HP Disk Filter Driver"
DiskId = "Install disk (1)"
REG_EXPAND_SZ          = 0x00020000
REG_DWORD              = 0x00010001

;
; The "standard" defines
;
SPSVCINST_TAGTOFRONT               = 0x00000001
SPSVCINST_ASSOCSERVICE             = 0x00000002
SPSVCINST_DELETEEVENTLOGENTRY      = 0x00000004
SPSVCINST_NOCLOBBER_DISPLAYNAME    = 0x00000008
SPSVCINST_NOCLOBBER_STARTTYPE      = 0x00000010
SPSVCINST_NOCLOBBER_ERRORCONTROL   = 0x00000020
SPSVCINST_NOCLOBBER_LOADORDERGROUP = 0x00000040
SPSVCINST_NOCLOBBER_DEPENDENCIES   = 0x00000080
SPSVCINST_NOCLOBBER_DESCRIPTION    = 0x00000100

COPYFLG_WARN_IF_SKIP         = 0x00000001
COPYFLG_NOSKIP               = 0x00000002
COPYFLG_NOVERSIONCHECK       = 0x00000004
COPYFLG_FORCE_FILE_IN_USE    = 0x00000008
COPYFLG_NO_OVERWRITE         = 0x00000010
COPYFLG_NO_VERSION_DIALOG    = 0x00000020
COPYFLG_OVERWRITE_OLDER_ONLY = 0x00000040
COPYFLG_REPLACEONLY          = 0x00000400
COPYFLG_REPLACE_BOOT_FILE    = 0x00001000
COPYFLG_NOPRUNE              = 0x00002000

SERVICE_KERNEL_DRIVER      = 0x00000001
SERVICE_FILE_SYSTEM_DRIVER = 0x00000002

SERVICE_BOOT_START   = 0x00000000
SERVICE_SYSTEM_START = 0x00000001
SERVICE_AUTO_START   = 0x00000002
SERVICE_DEMAND_START = 0x00000003
SERVICE_DISABLED     = 0x00000004

SERVICE_ERROR_IGNORE   = 0x00000000
SERVICE_ERROR_NORMAL   = 0x00000001
SERVICE_ERROR_SEVERE   = 0x00000002
SERVICE_ERROR_CRITICAL = 0x00000003



*/



#endif // disabled