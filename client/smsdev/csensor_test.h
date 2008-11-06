#ifndef _CSENSOR_TEST_H_
#define _CSENSOR_TEST_H_

/*
 *  csensor-test.h
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University
 *
 * This file contains an example usage of the CSensor class for QCN
 */

#include "csensor.h"

// this is an example declaration of a class derived from CSensor
// you probably will not need to modify CSensor -- just override the necessary functions here
class CSensorTest  : public CSensor
{
   private:   
      // private member vars if needed

      // you will need to define a read_xyz function (pure virtual function in CSensor)
      virtual bool read_xyz(float& x1, float& y1, float& z1);  

   public:
      // public member vars if needed

      CSensorTest();
      virtual ~CSensorTest();

      virtual void closePort(); // closes the port if open
      virtual bool detect();   // this detects & initializes the sensor

     // note that CSensor defines a mean_xyz method that uses the read_xyz declared above -- you shouldn't need to override mean_xyz but that option is there
};

#endif

