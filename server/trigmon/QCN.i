%module  QCN
%{ 
    #define SWIG_FILE_WITH_INIT
    #include "QCN.h"
%}

%include "Crust2.i"
%include "QCNTrigger.i"
%include "QCNEvent.i"
%include "QCNBounds.i"
%include "std_string.i"

class QCN
{

public :
    enum eOutput { OUT_EVENT, OUT_STATION, OUT_INTENSITY_MAP, OUT_CONT_TIME, OUT_CONT_LABEL, OUT_TIME_SCATTER };
    //constructor
    QCN(const Crust2& crust2, bool);
    //destructor
    ~QCN();

    void execute();
    void detectEvent();

    void setSleepInterval(double dSleepInterval);
    void setTriggerTimeInterval(int iTriggerTimeInterval);
    void setTriggerDeleteInterval(int iTriggerDeleteInterval);
 
    void setEVENT_URL_BASE(const std::string& str);    
    void setEVENT_PATH    (const std::string& str);    
    void setGMT_MAP_PHP   (const std::string& str);
    void setEMAIL_PATH    (const std::string& str);
    void setEMAIL_DIR     (const std::string& str);
    void setEMAIL_INC     (const std::string& str);
    void addTrigger(const QCNTrigger& trig);
    void isEmailOut(const bool email);
    void isIntensityMapGMTOut(const bool gmt);
    void isUpdateQuakeOut(const bool gmt);
};



