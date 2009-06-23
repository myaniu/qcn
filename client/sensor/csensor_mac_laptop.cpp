/*
 *  csensor_mac_laptop.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University.  All rights reserved.
 *
 * Implementation file for QCN Mac Laptop sensor class
 */

#include "main.h"
#include "csensor_mac_laptop.h"
#include <AvailabilityMacros.h>
#include <IOKit/IOKitLib.h>

CSensorMacLaptop::CSensorMacLaptop()
  : CSensor()
{ 
}

CSensorMacLaptop::~CSensorMacLaptop()
{
  closePort();
}

void CSensorMacLaptop::closePort()
{
  if (getPort() > -1) {
    fprintf(stdout, "Closing SMS sensor port...\n");
    setPort(-1);
    IOServiceClose(getPort());    // close port if used
    fprintf(stdout, "Port closed!\n");
    fflush(stdout);
  }
}

bool CSensorMacLaptop::detect()
{
/*  Initializes the sudden motion sensor by attempting to access each kind of 
 *  sensor, and thus figuring out which works.
 */
    int i;
    setType(SENSOR_NOTFOUND);
    for ( i = 1; i < 4; i++ ) {
        init_ppc(i);
        if ((int) getTypeEnum() == i) {
            fprintf(stdout, "SMS KERNEL: %d %d %s\n", (int) getTypeEnum(), m_iKernel, getTypeStr());
            return true; // found it, we can return
        }
    }
    i=4;
    init_intel(i);
    if ((int) getTypeEnum() == 4) {
        fprintf(stdout, "SMS KERNEL: %d %d %s\n", (int) getTypeEnum(), m_iKernel, getTypeStr());
        return true; // found it, we can return
    }
    return false;
}

void CSensorMacLaptop::init_intel(const int iType)
{
/*  Initializes the MacBookPro - or equivalent - Sudden Motion Sensor
 *    1) Sets up data structure for device
 *    2) Determines if port even exists
 *    3) Checks if the device is available
 *    4) Attempts to open the sensor
 *    5) Attempts to read the sensor
 */
      kern_return_t result;                    /* PORT KERNEL VARIABLES    */
      mach_port_t   masterPort;
      io_iterator_t iterator;
      io_object_t   aDevice;
      io_connect_t  dataPort;
	  
      m_iStruct = 1;                            /* PORT KERNEL VARIABLES    */
      m_iKernel = 5;
      setType(SENSOR_NOTFOUND);
      setSensorStr("SMCMotionSensor");

	
#if MAC_OS_X_VERSION_10_5
	size_t structureInputSize;          /* DATA STRUCTURE SIZE      */
	size_t structureOutputSize;
#else
	IOItemCount structureInputSize;          /* DATA STRUCTURE SIZE      */
	IOByteCount structureOutputSize;
#endif

      struct stDataMacIntel inputStructure;         /*  DATA STRUCTURE           */
      struct stDataMacIntel outputStructure;

      result = IOMasterPort(MACH_PORT_NULL, &masterPort);
      CFMutableDictionaryRef matchingDictionary = IOServiceMatching(getSensorStr());
      result = IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);
      if (result != KERN_SUCCESS) {
          fprintf(stdout, "Could not get services for SMS\n");
          return;
      };

      aDevice = IOIteratorNext(iterator);    /*CHECK THAT DEVICE IS AVAILABLE    */
      IOObjectRelease(iterator);
      if (aDevice == 0) {
          fprintf(stdout, "Could not get iterator for SMS\n");
          return;
      };
      
      result = IOServiceOpen(aDevice, mach_task_self(), 0, &dataPort);
      IOObjectRelease(aDevice);
      if(result != KERN_SUCCESS) {
         fprintf(stdout, "Could not open motion sensor device\n");
         return;
      };
      setPort(dataPort);

      structureInputSize = sizeof(stDataMacIntel);
      structureOutputSize = sizeof(stDataMacIntel);

      memset(&inputStructure, m_iStruct, sizeof(inputStructure));
      memset(&outputStructure,      0x00, sizeof(outputStructure));

#ifdef __LP64__ // MAC_OS_X_VERSION_10_5
// 'kern_return_t IOConnectCallStructMethod(mach_port_t, uint32_t, const void*, size_t, void*, size_t*)'

    result = IOConnectCallStructMethod(dataPort, 
									   m_iKernel,
								    &inputStructure, 
									   structureInputSize,
								   &outputStructure, 
									   &structureOutputSize );
#else
	result = IOConnectMethodStructureIStructureO(
												 dataPort,
												 m_iKernel,			           /* index to kernel ,5,21,24*/
												 structureInputSize,
												 &structureOutputSize,
												&inputStructure,
												 &outputStructure
												 );                                           /* get original position   */
#endif
	
      if (result != KERN_SUCCESS) {                /* NO MASS POSITION SO RETURN*/
           setPort(-1);
           printf("no coords returned, error!\n");
           return;
      };
      fprintf(stdout, "Intel Macbook Pro compatible motion sensor detected!\n");
      fflush(stdout);
      setType(SENSOR_MAC_INTEL); // must have been detected, so set the member var for iType
}

void CSensorMacLaptop::init_ppc(const int iType)
{
/*  Initiallizes the PowerBook & iBook Sudden Motion Sensors:
 *    1) Sets up data structure for device
 *    2) Determines if port even exists
 *    3) Checks if the device is available
 *    4) Attempts to open the sensor
 *    5) Attempts to read the sensor
 */
      kern_return_t result;                    /* time variables            */
      mach_port_t   masterPort;
      io_iterator_t iterator;
      io_object_t   aDevice;
      io_connect_t  dataPort;
      
      m_iStruct = 0;
      switch (iType) {
      case 1:
          m_iKernel = 21;
          setSensorStr("IOI2CMotionSensor");
	  break;
      case 2:
          m_iKernel = 21;
          setSensorStr("IOI2CMotionSensor");
	  break;
      case 3:
          m_iKernel = 21;
          setSensorStr("PMUMotionSensor");
          break;
      default:
	  setType(SENSOR_NOTFOUND);
	  m_iKernel = 0;
	  setSensorStr();
      }

      IOItemCount structureInputSize;          /*  */
      IOByteCount structureOutputSize;

      struct stDataMacPPC inputStructure;              /*  */
      struct stDataMacPPC outputStructure;

      result = IOMasterPort(MACH_PORT_NULL, &masterPort);
      CFMutableDictionaryRef matchingDictionary = IOServiceMatching(getSensorStr());
      result = IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);
      if (result != KERN_SUCCESS) {
//          fputs("IOServiceGetMatchingServices returned error.\n", stderr);
          return;
      };


      aDevice = IOIteratorNext(iterator);   /*CHECK THAT DEVICE IS AVAILABLE    */
      IOObjectRelease(iterator);
      if (aDevice == 0) {
          fprintf(stdout, "No motion sensor available of type # %d %s\n", iType, getSensorStr());
          return;
      };
      
      result = IOServiceOpen(aDevice, mach_task_self(), 0, &dataPort);
      IOObjectRelease(aDevice);
      if(result != KERN_SUCCESS) {
         setPort(-1);
         fprintf(stdout, "Could not open motion sensor device\n");
         return;
      };
      setPort(dataPort);

      structureInputSize = sizeof(struct stDataMacPPC);
      structureOutputSize = sizeof(struct stDataMacPPC);
      memset(&inputStructure,m_iStruct, sizeof(inputStructure));
      memset(&outputStructure,      0, sizeof(outputStructure));

#ifdef __LP64__ // MAC_OS_X_VERSION_10_5
	// 'kern_return_t IOConnectCallStructMethod(mach_port_t, uint32_t, const void*, size_t, void*, size_t*)'
	
    result = IOConnectCallStructMethod(dataPort, 
									   m_iKernel,
									   &inputStructure, 
									   structureInputSize,
									   &outputStructure, 
									   &structureOutputSize );
#else
	result = IOConnectMethodStructureIStructureO(
												 dataPort,
												 m_iKernel,			           /* index to kernel ,5,21,24*/
												 structureInputSize,
												 &structureOutputSize,
												 &inputStructure,
												 &outputStructure
												 );                                           /* get original position   */
#endif
	if (result != KERN_SUCCESS) {                /* NO MASS POSITION SO RETURN*/
           fprintf(stdout, "no coords\n");
           return;
      };  

      setType((e_sensor) iType);
};

inline bool CSensorMacLaptop::read_xyz(float& x1, float& y1, float& z1)
{
      kern_return_t result;                    /* time variables            */
      IOItemCount structureInputSize;          /*  */
      IOByteCount structureOutputSize;

      if (getTypeEnum() == SENSOR_MAC_INTEL)  {
         struct stDataMacIntel inputStructureIntel;              /*  */
         struct stDataMacIntel outputStructureIntel;
         structureInputSize = sizeof(struct stDataMacIntel);
         structureOutputSize = sizeof(struct stDataMacIntel);
         memset(&inputStructureIntel,  0x00, sizeof(inputStructureIntel));  // this was set to 0x01 originally, why?
         memset(&outputStructureIntel, 0x00, sizeof(outputStructureIntel));

#ifdef __LP64__ // MAC_OS_X_VERSION_10_5
		  // 'kern_return_t IOConnectCallStructMethod(mach_port_t, uint32_t, const void*, size_t, void*, size_t*)'
		  
		  result = IOConnectCallStructMethod(dataPort, 
											 m_iKernel,
											 &inputStructure, 
											 structureInputSize,
											 &outputStructure, 
											 &structureOutputSize );
#else
		  result = IOConnectMethodStructureIStructureO(
													   dataPort,
													   m_iKernel,			           /* index to kernel ,5,21,24*/
													   structureInputSize,
													   &structureOutputSize,
													   &inputStructure,
													   &outputStructure
													   );                                           /* get original position   */
#endif
		  x1 = outputStructureIntel.x;                     /* SIDE-TO-SIDE POSITION         */
         y1 = outputStructureIntel.y;                     /* FRONT-TO-BACK POSITION        */
         z1 = outputStructureIntel.z;                     /* VERTICAL POSITION  */
      }
      else {
         struct stDataMacPPC inputStructurePPC;              /*  */
         struct stDataMacPPC outputStructurePPC;
         structureInputSize = sizeof(struct stDataMacPPC);
         structureOutputSize = sizeof(struct stDataMacPPC);
         memset(&inputStructurePPC,  0x00, sizeof(inputStructurePPC));
         memset(&outputStructurePPC, 0x00, sizeof(outputStructurePPC));

#ifdef __LP64__ // MAC_OS_X_VERSION_10_5
		  // 'kern_return_t IOConnectCallStructMethod(mach_port_t, uint32_t, const void*, size_t, void*, size_t*)'
		  
		  result = IOConnectCallStructMethod(dataPort, 
											 m_iKernel,
											 &inputStructure, 
											 structureInputSize,
											 &outputStructure, 
											 &structureOutputSize );
#else
		  result = IOConnectMethodStructureIStructureO(
													   dataPort,
													   m_iKernel,			           /* index to kernel ,5,21,24*/
													   structureInputSize,
													   &structureOutputSize,
													   &inputStructure,
													   &outputStructure
													   );                                           /* get original position   */
#endif
		  x1 = outputStructurePPC.x;                     /* SIDE-TO-SIDE POSITION         */
         y1 = outputStructurePPC.y;                     /* FRONT-TO-BACK POSITION        */
         z1 = outputStructurePPC.z;                     /* VERTICAL POSITION  */
      } 
    
      // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
      // mac's seem to be 256 = 1g, so divide each value by 256 * multiply by EARTH_G
      x1 = (x1 / 256.0) * EARTH_G;
      y1 = (y1 / 256.0) * EARTH_G;
      z1 = (z1 / 256.0) * EARTH_G;
 
      return true;
}

