/*
 *  csensor_usb_phidgets_1056.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2012 Stanford University.  All rights reserved.
 *
 * Implementation file for cross-platform (Mac, Windows, Linux) Phidgets 1056 accelerometer USB sensor class
 *     http://www.phidgets.com/products.php?product_id=1056
 *   NB: some "Windows" terminology used, i.e. m_WinDLLHandle but it's really a shared object Mac dylib or Linux so of course
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

/*

const char* CSensorUSBPhidgets1056::getSensorStr() 
{
	return m_strSensor.c_str();
}

*/

CSensorUSBPhidgets1056::CSensorUSBPhidgets1056()
  : CSensor(), 
     m_WinDLLHandle(NULL), m_SymHandle(NULL), m_node(NULL)
{ 
}

CSensorUSBPhidgets1056::~CSensorUSBPhidgets1056()
{
   closePort();
}

void CSensorUSBPhidgets1056::closePort()
{
    if (getPort() > -1) {
        fprintf(stdout, "Closing %s sensor port...\n", getTypeStr());
    }
/*
    if (m_node) {
        if (m_node->is_connected() && m_node->is_reading()) {
           m_node->stop();  // if started & reading
        }
        m_node->close();
        delete m_node;
        m_node = NULL;
        setPort();
        setType();
        if (getPort() > -1) {
           fprintf(stdout, "Port closed!\n");
           fflush(stdout);
        }
    }
*/
    // close MN dll
    if (m_WinDLLHandle) {
#ifdef __USE_DLOPEN__
        if (dlclose(m_WinDLLHandle)) {
           fprintf(stderr, "%s: dlclose error %s\n", getTypeStr(), dlerror());
        }
#else // probably Windows - free library
   #ifdef _WIN32
        ::FreeLibrary(m_WinDLLHandle);
   #endif
#endif
	m_WinDLLHandle = NULL;
    }
}

bool CSensorUSBPhidgets1056::detect()
{
   setType();
   setPort();

   if (qcn_main::g_iStop) return false;
  
#ifdef __USE_DLOPEN__

	char strCWD[256];
	getcwd(strCWD, 256);
	if (!boinc_file_exists(m_cstrDLL)) {
		fprintf(stderr, "CSensorUSBPhidgets1056: dynamic library %s not found %s\n", m_cstrDLL, strCWD);
		return false;
	}
   m_WinDLLHandle = dlopen(m_cstrDLL, RTLD_LAZY | RTLD_GLOBAL); // default
   if (!m_WinDLLHandle) {
       fprintf(stderr, "CSensorUSBPhidgets1056: dynamic library %s dlopen error %s\n", m_cstrDLL, dlerror());
       return false;
   }

   if (qcn_main::g_iStop) return false;
/*
   m_SymHandle = (PtrMotionNodeAccelFactory) dlsym(m_WinDLLHandle, "MotionNodeAccel_Factory");
   if (!m_SymHandle) {
       fprintf(stderr, "CSensorUSBPhidgets1056: Could not get dlsym MotionNode Accel dylib file %s - error %s\n", sstrDLL.c_str(), dlerror());
       return false;
   }

   m_node = (*m_SymHandle)(MOTIONNODE_ACCEL_API_VERSION);
 */
#else // for Windows or not using dlopen just use the direct motionnode factory
   m_node = MotionNodeAccel::Factory();
#endif
	
/*

   if (!m_node) {
      fprintf(stderr, "CSensorUSBPhidgets1056: Could not make MotionNode Factory\n");
      return false; // not found
   }

   if (qcn_main::g_iStop) return false;

   // Detect the number of available devices.
   unsigned int count = 0;
   m_node->get_num_device(count);
   if (!count) {
       closePort();
       return false;
   }

   if (qcn_main::g_iStop) return false;

   // Set the G range. Default is 2.
   if (!m_node->set_gselect(2.0)) {
       fprintf(stderr, "CSensorUSBPhidgets1056: Could not set range on MotionNode Accel\n");
       closePort();
       return false;
   }

   // set the sample rate to 100Hz to get at least 1 & possibly 2 samples at 50Hz

   if (!m_node->set_delay(0.0f)) {
       fprintf(stderr, "CSensorUSBPhidgets1056: Could not set delay time on MotionNode Accel\n");
       closePort();
       return false;
   }
   
   if (qcn_main::g_iStop) return false;

   if (!m_node->connect()) { // connect to the sensor
       fprintf(stderr, "CSensorUSBPhidgets1056: Could not connect to MotionNode Accel\n");
       closePort();
       return false;
   }

   if (qcn_main::g_iStop) return false;

   if (!m_node->start()) {
       fprintf(stderr, "CSensorUSBPhidgets1056: Could not start MotionNode Accel\n");
       closePort();
       return false;
   }
*/
   // OK, at this point we should be connected, so from here on out can just read_xyz until closePort()
   // set as a single sample per point
   setSingleSampleDT(true);  // mn samples itself

   // NB: closePort resets the type & port, so have to set again 
   setType(SENSOR_USB_PHIDGETS_1056);
   setPort(getTypeEnum());

   return true;
}

inline bool CSensorUSBPhidgets1056::read_xyz(float& x1, float& y1, float& z1)
{
    bool bRet = false;
	/*
    //MotionNodeAccel::raw_type a[3];
    MotionNodeAccel::real_type a[3];  // for data calibrated to "g"
    a[0] = a[1] = a[2] = 0.0f;
    // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
    // MotionNode returns +/-2g values so just multiply by EARTH_G

    if (m_node && m_node->sample() && m_node->get_sensor(a)) {

#ifdef QCN_RAW_DATA	
        x1 = a[2];
        y1 = a[0];
        z1 = a[1];
#else
        x1 = a[2] * EARTH_G;
        y1 = a[0] * EARTH_G;
        z1 = a[1] * EARTH_G;
#endif
        bRet = true;
    }
	 */
    return bRet;
}



/*


 // - Spatial simple -
 // This simple example creates an spatial handle, initializes it, hooks the event handlers and opens it.  It then waits
 // for a spatial to be attached and waits for events to be fired. We preset the data rate to 16ms, but can be set higher (eg. 200)
 // in order to slow down the events to make them more visible.
 //
 // Copyright 2010 Phidgets Inc.  All rights reserved.
 // This work is licensed under the Creative Commons Attribution 2.5 Canada License. 
 // view a copy of this license, visit http://creativecommons.org/licenses/by/2.5/ca/
 
 #include <stdio.h>
 #include <phidget21.h>
 
 //callback that will run if the Spatial is attached to the computer
 int CCONV AttachHandler(CPhidgetHandle spatial, void *userptr)
 {
 int serialNo;
 CPhidget_getSerialNumber(spatial, &serialNo);
 printf("Spatial %10d attached!", serialNo);
 
 return 0;
 }
 
 //callback that will run if the Spatial is detached from the computer
 int CCONV DetachHandler(CPhidgetHandle spatial, void *userptr)
 {
 int serialNo;
 CPhidget_getSerialNumber(spatial, &serialNo);
 printf("Spatial %10d detached! \n", serialNo);
 
 return 0;
 }
 
 //callback that will run if the Spatial generates an error
 int CCONV ErrorHandler(CPhidgetHandle spatial, void *userptr, int ErrorCode, const char *unknown)
 {
 printf("Error handled. %d - %s \n", ErrorCode, unknown);
 return 0;
 }
 
 //callback that will run at datarate
 //data - array of spatial event data structures that holds the spatial data packets that were sent in this event
 //count - the number of spatial data event packets included in this event
 int CCONV SpatialDataHandler(CPhidgetSpatialHandle spatial, void *userptr, CPhidgetSpatial_SpatialEventDataHandle *data, int count)
 {
 int i;
 printf("Number of Data Packets in this event: %d\n", count);
 for(i = 0; i < count; i++)
 {
 printf("=== Data Set: %d ===\n", i);
 printf("Acceleration> x: %6f  y: %6f  x: %6f\n", data[i]->acceleration[0], data[i]->acceleration[1], data[i]->acceleration[2]);
 printf("Angular Rate> x: %6f  y: %6f  x: %6f\n", data[i]->angularRate[0], data[i]->angularRate[1], data[i]->angularRate[2]);
 printf("Magnetic Field> x: %6f  y: %6f  x: %6f\n", data[i]->magneticField[0], data[i]->magneticField[1], data[i]->magneticField[2]);
 printf("Timestamp> seconds: %d -- microseconds: %d\n", data[i]->timestamp.seconds, data[i]->timestamp.microseconds);
 }
 
 printf("---------------------------------------------\n");
 
 return 0;
 }
 
 //Display the properties of the attached phidget to the screen.  
 //We will be displaying the name, serial number, version of the attached device, the number of accelerometer, gyro, and compass Axes, and the current data rate
 // of the attached Spatial.
 int display_properties(CPhidgetHandle phid)
 {
 int serialNo, version;
 const char* ptr;
 int numAccelAxes, numGyroAxes, numCompassAxes, dataRateMax, dataRateMin;
 
 CPhidget_getDeviceType(phid, &ptr);
 CPhidget_getSerialNumber(phid, &serialNo);
 CPhidget_getDeviceVersion(phid, &version);
 CPhidgetSpatial_getAccelerationAxisCount((CPhidgetSpatialHandle)phid, &numAccelAxes);
 CPhidgetSpatial_getGyroAxisCount((CPhidgetSpatialHandle)phid, &numGyroAxes);
 CPhidgetSpatial_getCompassAxisCount((CPhidgetSpatialHandle)phid, &numCompassAxes);
 CPhidgetSpatial_getDataRateMax((CPhidgetSpatialHandle)phid, &dataRateMax);
 CPhidgetSpatial_getDataRateMin((CPhidgetSpatialHandle)phid, &dataRateMin);
 
 
 
 printf("%s\n", ptr);
 printf("Serial Number: %10d\nVersion: %8d\n", serialNo, version);
 printf("Number of Accel Axes: %i\n", numAccelAxes);
 printf("Number of Gyro Axes: %i\n", numGyroAxes);
 printf("Number of Compass Axes: %i\n", numCompassAxes);
 printf("datarate> Max: %d  Min: %d\n", dataRateMax, dataRateMin);
 
 return 0;
 }
 
 
 
 int result;
 const char *err;
 
 //Declare a spatial handle
 CPhidgetSpatialHandle spatial = 0;
 
 //create the spatial object
 CPhidgetSpatial_create(&spatial);
 
 //Set the handlers to be run when the device is plugged in or opened from software, unplugged or closed from software, or generates an error.
 CPhidget_set_OnAttach_Handler((CPhidgetHandle)spatial, AttachHandler, NULL);
 CPhidget_set_OnDetach_Handler((CPhidgetHandle)spatial, DetachHandler, NULL);
 CPhidget_set_OnError_Handler((CPhidgetHandle)spatial, ErrorHandler, NULL);
 
 //Registers a callback that will run according to the set data rate that will return the spatial data changes
 //Requires the handle for the Spatial, the callback handler function that will be called, 
 //and an arbitrary pointer that will be supplied to the callback function (may be NULL)
 CPhidgetSpatial_set_OnSpatialData_Handler(spatial, SpatialDataHandler, NULL);
 
 //open the spatial object for device connections
 CPhidget_open((CPhidgetHandle)spatial, -1);
 
 //get the program to wait for a spatial device to be attached
 printf("Waiting for spatial to be attached.... \n");
 if((result = CPhidget_waitForAttachment((CPhidgetHandle)spatial, 10000)))
 {
 CPhidget_getErrorDescription(result, &err);
 printf("Problem waiting for attachment: %s\n", err);
 return 0;
 }
 
 //Display the properties of the attached spatial device
 display_properties((CPhidgetHandle)spatial);
 
 //read spatial event data
 printf("Reading.....\n");
 
 //Set the data rate for the spatial events
 CPhidgetSpatial_setDataRate(spatial, 16);
 
 //run until user input is read
 printf("Press any key to end\n");
 getchar();
 
 //since user input has been read, this is a signal to terminate the program so we will close the phidget and delete the object we created
 printf("Closing...\n");
 CPhidget_close((CPhidgetHandle)spatial);
 CPhidget_delete((CPhidgetHandle)spatial);
 
 return 0;
 

*/
