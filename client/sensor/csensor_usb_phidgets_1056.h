#ifndef _CSENSOR_USB_PHIDGETS_1056_H_
#define _CSENSOR_USB_PHIDGETS_1056_H_

/*
 *  csensor_usb_phidgets_1056.h
 *  qcn
 *
 *  Created by Carl Christensen on 10/06/2008
 *  Copyright 2008 Stanford University
 *
 * This file contains the declarations for the CSensor-derived class for the Phidgets 1056 USB accelerometer
 *     http://www.phidgets.com/products.php?product_id=1056

 */

#include <stdio.h>

// for Mac & Linux we use dlopen into the MotionNodeAccel .dylib (Mac) or .so (Linux)
#ifndef _WIN32
  #define __USE_DLOPEN__
  #include <dlfcn.h>   // dlopen and dlclose
#endif

#include "main.h"
// #include "motionnodeaccel/MotionNodeAccelAPI.h"

using namespace std;

typedef MotionNodeAccel * (MOTIONNODE_CALL_C_API * PtrMotionNodeAccelFactory)(int);

// this is the Windows implementation of the sensor - IBM/Lenovo Thinkpad, HP, USB Stick
class CSensorUSBPhidgets1056  : public CSensor
{
   private:
#ifdef _WIN32
      HMODULE m_WinDLLHandle;
#else
	  void* m_WinDLLHandle;
#endif
      PtrMotionNodeAccelFactory m_SymHandle;
      MotionNodeAccel* m_node;

      static const char m_cstrDLL[];   // const name defined in the cpp file  

      virtual bool read_xyz(float& x1, float& y1, float& z1);  

   public:
      CSensorUSBPhidgets1056();
      virtual ~CSensorUSBPhidgets1056();
      virtual void closePort(); // closes the port if open
      virtual bool detect();   // this detects & initializes a sensor on a Mac G4/PPC or Intel laptop, sets m_iType to 0 if not found
};

#endif

