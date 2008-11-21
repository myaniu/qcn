/*
** QCN functions that are dynamically loaded into mysqld, based on mysql source code sql/udf_example.c
**
** by simply doing 'make udf_qcn.so'.
**
** After the library is made one must notify mysqld about the new
** functions with the commands:
**
** CREATE FUNCTION lat_lon_distance_m RETURNS REAL SONAME "udf_qcn.so";

**
** After this the functions will work exactly like native MySQL functions.
** Functions should be created only once.
**
** The functions can be deleted by:
**
** DROP FUNCTION lat_lon_distance_m;
**
** The CREATE FUNCTION and DROP FUNCTION update the func@mysql table. All
** Active function will be reloaded on every restart of server
** (if --skip-grant-tables is not given)
**
** If you get problems with undefined symbols when loading the shared
** library, you should verify that mysqld is compiled with the -rdynamic
** option.
**
** If you can't get AGGREGATES to work, check that you have the column
** 'type' in the mysql.func table.  If not, run 'mysql_fix_privilege_tables'.
**
*/

#ifdef STANDARD
/* STANDARD is defined, don't use any mysql functions */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;	/* Microsofts 64 bit types */
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#if defined(MYSQL_SERVER)
#include <m_string.h>		/* To get strmov() */
#else
/* when compiled as standalone */
#include <string.h>
#define strmov(a,b) stpcpy(a,b)
#define bzero(a,b) memset(a,0,b)
#define memcpy_fixed(a,b,c) memcpy(a,b,c)
#endif
#endif
#include <mysql.h>
#include <ctype.h>

static pthread_mutex_t LOCK_hostname;

// need dlopen to dynamically load symbols

#ifdef HAVE_DLOPEN

// lat_lon_distance_m --- find the distance in meters between two lat/lon points
my_bool lat_lon_distance_m_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void lat_lon_distance_m_deinit(UDF_INIT *initid);
double distance_vincenty(double lat1, double lon1, double lat2, double lon2, char *is_null);
double lat_lon_distance_m(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

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
  if (args->arg_count != 4 
       || args->arg_type[0] != REAL_RESULT
       || args->arg_type[1] != REAL_RESULT
       || args->arg_type[2] != REAL_RESULT
       || args->arg_type[3] != REAL_RESULT
       || args->args[0] == 0
       || args->args[1] == 0
       || args->args[2] == 0
       || args->args[3] == 0
  )
  {
    strcpy(message,"Wrong arguments to lat_lon_distance_m - need two lat/lon real pairs");
    return 1;
  }

  // check values of lat/lon
  double dLat[2], dLon[2];
  dLat[0] = atof(args->args[0]);
  dLon[0] = atof(args->args[1]);
  dLat[1] = atof(args->args[2]);
  dLon[1] = atof(args->args[3]);

  // check latitude
  if ( dLat[0] < -90.0f || dLat[1] < -90.0f 
    || dLat[0] > 90.0f  || dLat[1] > 90.0f) {
    strcpy(message,"Latitude must be in the range [-90.0, 90.0]");
    return 1;
  }

  // check longitude
  if ( dLon[0] < -180.0f || dLon[1] < -180.0f 
    || dLon[0] > 180.0f  || dLon[1] > 180.0f) {
    strcpy(message,"Longitude must be in the range [-180.0, 180.0]");
    return 1;
  }

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

double distance_vincenty(double lat1, double lon1, double lat2, double lon2, char *is_null)
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
  double lat1 = atof(args->args[0]);
  double lon1 = atof(args->args[1]);
  double lat2 = atof(args->args[2]);
  double lon2 = atof(args->args[3]);
  return distance_vincenty(lat1, lon1, lat2, lon2, is_null);
}

#endif /* HAVE_DLOPEN */
