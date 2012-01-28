/*
 *  csensor_linux_usb_onavi01.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 01/24/2012
 *  Copyright 2012 Stanford University.  All rights reserved.
 *
 * Implementation file for Linux ONavi 1 Serial Comunications
 */

#include "main.h"
#include "csensor_linux_usb_onavi01.h"
#include <fnmatch.h>
#include <glob.h>
#include "filesys.h"    // boinc_file_or_symlink_exists
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>

CSensorLinuxUSBONavi01::CSensorLinuxUSBONavi01()
  : CSensor(), m_fd(-1), m_usBitSensor(0)
{ 
}

CSensorLinuxUSBONavi01::~CSensorLinuxUSBONavi01()
{
  closePort();
}

void CSensorLinuxUSBONavi01::closePort()
{
    if (m_fd > -1) {
	  close(m_fd);
    }
    m_fd = -1;
    m_usBitSensor = 0;
    setPort();
    setType();
}

bool CSensorLinuxUSBONavi01::detect()
{
	// first see if the port actually exists (the device is a "file" at /dev/ttyACM0, given in STR_LINUX_USB_ONAVI01 

        // use glob to match names, if count is > 0, we found a match
        glob_t gt;
        memset(&gt, 0x00, sizeof(glob_t));
        if (glob(STR_LINUX_USB_ONAVI01, GLOB_NOCHECK, NULL, &gt) || !gt.gl_pathc) {  // either glob failed or no match
           // device string failed, but try the new string onavi (really Exar USB driver) may be using
           //if (glob(STR_USB_ONAVI02, GLOB_NOCHECK, NULL, &gt) || !gt.gl_matchc) {  // either glob failed or no match
             globfree(&gt);
             return false;
           //}
        }

        char* strDevice = new char[_MAX_PATH];
        memset(strDevice, 0x00, sizeof(char) * _MAX_PATH);
        strncpy(strDevice, gt.gl_pathv[0], _MAX_PATH);
        globfree(&gt); // can get rid of gt now

	if (!boinc_file_or_symlink_exists(strDevice)) {
           delete [] strDevice;
           strDevice = NULL;
           return false;
        }

	m_fd = open(strDevice, O_RDWR | O_NOCTTY | O_NDELAY);

        delete [] strDevice; // don't need strDevice after this call
        strDevice = NULL;

	if (m_fd == -1) { // failure
           return false;
        }
        fcntl(m_fd, F_SETFL, 0);

/*
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
	tty.c_iflag = 0;
	tty.c_oflag = 0;
        tty.c_cflag = CRTSCTS | CS8 | CLOCAL | CREAD;
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;

        if (tcflush(m_fd, TCIOFLUSH) == -1)  {
		closePort();
		return false;
        }
	if (tcsetattr(m_fd, TCSANOW, &tty) == -1) { //|| tcsendbreak(m_fd, 10) == -1 ) { 
            // tcflow(m_fd, TCION) == -1) { // || tcflush(m_fd, TCIOFLUSH) == -1) {
		closePort();
		return false;
	}
   
*/

    struct termios options;
    tcgetattr(m_fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    // set basic "modem" options
    options.c_cflag     |= (CLOCAL | CREAD);
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 10;
    if (tcsetattr(m_fd, TCSANOW, &options) == -1) {
		closePort();
		return false;
    }

        setPort(m_fd);

	setSingleSampleDT(true); // Onavi does the 50Hz rate

        // try to read a value and get the sensor bit-type (& hence sensor type)
        float x,y,z;
        m_usBitSensor = 0;
        if (read_xyz(x,y,z) && m_usBitSensor > 0) {
	   // exists, so setPort & Type
           switch(m_usBitSensor) {
             case 12:
	         setType(SENSOR_USB_ONAVI_A_12); break;
             case 16:
	         setType(SENSOR_USB_ONAVI_A_16);
                 break;
             case 24:
	         setType(SENSOR_USB_ONAVI_A_24);
                 break;
             default: // error!
               closePort();
               return false;
	   }
        }
        else {
           closePort();
           return false;
        }
 
    return true;
}

inline bool CSensorLinuxUSBONavi01::read_xyz(float& x1, float& y1, float& z1)
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
	

	static float x0 = 0.0f, y0 = 0.0f, z0 = 0.0f; // keep last values

	// first check for valid port
	if (getPort() < 0) {
		   return false;
        }
	
	bool bRet = true;
        fd_set rdfs;
	struct timeval timeout;
	
	const int ciLen = 8;  // use an 8 byte buffer + 1 \0 padding
        QCN_BYTE bytesIn[ciLen+1], cs;  // note pad bytesIn with null \0
	int x = 0, y = 0, z = 0;
	int iCS = 0;
	int iRead = 0;
	//x1 = y1 = z1 = 0.0f; // don't init to 0 as ONavi 24-bit is having errors we need to debug
        x1 = x0; y1 = y0; z1 = z0;  // use last good values
	const char cWrite = '*';

// CMC here
 /*
    iRead = write(m_fd, "ATZ\r", 4);
    if (iRead < 0)
      fprintf(stdout, "write() of 4 bytes failed!\n");
  */
	if ((iRead = write(m_fd, &cWrite, 1)) == 1) {   // send a * to the device to get back the data
	    memset(bytesIn, 0x00, ciLen+1);

	    // initialise the timeout structure
	    timeout.tv_sec = 1; // 1 second timeout
	    timeout.tv_usec = 0;
            FD_ZERO(&rdfs);
            FD_SET(m_fd, &rdfs);

            int n = select(m_fd + 1, &rdfs, NULL, NULL, &timeout);

            if (n < 0) { // failed
	       bRet = false; // error
	       fprintf(stderr, "%f: ONavi Error in read_xyz() - select(m_fd) returned %d\n", sm->t0active, n);
            }
            else if (n == 0) { // timeout
	       bRet = false; // error
	       fprintf(stderr, "%f: ONavi Error in read_xyz() - select(m_fd) returned %d (timeout)\n", sm->t0active, n);
            }
            else if (FD_ISSET(m_fd, &rdfs)) { // select OK, now get the data
                // process the file descriptor
                if ((iRead = read(m_fd, bytesIn, ciLen)) == ciLen) {
				// good data length read in, now test for appropriate characters
				if (bytesIn[ciLen] == 0x00) { // && bytesIn[0] == 0x23 && bytesIn[1] == 0x23) {
					// format is ##XXYYZZC\0
					// we found both, the bytes in between are what we want (really bytes after lOffset[0]
                                        if (m_usBitSensor == 0) { // need to find sensor bit type i.e. 12/16/24-bit ONavi
					   if (bytesIn[0] == 0x2A && bytesIn[1] == 0x2A) {  // **
                                              m_usBitSensor = 12;
                                           }
					   else if (bytesIn[0] == 0x23 && bytesIn[1] == 0x23) { // ##
                                              m_usBitSensor = 16;
                                           }
					   else if (bytesIn[0] == 0x24 && bytesIn[1] == 0x24) {  // $$
                                              m_usBitSensor = 24;
                                           }
                                        }

					x = (bytesIn[2] * 255) + bytesIn[3];
					y = (bytesIn[4] * 255) + bytesIn[5];
					z = (bytesIn[6] * 255) + bytesIn[7];
					cs   = bytesIn[8];
					for (int i = 2; i <= 7; i++) iCS += bytesIn[i];

#ifdef QCN_RAW_DATA	
					// for testing on USGS shake table - they just want the raw integer data sent out
					x1 = (float) x;
					y1 = (float) y;
					z1 = (float) z;
#else
					// convert to g decimal value
					// g  = x - 32768 * (5 / 65536) 
					// Where: x is the data value 0 - 65536 (x0000 to xFFFF). 
					
					x1 = ((float) x - 32768.0f) * FLOAT_LINUX_ONAVI_FACTOR * EARTH_G;
					y1 = ((float) y - 32768.0f) * FLOAT_LINUX_ONAVI_FACTOR * EARTH_G;
					z1 = ((float) z - 32768.0f) * FLOAT_LINUX_ONAVI_FACTOR * EARTH_G;
#endif
					x0 = x1; y0 = y1; z0 = z1;  // preserve values
					
					bRet = true;
				}
                    }  // read()
                    else {
		      fprintf(stderr, "%f: ONavi Error in read_xyz() - read error after select %d\n", sm->t0active, iRead);
		      bRet = false;
                    }
            }
            else {  // error ie in the read select
		fprintf(stderr, "%f: ONavi Error in read_xyz() - read select(m_fd) failed %d\n", sm->t0active, n);
		bRet = false;
            }
	} // write *
	else {
		fprintf(stderr, "%f: ONavi Error in read_xyz() - write(m_fd) returned %d\n", sm->t0active, iRead);
		bRet = false;
	}
	return bRet;
}


