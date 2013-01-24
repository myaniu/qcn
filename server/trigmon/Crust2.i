%module Crust2
%{ 
    #define SWIG_FILE_WITH_INIT
    #include "Crust2.h"
%}

class Crust2
{

public :
    enum crust_types { CRUST_KEY, CRUST_MAP, CRUST_ELEV };
    //constructor
    Crust2();

    int load();
    void  getMeanVel(const float qdep, const float qlon, const float qlat, vector<float> ) const;
    //vector<float> getMeanVel(const float qdep, const float qlon, const float qlat);

};



