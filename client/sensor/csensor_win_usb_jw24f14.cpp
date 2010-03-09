/*
 *  csensor_win_usb_jw.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University.  All rights reserved.
 *
 * Implementation file for Windows USB JoyWarrior sensor class
 */

#include "main.h"
#include "csensor_win_usb_jw.h"

CSensorWinUSBJW24F14::CSensorWinUSBJW24F14()
  : CSensor(), m_USBHandle(NULL)
{ 
   m_USBDevHandle[0] = NULL;
   m_USBDevHandle[1] = NULL;
}

CSensorWinUSBJW24F14::~CSensorWinUSBJW24F14()
{
  closePort();
}

void CSensorWinUSBJW24F14::closePort()
{
  for (int i = 0; i < 2; i++) {
     if (m_USBDevHandle[i]) {
       try {
          // don't think we need the next line, just close & Release
      WriteData(m_USBDevHandle[i], 0x02, 0x00, 0x00);  // Free JW24F14
	  ::CancelIo(m_USBDevHandle[i]);
	  ::CloseHandle(m_USBDevHandle[i]);
	  m_USBDevHandle[i] = NULL;
          // mac version:
          //WriteData(m_USBDevHandle[i], 0x02, 0x00, 0x00, "closePort()::Free JW24F14");
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

bool CSensorWinUSBJW24F14::detect()
{
    // tries to detect & initialize the USB JW24F14 Sensor
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
		esTmp = SENSOR_USB_JW24F14;
		iPort = getPort();
        // no single sample, JW24F14 actually needs to sample within the 50hz,
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

	return (bool)(getTypeEnum() == SENSOR_USB_JW24F14);
}

// USB stick accelerometer specific stuff (codemercs.com JoyWarrior 24F8)
// http://codemercs.com/JW24F1424F8_E.html

void CSensorWinUSBJW24F14::GetCapabilities(HANDLE handle)
{
	PHIDP_PREPARSED_DATA PreparsedData;
	::HidD_GetPreparsedData(handle, &PreparsedData);
	::HidP_GetCaps(PreparsedData, &m_USBCapabilities);
	::HidD_FreePreparsedData(PreparsedData);
}

int CSensorWinUSBJW24F14::SetupJoystick()
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

inline bool CSensorWinUSBJW24F14::read_xyz(float& x1, float& y1, float& z1)
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
unsigned char CSensorWinUSBJW24F14::ReadData(HANDLE handle, unsigned char cmd, unsigned char addr)
{
	unsigned char			WriteBuffer[10];
	unsigned char			ReadBuffer[10];
	unsigned char			newAddr;
	long			BytesWritten = 0;
	long			NumberOfBytesRead = 0;
	BOOL			Result;

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

float CSensorWinUSBJW24F14::ReadedData(unsigned char addr_LSB, unsigned char addr_MSB, char axe)
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
unsigned char CSensorWinUSBJW24F14::ReadData(HANDLE handle, unsigned char addr)
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
bool CSensorWinUSBJW24F14::WriteData(HANDLE handle, unsigned char cmd, unsigned char addr, unsigned char data)
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

void CSensorWinUSBJW24F14::SetQCNState()
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
short CSensorWinUSBJW24F14::CalcMsbLsb(unsigned char lsb, unsigned char msb)
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
