#ifndef _CSENSOR_LINUX_USB_ONAVI01_H_
#define _CSENSOR_LINUX_USB_ONAVI01_H_

/*
 *  csensor_linux_usb_onavi01.h
 *  qcn
 *
 *  Created by Carl Christensen on 01/24/2012
 *  Copyright 2012 Stanford University
 *
 * This file contains the definition of the Linux JoyWarrior USB Sensor class
 */

#include <stdio.h>
#include "main.h"
using namespace std;

// this is the Linux tty device for the ONavi-1 Mac kernel extension driver ie /dev/ttyACM0  /dev/ttyACM1 etc
#define STR_LINUX_USB_ONAVI01     "/dev/ttyACM*"
#define FLOAT_LINUX_ONAVI_FACTOR  7.629394531250e-05f

// this is the Windows implementation of the sensor - IBM/Lenovo Thinkpad, HP, USB Stick
class CSensorLinuxUSBONavi01  : public CSensor
{
   private:
    int m_fd;
     unsigned short m_usBitSensor;

      virtual bool read_xyz(float& x1, float& y1, float& z1);

   public:
      CSensorLinuxUSBONavi01();
      virtual ~CSensorLinuxUSBONavi01();

     virtual void closePort(); // closes the port if open
     virtual bool detect(); 
};



#endif
