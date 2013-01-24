//  QCNTrigger class

#ifndef QCNTrigger_H
#define QCNTrigger_H

#include <vector>
#include <string>
//#include <map>
//#include <blitz/array.h>
//#include <iostream>


using namespace std;

class DB_QCN_TRIGGER_MEMORY;

class QCNTrigger
{

public :
    enum { P = 1, S = 2};  //P or S wave
    //constructor
    QCNTrigger();
    //destructor
    ~QCNTrigger();

    void copyFromUSBTrigger( const DB_QCN_TRIGGER_MEMORY& qtm);
    void print();
    void setMagnitude();


    /*  Data structure for input trigger data to be used with QCN MySQL output & location
      program. This structure written by Jesse Lawrence (April 2010) - 
      Contact: jflawrence@stanford.edu                                                  */
    static const int N_SHORT = 255;
    int    qcn_quakeid;           // QCN database (qcn_quake table) ID of this event
    bool   posted;   // was this updated in the database

    int    hostid;                   // Host ID (Sensor number)
    int    triggerid;                   // Trigger ID

    char   db[64];               // Database
    char   file[64];              // File name
    char   result_name[64];       // qcn db result name
    float  longitude, latitude;            // Sensor location
    double time_trigger, time_received, time_est;       // Time of trigger & Time received
    float  significance, magnitude;              // Significance and magnitude (sig/noise)
    float  pgah[4],pgaz[4];       // Peak Ground Acceleration (Horizontal & vertical)
    int    c_cnt;                 // Count of correlated triggers
    vector<int>    c_ind;        // Correlated trigger IDs  (really the index into the trigger vector)
    vector<int>    c_hid;        // Correlated host IDs
    float  dis;                   // Event to station distance (km)
    int    pors;                   // 1=P, 2=S wave
    bool   dirty;    // if this is true, we changed and should update the qcn_trigger table ie for quakeid etc

 

private :

    void  init();
    //        void clear() {
    //            memset(this, 0x00, sizeof(trigger));
    //        }


};


#endif

