#ifndef _CSENSOR_H_
#define _CSENSOR_H_

/*
 *  csensor.h
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University
 *
 * This file contains the base class for QCN-approved accelerometers, mostly pure virtual functions requiring definition in csensor-*
 */

#include "define.h"
#include <string>

// this is the base class for all QCN sensors
class CSensor
{
   private:
      // private member vars
      e_sensor m_iType; // what type of sensor, i.e. Thinkpad, HP, USB?
      int m_port;  // port number, -1 if no sensor opened, if >-1 then we have a port number (i.e. joystick port, Apple I/O port, subclass-specific)
      bool m_bSingleSampleDT; // set to true if just want a single sample per dt interval
      std::string m_strSensor;  // identifying string (optional, can also use getTypeStr() for a generic sensor name)  
 
      // private function
      // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
      virtual bool read_xyz(float& x1, float& y1, float& z1) = 0;   // read raw sensor data, pure virtual function subclass implemented  

   public:
     CSensor();
     virtual ~CSensor();  // virtual destructor that will basically just call closePort

     void setPort(const int iPort = -1);
     int getPort();

     void setType(e_sensor esType = SENSOR_NOTFOUND);

     const char* getSensorStr();
     void setSensorStr(const char* strIn = NULL);

     bool getSingleSampleDT();
     void setSingleSampleDT(const bool bSingle);

     // pure virtual functions that subclasses of CSensor (for specific sensor types) need to implement
     virtual bool detect() = 0;   // this detects & initializes a sensor on a Mac G4/PPC or Intel laptop, sets m_iType to 0 if not found

     // public virtual functions implemented in CSensor but can be overridden
     virtual void closePort(); // closes the port if open
     virtual const e_sensor getTypeEnum(); // return the iType member variable
     virtual const char* getTypeStr();  // return the iType member variable
     virtual const char* getTypeStrShort();  // return the iType member variable
     virtual bool mean_xyz();   // mean sensor data, implemented here but can be overridden
};

#endif

