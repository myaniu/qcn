//  Crust2 class

#ifndef Crust2_H
#define Crust2_H

#include <vector>
#include <string>


using namespace std;

class Crust2
{

public :
    enum crust_types { CRUST_KEY, CRUST_MAP, CRUST_ELEV };
    //constructor
    Crust2();
    //destructor
    ~Crust2();

    int load();
    void getMeanVel(const float qdep, const float qlon, const float qlat, vector<float>& v) const;
    
    //vector<float> getMeanVel(const float qdep, const float qlon, const float qlat);


private :

    void  init();
    int type(const float lon, const float lat) const;

    string _CKF;
    string _CMF;
    string _CEF;


    static  const int  mx_cr_type = 360;  // Number of crust2.0 crust types
    static  const int  mx_cr_lt   =  90;  // Number of latitude nodes
    static  const int  mx_cr_ln   = 180;  // Number of longitude nodes
    static  const int  dx_cr = 2;         // Sampling interval of Crust2.0
    struct cr_key
    {
        float vp[8];
        float vs[8];
        float rh[8];
        float dp[8];
    };

    struct cr_mod
    {
        float  elev[mx_cr_lt][mx_cr_ln];
        int    ikey[mx_cr_lt][mx_cr_ln];
        float  lat[mx_cr_lt];
        float  lon[mx_cr_ln];
    };



    struct cr_mod   _crm;                        //The Curst2.0 model map
    struct cr_key   _crk[mx_cr_type];            //The Crust2.0 model key







};


#endif

