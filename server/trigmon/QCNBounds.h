//  QCNBounds class

#ifndef QCNBounds_H
#define QCNBounds_H


class QCNBounds
{

public :

    //constructors
    QCNBounds(float lon, float lat, float depth, float width,  float dx, float zrange, float dz);
    ~QCNBounds();

    void print();

    float xw;                      // Longitudinal grid width
    float yw;                      // Latitudinal grid width
    float zw;                      // Depth grid range

    float x_min;                   // Min longitude
    float x_max;                   // Max longitude
    float y_min;                   // Min latitude
    float y_max;                   // Max latitude
    float z_min;                   // Min depth
    float z_max;                   // Max depth

    float dx;                      // Longitudinal step size of grid
    float dy;                      // Latitudinal step size of grid
    float dz;                      // depth step size of grid

    int   nx;                      // Number of longitudinal grid steps
    int   ny;                      // Number of latitudinal grid steps
    int   nz;                      // Number of depth grid steps

    float lon_factor;              // Longitude/latitude factor as approach poles


private :

    void  init();

};


#endif

