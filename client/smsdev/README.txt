This demonstration system shows how sensor/accelerometers are added to the Quake Catcher Network.  The system is written in C++ and consists of a main 
program file (main.cpp), a virtual base class for QCN sensors (CSensor - csensor.h/.cpp), and an example test sensor class derived from CSensor
(CSensorTest - csensor_test.h/.cpp).  You should just need to define the methods in CSensorTest, and see the output from running the program

The program will run for 10 seconds by default, you can change the following line in main() to increase the run-time:

    const float RUN_SECONDS = 10.0f; // how many seconds to run -- max is 200 seconds (or bump up MAXI in define.h to RUN_SECONDS / DT )

You can use the Makefile on Mac or Linux:  make clean && make
If that compiles OK you will have an exectuable which you can run:  ./smstest

Or open up the project using XCode on a Mac, or open up the project using Visual Studio in Windows

The sensors are based on the CSensor class (csensor.h & .cpp) -- you should create a class that is derived from that as in the
example CSensorTest (csensor_test.h & .cpp) -- which just creates random floats.

You should not edit csensor.h & .cpp if at all possible -- everything should be handled in your derived class which inherits CSensor i.e. csensor_test.h/.cpp
Of course you may need extraneous files & libraries which you should document and include in the Makefile, projects etc.

If successful the main program will print out 10 seconds of monitoring values.

NB -- QCN needs to get data at 50Hz, i.e. for accelerometers that do hardware sampling, get a point ever .02 seconds (50 times a second i.e. 50Hz) -- but try
to sample more and average points (see "mean_xyz()" function) up to 500Hz (i.e. 10 samples averaged every .02 seconds).

