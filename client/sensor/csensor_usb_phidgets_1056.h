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
#include <phidgets/phidget21.h>

// for Mac & Linux we use dlopen into the MotionNodeAccel .dylib (Mac) or .so (Linux)
#ifndef _WIN32
#define __USE_DLOPEN__
#include <dlfcn.h>   // dlopen and dlclose
#endif

#include "main.h"

using namespace std;

#if defined(WIN32)
#  if defined(BUILD_ACCEL_API)
#    define PHIDGETS_IMPORT_API __declspec(dllexport)
#  else
#    define PHIDGETS_IMPORT_API __declspec(dllimport)
#  endif // BUILD_ACCEL_API
#  define PHIDGETS_CALL_API __stdcall
#  define PHIDGETS_CALL_C_API __cdecl
#else
#  define PHIDGETS_IMPORT_API
#  define PHIDGETS_CALL_API
#  define PHIDGETS_CALL_C_API
#endif // WIN32

// function pointers in the Phidgets shared object/dylib/DLL
// CMC HERE
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_open)  (CPhidgetHandle phid, int serialNumber);
typedef int 	CPhidget_close (CPhidgetHandle phid)
typedef int 	CPhidget_delete (CPhidgetHandle phid)

typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_waitForAttachment)  (CPhidgetHandle phid, int milliseconds);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getDeviceName)  (CPhidgetHandle phid, const char **deviceName);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getSerialNumber)  (CPhidgetHandle phid, int *serialNumber);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getDeviceVersion)  (CPhidgetHandle phid, int *deviceVersion);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getDeviceStatus)  (CPhidgetHandle phid, int *deviceStatus);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getLibraryVersion)  (const char **libraryVersion);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getDeviceType)  (CPhidgetHandle phid, const char **deviceType);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getDeviceLabel) (CPhidgetHandle phid, const char **deviceLabel);

// callback function handlers
typedef int		(PHIDGETS_CALL_API * PtrCPhidgetSpatial_create) (CPhidgetSpatialHandle *phid);
int 	CPhidget_set_OnDetach_Handler (CPhidgetHandle phid, int(*fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
int 	CPhidget_set_OnAttach_Handler (CPhidgetHandle phid, int(*fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
int 	CPhidget_set_OnServerConnect_Handler (CPhidgetHandle phid, int(*fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
int 	CPhidget_set_OnServerDisconnect_Handler (CPhidgetHandle phid, int(*fptr)(CPhidgetHandle phid, void *userPtr), void *userPtr)
int 	CPhidget_set_OnError_Handler (CPhidgetHandle phid, int(*fptr)(CPhidgetHandle phid, void *userPtr, int errorCode, const char *errorString), void *userPtr)

CPhidget_set_OnAttach_Handler((CPhidgetHandle)spatial, AttachHandler, NULL);
CPhidget_set_OnDetach_Handler((CPhidgetHandle)spatial, DetachHandler, NULL);
CPhidget_set_OnError_Handler((CPhidgetHandle)spatial, ErrorHandler, NULL);
CPhidgetSpatial_set_OnSpatialData_Handler(spatial, SpatialDataHandler, NULL);

// CPhidgetSpacial specific functions
typedef int		(PHIDGETS_CALL_API * PtrCPhidgetSpatial_create) (CPhidgetSpatialHandle *phid);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidget_getErrorDescription) (int errorCode, const char **errorString);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getAccelerationAxisCount) (CPhidgetSpatialHandle phid, int *count);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getGyroAxisCount) (CPhidgetSpatialHandle phid, int *count);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getCompassAxisCount) (CPhidgetSpatialHandle phid, int *count);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getAcceleration) (CPhidgetSpatialHandle phid, int index, double *acceleration);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getAccelerationMax) (CPhidgetSpatialHandle phid, int index, double *max);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getAccelerationMin) (CPhidgetSpatialHandle phid, int index, double *min);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getDataRate) (CPhidgetSpatialHandle phid, int *milliseconds);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_setDataRate) (CPhidgetSpatialHandle phid, int milliseconds);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getDataRateMax) (CPhidgetSpatialHandle phid, int *max);
typedef int 	(PHIDGETS_CALL_API * PtrCPhidgetSpatial_getDataRateMin) (CPhidgetSpatialHandle phid, int *min);


// this is the Windows implementation of the sensor - IBM/Lenovo Thinkpad, HP, USB Stick
class CSensorUSBPhidgets1056  : public CSensor
{
  private:
	int m_iSerialNum;
	int m_iVersion;
	int m_iNumAccelAxes;
	int m_iNumGyroAxes;
	int m_iNumCompassAxes;
	int m_iDataRateMax;
	int m_iDataRateMin;
	
	static const char m_cstrDLL[];
	
	
#ifdef _WIN32
	HMODULE m_handleObject;
#else
	void* m_handleObject;
#endif
	
	PtrCPhidget_open m_PtrCPhidget_open;
	PtrCPhidget_waitForAttachment m_PtrCPhidget_waitForAttachment;
	PtrCPhidget_getDeviceName m_PtrCPhidget_getDeviceName;
	PtrCPhidget_getSerialNumber m_PtrCPhidget_getSerialNumber;
	PtrCPhidget_getDeviceVersion m_PtrCPhidget_getDeviceVersion;
	PtrCPhidget_getDeviceStatus m_PtrCPhidget_getDeviceStatus;
	PtrCPhidget_getLibraryVersion m_PtrCPhidget_getLibraryVersion;
	PtrCPhidget_getDeviceType m_PtrCPhidget_getDeviceType;
	PtrCPhidget_getDeviceLabel m_PtrCPhidget_getDeviceLabel;
	
	// callback function handlers
	CPhidget_set_OnAttach_Handler((CPhidgetHandle)spatial, AttachHandler, NULL);
	CPhidget_set_OnDetach_Handler((CPhidgetHandle)spatial, DetachHandler, NULL);
	CPhidget_set_OnError_Handler((CPhidgetHandle)spatial, ErrorHandler, NULL);
	CPhidgetSpatial_set_OnSpatialData_Handler(spatial, SpatialDataHandler, NULL);
	
	
	PtrCPhidgetSpatial_create m_PtrCPhidgetSpatial_create;
	PtrCPhidget_getErrorDescription m_PtrCPhidget_getErrorDescription;
	PtrCPhidgetSpatial_getAccelerationAxisCount m_PtrCPhidgetSpatial_getAccelerationAxisCount;
	PtrCPhidgetSpatial_getGyroAxisCount m_PtrCPhidgetSpatial_getGyroAxisCount;
	PtrCPhidgetSpatial_getCompassAxisCount m_PtrCPhidgetSpatial_getCompassAxisCount;
	PtrCPhidgetSpatial_getAcceleration m_PtrCPhidgetSpatial_getAcceleration;
	PtrCPhidgetSpatial_getAccelerationMax m_PtrCPhidgetSpatial_getAccelerationMax;
	PtrCPhidgetSpatial_getAccelerationMin m_PtrCPhidgetSpatial_getAccelerationMin;
	PtrCPhidgetSpatial_getDataRate m_PtrCPhidgetSpatial_getDataRate;
	PtrCPhidgetSpatial_setDataRate m_PtrCPhidgetSpatial_setDataRate;
	PtrCPhidgetSpatial_getDataRateMax m_PtrCPhidgetSpatial_getDataRateMax;
	PtrCPhidgetSpatial_getDataRateMin m_PtrCPhidgetSpatial_getDataRateMin;
		
	bool setupFunctionPointers();
	virtual bool read_xyz(float& x1, float& y1, float& z1);  

   public:
      CSensorUSBPhidgets1056();
      virtual ~CSensorUSBPhidgets1056();
      virtual void closePort(); // closes the port if open
      virtual bool detect();   // this detects & initializes a sensor on a Mac G4/PPC or Intel laptop, sets m_iType to 0 if not found
};

#endif

