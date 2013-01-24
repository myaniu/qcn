#include <iostream>
#include <cassert>

#include "QCNBounds.h"


using namespace std;

//Constructors
QCNBounds::QCNBounds(float lon, float lat, float depth, float width,  float dx, float zrange, float dz):
        xw(width),
        yw(width),
        zw(zrange),
        x_min(lon - 0.5 * xw),
        x_max(lon + 0.5 * xw),
        y_min(lat - 0.5 * yw),
        y_max(lat + 0.5 * yw),
        z_min( (depth - 0.5 * zw)<0.0? 0.0:(depth - 0.5 * zw) ),
        z_max(depth + 0.5 * zw),
        dx(dx),
        dy(dx),
        dz(dz),
        nx(int((x_max - x_min) / dx)),
        ny(int((y_max - y_min) / dy)),
        nz(int((z_max - z_min) / dz)),
        lon_factor(1.0)
{
    init();
}

//Destructor
QCNBounds::~QCNBounds()
{
}

/////////////////////////////////PUBLIC METHODS::///////////////////////////////////////////



void
QCNBounds::print()
{
      cout << " xw   = " <<   xw  << ", yw   = " << yw    << ", zw   = " << zw    << endl;
      cout << " xmin = " << x_min << ", ymin = " << y_min << ", zmin = " << z_min << endl;
      cout << " xmax = " << x_max << ", ymax = " << y_max << ", zmax = " << z_max << endl;
      cout << " dx   = " << dx    << ", dy   = " << dy    << ", dz   = " << dz    << endl;
      cout << " nx   = " << nx    << ", ny   = " << ny    << ", nz   = " << nz    << endl;


}


////////////////PRIVATE METHODS//////////////////////////////
void
QCNBounds::init()
{

    //cout << " QCNBounds is being initialized................" << endl;

}
