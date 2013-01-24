%module  QCNTrigger
%{ 
    #define SWIG_FILE_WITH_INIT
    #include "QCNTrigger.h"
%}
%include "carrays.i"
%array_class(double, doubleArray);
%array_class(float,  floatArray);
%array_class(int,    intArray);
%include "QCNTrigger.h"

