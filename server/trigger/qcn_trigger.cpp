#include "qcn_trigger.h"
#include <curl/curl.h>

// CMC Note -- this file is actually included from boinc/sched/handle_request.C so as to make it easier
//             to add the necessary functions and code

// CMC -- now we need to handle triggers differently and put them into the appropriate table
//     at the point this function is called we know this is a trigger trickle (i.e. variety='trigger')

   /*
      Note: all references to "IP address" are really the first 3 bytes of an ipaddress, i.e. the 127.0.0 part of 127.0.0.1
      This is done to save lookups on the GeoIP database (which costs money)

      At this point we are given the trickle (trigger) message as a const ptr to DB_MSG_FROM_HOST class
      from this class we would be interested in the create_time, hostid, and xml fields; particular the
      xml field which should be parsed into the various DB/mysql structs we would need for insertion into the
      qcn_trigger table

      However before insertion much logic needs to be done to get a nominal lat/long for this trigger:

         1) We need to match the IP address in pmfh->xml<extip> with a value in the qcn_host_ipaddr table.
            Note that we need an outer join on IP addr to do this, if lat/lng null then....
           a) If this IP addr is found in qcn_host_ipaddr, get the lat/lng from there for insertion into qcn_trigger.
               1) Note there could be more than one IP->lat/lng mapping, i.e. it could have been a user inputted IP->lat/lng map
                   as well as a GEO IP lookup -- so sort results in order of geoipaddrid (so the user added one, geoipaddrid=0 will be 
                   the top one)
               2) Note we are always "favoring" a user-inputted lat/lng mapping
   
            b) if no IP addr found for this host in the qcn_host_ipaddr table, look up in the qcn_geo_ipaddr table
               1) if found (and record isn't older than 6 months say), then we can use this record (i.e. no need to do another GeoIP lookup, saving a penny ;-)
               2) if not found, we'll need to do a GeoIP web service lookup and insert the record into both the qcn_geo_ipaddr and
                  the qcn_host_ipaddr tables (with a pointer qcn_host_ipaddr.geoipaddr to the qcn_geo_ipaddr.id record)

         2) At this point we should have a "user-guided" or GeoIP mapping between the incoming IP address and their lat/long location, so
            we can insert the trigger.  

         3) Triggers will probably be displayed on the user's host page and perhaps edit lat/long after an event to update qcn_host_ipaddr?

      Here is an example of the data in pmfh->xml :

<result_name>qcne_000105_0</result_name>
      <time>1201209164</time>
<vr>2.20</vr>
<sms>1</sms>
<ctime>1201209161.669771</ctime>
<fsig>3.929932</fsig>
<fmag>7.179575</fmag>
<file>qcne_000105_000110_1201209161.zip</file>
<reset>1</reset>
<dt>0.020000</dt>
<loc></loc>
<tsync>1201208379.748502</tsync>
<toff>-2.146548</toff>
<wct>234.34</wct>
<cpt>4.5632</cpt>
<extip>171.64.173.42</extip>

        This should be parsed out to give us matching fields in qcn_host_ipaddr:

 id             int(11)      NO    PRI                  
 hostid         int(11)      YES   MUL  NULL            
 ipaddr         varchar(32)  YES        NULL            
 result_name    varchar(64)  NO
 time_trigger   double       YES   MUL  NULL            
 time_received  double       YES        NULL            
 time_sync      double       YES        NULL            
 sync_offset    double       YES        NULL            
 significance   double       YES        NULL            
 magnitude      double       YES        NULL            
 latitude       double       YES        NULL            
 longitude      double       YES        NULL            
 depth_km       double       YES        NULL            
 file           varchar(64)  YES        NULL            
 dt             float        YES        NULL            
 numreset       int(6)       YES        NULL            
 type_sensor    int(2)       YES        NULL            
 sw_version     varchar(8)
 os_type        varchar(8)
 usgs_quakeid   int(11)
 received_file  bool
 file_url       varchar(128)

  (note we store result_name and not resultid to save time on the lookup, usgs_quakeid can also be used later to link a known event with our trigger)

   */

long g_curlBytes = 0L;

// forward declaration for the big function at the bottom of this file
int lookupGeoIPWebService(
   const int iRetGeo,
   DB_QCN_HOST_IPADDR& qhip,
   DB_QCN_GEO_IPADDR&  qgip,
   DB_QCN_TRIGGER&     qtrig
);

// decl for curl function
bool execute_curl(const char* strURL, char* strReply, const int iLen);

// decl for curl write function
size_t qcn_curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream);

// the qcn_quakelist will insert a record from a quakelist trigger 
// so that we can get basic diagnostics on a host running QCN (i.e. number of reset errors, last time sync & offset etc)
int handle_qcn_quakelist(const DB_MSG_FROM_HOST* pmfh)
{
    return handle_qcn_trigger(pmfh, true);
}

// handle_qcn_trigger processes the trigger trickle, does the geoip or database lookup as appropriate, inserts into qcn_trigger
int handle_qcn_trigger(const DB_MSG_FROM_HOST* pmfh, bool bPing)
{
     // instantiate the objects
     DB_QCN_HOST_IPADDR qhip;
     DB_QCN_GEO_IPADDR  qgip;
     DB_QCN_TRIGGER     qtrig;
     
     char strIP[32]; // temp holder for IP address
     int iRetVal = 0;

     // parse out all the data into the qtrig object;
     qtrig.hostid = pmfh->hostid; // don't parse hostid, it's in the msg_from_host struct!
     if (!parse_str(pmfh->xml, "<result_name>", qtrig.result_name, sizeof(qtrig.result_name))) memset(qtrig.result_name, 0x00, sizeof(qtrig.result_name));
     if (!parse_str(pmfh->xml, "<vr>", qtrig.sw_version, sizeof(qtrig.sw_version))) memset(qtrig.sw_version, 0x00, sizeof(qtrig.sw_version));
     if (!parse_str(pmfh->xml, "<os>", qtrig.os_type, sizeof(qtrig.os_type))) memset(qtrig.os_type, 0x00, sizeof(qtrig.os_type));
     parse_int(pmfh->xml, "<sms>", qtrig.type_sensor);
     parse_int(pmfh->xml, "<reset>", qtrig.numreset);
     parse_double(pmfh->xml, "<dt>", qtrig.dt);
     parse_double(pmfh->xml, "<tsync>", qtrig.time_sync);
     parse_double(pmfh->xml, "<toff>", qtrig.sync_offset);
     parse_double(pmfh->xml, "<wct>", qtrig.runtime_clock);
     parse_double(pmfh->xml, "<cpt>", qtrig.runtime_cpu);
     if (!parse_str(pmfh->xml, "<extip>", strIP, 32)) memset(strIP, 0x00, sizeof(char) * 32);
     parse_double(pmfh->xml, "<ctime>", qtrig.time_trigger);

     qtrig.time_received = dtime();  // mark current server time as time_received, this gets overridden by database unix_timestamp() in qcn_trigger.h db_print

     if (bPing) {
       if (qtrig.time_trigger < 1.0f) qtrig.time_trigger = qtrig.time_received; // sometimes trigtime/<ctime> is sent, older versions it wasn't
       qtrig.significance = 0.0f;
       qtrig.magnitude = 0.0f;
       strcpy(qtrig.file,"");
       qtrig.ping = 1;
     }
     else {
       parse_double(pmfh->xml, "<fsig>", qtrig.significance);
       parse_double(pmfh->xml, "<fmag>", qtrig.magnitude);
       if (!parse_str(pmfh->xml, "<file>", qtrig.file, sizeof(qtrig.file))) memset(qtrig.file, 0x00, sizeof(qtrig.file));
       qtrig.ping = 0;
     }


     /*
       // so at this point, for qtrig we lack:
       latitude       double       YES        NULL            
       longitude      double       YES        NULL            
       depth_km       double       YES        NULL            
       ipaddr         varchar(32)
       received_file & file_url & usgs_quakeid should be null
     */

     // eventid & usgsid will be used later, in case we want to tie in some usgs or other event id to this trigger
     // also depth_km would be added later
     qtrig.usgs_quakeid   = 0;
     qtrig.received_file = 0;
     strcpy(qtrig.file_url,"");
     qtrig.depth_km = 0;

     // so it's really latitude, longitude and ipaddr left to add to qcn_trigger object & database table

     // let's get the proper ipaddr form
     // don't forget we only "count" the first three bytes of an IP address, so process strIP for qtrig.ipaddr...
     char* strLast = strrchr(strIP, '.');  // this finds the last . in an ip address, so we want everything before that
     memset(qtrig.ipaddr, 0x00, sizeof(qtrig.ipaddr));
     if (strLast) { // we found the last bit, so let's copy over everything between to the qtrig.ipaddr structure
        strncpy(qtrig.ipaddr, strIP, strLast-strIP); 
     }

     // now validate the IP address prefix in qtrig.ipaddr, if not in the form X.Y.Z then it's invalid
     // and should be set to null ("")
     strLast = strchr(strIP, '.');  // this finds the first . in an ip address, so we want everything before that
     if (strlen(strLast)<=1 || !strchr(strLast+1, '.')) { // this finds the next ., if this is null, reset qtrig.ipaddr as it's a bad IP addr
         // can't be a good IP prefix since strlen too small and/or only 1 . found 
         log_messages.printf(
           SCHED_MSG_LOG::MSG_CRITICAL,
           "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [0] Invalid IP address detected: %s\n",
           qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.ipaddr
         );
         memset(qtrig.ipaddr, 0x00, sizeof(qtrig.ipaddr));
     }


     // OK, now just the lat/lng lookup
     // the first step will be to search into qcn_host_ipaddr to see if this exists already, sorted by geoip so user setting will be preferred
     char strWhere[_MAX_PATH];
     
     qhip.hostid = qtrig.hostid; // important -- copy over ipaddr & hostid into other structs
     strcpy(qhip.ipaddr, qtrig.ipaddr);
     strcpy(qgip.ipaddr, qtrig.ipaddr);

     log_messages.printf(
           SCHED_MSG_LOG::MSG_NORMAL,
           "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] Processing QCN %s trickle message from IP %s\n",
           qtrig.hostid, qtrig.result_name, qtrig.time_received, bPing ? "ping" : "trigger", qtrig.ipaddr
     );

     // note if IP address is '' then this just becomes the user pref lat/lng if they input a record with no IP address
     // also note the sort order, non-geoip lookups (i.e. user-entered lat/lng/ipaddr) get sorted to the top
     sprintf(strWhere, "WHERE hostid=%d AND (ipaddr='' OR ipaddr='%s') ORDER BY geoipaddrid,ipaddr", qtrig.hostid, qtrig.ipaddr);
     iRetVal = qhip.lookup(strWhere);
     switch(iRetVal) {
        case ERR_DB_NOT_FOUND:  // this host & IP addr not found (or else it's a geopip record), check for host record with no IP, then need to lookup in geoip
           // OK, if ipaddr is empty and record not found, we can't do much (i.e. no geoip lookup on an empty ip address!), so just return 0
           if (!strlen(qtrig.ipaddr)) {
              log_messages.printf(
                SCHED_MSG_LOG::MSG_CRITICAL,
                "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [1] Invalid IP address and no host/lat/lng default record found\n",
                qtrig.hostid, qtrig.result_name, qtrig.time_received
              );
              return 0;
           }

           // no record, need to do a maxmind/geoip database table lookup, and possibly web service lookup!
           sprintf(strWhere, "WHERE ipaddr='%s'", qtrig.ipaddr);
           iRetVal = lookupGeoIPWebService(qgip.lookup(strWhere), qhip, qgip, qtrig); 

           /*

           // first off, let's search for a hostid record in qcn_host_ipaddr, WITHOUT an ipaddr
           // i.e. perhaps the user entered a lat/lng to always apply to this host?
           sprintf(strWhere, "WHERE hostid=%d AND ipaddr='' AND geoipaddrid=0", qtrig.hostid);
           iRetVal = qhip.lookup(strWhere);
           switch(iRetVal) {
              case ERR_DB_NOT_FOUND:  // no record, need to do a maxmind/geoip database table lookup, and possibly web service lookup!
                 sprintf(strWhere, "WHERE ipaddr='%s'", qtrig.ipaddr);
                 iRetVal = lookupGeoIPWebService(qgip.lookup(strWhere), qhip, qgip, qtrig); 
                 break;
              case 0:  // a qcn_host_ipaddr record found, insert into host table and set lat/lng for qcn_trigger
                 // copy over the lat/lng for this record into qtrig
                 memset(qtrig.ipaddr, 0x00, sizeof(qtrig.ipaddr));
                 qtrig.latitude  = qhip.latitude;
                 qtrig.longitude = qhip.longitude;
                 iRetVal = qtrig.insert();  // note if the insert fails, return code will be set and returned below
                 if (!iRetVal) { // trigger got in OK
                    log_messages.printf(
                          SCHED_MSG_LOG::MSG_NORMAL,
                          "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [1] Trigger inserted after qcn_host_ipaddr lookup of blank IP, mag=%lf at (%lf, %lf)!\n",
                          qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.magnitude, qtrig.latitude, qtrig.longitude
                    );
                 }
                 break;
              default:  // other database error, will be returned below and trigger will be input later
                 log_messages.printf(
                    SCHED_MSG_LOG::MSG_CRITICAL,
                    "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [1] Database error encountered on qcn_host_ipaddr lookup!\n",
                    qtrig.hostid, qtrig.result_name, qtrig.time_received 
                 );
           }
           */
           break;
        case 0:   // a qcn_host_ipaddr record found for this host & ipaddr 
           // (either a user entry with or without ipaddr key,  or previous geoip/maxmind lookup for this ipaddr)
           // copy over the lat/lng for this record into qtrig
           qtrig.latitude  = qhip.latitude;
           qtrig.longitude = qhip.longitude;
           iRetVal = qtrig.insert();  // note if the insert fails, return code will be set and returned below, for update later
           if (!iRetVal) { // trigger got in OK
                log_messages.printf(
                  SCHED_MSG_LOG::MSG_NORMAL,
                  "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [1] Trigger inserted after qcn_host_ipaddr lookup of IP %s, mag=%lf at (%lf, %lf) - sync offset %f at %f!\n",
                  qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.ipaddr, qtrig.magnitude, qtrig.latitude, qtrig.longitude, qtrig.sync_offset, qtrig.time_sync
                );
           }
           break;
        default:   // other database error, so return error
           log_messages.printf(
            SCHED_MSG_LOG::MSG_CRITICAL,
            "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [9] Database error encountered on trigger processing!\n",
            qtrig.hostid, qtrig.result_name, qtrig.time_received 
           );
     }

     // at this point we've inserted the appropriate records or had a database error, 
     // so return with iRetVal (0=good, otherwise a database error which will "nak" so the trigger will be resent)
     return iRetVal; // returns qhip.lookup error if there was a database error, so trigger won't be "ack'd" and will try again
}

int lookupGeoIPWebService(
   const int iRetGeo,
   DB_QCN_HOST_IPADDR& qhip,
   DB_QCN_GEO_IPADDR&  qgip,
   DB_QCN_TRIGGER&     qtrig
)
{
                 int iReturn = iRetGeo; // "seed" our return code with the initial return code value
                 char *strURL = NULL, *strReply = NULL;

                 // the switch is the reply/return code from the lookup query into the qcn_geo_ipaddr table
                 // note that we may want to set error return code (iRetGeo) to 0 if it seems that this record
                 // will never get input, otherwise it will "nak" and retry (but if it's a bad GeoIP lookup, why bother?)

                 switch(iRetGeo) {
                    case ERR_DB_NOT_FOUND:  // no record, need to do a maxmind/geoip web service lookup!
                       strURL   = new char[BYTESIZE_URL];
                       strReply = new char[BYTESIZE_CURL];
                       memset(strURL, 0x00, sizeof(char) * BYTESIZE_URL);
                       memset(strReply, 0x00, sizeof(char) * BYTESIZE_CURL);
                       sprintf(strURL, FORMAT_GEOIP, qtrig.ipaddr);
                       if (execute_curl(strURL, strReply, 512))  {
                          // returned OK, now check strReply -- should be a single line of comma-delimited fields:
                          // Returns: ISO 3166 Two-letter Country Code, Region Code, City, Latitude, Longitude, Error code
                          // good reply: (note 4 commas/5 fields)
                          //    GB,K2,Oxford,51.750000,-1.250000
                          // error reply: (note 5 commas/6 fields all null except last)
                          //    ,,,,,IP_NOT_FOUND
                          char* strComma[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
                          int i = 0;
                          strComma[0] = strchr(strReply, ',');  // get the first comma
                          for (i = 1; strComma[0] && i < 6; i++)  { // parse out fields
                             // search for next comma if last value wasn't NULL and haven't gone past the end of strReply
                             if (strComma[i-1] && strlen(strReply) > (size_t)(strComma[i-1] - strReply + 1)) {
                                strComma[i] = strchr(strComma[i-1]+1, ','); // note we skip a char to move off current comma ptr
                                if (!strComma[i]) break;  // if this is null, i.e.no comma found, may as well break
                             }
                          }
                          if (i<4 || strComma[4] || strComma[5]) { // if this isn't null, or less than 4 commas found, then there was an error
                            iReturn = 0; // ip not found, but this is a bad format web service lookup, so let's not bother retrying...
                            log_messages.printf(
                              SCHED_MSG_LOG::MSG_CRITICAL,
                              "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [2] Maxmind/GeoIP web lookup for IP %s via %s to %s failed\n",
                              qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.ipaddr, "libcurl", strURL
                            );
                          }
                          else {
                            // seems like a legit reply, so parse out
                            // note we need to insert into qcn_geo_ipaddr, get insert_id(), then insert into host_ipaddr & trigger
                            iReturn = 0; // success

                            // initialize vars for the copies
                            memset(&qgip.country, 0x00, sizeof(qgip.country));
                            memset(&qgip.region, 0x00, sizeof(qgip.region));
                            memset(&qgip.city, 0x00, sizeof(qgip.city));

                            // set location as geoip
                            strcpy(qhip.location, "geoip");    // mark as geoip (also noted in field  geoipaddr i.e points to qcn_geo_ipaddr record)

                            qgip.time_lookup = dtime(); // returns a double of current time, # of seconds since epoch

                            strncpy(qgip.country, strReply, strComma[0]-strReply);  // first comma entry is country; note length is OK starting from strReply
                            strncpy(qgip.region, strComma[0]+1, strComma[1]-strComma[0]-1);  // next is region, note subtract 1 from length (, position)
                            strncpy(qgip.city, strComma[1]+1, strComma[2]-strComma[1]-1);  // next is city

                            char *strTmp = new char[32];
                            memset(strTmp, 0x00, sizeof(char) * 32);
                            strncpy(strTmp, strComma[2]+1, strComma[3]-strComma[2]-1);  // next is latitude
                            qgip.latitude = safe_atof(strTmp);

                            memset(strTmp, 0x00, sizeof(char) * 32);
                            strComma[4] = strReply + strlen(strReply);  // make a fake endpoint
                            strncpy(strTmp, strComma[3]+1, strComma[4]-strComma[3]-1);  // next is longitude
                            qgip.longitude = safe_atof(strTmp);

                            delete [] strTmp;

                            iReturn = qgip.insert();
                            if (!iReturn) { // success, get insert_id
                               int iInsertID = qgip.db->insert_id();
                               if (iInsertID>0) {
                                    // now make a host record
                                    qhip.geoipaddrid = iInsertID;  // mark the geoip database id used 
                                    qhip.latitude    = qgip.latitude;
                                    qhip.longitude   = qgip.longitude;
                                    qtrig.latitude   = qgip.latitude;
                                    qtrig.longitude  = qgip.longitude;
                                    iReturn = qhip.insert();
                                    if (!iReturn) { // success, insert trigger, if fails retcode sent below
                                        iReturn = qtrig.insert();
                                        if (iReturn) {
                                            log_messages.printf(
                                              SCHED_MSG_LOG::MSG_CRITICAL,
                                              "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [2] Maxmind/GeoIP web lookup -- trigger %s insert failed\n",
                                              qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.ipaddr
                                            );
                                        }
                                        else {
                                            log_messages.printf(
                                              SCHED_MSG_LOG::MSG_NORMAL,
                                              "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [2] Maxmind/GeoIP web lookup -- trigger %s insert success\n",
                                              qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.ipaddr
                                            );
                                        }
                                    }
                                    else {
                                       iReturn = 0; // well we tried, return 0 so doesn't bother with this trigger again
                                       log_messages.printf(
                                            SCHED_MSG_LOG::MSG_CRITICAL,
                                            "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [2] Maxmind/GeoIP web lookup -- host_ipaddr %s insert failed\n",
                                            qtrig.hostid, qtrig.result_name, qtrig.time_received, qhip.ipaddr
                                       );
                                    } // failed qhip insert
                               } // insertid
                               else { // failed qgip insert id
                                  log_messages.printf(
                                    SCHED_MSG_LOG::MSG_CRITICAL,
                                    "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [2] Maxmind/GeoIP web lookup -- invalid insert id on geo_ipaddr %s\n",
                                    qtrig.hostid, qtrig.result_name, qtrig.time_received, qhip.ipaddr
                                  );
                               }
                            }
                            else { // bad geo_ipaddr insert
                                  log_messages.printf(
                                    SCHED_MSG_LOG::MSG_CRITICAL,
                                    "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [2] Maxmind/GeoIP web lookup -- geo_ipaddr %s insert failed\n",
                                    qtrig.hostid, qtrig.result_name, qtrig.time_received, qhip.ipaddr
                                  );
                            }
                          }
                       }
                       else { // error in curl execution, set iReturn to non-zero so it can try again, should we insert trigger anyway?
                          iReturn = 2;
                          log_messages.printf(
                             SCHED_MSG_LOG::MSG_CRITICAL,
                             "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [2a] Maxmind/GeoIP web lookup of IP %s via %s to %s failed\n",
                             qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.ipaddr, "libcurl", strURL
                          );
                       }
                       break;
                    case 0:  // record found already in geoip table, insert into host table and set lat/lng for qcn_trigger
                       qhip.geoipaddrid = qgip.id;  // mark the geoip database id used 
                       qhip.latitude    = qgip.latitude;
                       qhip.longitude   = qgip.longitude;
                       qtrig.latitude   = qgip.latitude;
                       qtrig.longitude  = qgip.longitude;

                       iReturn = qhip.insert();  // note if the insert fails, return code will be set and returned below
                       if (!iReturn) iReturn = qtrig.insert();  // note if the insert fails, return code will be set and returned below
                       if (!iReturn) { // trigger got in OK
                           log_messages.printf(
                             SCHED_MSG_LOG::MSG_NORMAL,
                             "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [3] Trigger inserted after qcn_geo_ipaddr lookup; mag=%lf at (%lf, %lf) - sync offset %f at %f!\n",
                             qtrig.hostid, qtrig.result_name, qtrig.time_received, qtrig.magnitude, qtrig.latitude, qtrig.longitude, qtrig.sync_offset, qtrig.time_sync
                           );
                       }
                       break;
                    default:  // other database error, iReturn will be returned below
                       log_messages.printf(
                          SCHED_MSG_LOG::MSG_CRITICAL,
                          "[QCN] [HOST#%d] [RESULTNAME=%s] [TIME=%lf] [3] - Database error encountered on qcn_geo_ipaddr lookup!\n",
                          qtrig.hostid, qtrig.result_name, qtrig.time_received
                       );
                 }
                 if (strURL) delete [] strURL; // don't forget to get rid of these dynamic strings!
                 if (strReply) delete [] strReply;

                 return iReturn;
}

bool execute_curl(const char* strURL, char* strReply, const int iLen)
{
   // easycurl should be fine, just send a request to maxmind (strURL has the key & ip etc),
   // and output to strReply up to iLen size
   CURLcode cc;
   CURL* curlHandle = curl_easy_init();

   g_curlBytes = 0L;  // reset long num of curl bytes read

   if (!curlHandle) return false;  // problem with init

   cc = curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 0L);
   cc = curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1L);
   cc = curl_easy_setopt(curlHandle, CURLOPT_URL, strURL);
   cc = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, strReply);
   cc = curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, qcn_curl_write_data);

   cc = curl_easy_perform(curlHandle);

   curl_easy_cleanup(curlHandle);

   return (bool) (cc == 0 && sizeof(strReply)>0);  // 0 is good CURLcode
}

size_t qcn_curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
   int iLeft = BYTESIZE_CURL - g_curlBytes - 1;
   if (iLeft > 0 && size > 0) { // we have some room left to write
      strlcat((char*) stream, (char*) ptr, BYTESIZE_CURL);
   }
   g_curlBytes += (size * nmemb);
   return size * nmemb;
}


