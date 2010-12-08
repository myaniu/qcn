package edu.stanford.qcn.qcnlive;

/**
 *  
 * @author Carl Christensen
 *
 * This is the base interface for all the derived sensors on android
 * (right now just implemented by the Android sensor ie built in to the device
 *
 */
public class CSensor {
	protected 
	   int m_iType; // what type of sensor, i.e. Thinkpad, HP, USB?

	private
      // private member vars
      int m_port;  // port number, -1 if no sensor opened, if >-1 then we have a port number (i.e. joystick port, Apple I/O port, subclass-specific)
      boolean m_bSingleSampleDT; // set to true if just want a single sample per dt interval
      String m_strSensor;  // identifying string (optional, can also use getTypeStr() for a generic sensor name)

      // private function
      // note that x/y/z should be scaled to +/- 2g, return values as +/- 2.0f*EARTH_G (in define.h: 9.78033 m/s^2)
      virtual boolean read_xyz(float& x1, float& y1, float& z1) = 0;   // read raw sensor data, pure virtual function subclass implemented

          static map<int, CSensorType> m_map;  // map enum ID's to sensor name

   public
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
     bool detect();   // this detects & initializes a sensor on a Mac G4/PPC or Intel laptop, sets m_iType to 0 if not found

         // get sensor id or string based on the map of sensors (m_map private member var initialized in constructor)
         const char* getTypeStr(int iType = -1);  // return the sensor name for this iType
         const char* getTypeStrShort(int iType = -1);  // return the sensor name (short) for this iType or m_Type if not entered
         // const int getID();

     // public virtual functions implemented in CSensor but can be overridden
     virtual void closePort(); // closes the port if open
     virtual e_sensor getTypeEnum(); // return the iType member variable

     virtual bool mean_xyz();   // mean sensor data, implemented here but can be overridden
}
