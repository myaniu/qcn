/*
 *  csensor_mac_usb_onavi01.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 10/25/2009.
 *  Copyright 2009 Stanford University.  All rights reserved.
 *
 * Implementation file for Mac ONavi 1 Serial Comunications
 */

#include "main.h"
#include "csensor_mac_usb_onavi01.h"
#include <fnmatch.h>
#include <glob.h>
#include "filesys.h"    // boinc_file_or_symlink_exists
#include <termios.h>

CSensorMacUSBONavi01::CSensorMacUSBONavi01()
  : CSensor(), m_fd(-1)
{ 
}

CSensorMacUSBONavi01::~CSensorMacUSBONavi01()
{
  closePort();
}

void CSensorMacUSBONavi01::closePort()
{
	if (m_fd > -1) {
	  close(m_fd);
    }
    m_fd = -1;
	setPort();
	setType();
}

bool CSensorMacUSBONavi01::detect()
{
	// first see if the port actually exists (the device is a "file" at /dev/tty.xrusbmodemNNN, given in STR_USB_ONAVI01

        // use glob to match names, if count is > 0, we found a match
        glob_t gt;
        memset(&gt, 0x00, sizeof(glob_t));
        int iRet = glob(STR_USB_ONAVI01, flags, NULL, &gt);
        if (iRet || !gt.gl_matchc) {  // either glob failed or no match
           globfree(&gt);
           return false;
        }
        char* strDevice = new char[_MAX_PATH];
        memset(strDevice, 0x00, sizeof(char) * _MAX_PATH);
        strncpy(strDevice, gt.gl_pathv, _MAX_PATH);
        globfree(&gt); // can get rid of gt now

	if (!boinc_file_or_symlink_exists(strDevice) {
           delete [] strDevice;
           strDevice = NULL;
           return false;
        }
	
	m_fd = open(strDevice, O_RDONLY | O_NOCTTY | O_NONBLOCK); 
        delete [] strDevice; // don't need strDevice after this call
        strDevice = NULL;

	if (m_fd == -1) { //failure
           return false;
        }
        

	// if here we opened the port, now set comm params
	struct termios tty;
	if (tcgetattr(m_fd, &tty) == -1) {  // get current terminal state
		closePort();
		return false;
	}

	cfmakeraw(&tty);  // get raw tty settings
	
	// set terminal speed 115.2K
	if (cfsetspeed(&tty, B115200) == -1) {
		closePort();
		return false;
	}
	
	// flow contol
	tty.c_cflag |= (CS8 | CREAD);

	if (tcsetattr(m_fd, TCSAFLUSH, &tty) == -1 || tcflush(m_fd, TCIOFLUSH) ) {
		closePort();
		return false;
	}
	
	// exists, so setPort & Type
	setType(SENSOR_USB_ONAVI_1);
	setPort(1);
	
	setSingleSampleDT(true);

	return true;
}

inline bool CSensorMacUSBONavi01::read_xyz(float& x1, float& y1, float& z1)
{
	/*
We tried to keep the data as simple as possible. The data looks like: 

##xxyyzzs 

Two ASCII '#' (x23) followed by the X value upper byte, X value lower byte, Y value upper byte, Y value lower byte, Z value upper byte, Z value lower byte and an eight bit checksum.  

The bytes are tightly packed and there is nothing between the data values except for the sentence start ##.  

The X and Y axis will have a 0g offset at 32768d (x8000) and the Z axis offset at +1g 45874d (xB332) when oriented in the X/Y horizontal and Z up position.  The  s  value is a one byte checksum.  

It is a sum of all of the bytes, truncated to the lower byte.  This firmware does not transmit the temperature value. 

Finding g as a value:

g  = x - 32768 * (5 / 65536) 

Where: x is the data value 0 - 65536 (x0000 to xFFFF). 

Values >32768 are positive g and <32768 are negative g. The sampling rate is set to 200Hz, and the analog low-pass filters are set to 50Hz.  The data is oversampled 2X over Nyquist. We are going to make a new version of the module, with 25Hz LP analog filters and dual sensitivity 2g / 6g shortly.  Same drivers, same interface.  I ll get you one as soon as I we get feedback on this and make a set of those.

	*/
	
	static float x0 = 0.0f, y0 = 0.0f, z0 = 0.0f;

	// first check for valid port
	if (getPort() < 0) return false;

	bool bRet = true;
	
	const int ciLen = 24;  // use a 24 byte buffer
	short int lOffset[2] = { 0, 0 };
	QCN_BYTE bytesIn[ciLen], cs;
	int x = 0, y = 0, z = 0;
	int iCS = 0;
	int iRead = 0;
	x1 = y1 = z1 = 0.0f;

	/*
	QCN_BYTE cc[2048];
	memset(cc, 0x00, 2048);
	iRead = read(m_fd, cc, 2048);
	FILE* fcc = fopen("/tmp/cc.txt", "w");
	if (fcc) {
		fprintf(fcc, "\n%s\n", cc);
		fclose(fcc);
	}
	*/
	
/*
   if (lseek(m_fd, -ciLen, SEEK_END) == -1) {
  	    x1 = x0; y1 = y0; z1 = z0;  // use last good values
		return false;  // go to end of stream (32 bytes from end), if fail return
	}
*/	
	
	memset(bytesIn, 0x00, ciLen);
	if ((iRead = read(m_fd, bytesIn, ciLen)) > 8) {
		for (int i = ciLen-1; i >= 0; i--) { // look for hash-mark i.e. ## boundaries (two sets of ##)
			if (bytesIn[i] == 0x23 && bytesIn[i-1] == 0x23) { // found a hash-mark set
				if (!lOffset[1]) {
					lOffset[1] = i;
					i-=8;
				}
				else {
					lOffset[0] = i+1; // must be the start
					break;  // found both hash marks - can leave loop
				}
			}
 		}
		if (lOffset[0] && lOffset[1] && lOffset[1] == (lOffset[0] + 8)) { 
			// we found both, the bytes in between are what we want (really bytes after lOffset[0]
			x = (bytesIn[lOffset[0]] * 255) + bytesIn[lOffset[0]+1];
			y = (bytesIn[lOffset[0]+2] * 255) + bytesIn[lOffset[0]+3];
			z = (bytesIn[lOffset[0]+4] * 255) + bytesIn[lOffset[0]+5];
			cs   = bytesIn[lOffset[0]+6];
			for (int i = 0; i <= 5; i++) iCS += bytesIn[lOffset[0] + i];

			// convert to g decimal value
			// g  = x - 32768 * (5 / 65536) 
			// Where: x is the data value 0 - 65536 (x0000 to xFFFF). 

			x1 = ((float) x - 32768.0f) * FLOAT_ONAVI_FACTOR * EARTH_G;
			y1 = ((float) y - 32768.0f) * FLOAT_ONAVI_FACTOR * EARTH_G;
			z1 = ((float) z - 32768.0f) * FLOAT_ONAVI_FACTOR * EARTH_G;
			
			x0 = x1; y0 = y1; z0 = z1;  // preserve values

			bRet = true;
		}
		else {
			x1 = x0; y1 = y0; z1 = z0;  // use last good values
			bRet = false;  // could be just empty, return
		}
	}
	else {
		bRet = false;
	}

	tcflush(m_fd, TCIOFLUSH);
	return bRet;
}


