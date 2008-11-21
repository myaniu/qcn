#include <stdio.h>
#include <math.h>

float get_ang_dist(float lon1, float lat1, float lon2, float lat2);
int u_sph_2_car(float lon, float lat, float *x1, float *y1, float *z1);
float distVincenty(float lat1, float lon1, float lat2, float lon2);

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

   angdist = distVincenty(lat1, lon1, lat2, lon2);
   printf("Distance: %f, in km. \n", angdist);

}

/*  Vincenty Inverse Solution of Geodesics on the Ellipsoid */

inline float toRad(float f1)
{
  return f1 * 3.14159265f / 180.0f;
}

/*
 * Calculate geodesic distance (in m) between two points specified by latitude/longitude 
 * (in numeric degrees) using Vincenty inverse formula for ellipsoids
 */
float distVincenty(float lat1, float lon1, float lat2, float lon2)
{
  const double a = 6378137.0f, b = 6356752.3142f,  f = 1.0f / 298.257223563f;  // WGS-84 ellipsiod
  float L = toRad(fabs(lon2-lon1));
  float U1 = atan((1.0f-f) * tan(toRad(lat1)));
  float U2 = atan((1.0f-f) * tan(toRad(lat2)));
  float sinU1 = sin(U1), cosU1 = cos(U1);
  float sinU2 = sin(U2), cosU2 = cos(U2);
  
  float lambda = L, lambdaP;
  float sinLambda, cosLambda, sinSigma, sigma, cosSigma, sinAlpha, cosSqAlpha, cos2SigmaM;
  int iterLimit = 0;

  do {
    sinLambda = sin(lambda);
    cosLambda = cos(lambda);
    sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) + 
      (cosU1*sinU2-sinU1*cosU2*cosLambda) * (cosU1*sinU2-sinU1*cosU2*cosLambda));
    if (sinSigma==0.0f) return 0.0f;  // co-incident points
    cosSigma = sinU1*sinU2 + cosU1*cosU2*cosLambda;
    sigma = atan2(sinSigma, cosSigma);
    sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
    cosSqAlpha = 1.00f - sinAlpha*sinAlpha;

    // equatorial line: cosSqAlpha=0 (§6)
    if (cosSqAlpha == 0.0f) cos2SigmaM = 0.0f;
    else cos2SigmaM = cosSigma - (2.0f*sinU1*sinU2/cosSqAlpha);

    float C = f/16.0f * cosSqAlpha*(4.0f + f * (4.0f - (3.0f*cosSqAlpha)));
    lambdaP = lambda;
    lambda = L + (1.0f-C) * f * sinAlpha *
      (sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1.0f+2.0f*cos2SigmaM*cos2SigmaM)));
  } while (fabs(lambda-lambdaP) > 1.0e-12f && ++iterLimit<20);

  if (iterLimit==20) return 0.0f; // formula failed to converge

  float uSq = cosSqAlpha * (a*a - b*b) / (b*b);
  float A = 1.0f + uSq/16384.0f*(4096.0f+uSq*(-768.0f+uSq*(320.0f-175.0f*uSq)));
  float B = uSq/1024.0f * (256.0f+uSq*(-128.0f+uSq*(74.0f-47.0f*uSq)));
  float deltaSigma = B*sinSigma*(cos2SigmaM+B/4.0f*(cosSigma*(-1.0f+2.0f*cos2SigmaM*cos2SigmaM)-
    B/6.0f*cos2SigmaM*(-3.0f+4.0f*sinSigma*sinSigma)*(-3.0f+4.0f*cos2SigmaM*cos2SigmaM)));
  
  return b*A*(sigma-deltaSigma)/1000.0f;
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
