//  QCNEvent class

#ifndef QCNEvent_H
#define QCNEvent_H

//#include <vector>
#include <string>
//#include <map>
//#include <blitz/array.h>
//#include <iostream>


//using namespace std;

class QCNEvent
{

public :

    enum { VERIFICATION_TEST = 0,DIFFUSION_TEST = 1, WAVE_CURRENT = 2, DEPTH_INDUCED = 3, WAVE_GROWTH = 4};
    //constructors
    QCNEvent();
    ~QCNEvent();

    /*  Data structure for events. To be used with QCN location
        program. This structure written by Jesse Lawrence (April 2010) - 
        Contact: jflawrence@stanford.edu                                                  */

    int    eventid;                   // internal Event ID
    int    qcn_quakeid;           // QCN database (qcn_quake table) ID of this event
    float  longitude, latitude, depth;        // Event Longitude, Latitude, & Depth
    double  e_time;               // Event Origin Time
    int    e_t_now;               // Event ID Time
    float  e_r2;                  // r-squared correlation
    float  magnitude; float e_std;    // Event magnitude & magnitude standard deviation
    int    e_cnt;
    float  e_msfit;                 // event misfit
    double e_t_detect;            // Time detected
    float  e_dt_detect;           // Time from event origin time to detection
    bool   dirty;    // if this is true, we changed and should update the qcn_trigger table ie for quakeid etc




private :

    void  init();




};


#endif

