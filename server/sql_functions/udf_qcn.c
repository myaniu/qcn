/*
** QCN functions that are dynamically loaded into mysqld, based on mysql source code sql/udf_example.c
** see README.txt for usage
**
*/

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <math.h>
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>		/* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>
#endif
#endif
#include <mysql.h>
#include <ctype.h>

// need dlopen to dynamically load symbols

#ifdef HAVE_DLOPEN

// lat_lon_distance_m --- find the distance in meters between two lat/lon points
my_bool lat_lon_distance_m_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void lat_lon_distance_m_deinit(UDF_INIT *initid);
double distance_vincenty(const double lat1, const double lon1, const double lat2, const double lon2, char *is_null);
double lat_lon_distance_m(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

// lat_lon_distance_m --- find the distance in meters between two lat/lon points
my_bool quake_hit_test_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void quake_hit_test_deinit(UDF_INIT *initid);
longlong quake_hit_test(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

/*************************************************************************
** Example of init function
** Arguments:
** initid	Points to a structure that the init function should fill.
**		This argument is given to all other functions.
**	my_bool maybe_null	1 if function can return NULL
**				Default value is 1 if any of the arguments
**				is declared maybe_null.
**	unsigned int decimals	Number of decimals.
**				Default value is max decimals in any of the
**				arguments.
**	unsigned int max_length  Length of string result.
**				The default value for integer functions is 21
**				The default value for real functions is 13+
**				default number of decimals.
**				The default value for string functions is
**				the longest string argument.
**	char *ptr;		A pointer that the function can use.
**
** args		Points to a structure which contains:
**	unsigned int arg_count		Number of arguments
**	enum Item_result *arg_type	Types for each argument.
**					Types are STRING_RESULT, REAL_RESULT
**					and INT_RESULT.
**	char **args			Pointer to constant arguments.
**					Contains 0 for not constant argument.
**	unsigned long *lengths;		max string length for each argument
**	char *maybe_null		Information of which arguments
**					may be NULL
**
** message	Error message that should be passed to the user on fail.
**		The message buffer is MYSQL_ERRMSG_SIZE big, but one should
**		try to keep the error message less than 80 bytes long!
**
** This function should return 1 if something goes wrong. In this case
** message should contain something usefull!
**************************************************************************/

my_bool lat_lon_distance_m_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{ // args should be 4 floats or double ("real")
  if (args->arg_count != 4) 
  {
    sprintf(message,"Wrong %d arguments to lat_lon_distance_m - need two lat/lon real pairs", args->arg_count);
    return 1;
  }

  // force args to be real
  args->arg_type[0] = REAL_RESULT;
  args->arg_type[1] = REAL_RESULT;
  args->arg_type[2] = REAL_RESULT;
  args->arg_type[3] = REAL_RESULT;

  args->maybe_null[0] = args->maybe_null[1] = args->maybe_null[2] = args->maybe_null[3] = 0x00;
  initid->maybe_null = 0; // return null on error

  return 0;
}

my_bool quake_hit_test_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{ // args should be 8 floats or double ("real")
  if (args->arg_count != 8) 
  {
    sprintf(message,"params (tr_lat, tr_lon, tr_time_utc, tr_sensor, qu_lat, qu_lon, qu_time_utc, qu_mag)");
    return 1;
  }

  // force args to be real
  args->arg_type[0] = REAL_RESULT;
  args->arg_type[1] = REAL_RESULT;
  args->arg_type[2] = REAL_RESULT;
  args->arg_type[3] = REAL_RESULT;
  args->arg_type[4] = REAL_RESULT;
  args->arg_type[5] = REAL_RESULT;
  args->arg_type[6] = REAL_RESULT;
  args->arg_type[7] = REAL_RESULT;

  args->maybe_null[0] = args->maybe_null[1] = args->maybe_null[2] = args->maybe_null[3] = 0x00;
  args->maybe_null[4] = args->maybe_null[5] = args->maybe_null[6] = args->maybe_null[7] = 0x00;

  initid->maybe_null = 0; // never null, either 0 or 1 (FALSE/TRUE)

  return 0;
}

/****************************************************************************
** Deinit function. This should free all resources allocated by
** this function.
** Arguments:
** initid	Return value from xxxx_init
****************************************************************************/

void lat_lon_distance_m_deinit(UDF_INIT *initid)
{
}

void quake_hit_test_deinit(UDF_INIT *initid)
{
}

/***************************************************************************
/***************************************************************************
** UDF double function.
** Arguments:
** initid       Structure filled by xxx_init
** args         The same structure as to xxx_init. This structure
**              contains values for all parameters.
**              Note that the functions MUST check and convert all
**              to the type it wants!  Null values are represented by
**              a NULL pointer
** is_null      If the result is null, one should store 1 here.
** error        If something goes fatally wrong one should store 1 here.
**
** This function should return the result.
***************************************************************************/

/*  Vincenty Inverse Solution of Geodesics on the Ellipsoid */
#define PI 3.14159265f
#define TO_RAD(Z) ((Z) * PI / 180.0f)

/*
 * Calculate geodesic distance (in m) between two points specified by latitude/longitude 
 * (in numeric degrees) using Vincenty inverse formula for ellipsoids
 */
// the implementation of lat_lon_distance_m

double distance_vincenty(const double lat1, const double lon1, const double lat2, const double lon2, char *is_null)
{
  const double a = 6378137.0f, b = 6356752.3142f,  f = 1.0f / 298.257223563f;  // WGS-84 ellipsiod

  double L = TO_RAD(fabs(lon2-lon1));
  double U1 = atan((1.0f-f) * tan(TO_RAD(lat1)));
  double U2 = atan((1.0f-f) * tan(TO_RAD(lat2)));
  double sinU1 = sin(U1), cosU1 = cos(U1);
  double sinU2 = sin(U2), cosU2 = cos(U2);

  double lambda = L, lambdaP;
  double sinLambda, cosLambda, sinSigma, sigma, cosSigma, sinAlpha, cosSqAlpha, cos2SigmaM;
  int iterLimit = 0;

  *is_null = 0x00;

  do {
    sinLambda = sin(lambda);
    cosLambda = cos(lambda);
    sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) +
      (cosU1*sinU2-sinU1*cosU2*cosLambda) * (cosU1*sinU2-sinU1*cosU2*cosLambda));
    if (sinSigma==0.0f) {
        return 0.0f;  // co-incident points
    }
    cosSigma = sinU1*sinU2 + cosU1*cosU2*cosLambda;
    sigma = atan2(sinSigma, cosSigma);
    sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
    cosSqAlpha = 1.00f - sinAlpha*sinAlpha;

    // equatorial line: cosSqAlpha=0 (§6)
    if (cosSqAlpha == 0.0f) cos2SigmaM = 0.0f;
    else cos2SigmaM = cosSigma - (2.0f*sinU1*sinU2/cosSqAlpha);

    double C = f/16.0f * cosSqAlpha*(4.0f + f * (4.0f - (3.0f*cosSqAlpha)));
    lambdaP = lambda;
    lambda = L + (1.0f-C) * f * sinAlpha *
      (sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1.0f+2.0f*cos2SigmaM*cos2SigmaM)));
  } while (fabs(lambda-lambdaP) > 1.0e-12f && ++iterLimit<20);

  if (iterLimit==20) {
      *is_null = 0x01;
      return 0.0f; // formula failed to converge
  }

  double uSq = cosSqAlpha * (a*a - b*b) / (b*b);
  double A = 1.0f + uSq/16384.0f*(4096.0f+uSq*(-768.0f+uSq*(320.0f-175.0f*uSq)));
  double B = uSq/1024.0f * (256.0f+uSq*(-128.0f+uSq*(74.0f-47.0f*uSq)));
  double deltaSigma = B*sinSigma*(cos2SigmaM+B/4.0f*(cosSigma*(-1.0f+2.0f*cos2SigmaM*cos2SigmaM)-
    B/6.0f*cos2SigmaM*(-3.0f+4.0f*sinSigma*sinSigma)*(-3.0f+4.0f*cos2SigmaM*cos2SigmaM)));

  return b*A*(sigma-deltaSigma);
}

double lat_lon_distance_m(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  const double lat1 = *((double*) args->args[0]);
  const double lon1 = *((double*) args->args[1]);
  const double lat2 = *((double*) args->args[2]);
  const double lon2 = *((double*) args->args[3]);

  // check values of lat/lon
  // check latitude
  if ( lat1 < -90.0f || lat2 < -90.0f
    || lat1 > 90.0f  || lat2 > 90.0f) {
    //strcpy(message,"Latitude must be in the range [-90.0, 90.0]");
    return 0.0f;
  }

  // check longitude
  if ( lon1 < -180.0f || lon2 < -180.0f
    || lon1 > 180.0f  || lon2 > 180.0f) {
    //strcpy(message,"Longitude must be in the range [-180.0, 180.0]");
    return 0.0f;
  }

  return distance_vincenty(lat1, lon1, lat2, lon2, is_null);
}

/* quake hit test - return 1 if the host trigger was within range of the quake, 0 otherwise
  // parameters will have been verified by the _init():
  //  args: trig_lat, trig_lng, trig_time_utc, trig_sensor, quake_lat, quake_lon, quake_time_utc, quake_mag

  // note that times are UTC-coordinated and are the number of seconds since the 'epoch' (1/1/1970)

iSensor is:

mysql> select * from qcn_sensor;
+----+--------+---------------------------+
| id | is_usb | description               |
+----+--------+---------------------------+
|  0 |      0 | Not Found                 |
|  1 |      0 | Mac PPC 1                 |
|  2 |      0 | Mac PPC 2                 |
|  3 |      0 | Mac PPC 3                 |
|  4 |      0 | Mac Intel                 |
|  5 |      0 | Lenovo Thinkpad (Windows) |
|  6 |      0 | HP Laptop (Windows)       |
|  7 |      1 | JoyWarrior 24F8 USB       |
|  8 |      1 | MotionNode Accel USB      |
+----+--------+---------------------------+
9 rows in set (0.00 sec)

*/

longlong quake_hit_test(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
  const int iSensor = (int) (*((double*) args->args[3]));
  const double lat1 = *((double*) args->args[0]);
  const double lon1 = *((double*) args->args[1]);
  const double lat2 = *((double*) args->args[4]);
  const double lon2 = *((double*) args->args[5]);
  const double dTimeTrig  = *((double*) args->args[2]);
  const double dTimeQuake = *((double*) args->args[6]);
  const double dMagnitude = *((double*) args->args[7]);
 
  const double dVelocityFast = 7700.0f;  // P-wave speed in meters/second
  const double dVelocitySlow = 2800.0f;  // S-wave speed in meters/second

  double dDistanceMeters = 0.0f;
  double dDistanceMax = 0.0f;
  double dTimeWindow = 0.0f;
  int iSensorFactor = 0;

  *is_null = 0x00;
/*
  if ( args->args[0] == 0
    || args->args[1] == 0
    || args->args[2] == 0
    || args->args[3] == 0
    || args->args[4] == 0
    || args->args[5] == 0
    || args->args[6] == 0
    || args->args[7] == 0
  )
  {
//    sprintf(message,"%d null params (tr_lat, tr_lon, tr_time_utc, tr_sensor, qu_lat, qu_lon, qu_time_utc, qu_mag)", args->arg_count );
    return -100L;
  }
*/

  // see if magnitude is normal
  if (dMagnitude < 0.0f || dMagnitude > 11.0f) {  // check for ridiculous mag value
    //sprintf(message,"Invalid magnitude %f", dMag);
    return -101L;
  }

  // check values of lat/lon
  // check latitude
  if ( lat1 < -90.0f || lat2 < -90.0f
    || lat1 > 90.0f  || lat2 > 90.0f) {
    //strcpy(message,"Latitude must be in the range [-90.0, 90.0]");
    return -102L;
  }

  // check longitude
  if ( lon1 < -180.0f || lon2 < -180.0f
    || lon1 > 180.0f  || lon2 > 180.0f) {
    //strcpy(message,"Longitude must be in the range [-180.0, 180.0]");
    return -103L;
  }

  // first off check the time, if not close then can just return without the distance check  
  if (fabs(dTimeQuake-dTimeTrig) > 300.0f) { // don't bother if the quake was more than 5 minutes, no matter distance & mag
     return -1L;
  }
 
  // get the distance between the trigger & quake event
  dDistanceMeters = distance_vincenty(lat1, lon1, lat2, lon2, is_null);
  if (*is_null != 0x00) return -2L; // invalid distance (or else they're right on top of the quake? :-)

  // OK, now check the time, based on the distance and the slowest wave
  dTimeWindow = dDistanceMeters / dVelocitySlow;  // this will be the time window to check
  if (dTimeWindow < 60.0f) dTimeWindow = 60.0f;  // always test within 60 seconds

  // if the trigger falls within the time window of the quake, continue to evaluate based on distance, else return 0
  if (fabs(dTimeQuake-dTimeTrig) > dTimeWindow) { // too far away based on time to bother with detection
     return -3L;  
  }

  // see if the distance is within our magnitude check, based on sensor type
  switch (iSensor) { // depending on sensor type, we may want to widen the range, i.e. THinkpads suck & MotionNodes are good
     case 1:
     case 2:
     case 3:
     case 4:  // Mac's are pretty good
        iSensorFactor = 2;
        break;
     case 7:  // JoyWarrior is pretty sensitive
        iSensorFactor = 3;
        break;
     case 8:   // MotionNode is most sensitive
        iSensorFactor = 4;
        break;
     default:  // default to "coarse" sensor i.e. Thinkpad
        iSensorFactor = 1;
  }

   // get a value for max distance to check (in meters) based on quake magnitude & sensor type
   // the idea is that a smaller quake but with a better sensor will have a greater search range in distance
   // than a different (coarser) sensor, etc
   // note multiply by 100 kilometer, this gives a range from 100-1000km from mag 4 through 8
   dDistanceMax = 100000.0f * powf( 2.0f , dMagnitude - (5.4f - sqrt(iSensorFactor-1)) );
   // if our max distance exceeds trigger distance, return >0 (actually distance/diff in m), else 0 
   return (longlong) (dDistanceMax > dDistanceMeters ? ceil(dDistanceMax-dDistanceMeters) : 0L);  

}

#endif /* HAVE_DLOPEN */
