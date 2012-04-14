/*
 *  csensor_usb_phidgets_1056.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2012 Stanford University.  All rights reserved.
 *
 * Implementation file for cross-platform (Mac, Windows, Linux) Phidgets 1056 accelerometer USB sensor class
 *     http://www.phidgets.com/products.php?product_id=1056
 *   NB: some "Windows" terminology used, i.e. m_handleObject but it's really a shared object Mac dylib or Linux so of course
 */

#include "main.h"
#include "csensor_usb_phidgets_1056.h"

//set the dll/so/dylib name
#ifdef _WIN32
const char CSensorUSBPhidgets1056::m_cstrDLL[] = {"phidgets21.dll"};
#else
#ifdef __APPLE_CC__
const char CSensorUSBPhidgets1056::m_cstrDLL[] = {"phidget21.dylib"};   
#else
const char CSensorUSBPhidgets1056::m_cstrDLL[] = {"phidgets21.so"};   
#endif // apple or linux
#endif // windows˚˚

CSensorUSBPhidgets1056::CSensorUSBPhidgets1056()
  : CSensor(), 
     m_handlePhidgetSpatial(NULL), m_handleObject(NULL)
{ 
	m_iSerialNum = 0;
	m_iVersion = 0;
	m_iNumAccelAxes = 0;
	m_iNumGyroAxes = 0;
	m_iNumCompassAxes = 0;
	m_iDataRateMax = 0;
	m_iDataRateMin = 0;	
}

CSensorUSBPhidgets1056::~CSensorUSBPhidgets1056()
{
   closePort();
}

bool CSensorUSBPhidgets1056::setupFunctionPointers()
{
#ifndef __USE_DLOPEN__
	return true;
#endif

	if (!m_handleObject) return false;

	m_PtrCPhidget_open = (PtrCPhidget_open) dlsym(m_handleObject, "CPhidget_open");
	m_PtrCPhidget_close = (PtrCPhidget_close) dlsym(m_handleObject, "CPhidget_close");
	m_PtrCPhidget_delete = (PtrCPhidget_delete) dlsym(m_handleObject, "CPhidget_delete");
	m_PtrCPhidget_waitForAttachment = (PtrCPhidget_waitForAttachment) dlsym(m_handleObject, "CPhidget_waitForAttachment");
	m_PtrCPhidget_getDeviceName = (PtrCPhidget_getDeviceName) dlsym(m_handleObject, "CPhidget_getDeviceName");
	m_PtrCPhidget_getSerialNumber = (PtrCPhidget_getSerialNumber) dlsym(m_handleObject, "CPhidget_getSerialNumber");
	m_PtrCPhidget_getDeviceVersion = (PtrCPhidget_getDeviceVersion) dlsym(m_handleObject, "CPhidget_getDeviceVersion");
	m_PtrCPhidget_getDeviceStatus = (PtrCPhidget_getDeviceStatus) dlsym(m_handleObject, "CPhidget_getDeviceStatus");
	m_PtrCPhidget_getLibraryVersion = (PtrCPhidget_getLibraryVersion) dlsym(m_handleObject, "CPhidget_getLibraryVersion");
	m_PtrCPhidgetSpatial_create = (PtrCPhidgetSpatial_create) dlsym(m_handleObject, "CPhidgetSpatial_create");
	m_PtrCPhidget_getDeviceType = (PtrCPhidget_getDeviceType) dlsym(m_handleObject, "CPhidget_getDeviceType");
	m_PtrCPhidget_getDeviceLabel = (PtrCPhidget_getDeviceLabel) dlsym(m_handleObject, "CPhidget_getDeviceLabel");

	m_PtrCPhidget_set_OnAttach_Handler = (PtrCPhidget_set_OnAttach_Handler) dlsym(m_handleObject, "CPhidget_set_OnAttach_Handler");
	m_PtrCPhidget_set_OnDetach_Handler = (PtrCPhidget_set_OnDetach_Handler) dlsym(m_handleObject, "CPhidget_set_OnDetach_Handler");
	m_PtrCPhidget_set_OnError_Handler = (PtrCPhidget_set_OnError_Handler) dlsym(m_handleObject, "CPhidget_set_OnError_Handler");
	m_PtrCPhidgetSpatial_set_OnSpatialData_Handler = (PtrCPhidgetSpatial_set_OnSpatialData_Handler) dlsym(m_handleObject, "CPhidgetSpatial_set_OnSpatialData_Handler");
	
	m_PtrCPhidget_getErrorDescription = (PtrCPhidget_getErrorDescription) dlsym(m_handleObject, "CPhidget_getErrorDescription");
	m_PtrCPhidget_waitForAttachment = (PtrCPhidget_waitForAttachment) dlsym(m_handleObject, "CPhidget_waitForAttachment");
	m_PtrCPhidgetSpatial_getAccelerationAxisCount = (PtrCPhidgetSpatial_getAccelerationAxisCount) dlsym(m_handleObject, "CPhidgetSpatial_getAccelerationAxisCount");
	m_PtrCPhidgetSpatial_getGyroAxisCount = (PtrCPhidgetSpatial_getGyroAxisCount) dlsym(m_handleObject, "CPhidgetSpatial_getGyroAxisCount");
	m_PtrCPhidgetSpatial_getCompassAxisCount = (PtrCPhidgetSpatial_getCompassAxisCount) dlsym(m_handleObject, "CPhidgetSpatial_getCompassAxisCount");
	m_PtrCPhidgetSpatial_getAcceleration = (PtrCPhidgetSpatial_getAcceleration) dlsym(m_handleObject, "CPhidgetSpatial_getAcceleration");
	m_PtrCPhidgetSpatial_getAccelerationMax = (PtrCPhidgetSpatial_getAccelerationMax) dlsym(m_handleObject, "CPhidgetSpatial_getAccelerationMax");
	m_PtrCPhidgetSpatial_getAccelerationMin = (PtrCPhidgetSpatial_getAccelerationMin) dlsym(m_handleObject, "CPhidgetSpatial_getAccelerationMin");
	m_PtrCPhidgetSpatial_getDataRate = (PtrCPhidgetSpatial_getDataRate) dlsym(m_handleObject, "CPhidgetSpatial_getDataRate");
	m_PtrCPhidgetSpatial_setDataRate = (PtrCPhidgetSpatial_setDataRate) dlsym(m_handleObject, "CPhidgetSpatial_setDataRate");
	m_PtrCPhidgetSpatial_getDataRateMax = (PtrCPhidgetSpatial_getDataRateMax) dlsym(m_handleObject, "CPhidgetSpatial_getDataRateMax");
	m_PtrCPhidgetSpatial_getDataRateMin = (PtrCPhidgetSpatial_getDataRateMin) dlsym(m_handleObject, "CPhidgetSpatial_getDataRateMin");
	
	// test that some choice functions aren't null
	return (bool) (m_PtrCPhidget_open && m_PtrCPhidget_close && m_PtrCPhidget_waitForAttachment && m_PtrCPhidget_set_OnAttach_Handler 
				   && m_PtrCPhidgetSpatial_getAccelerationAxisCount && m_PtrCPhidgetSpatial_getAcceleration && m_PtrCPhidgetSpatial_getDataRate);
}



void CSensorUSBPhidgets1056::closePort()
{
	if (m_handlePhidgetSpatial) {
		m_PtrCPhidget_close((CPhidgetHandle) m_handlePhidgetSpatial);
		m_PtrCPhidget_delete((CPhidgetHandle) m_handlePhidgetSpatial);
		m_handlePhidgetSpatial = NULL;
	}

	if (m_handleObject) {
#ifdef __USE_DLOPEN__
        if (dlclose(m_handleObject)) {
           fprintf(stderr, "%s: dlclose error %s\n", getTypeStr(), dlerror());
        }
#else // probably Windows - free library
   #ifdef _WIN32
        ::FreeLibrary(m_handleObject);
   #endif
#endif
		m_handleObject = NULL;
    }
	
	m_iSerialNum = 0;
	m_iVersion = 0;
	m_iNumAccelAxes = 0;
	m_iNumGyroAxes = 0;
	m_iNumCompassAxes = 0;
	m_iDataRateMax = 0;
	m_iDataRateMin = 0;
	
	setPort();
	setType();

}

bool CSensorUSBPhidgets1056::detect()
{
	int ret;
	//const char *err;
	float x,y,z; //test read_xyz
	
   setType();
   setPort();

   if (qcn_main::g_iStop) return false;
  
#ifdef __USE_DLOPEN__
	
   m_handleObject = dlopen(m_cstrDLL, RTLD_LAZY | RTLD_GLOBAL); // default
   if (!m_handleObject) {
       fprintf(stderr, "CSensorUSBPhidgets1056: dynamic library %s dlopen error %s\n", m_cstrDLL, dlerror());
       return false;
   }
	
	// check for stop signal and function pointers
	if (qcn_main::g_iStop || ! setupFunctionPointers()) return false;

#else // for Windows or not using dlopen just use the direct motionnode factory
//   m_node = MotionNodeAccel::Factory();
#endif
	
	//Declare a spatial handle
	m_handlePhidgetSpatial = NULL;
	
	//create the spatial object
	m_PtrCPhidgetSpatial_create(&m_handlePhidgetSpatial);
	if (!m_handlePhidgetSpatial) return false; // can't create spatial handle
	
	//Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
	//CPhidget_set_OnAttach_Handler((CPhidgetHandle) m_handlePhidgetSpatial, AttachHandler, NULL);
	//CPhidget_set_OnDetach_Handler((CPhidgetHandle) m_handlePhidgetSpatial, DetachHandler, NULL);
	//CPhidget_set_OnError_Handler((CPhidgetHandle) m_handlePhidgetSpatial, ErrorHandler, NULL);
	
	//Registers a callback that will run according to the set data rate that will return the spatial data changes
	//Requires the handle for the Spatial, the callback handler function that will be called, 
	//and an arbitrary pointer that will be supplied to the callback function (may be NULL)
	//CPhidgetSpatial_set_OnSpatialData_Handler(m_handlePhidgetSpatial, SpatialDataHandler, NULL);
	
	//open the spatial object for device connections
	if ((ret = m_PtrCPhidget_open((CPhidgetHandle) m_handlePhidgetSpatial, -1))) {
		closePort();
		return false;
	}
	
	// try a second to open
	double dTime = dtime();
	if((ret = m_PtrCPhidget_waitForAttachment((CPhidgetHandle)m_handlePhidgetSpatial, 1000))) {
		//m_PtrCPhidget_getErrorDescription(ret, &err);
		//fprintf(stderr, "Phidgets error waitForAttachment %d = %s\n", ret, err);
		closePort();
		return false;
	}
	
	//Display the properties of the attached spatial device
	//display_properties((CPhidgetHandle)spatial);
	
	//Set the data rate for the spatial events
	// CPhidgetSpatial_setDataRate(spatial, 16);
	//Display the properties of the attached phidget to the screen.  
	//We will be displaying the name, serial number, version of the attached device, the number of accelerometer, gyro, and compass Axes, and the current data rate
	// of the attached Spatial.

	m_PtrCPhidget_getSerialNumber((CPhidgetHandle) m_handlePhidgetSpatial, &m_iSerialNum);
	m_PtrCPhidget_getDeviceVersion((CPhidgetHandle) m_handlePhidgetSpatial, &m_iVersion);
	m_PtrCPhidgetSpatial_getAccelerationAxisCount(m_handlePhidgetSpatial, &m_iNumAccelAxes);
	m_PtrCPhidgetSpatial_getGyroAxisCount(m_handlePhidgetSpatial, &m_iNumGyroAxes);
	m_PtrCPhidgetSpatial_getCompassAxisCount(m_handlePhidgetSpatial, &m_iNumCompassAxes);
	m_PtrCPhidgetSpatial_getDataRateMax(m_handlePhidgetSpatial, &m_iDataRateMax);
	m_PtrCPhidgetSpatial_getDataRateMin(m_handlePhidgetSpatial, &m_iDataRateMin);
	
	if (m_iNumAccelAxes < 3) { // error as we should have 3 axes
		fprintf(stderr, "Error - Phidgets Accel with %d axes\n", m_iNumAccelAxes);
		closePort();
		return false;
	}

	char *strSensor = new char[256];
	sprintf(strSensor, "Phidgets 1056 v.%d (Serial # %d) USB", m_iVersion, m_iSerialNum);
	setSensorStr(strSensor);
	delete [] strSensor;
	
	fprintf(stdout, "Phidgets detected in %f milliseconds\n", (dtime() - dTime) * 1000.0);

   // OK, at this point we should be connected, so from here on out can just read_xyz until closePort()
   // set as a single sample per point
   setSingleSampleDT(false);  // mn samples itself

   // NB: closePort resets the type & port, so have to set again 
   setType(SENSOR_USB_PHIDGETS_1056);
   setPort(getTypeEnum());
	
   // one last sanity check, test the xyz reading
   if (!read_xyz(x,y,z)) {
	   closePort();
	   return false;
   }

   // last setup a detach callback function if device is removed
	m_PtrCPhidget_set_OnDetach_Handler((CPhidgetHandle) m_handlePhidgetSpatial, Phidgets1056DetachHandler, NULL);
	

   return true;
}

inline bool CSensorUSBPhidgets1056::read_xyz(float& x1, float& y1, float& z1)
{
	if (qcn_main::g_iStop || !m_handlePhidgetSpatial) return false; // invalid handle
	// NB: acceleration is in G's already so just multiply by Earth g 9.8
	double x = 0., y = 0., z = 0.;
	m_PtrCPhidgetSpatial_getAcceleration(m_handlePhidgetSpatial, 0, &x);
	m_PtrCPhidgetSpatial_getAcceleration(m_handlePhidgetSpatial, 1, &y);
	m_PtrCPhidgetSpatial_getAcceleration(m_handlePhidgetSpatial, 2, &z);
	x1 = x * EARTH_G;
	y1 = y * EARTH_G;
	z1 = z * EARTH_G;
    return true;
}

// overloaded so we can substitute with the serial # and versio of the phidget
const char* CSensorUSBPhidgets1056::getTypeStr(int iType)
{
	return getSensorStr();
}
