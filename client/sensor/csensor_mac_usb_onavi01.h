#ifndef _CSENSOR_MAC_USB_ONAVI01_H_
#define _CSENSOR_MAC_USB_ONAVI01_H_

/*
 *  csensor-mac-usb-onavi01.h
 *  qcn
 *
 *  Created by Carl Christensen on 10/10/2009.
 *  Copyright 2009 Stanford University
 *
 * This file contains the declarations for the CSensor-derived class for Windows USB JoyWarrior accelerometer
 */

#include <stdio.h>
#include <windows.h>

#include "main.h"
using namespace std;

// this is the Windows implementation of the sensor - IBM/Lenovo Thinkpad, HP, USB Stick
class CSensorMacUSBONavi01  : public CSensor
{
   private:
      virtual bool read_xyz(float& x1, float& y1, float& z1);  

   public:
      CSensorMacUSBONavi01();
      virtual ~CSensorMacUSBONavi01();

     virtual void closePort(); // closes the port if open
     virtual bool detect();   // this detects & initializes a sensor on a Mac G4/PPC or Intel laptop, sets m_iType to 0 if not found
};

#endif

