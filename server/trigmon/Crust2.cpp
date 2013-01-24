#include <iostream>
#include <cassert>

#include "Crust2.h"
#include "sched_msgs.h"



using namespace std;


Crust2::Crust2() :
        _CKF("/var/www/qcn/earthquakes/inc/CNtype2_key.txt"),
        _CMF("/var/www/qcn/earthquakes/inc/CNtype2.txt"),
        _CEF("/var/www/qcn/earthquakes/inc/CNelevatio2.txt")
{
    init();
}

//Destructor
Crust2::~Crust2()
{}

/////////////////////////////////PUBLIC METHODS::///////////////////////////////////////////


// This function reads in the crust2.0 model and indexes the model map by number rather than by letter
int
Crust2::load()
{

    FILE *fpCrust[3] = {NULL, NULL, NULL};  // Crust2 files
    int retval = 0;

    // Open input files. These are defined in crust_2.0_subs.h
    fpCrust[CRUST_KEY]  = fopen(_CKF.c_str(),  "r");     // Open key file describing each model
    fpCrust[CRUST_MAP]  = fopen(_CMF.c_str(),  "r");     // Open map file with key at each lat/lon location
    fpCrust[CRUST_ELEV] = fopen(_CEF.c_str(),  "r");    // Open elevation file w/ data at each lat/lon

    if (!fpCrust[CRUST_KEY] || !fpCrust[CRUST_MAP] || !fpCrust[CRUST_ELEV]) {
        log_messages.printf(MSG_CRITICAL,
                            "File Open Error %lx %lx %lx\n",
                            (unsigned long) fpCrust[CRUST_KEY],
                            (unsigned long) fpCrust[CRUST_MAP],
                            (unsigned long) fpCrust[CRUST_ELEV]
                           );
        retval = 1;
        for (int i = 0; i < 3; i++) {
           if (fpCrust[i]) fclose(fpCrust[i]);
              fpCrust[i] = NULL;
        }
        return retval; 
    }

    
    char  aline[1000];                      // A full line of characters

    // Temporary variales because I was worried about reading in directly to the variable
    char key_string[mx_cr_type][255];   // Temporary key string variable (easier to index by number)
    char mod_string[mx_cr_lt][mx_cr_ln][255];  // Temporary map key string variable (easier to index by number)

    for (int i=0;i<=4;i++) {
        char* retchar = fgets(aline,1000,fpCrust[CRUST_KEY]); //printf("Skipping Line: %s \n",aline);
        if ( retchar == NULL )
             cout << " error opening file " << _CKF << endl;
        
    }            // Skip first 5 lines of key file
    
    char* retchar= fgets(aline,1000,fpCrust[CRUST_MAP]);         // Skip first 1 lines of map file
    if ( retchar == NULL ){
       cout << " error opening file " << _CKF << endl;
    } 
    retchar      = fgets(aline,1000,fpCrust[CRUST_ELEV]);        // Skip first 1 lines of elevation file
    if ( retchar == NULL ){
       cout << " error opening file " << _CEF << endl;
    }        
    
    // Read in model Key (the two-letter ID and the Vp, Vs, rho, thickness model):
    for (int i=0;i<mx_cr_type;i++) {   // Read in Crust 2.0 key
        retchar = fgets(aline,1000,fpCrust[CRUST_KEY]);        // retrieve whole line of Key file
        sscanf(aline,"%s",key_string[i]);  // Read in key code
        retchar = fgets(aline,1000,fpCrust[CRUST_KEY]);     // retrieve whole line of Key file
        sscanf(aline,"%f %f %f %f %f %f %f %f",
               &_crk[i].vp[1],&_crk[i].vp[0],&_crk[i].vp[2],&_crk[i].vp[3],
               &_crk[i].vp[4],&_crk[i].vp[5],&_crk[i].vp[6],&_crk[i].vp[7]);    // Read P velocity

        retchar = fgets(aline,1000,fpCrust[CRUST_KEY]);                                         // retrieve whole line of Key file
        sscanf(aline,"%f %f %f %f %f %f %f %f",
               &_crk[i].vs[1],&_crk[i].vs[0],&_crk[i].vs[2],&_crk[i].vs[3],
               &_crk[i].vs[4],&_crk[i].vs[5],&_crk[i].vs[6],&_crk[i].vs[7]);  // Read S velocity

        retchar = fgets(aline,1000,fpCrust[CRUST_KEY]);                                           // retrieve whole line of Key file
        sscanf(aline,"%f %f %f %f %f %f %f %f",
               &_crk[i].rh[1],&_crk[i].rh[0],&_crk[i].rh[2],&_crk[i].rh[3],
               &_crk[i].rh[4],&_crk[i].rh[5],&_crk[i].rh[6],&_crk[i].rh[7]);   // Read Density

        retchar = fgets(aline,1000,fpCrust[CRUST_KEY]);                                          // Retrieve whole line of Key file
        sscanf(aline,"%f %f %f %f %f %f %f   ",
               &_crk[i].dp[1],&_crk[i].dp[0],&_crk[i].dp[2],&_crk[i].dp[3],
               &_crk[i].dp[4],&_crk[i].dp[5],&_crk[i].dp[6]              ); // Read layer thickness
        if ( retchar == NULL ){
           cout << " error opening file " <<  CRUST_KEY << _CKF << endl;
        } 
    }

    // Read in map and associate letters with key index:
    for (int i=0; i<=mx_cr_lt-1; i++) {                          // For each latitude
        int   ilat;                             // Index of latitude
        fscanf(fpCrust[CRUST_MAP],"%d",&ilat);                              // Read latitude at beginning of each line
        fscanf(fpCrust[CRUST_ELEV],"%d",&ilat);                              // Read latitude at beginning of each line

        for(int j=0; j<=mx_cr_ln-1; j++) {                        // For each longitude
            fscanf(fpCrust[CRUST_MAP],"%s",mod_string[i][j]);                // Read in key letters for that lat/lon location
            fscanf(fpCrust[CRUST_ELEV],"%f",&_crm.elev[i][j]);                 // Read elevations for each longitude at this latitude

            for(int k=0;k<=mx_cr_type-1; k++) {                   // Index the map key by searching through crk.tp

                if (memcmp(key_string[k],mod_string[i][j],2)==0) {    // If key found, store the key and stop searching
                    break;                                             // Stop looking for additional matches since match found
                }
            }
        }
    }

    // Set the longitude and latitude for each node:
    for (int i=0; i<mx_cr_lt; i++)
        _crm.lat[i]=90.f-( ((float)i)+0.5)*dx_cr ;    //for each latitude

    for (int i=0; i<mx_cr_ln; i++)
        _crm.lon[i]=     ( ((float)i)+0.5)*dx_cr ;    //for each longitude

                                                                                        // Done with function

    return retval;



}

/*
   This fuction returns the average seismic velocity integrated from the surface to some depth (qdep). 
        The velocity is for P:v[0] and S:v[1] waves.  Note: for a zero depth qdep, use the first 
        non-zero thickness layer.
*/
void
Crust2::getMeanVel(const float qdep, const float qlon, const float qlat, vector<float>& v) const
{
    v[0]=0.0;
    v[1]=0.0;                               // set P(0) and S(1) velocity to 0

    int itype = type(qlon,qlat);                   // determine the crust type

    // For an event at depth, integrate down to depth (layers 0-6 in crust, 7 is mantle):

    float d=0.0;                                        // set depth to 0
    if (qdep>0.f) {                                   // If depth greater than 0

        for ( int i=1;i<7;i++) {                            // go through each layer
            // If the depth is greater than the last depth plus the thickness of this layer, then it goes all the way through
            if (qdep>=d+_crk[itype].dp[i]) {
                v[0] += _crk[itype].vp[i] * _crk[itype].dp[i]; // Add thickness weighted Vp from crust2.0
                v[1] += _crk[itype].vs[i] * _crk[itype].dp[i]; // Add Thickness weighted Vs from crust2.0
                d    += _crk[itype].dp[i];                  // Add layer thickness to accumulated depth.
                // If the depth is not greater than the last depth plust the layer thickness, use the difference between qdep and last depth
            } else {

                v[0]+=_crk[itype].vp[i] * (qdep-d);         // Add to average
                v[1]+=_crk[itype].vs[i] * (qdep-d);         // Add to average
                d=qdep;                                  // Total depth is the depth of the earthquake
                break;                                   // Stop looking though layers if goal depth is achieved.

            }

        }

        if (qdep>d) {                                  // If depth greater than crustal depth, then ...

            v[0]+=_crk[itype].vp[7]*(qdep-d);              // Add mantle velocity to average
            v[1]+=_crk[itype].vs[7]*(qdep-d);              // Add mantle velocity to average
            d=qdep;                                       // Total depth is depth of the earthquake

        }
        // Normalize weighted sum by cumulative weight (cumulative depth) for average
        v[0]/=d;
        v[1]/=d;

        // Zero depth (surface rupture), means we need velocity below surface at first non-zero thickness layer
    } else {                             // If zero depth, use the velocity of the first layer

        for (int i=1;i<7;i++) {               // go through layers until first non-zero layer is encountered.
            if (_crk[itype].dp[i]>0.f) {    // if non-zero layer encountered, then
                v[0]=_crk[itype].vp[i];      // set Vp to Crust2.0
                v[1]=_crk[itype].vs[i];      // set Vs to Crust2.0
                break;                      // stop going through layers

            }
        }
    }


}

int
Crust2::type(const float lon, const float lat) const
{
    // This function returns the index of the model key for the map
    float lon2=lon;
    if (lon2<0.f)
        lon2+=360.f;        // make sure 0<lon<360 rather than -180<lon<180

    int ilat = int( (90.f-lat)/dx_cr );               // index of longitude
    int ilon = int( (    lon2)/dx_cr );               // index of latitude
    log_messages.printf(MSG_DEBUG,
                        "ilon=%d    ilat=%d     type=%d\n", ilon, ilat, _crm.ikey[ilat][ilon]
                       );

    return _crm.ikey[ilat][ilon];                                                                    // index key for this lon/lat location
}


////////////////PRIVATE METHODS//////////////////////////////
void
Crust2::init()
{
}
