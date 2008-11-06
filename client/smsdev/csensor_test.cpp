/*
 *  csensor_test.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University.  All rights reserved.
 *
 * Implementation file for a new QCN sensor derived from the CSensor class
 *
 * Ideally you will just need to edit & implement this class to add a new sensor to QCN (i.e. via csensor_test.h & .cpp)
 */

#include "define.h"
#include "csensor_test.h"

CSensorTest::CSensorTest()
  : CSensor()
{  // any initializations for public/private member vars here 
}

CSensorTest::~CSensorTest()
{  // at the very least close the port if open!
  closePort();
}

void CSensorTest::closePort()
{
  if (getPort() > -1) {
    fprintf(stdout, "Closing SMS sensor port...\n");

    // what ever port closure logic is required can go here...

    fprintf(stdout, "Port closed!\n");
    fflush(stdout);
    setPort(); // sets to -1 i.e. inactive/notfound
  }
}

bool CSensorTest::detect()
{
   // this is where you try to detect & open the sensor
   // it's pretty obvious -- if detected/opened return true, if not return false
   // if detected you'll want to set the port & type (see csensor.h & define.h for types)
   // we'll just call it SENSOR_TEST for now and set the port to the enum value
   // (the m_port member var can be used to set an int number to a specific port, if necessary)

    setType(SENSOR_TEST);
    setPort((int) getTypeEnum());
	
    fprintf(stdout, "Test sensor detected.\n");
    return true;
}

// now all you need to do is define how to read x/y/z values from the sensor
// for now just return random numbers
bool CSensorTest::read_xyz(float& x1, float& y1, float& z1)
{
    // return true if read OK, false if error
    // the real timing work, i.e. the 50Hz sampling etc is done in mean_xyz in CSensor -- you shouldn't need (or want!) to edit that

    // QCN will try and sample up to 10 times every .02 seconds by default (up to 500Hz of samples which is software downsampled to 50Hz)
    // You can also set a boolean flag if your hardware does the sub-sampling and we just need to read at 50Hz
    // If your hardware supports it, just add the following line to the detect() logic above:
    //    setSingleSampleDT(true);

    // try to also scale your output (xyz) to be a +/- 2g range

    // also if you only have a 2-axis accelerometer don't forget to set z1 to 0.0f!

    bool bRetVal = true;
    try {
		x1 = (float) ((rand() % 100) - 50) / 25.0f;    // NB this will give random floats between -2.0 & 2.0
		y1 = (float) ((rand() % 100) - 50) / 25.0f;    // NB this will give random floats between -2.0 & 2.0
		z1 = (float) ((rand() % 100) - 50) / 25.0f;    // NB this will give random floats between -2.0 & 2.0
    }
    catch(...) {
		bRetVal = false;
    }
	//usleep(1000000);  // uncommenting this line will force timing errors since it sleeps a second every read!
    return bRetVal;
}

