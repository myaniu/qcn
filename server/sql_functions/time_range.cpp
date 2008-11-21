#include <stdio.h>
#include <math.h>

float get_ang_dist(float lon1, float lat1, float lon2, float lat2);
int u_sph_2_car(float lon, float lat, float *x1, float *y1, float *z1);

int main()
{
   float angdist, dist;
   float lon1, lat1, lon2, lat2;

   printf("Enter location (lon,lat) of first point: \n");
   scanf("%f %f", &lon1,&lat1);
   printf("Enter location (lon,lat) of second point: \n");
   scanf("%f %f", &lon2,&lat2);
   
   angdist = get_ang_dist(lon1, lat1, lon2, lat2);
   printf("Distance: %f, in degrees. \n", angdist);
   
   float v_fast = 7.7;                   //fast velocity 
   float v_slow = 2.8;                   //slow velocity 
   
   float pi = 4*atan(1);                 //pi = 3.14....
   float deg_2_km = 6371*pi/180;         //Conversion for degrees to kilometers
   
   float t_min = angdist*deg_2_km/v_fast; //First expected time of P-wave
   float t_max = angdist*deg_2_km/v_slow; //Expected time of Surface Wave
   
   printf("Time Window: %f %f seconds.\n", t_min, t_max);
   
}

float get_ang_dist(float lon1, float lat1, float lon2, float lat2)
/*  | --- --------- --------- --------- --------- --------- --------- -- |  */
/*  |THIS SUBROUTINE USES USPH2CAR TO DETERMINE THE DISTANCE TWO POINTS  |  */
/*  |     GIVEN LONGITUDE, LATITUDE FOR EACH POINT ALL IN DEGREES.       |  */
/*  |     THIS SUBROUTINE DOES ALL THE RADIAN TO DEGREE CONVERSIONS.     |  */
/*  | --- --------- --------- --------- --------- --------- --------- -- |  */
{
   float pi = 4*atan(1);                     //pi = 3.14....
   float x1,x2,x3,y1,y2,y3;                  //Cartesian Coordinates
   
   u_sph_2_car(lon1,lat1,&x1,&x2,&x3);       //convert spherical to cartesian.
   u_sph_2_car(lon2,lat2,&y1,&y2,&y3);       //convert spherical to cartesian.

   float angdist = fabs(acos(x1*y1 + x2*y2 + x3*y3))*180/pi;

   return angdist;
}


int u_sph_2_car(float lon, float lat, float *x1, float *y1, float *z1)
/*  | --- --------- --------- --------- --------- --------- --------- -- |  */
/*  |THIS SUBROUTINE CONVERTS SPHERICAL COORDINATES (LON,LAT) INTO       |  */
/*  |     CARTESIAN COORDINATES (X,Y,Z), WITH RADIUS = 1.                |  */
/*  |                                                                    |  */
/*  |THIS SUBROUTINE WAS WRITTEN BY JESSE F. LAWRENCE.                   |  */
/*  |    CONTACT: jflawrence@stanford.edu                                |  */ 
/*  | --- --------- --------- --------- --------- --------- --------- -- |  */
/*  | DECLARE VARIABLES:                                                 |  */
{
   float pi,d2r;                            //PI, DEGREE TO RADIANS
   pi  = atan(1)*4;                         //PI = 3.14...
   d2r = pi/180;                            //DEGREE TO RADIANS
   *x1 = cos(lat*d2r) * cos(lon*d2r);       //CARTESIAN POSITION
   *y1 = cos(lat*d2r) * sin(lon*d2r);       //
   *z1 = sin(lat*d2r);                      //
      
   return 0;                                   
}
