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

//---------------------------------------------------------------
// Helpers for enumerating the available serial ports.
// These throw a std::string on failure, describing the nature of
// the error that occurred.

void EnumPortsWdm(std::vector<SSerInfo> &asi);
void EnumPortsWNt4(std::vector<SSerInfo> &asi);
void EnumPortsW9x(std::vector<SSerInfo> &asi);
void SearchPnpKeyW9x(HKEY hkPnp, bool bUsbDevice,
					 std::vector<SSerInfo> &asi);



CSensorWinUSBONavi01::CSensorWinUSBONavi01()
  : CSensor(), m_USBHandle(NULL)
{ 
   m_USBDevHandle[0] = NULL;
   m_USBDevHandle[1] = NULL;
}

CSensorWinUSBONavi01::~CSensorWinUSBONavi01()
{
  closePort();
}

void CSensorWinUSBONavi01::closePort()
{
  for (int i = 0; i < 2; i++) {
     if (m_USBDevHandle[i]) {
       try {
          // don't think we need the next line, just close & Release
      WriteData(m_USBDevHandle[i], 0x02, 0x00, 0x00);  // Free JW
	  ::CancelIo(m_USBDevHandle[i]);
	  ::CloseHandle(m_USBDevHandle[i]);
	  m_USBDevHandle[i] = NULL;
          // mac version:
          //WriteData(m_USBDevHandle[i], 0x02, 0x00, 0x00, "closePort()::Free JW");
          //(*m_USBDevHandle[i])->close(m_USBDevHandle[i]);
          //(*m_USBDevHandle[i])->Release(m_USBDevHandle[i]);
          //m_USBDevHandle[i] = NULL;
        }
        catch(...) {
            fprintf(stderr, "Could not close JoyWarrior USB port %d...\n", i);
        }
     }
  }

  if (m_USBHandle) {
		::CloseHandle(m_USBHandle);
		m_USBHandle = NULL;
  }

  if (getPort() > -1) {
    setPort();
    fprintf(stdout, "Port closed!\n");
    fflush(stdout);
  }
}

bool CSensorWinUSBONavi01::detect()
{
    // tries to detect & initialize the USB JW Sensor
	setType();
    e_sensor esTmp = SENSOR_NOTFOUND; 
	int iPort = -1;  
	if (m_USBHandle) {
		::CloseHandle(m_USBHandle);
		m_USBHandle = NULL;
	}

   // enumerate usb ports looking for the joywarrior or mousewarrior device
   // taken from the codemercs CNeigungswinkelDlg class

	HIDD_ATTRIBUTES	Attributes;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;
	HDEVINFO hDevInfo;

	bool bStart = false;
	int	MemberIndex = 0;
	int	DeviceIndex = 0;
	LONG DevInfo;
	ULONG Length = 0;
	ULONG Required;
	GUID HidGuid;

    ZeroMemory(&devInfoData, sizeof(devInfoData));
    devInfoData.cbSize = sizeof(devInfoData);

	HidD_GetHidGuid(&HidGuid);	

	hDevInfo = SetupDiGetClassDevsW(&HidGuid, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	do
	{
		DevInfo = SetupDiEnumDeviceInterfaces (hDevInfo, NULL, &HidGuid, MemberIndex, &devInfoData);

		if (DevInfo != 0)
		{
			SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInfoData, NULL, 0, &Length, NULL);

			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc(Length);
			detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//SetupDiGetDeviceInterfaceDetailW(hDevInfo, &devInfoData, detailData, Length, &Required, NULL);
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, Length, &Required, NULL);

			m_USBHandle = ::CreateFile (detailData->DevicePath, 0, FILE_SHARE_READ|FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);

			Attributes.Size = sizeof(Attributes);

			HidD_GetAttributes(m_USBHandle, &Attributes);

			if (m_USBHandle && Attributes.VendorID == USB_VENDOR) 
			{
				if((Attributes.ProductID == USB_JOYWARRIOR) || (Attributes.ProductID == USB_MOUSEWARRIOR))
				{
					//if(Attributes.ProductID == USB_JOYWARRIOR) GetDlgItem(IDC_STATIC_DEVICE)->SetWindowTextW(_T("JoyWarrior"));
					//if(Attributes.ProductID == USB_MOUSEWARRIOR) GetDlgItem(IDC_STATIC_DEVICE)->SetWindowTextW(_T("MouseWarrior"));

					GetCapabilities(m_USBHandle);

					//m_USBDevHandle[DeviceIndex] = CreateFileW(detailData->DevicePath, 
					m_USBDevHandle[DeviceIndex] = CreateFile(detailData->DevicePath, 
														GENERIC_WRITE | GENERIC_READ, 
														FILE_SHARE_READ|FILE_SHARE_WRITE, 
														(LPSECURITY_ATTRIBUTES)NULL, 
														OPEN_EXISTING, 
														0, 
														NULL);

					DeviceIndex++;
					//GetDlgItem(IDC_STATIC_SERIAL)->SetWindowTextW(GetSerialNumber(m_USBDevHandle[1]));
					bStart = true;
				}
				else
					NULL;
			}
			else {
				if (m_USBHandle) {
					::CloseHandle(m_USBHandle);
					m_USBHandle = NULL;
				}
			}

			free(detailData);
		}

		MemberIndex++;

	} while (DevInfo != NULL);

	SetupDiDestroyDeviceInfoList(hDevInfo);

	if (bStart && SetupJoystick() >= 0) {
		esTmp = SENSOR_USB_JW;
		iPort = getPort();
        // no single sample, JW actually needs to sample within the 50hz,
        // since we're reading from joystick port, not the downsampling "chip"
		//setSingleSampleDT(true);  // note the usb sensor just requires 1 sample per dt, hardware does the rest
		fprintf(stdout, "USB sensor detected on Windows joystick port %d\n"
			"Set to 50Hz internal bandwidth, +/- 2g acceleration.\n", getPort());

        SetQCNState();
	}

    closePort();  // close the HID USB stuff and just use joystick calls from here on out

	// NB: closePort resets the type & port, so have to set again 
    setType(esTmp);
	setPort(iPort);

	return (bool)(getTypeEnum() == SENSOR_USB_JW);
}

// USB stick accelerometer specific stuff (codemercs.com JoyWarrior 24F8)
// http://codemercs.com/JW24F8_E.html

void CSensorWinUSBONavi01::GetCapabilities(HANDLE handle)
{
	PHIDP_PREPARSED_DATA PreparsedData;
	::HidD_GetPreparsedData(handle, &PreparsedData);
	::HidP_GetCaps(PreparsedData, &m_USBCapabilities);
	::HidD_FreePreparsedData(PreparsedData);
}

int CSensorWinUSBONavi01::SetupJoystick()
{
	const int cnumJoy = ::joyGetNumDevs();
	LPJOYCAPS pjc = new JOYCAPS;
	const int isizeJC = sizeof(JOYCAPS);
	int i;
	setPort();
	// enumerate joysticks and find a match for the JoyWarrior
	for (i = JOYSTICKID1; i < JOYSTICKID1 + cnumJoy; i++)  {
		memset(pjc, 0x00, isizeJC);
		if (::joyGetDevCaps(i, pjc, isizeJC) == JOYERR_NOERROR) {
			// see if it matches up to the Product & Vendor ID for codemercs.com JoyWarrior
			if (pjc->wMid == USB_VENDOR && pjc->wPid == USB_JOYWARRIOR) {
				// this is the joystick
				setPort(i);
				break;
			}
		}
	}
	delete pjc;
	if (i == cnumJoy) setPort();  // error, didn't break
	return getPort();
}

inline bool CSensorWinUSBONavi01::read_xyz(float& x1, float& y1, float& z1)
{
	// joystick fn usage
	if (getPort() < 0) return false;
	static JOYINFOEX jix;
	static int iSize = sizeof(JOYINFOEX);

	memset(&jix, 0x00, iSize);
	jix.dwSize = iSize;
	//jix.dwFlags = JOY_RETURNALL; // JOY_RETURNRAWDATA; // JOY_RETURNALL; // JOY_CAL_READ5; //JOY_RETURNRAWDATA | JOY_RETURNALL; // JOY_RETURNX | JOY_RETURNY | JOY_RETURNZ;
    jix.dwFlags = JOY_CAL_READ5; // read 5 axes calibration info
	MMRESULT mres = ::joyGetPosEx(getPort(), &jix);
        // note x/y/z values should be +/-2g where g = 9.78 (see define.h:: EARTH_G)
	if (mres == JOYERR_NOERROR) { // successfully read the joystick, -2g = 0, 0g = 32767, 2g = 65535
		x1 = (((float) jix.dwXpos - 32767.0f) / 16383.0f) * EARTH_G;
		y1 = (((float) jix.dwYpos - 32767.0f) / 16383.0f) * EARTH_G;
		z1 = (((float) jix.dwZpos - 32767.0f) / 16383.0f) * EARTH_G;
	}
	else {
		x1 = 0.0f;
		y1 = 0.0f;
		z1 = 0.0f;
	}

	/*  // read device 1 which is too slow
	x = ReadedData(0x02, 0x03, 'x');
	y = ReadedData(0x04, 0x05, 'y');
	z = ReadedData(0x06, 0x07, 'z');
    */

	return true;
}

/*
unsigned char CSensorWinUSBONavi01::ReadData(HANDLE handle, unsigned char cmd, unsigned char addr)
{
	unsigned char			WriteBuffer[10];
	unsigned char			ReadBuffer[10];
	unsigned char			newAddr;
	long			BytesWritten = 0;
	long			NumberOfBytesRead = 0;
	bool			Result;

	newAddr = 0x80 | addr;
	memset(&WriteBuffer, 0, m_USBCapabilities.OutputReportByteLength+1);

	WriteBuffer[0] = 0x00;
	WriteBuffer[1] = cmd;
	WriteBuffer[2] = newAddr;

	Result = WriteFile(handle, &WriteBuffer, m_USBCapabilities.OutputReportByteLength, (LPDWORD) &BytesWritten, NULL);

	if(Result != NULL)
	{
		memset(&ReadBuffer, 0, m_USBCapabilities.InputReportByteLength+1);
		ReadBuffer[0] = 0x00;
	
		ReadFile(handle, &ReadBuffer, m_USBCapabilities.InputReportByteLength, (LPDWORD) &NumberOfBytesRead, NULL);
		return ReadBuffer[3];
	}
	else
		return 0;
}

float CSensorWinUSBONavi01::ReadedData(unsigned char addr_LSB, unsigned char addr_MSB, char axe)
{
	unsigned char MSB, LSB;

	// use the 0 interface for better speed
	//LSB = ReadData(m_USBDevHandle[1], 0x82, addr_LSB);
	//MSB = ReadData(m_USBDevHandle[1], 0x82, addr_MSB);

	LSB = ReadData(m_USBDevHandle[0], 0x82, addr_LSB);
	MSB = ReadData(m_USBDevHandle[0], 0x82, addr_MSB);

	return (float) CalcMsbLsb(LSB, MSB);
}
*/

// USB read function
unsigned char CSensorWinUSBONavi01::ReadData(HANDLE handle, unsigned char addr)
{
	unsigned char			WriteBuffer[10];
	unsigned char			ReadBuffer[10];
	unsigned char			newAddr;
	long			BytesWritten = 0;
	long			NumberOfBytesRead = 0;
	int			Result;

	HidD_FlushQueue(handle);
	newAddr = 0x80 | addr;

	memset(&WriteBuffer, 0, m_USBCapabilities.OutputReportByteLength+1);

	/*Enable command-mode from Jw*/
	WriteBuffer[0] = 0x00; //ReportID
	WriteBuffer[1] = 0x82; //CMD-Mode
	WriteBuffer[2] = newAddr; //CMD + Addr

	Result = WriteFile(handle, &WriteBuffer, m_USBCapabilities.OutputReportByteLength, (LPDWORD) &BytesWritten, NULL);

	if(Result != NULL)
	{
		memset(&ReadBuffer, 0, m_USBCapabilities.InputReportByteLength+1);
		ReadBuffer[0] = 0x00;
	
		ReadFile(handle, &ReadBuffer, m_USBCapabilities.InputReportByteLength, (LPDWORD) &NumberOfBytesRead, NULL);
		return ReadBuffer[3];
	}
	else
		return 0;
}


// USB write function
bool CSensorWinUSBONavi01::WriteData(HANDLE handle, unsigned char cmd, unsigned char addr, unsigned char data)
{
	unsigned char			WriteBuffer[10];
	unsigned char			ReadBuffer[10];
	int			Result;
	long			BytesWritten = 0;
	long			NumberOfBytesRead = 0;

	memset(&WriteBuffer, 0, m_USBCapabilities.OutputReportByteLength+1);

	WriteBuffer[0] = 0x00;
	WriteBuffer[1] = cmd;
	WriteBuffer[2] = addr;
	WriteBuffer[3] = data;

	Result = WriteFile(handle, &WriteBuffer, m_USBCapabilities.OutputReportByteLength, (LPDWORD) &BytesWritten, NULL);

	if(Result != NULL)
	{
		return true;
		memset(&ReadBuffer, 0, m_USBCapabilities.InputReportByteLength+1);
		ReadBuffer[0] = 0x00;
	
		ReadFile(handle, &ReadBuffer, m_USBCapabilities.InputReportByteLength, (LPDWORD) &NumberOfBytesRead, NULL);
	}
	else
		return false;
}

void CSensorWinUSBONavi01::SetQCNState()
{ // puts the Joystick Warrior USB sensor into the proper state for QCN (50Hz, +/- 2g)
  // and also writes these settings to EEPROM (so each device needs to just get set once hopefully)

   unsigned char mReg14 = ReadData(m_USBDevHandle[1], 0x14);  // get current settings of device
   // if not set already, set it to +/-2g accel (0x00) and 50Hz internal bandwidth 0x01
   // NB: 0x08 & 0x10 means accel is set to 4 or 8g, if not bit-and with 0x01 bandwidth is other than 50Hz
   if ((mReg14 & 0x08) || (mReg14 & 0x10) || ((mReg14 & 0x01) != 0x01)) {
        mReg14 = 0x01 | (ReadData(m_USBDevHandle[1], 0x14) & 0xE0);

        // write settings to register
        WriteData(m_USBDevHandle[1], 0x82, 0x14, mReg14);

        // write settings to EEPROM for persistent state
        WriteData(m_USBDevHandle[1], 0x82, 0x0A, 0x10);  // start EEPROM write
        boinc_sleep(.050f);
        WriteData(m_USBDevHandle[1], 0x82, 0x34, mReg14);
        boinc_sleep(.050f);
        WriteData(m_USBDevHandle[1], 0x82, 0x0A, 0x02);  // end EEPROM write
        boinc_sleep(.100f);
   }
}

/*
// Calculate a 10 bit value with MSB and LSB
short CSensorWinUSBONavi01::CalcMsbLsb(unsigned char lsb, unsigned char msb)
{
	short erg;
	short LSB, MSB, EXEC;

	EXEC = (msb & 0x80) << 8;
	EXEC = EXEC & 0x8000;

	// Calculate negative value
	if(EXEC & 0x8000)
		EXEC = EXEC | 0x7C00;

	MSB = msb << 2;
	MSB = MSB & 0x03FC;
	LSB = (lsb & 0xC0) >> 6;
	LSB = LSB & 0x0003;

	erg = MSB | LSB | EXEC;

	return erg;
}
*/

//---------------------------------------------------------------
// Routine for enumerating the available serial ports.
// Throws a std::string on failure, describing the error that
// occurred. If bIgnoreBusyPorts is TRUE, ports that can't
// be opened for read/write access are not included.

void EnumSerialPorts(std::vector<SSerInfo> &asi, bool bIgnoreBusyPorts)
{
	// Clear the output array
	asi.clear();

	// Use different techniques to enumerate the available serial
	// ports, depending on the OS we're using
	OSVERSIONINFO vi;
	vi.dwOSVersionInfoSize = sizeof(vi);
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
		// Win2k and later support a standard API for
		// enumerating hardware devices.
		EnumPortsWdm(asi);
	}

	std::vector<SSerInfo>::iterator iSI;   // iterator for a SSerInfo vector
	for (iSI=asi.begin(); iSI != asi.end(); ++iSI)
	{
		if (bIgnoreBusyPorts) {
			// Only display ports that can be opened for read/write
			HANDLE hCom = ::CreateFile((LPCSTR) iSI->strDevPath.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0,    /* comm devices must be opened w/exclusive-access */
				NULL, /* no security attrs */
				OPEN_EXISTING, /* comm devices must use OPEN_EXISTING */
				0,    /* not overlapped I/O */
				NULL  /* hTemplate must be NULL for comm devices */
				);
			if (hCom == INVALID_HANDLE_VALUE) {
				// It can't be opened; remove it.
				asi.erase(iSI);
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
	}
}

// Helpers for EnumSerialPorts

void EnumPortsWdm(std::vector<SSerInfo> &asi)
{
	std::string strErr;
	// Create a device information set that will be the container for 
	// the device interfaces.
	GUID *guidDev = (GUID*) &GUID_CLASS_COMPORT;

	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = NULL;

	try {
		hDevInfo = SetupDiGetClassDevs( guidDev,
			NULL,
			NULL,
			DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
			);

		if(hDevInfo == INVALID_HANDLE_VALUE) 
		{
			strErr = "SetupDiGetClassDevs failed. (err=%lx)";
			throw strErr;
		}

		// Enumerate the serial ports
		bool bOk = TRUE;
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
					bool bUsbDevice = FALSE;
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
				bool bDup = FALSE;
				for (int ii=0; ii<asi.size() && !bDup; ii++)
				{
					if (asi[ii].strPortName == strPortName) {
						bDup = TRUE;
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

