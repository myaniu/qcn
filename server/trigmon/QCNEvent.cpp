#include <iostream>
#include <cassert>

#include "QCNEvent.h"


using namespace std;

//Constructors
QCNEvent::QCNEvent() :
        eventid(-1),
        qcn_quakeid(-1),
        longitude(0.0),
        latitude(0.0),
        depth(0.0),
        e_time(0.0),
        e_t_now(0),
        e_r2(0.0),
        magnitude(0.0),
        e_std(0.0),
        e_cnt(0),
        e_msfit(0.0),
        e_t_detect(0.0),
        e_dt_detect(0.0),
        dirty(false)
{
    init();
}

//Destructor
QCNEvent::~QCNEvent()
{
}

/////////////////////////////////PUBLIC METHODS::///////////////////////////////////////////




////////////////PRIVATE METHODS//////////////////////////////
void
QCNEvent::init()
{
}
