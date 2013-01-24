//  QCN class

#ifndef QCN_H
#define QCN_H

#include <vector>
#include <string>
#include <iostream>
#include "boinc_db.h"
//extern DB_CONN trigmem_db;
#include "sched_config.h"

#include "QCNTrigger.h"
//#include "QCNTrigger.cpp"
//#include <string>


using namespace std;

//forward decleartion
//class QCNTrigger;
class QCNEvent;
class DB_CONN;
struct SCHED_CONFIG;
class Crust2;



class QCN
{

public :
    enum eOutput { OUT_EVENT, OUT_STATION, OUT_INTENSITY_MAP, OUT_CONT_TIME, OUT_CONT_LABEL, OUT_TIME_SCATTER };
    //constructor
    QCN(const Crust2& crust2, bool isDBActivate=true);
    //destructor
    ~QCN();

    void execute();
    void detectEvent();

    void setSleepInterval(double dSleepInterval){
        if (dSleepInterval < 0) {
            _dSleepInterval = TRIGGER_SLEEP_INTERVAL;
        } else {
            _dSleepInterval = dSleepInterval;
        }
    }

    /////////////////////SET METHODS////////////////////
    void setTriggerTimeInterval(int iTriggerTimeInterval){
        if (iTriggerTimeInterval < 0) {
            _iTriggerTimeInterval = TRIGGER_TIME_INTERVAL;
        } else {
            _iTriggerTimeInterval = iTriggerTimeInterval;
        }
    }


    void setTriggerDeleteInterval(int iTriggerDeleteInterval){
        _iTriggerDeleteInterval = iTriggerDeleteInterval;
    }
    
    
    void setEVENT_URL_BASE(const string& str){
         EVENT_URL_BASE = str;
    }
    
    void setEVENT_PATH(const string& str){
         EVENT_PATH = str;
    }
    
    void setGMT_MAP_PHP(const string& str){
         GMT_MAP_PHP = str;
    }
    
    void setEMAIL_PATH(const string& str){
         EMAIL_PATH = str;
    }
    
    void setEMAIL_DIR(const string& str){
         EMAIL_DIR = str;
    }
    
    void setEMAIL_INC(const string& str){
         EMAIL_INC = str;
    }
    
    void addTrigger(const QCNTrigger& trig){
         _vt.push_back(trig);
    }     
        
    void isEmailOut(const bool email){
        _EMAIL = email;
    }
    
    void isIntensityMapGMTOut(const bool gmt){
        _mapGMT = gmt;
    }
    
    void isUpdateQuakeOut(const bool gmt){
         _isUpdateQuakeOut = gmt;
    }
    
    
    
private :

    void  init();
    int   DBOpen();
    void  DBClose();
    bool eventLocate(const bool bEventFound,  QCNEvent& e, const int ciOff);

    float ang_dist_km(const float lon1, const float lat1, const float lon2, const float lat2);
    float correlate(const vector<float>& datx, vector<float>& daty, const int ndat);
    float stdDev(const vector<float>& dat, const int ndat, const float dat_ave);   
    float stdDev(const vector<float>& dat, const int ndat);
    float average(const vector<float>& dat, const int ndat);
    void  estimateMagnitude(QCNEvent& e, const int ciOff);
    void  updateQuake(const bool bInsert, QCNEvent& e, const int ciOff);
    bool  sendTriggerFileRequest(const char* strFile, const char* strResult, const int hostid, const char* strDB);
    int   intensityMap(const bool bInsertEvent, QCNEvent& e, const int ciOff);
    int   intensityMapGMT(const char* epath);
    //void  php_event_page(const QCNEvent& e, const char* epath);
    void  php_event_email(const QCNEvent& e, const char* epath);
    int getTriggers();
    
    void  do_delete_trigmem();

    const Crust2&  _crust2;
    const int ID_USB_SENSOR_START;
    const int C_CNT_MIN;
    const int N_LONG;
    const int T_MAX;
    const double Vs;         // S wave velocity (km/s)
    const double Vp;         // P wave velocity (km/s)
    const double D_MAX;       // Maximum distance between triggers in km
    const int MAX_PATH;
    const double TRIGGER_SLEEP_INTERVAL;
    const int    TRIGGER_TIME_INTERVAL;
    string  EVENT_URL_BASE;
    string  EVENT_PATH;
    string  PHP_CMD;
    string GMT_MAP_PHP;
    string CSHELL_CMD;
    string EMAIL_PATH;
    string EMAIL_DIR;
    string EMAIL_INC;

    long int getMySQLUnixTime();

    vector<QCNTrigger>  _vt;
    vector<QCNEvent>    _ve;
    double _dSleepInterval;
    int    _iTriggerTimeInterval;
    int    _iTriggerDeleteInterval;
    bool   _EMAIL;
    bool   _mapGMT;
    bool   _isUpdateQuakeOut;
    bool   _isDBActivate;
    //      DB_CONN       _boincDB;
    //       DB_CONN       _trigmemDB;
    //      SCHED_CONFIG  _config;

};


#endif

