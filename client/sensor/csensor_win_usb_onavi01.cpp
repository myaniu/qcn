/*
 *  csensor_win_usb_onavi01.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University.  All rights reserved.
 *
 * Implementation file for Windows USB JoyWarrior sensor class
 */

#include "main.h"
#include "csensor_win_usb_onavi01.h"

// enum serial port
// The next 3 includes are needed for serial port enumeration
#include <objbase.h>
#include <initguid.h>
#include <Setupapi.h>

// The following define is from ntddser.h in the DDK. It is also
// needed for serial port enumeration.
#ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, \
			0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif

//{4D36E978-E325-11CE-BFC1-08 00 2B E1 03 18}
DEFINE_GUID(GUID_CLASS_ONAVI_1, 0x4d36e978L, 0xE325, 0x11CE, 0xbf, 0xc1, \
			0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18);

#define STR_ONAVI_1 "XR21V1410"

//---------------------------------------------------------------
// Helpers for enumerating the available serial ports.
// These throw a std::string on failure, describing the nature of
// the error that occurred.

void EnumPortsWdm(std::vector<SSerInfo> &asi);
//void EnumPortsWNt4(std::vector<SSerInfo> &asi);
//void EnumPortsW9x(std::vector<SSerInfo> &asi);
//void SearchPnpKeyW9x(HKEY hkPnp, bool bUsbDevice,
//					 std::vector<SSerInfo> &asi);



CSensorWinUSBONavi01::CSensorWinUSBONavi01()
  : CSensor(), m_hcom(INVALID_HANDLE_VALUE)
{ 
   memset(&m_si, 0x00, sizeof(SSerInfo));
}

CSensorWinUSBONavi01::~CSensorWinUSBONavi01()
{
  closePort();
}

void CSensorWinUSBONavi01::closePort()
{
    memset(&m_si, 0x00, sizeof(SSerInfo));
	setPort();
	setType();
	if (m_hcom != INVALID_HANDLE_VALUE) { // handle exists
		::CloseHandle(m_hcom);
	}
}

bool CSensorWinUSBONavi01::detect()
{
	bool bRet = false;
	if (SearchONaviSerialPort(m_si)) {
		// open & validate the virtual com port persistent
			m_hcom = ::CreateFile((LPCSTR) m_si.strDevPath.c_str(),
				GENERIC_READ, // | GENERIC_WRITE,
				0,    /* comm devices must be opened w/exclusive-access */
				NULL, /* no security attrs */
				OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
				NULL,    /* not overlapped I/O */
				NULL  /* hTemplate must be NULL for comm devices */				);
			if (m_hcom == INVALID_HANDLE_VALUE) {
				closePort();
			}
			else {
				setType(SENSOR_USB_ONAVI_1);
				setPort(1);
				bRet = true;
			}
	}
	return bRet;
}

inline bool CSensorWinUSBONavi01::read_xyz(float& x1, float& y1, float& z1)
{
	// first check for valid port
	if (getPort() < 0 || m_hcom == INVALID_HANDLE_VALUE) return false;

	bool bRet = false;
	DWORD dwRead = 0L;

	QCN_BYTE bytesIn[16];
	memset(bytesIn, 0x00, 16);

	bRet = (bool) ::ReadFile(m_hcom, bytesIn, 16, &dwRead, NULL);

	return bRet;
}


//---------------------------------------------------------------
// Routine for enumerating the available serial ports.
// Throws a std::string on failure, describing the error that
// occurred. If bIgnoreBusyPorts is true, ports that can't
// be opened for read/write access are not included.

bool CSensorWinUSBONavi01::SearchONaviSerialPort(SSerInfo& si, const bool bIgnoreBusyPorts)
{
	bool bRet = false;
	memset(&si, 0x00, sizeof(SSerInfo)); 
	vector<SSerInfo> asi;

	// Use different techniques to enumerate the available serial
	// ports, depending on the OS we're using
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
	/*
	if (!::GetVersionEx(&vi)) {
		std::string str = "Could not get OS version." ;
		throw str;
	}
	// Handle windows 9x and NT4 specially
	if (vi.dwMajorVersion < 5) {
		if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT)
			EnumPortsWNt4(asi);
		else
			EnumPortsW9x(asi);
	}
	else {
		*/
		// Win2k and later support a standard API for
		// enumerating hardware devices.
	try {
		EnumPortsWdm(asi);
	}
	catch(std::string strCatchErr) {
		fprintf(stderr, "ONavi Err: %s\n", strCatchErr.c_str());
		return false;
	}

	//}

	std::vector<SSerInfo>::iterator iSI;   // iterator for a SSerInfo vector
	for (iSI=asi.begin(); iSI != asi.end(); iSI++)
	{
		if (bIgnoreBusyPorts) {
			// Only display ports that can be opened for read/write
			HANDLE hCom = ::CreateFile((LPCSTR) iSI->strDevPath.c_str(),
				GENERIC_READ, // | GENERIC_WRITE,
				0,    /* comm devices must be opened w/exclusive-access */
				NULL, /* no security attrs */
				OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
				0,    /* not overlapped I/O */
				NULL  /* hTemplate must be NULL for comm devices */				);
			if (hCom == INVALID_HANDLE_VALUE) {
				// It can't be opened; remove it.
				//asi.erase(iSI); // no need to erase
				break;
			}
			else {
				// It can be opened! Close it and add it to the list
				::CloseHandle(hCom);
			}
		}

		// Come up with a name for the device.
		// If there is no friendly name, use the port name.
		if (iSI->strFriendlyName.empty())
			iSI->strFriendlyName = iSI->strPortName;

		// If there is no description, try to make one up from
		// the friendly name.
		if (iSI->strPortDesc.empty()) {
			// If the port name is of the form "ACME Port (COM3)"
			// then strip off the " (COM3)"
			iSI->strPortDesc = iSI->strFriendlyName;
			int startdex = (int) iSI->strPortDesc.find(" (");
			int enddex = (int) iSI->strPortDesc.find(")");
			if (startdex > 0 && enddex == 
				(iSI->strPortDesc.length()-1))
				iSI->strPortDesc = iSI->strPortDesc.substr(0,startdex);
		}
		if (iSI->strFriendlyName.find(STR_ONAVI_1) == 0) {
			si = *iSI;
			bRet = true;
		}

	} // for loop
	return bRet;
}

// Helpers for EnumSerialPorts

void EnumPortsWdm(std::vector<SSerInfo> &asi)
{
	std::string strErr;
	// Create a device information set that will be the container for 
	// the device interfaces.
	GUID *guidDev = (GUID*) &GUID_CLASS_ONAVI_1; //&GUID_CLASS_COMPORT;

	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

	try {
		hDevInfo = SetupDiGetClassDevs( guidDev,
			NULL,
			NULL,
			DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
			
		if(hDevInfo == INVALID_HANDLE_VALUE) 
		{
			strErr = "SetupDiGetClassDevs failed. (err=%lx)";
			throw strErr;
		}

		// Enumerate the serial ports
		bool bOk = true;
		SP_DEVICE_INTERFACE_DATA ifcData;
		DWORD dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
		pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[dwDetDataSize];
		// This is required, according to the documentation. Yes,
		// it's weird.
		ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		for (DWORD ii=0; bOk; ii++) {
			bOk = (bool) SetupDiEnumDeviceInterfaces(hDevInfo,
				NULL, guidDev, ii, &ifcData);
			if (bOk) {
				// Got a device. Get the details.
				SP_DEVINFO_DATA devdata = {sizeof(SP_DEVINFO_DATA)};
				bOk = (bool) SetupDiGetDeviceInterfaceDetail(hDevInfo,
					&ifcData, pDetData, dwDetDataSize, NULL, &devdata);
				if (bOk) {
					std::string strDevPath(pDetData->DevicePath);
					// Got a path to the device. Try to get some more info.
					TCHAR fname[256];
					TCHAR desc[256];
					bool bSuccess = (bool) SetupDiGetDeviceRegistryProperty(
						hDevInfo, &devdata, SPDRP_FRIENDLYNAME, NULL,
						(PBYTE)fname, sizeof(fname), NULL);
					bSuccess = bSuccess && SetupDiGetDeviceRegistryProperty(
						hDevInfo, &devdata, SPDRP_DEVICEDESC, NULL,
						(PBYTE)desc, sizeof(desc), NULL);
					bool bUsbDevice = false;
					TCHAR locinfo[256];
					if (SetupDiGetDeviceRegistryProperty(
						hDevInfo, &devdata, SPDRP_LOCATION_INFORMATION, NULL,
						(PBYTE)locinfo, sizeof(locinfo), NULL))
					{
						// Just check the first three characters to determine
						// if the port is connected to the USB bus. This isn't
						// an infallible method; it would be better to use the
						// BUS GUID. Currently, Windows doesn't let you query
						// that though (SPDRP_BUSTYPEGUID seems to exist in
						// documentation only).
						bUsbDevice = (strncmp(locinfo, "USB", 3)==0);
					}
					if (bSuccess) {
						// Add an entry to the array
						SSerInfo si;
						si.strDevPath = strDevPath;
						si.strFriendlyName = fname;
						si.strPortDesc = desc;
						si.bUsbDevice = bUsbDevice;
						asi.push_back(si);
					}

				}
				else {
					strErr = "SetupDiGetDeviceInterfaceDetail failed. (err=%lx)";
					throw strErr;
				}
			}
			else {
				DWORD err = GetLastError();
				if (err != ERROR_NO_MORE_ITEMS) {
					strErr = "SetupDiEnumDeviceInterfaces failed. (err=%lx)";
					throw strErr;
				}
			}
		}
	}
	catch (std::string strCatchErr) {
		strErr = strCatchErr;
	}

	if (pDetData != NULL)
		delete [] (char*)pDetData;
	if (hDevInfo != INVALID_HANDLE_VALUE)
		SetupDiDestroyDeviceInfoList(hDevInfo);

	if (!strErr.empty())
		throw strErr;
}

/*
void EnumPortsWNt4(std::vector<SSerInfo> &asi)
{
	// NT4's driver model is totally different, and not that
	// many people use NT4 anymore. Just try all the COM ports
	// between 1 and 16
	SSerInfo si;
	for (int ii=1; ii<=16; ii++) {
		char strTmp[16];
		sprintf(strTmp, "COM%d", ii);
		std::string strPort = strTmp;
		si.strDevPath = std::string("\\\\.\\") + strPort;
		si.strPortName = strPort;
		asi.push_back(si);
	}
}

void EnumPortsW9x(std::vector<SSerInfo> &asi)
{
	// Look at all keys in HKLM\Enum, searching for subkeys named
	// *PNP0500 and *PNP0501. Within these subkeys, search for
	// sub-subkeys containing value entries with the name "PORTNAME"
	// Search all subkeys of HKLM\Enum\USBPORTS for PORTNAME entries.

	// First, open HKLM\Enum
	HKEY hkEnum = NULL;
	HKEY hkSubEnum = NULL;
	HKEY hkSubSubEnum = NULL;

	try {
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Enum", 0, KEY_READ,
			&hkEnum) != ERROR_SUCCESS)
			throw std::string("Could not read from HKLM\\Enum");

		// Enumerate the subkeys of HKLM\Enum
		char acSubEnum[128];
		DWORD dwSubEnumIndex = 0;
		DWORD dwSize = sizeof(acSubEnum);
		while (RegEnumKeyEx(hkEnum, dwSubEnumIndex++, acSubEnum, &dwSize,
			NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			HKEY hkSubEnum = NULL;
			if (RegOpenKeyEx(hkEnum, acSubEnum, 0, KEY_READ,
				&hkSubEnum) != ERROR_SUCCESS)
				throw std::string("Could not read from HKLM\\Enum\\")+acSubEnum;

			// Enumerate the subkeys of HKLM\Enum\*\, looking for keys
			// named *PNP0500 and *PNP0501 (or anything in USBPORTS)
			bool bUsbDevice = (strcmp(acSubEnum,"USBPORTS")==0);
			char acSubSubEnum[128];
			dwSize = sizeof(acSubSubEnum);  // set the buffer size
			DWORD dwSubSubEnumIndex = 0;
			while (RegEnumKeyEx(hkSubEnum, dwSubSubEnumIndex++, acSubSubEnum,
				&dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
			{
				bool bMatch = (strcmp(acSubSubEnum,"*PNP0500")==0 ||
					strcmp(acSubSubEnum,"*PNP0501")==0 ||
					bUsbDevice);
				if (bMatch) {
					HKEY hkSubSubEnum = NULL;
					if (RegOpenKeyEx(hkSubEnum, acSubSubEnum, 0, KEY_READ,
						&hkSubSubEnum) != ERROR_SUCCESS)
						throw std::string("Could not read from HKLM\\Enum\\") + 
						acSubEnum + "\\" + acSubSubEnum;
					SearchPnpKeyW9x(hkSubSubEnum, bUsbDevice, asi);
					RegCloseKey(hkSubSubEnum);
					hkSubSubEnum = NULL;
				}

				dwSize = sizeof(acSubSubEnum);  // restore the buffer size
			}

			RegCloseKey(hkSubEnum);
			hkSubEnum = NULL;
			dwSize = sizeof(acSubEnum); // restore the buffer size
		}
	}
	catch (std::string strError) {
		if (hkEnum != NULL)
			RegCloseKey(hkEnum);
		if (hkSubEnum != NULL)
			RegCloseKey(hkSubEnum);
		if (hkSubSubEnum != NULL)
			RegCloseKey(hkSubSubEnum);
		throw strError;
	}

	RegCloseKey(hkEnum);
}

void SearchPnpKeyW9x(HKEY hkPnp, bool bUsbDevice,
					 std::vector<SSerInfo> &asi)
{
	// Enumerate the subkeys of the given PNP key, looking for values with
	// the name "PORTNAME"
	// First, open HKLM\Enum
	HKEY hkSubPnp = NULL;

	try {
		// Enumerate the subkeys of HKLM\Enum\*\PNP050[01]
		char acSubPnp[128];
		DWORD dwSubPnpIndex = 0;
		DWORD dwSize = sizeof(acSubPnp);
		while (RegEnumKeyEx(hkPnp, dwSubPnpIndex++, acSubPnp, &dwSize,
			NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			HKEY hkSubPnp = NULL;
			if (RegOpenKeyEx(hkPnp, acSubPnp, 0, KEY_READ,
				&hkSubPnp) != ERROR_SUCCESS)
				throw std::string("Could not read from HKLM\\Enum\\...\\")
				+ acSubPnp;

			// Look for the PORTNAME value
			char acValue[128];
			dwSize = sizeof(acValue);
			if (RegQueryValueEx(hkSubPnp, "PORTNAME", NULL, NULL, (BYTE*)acValue,
				&dwSize) == ERROR_SUCCESS)
			{
				std::string strPortName(acValue);

				// Got the portname value. Look for a friendly name.
				std::string strFriendlyName;
				dwSize = sizeof(acValue);
				if (RegQueryValueEx(hkSubPnp, "FRIENDLYNAME", NULL, NULL, (BYTE*)acValue,
					&dwSize) == ERROR_SUCCESS)
					strFriendlyName = acValue;

				// Prepare an entry for the output array.
				SSerInfo si;
				si.strDevPath = std::string("\\\\.\\") + strPortName;
				si.strPortName = strPortName;
				si.strFriendlyName = strFriendlyName;
				si.bUsbDevice = bUsbDevice;

				// Overwrite duplicates.
				bool bDup = false;
				for (int ii=0; ii<asi.size() && !bDup; ii++)
				{
					if (asi[ii].strPortName == strPortName) {
						bDup = true;
						asi[ii] = si;
					}
				}
				if (!bDup) {
					// Add an entry to the array
					asi.push_back(si);
				}
			}

			RegCloseKey(hkSubPnp);
			hkSubPnp = NULL;
			dwSize = sizeof(acSubPnp);  // restore the buffer size
		}
	}
	catch (std::string strError) {
		if (hkSubPnp != NULL)
			RegCloseKey(hkSubPnp);
		throw strError;
	}
}

*/

