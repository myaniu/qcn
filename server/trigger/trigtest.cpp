/* CMC test program to simulate a scheduler / handle_request transaction and fake a trigger trickle

used to test the various scenarios in qcn_trigger, such as database and maxmind/geoip web service lookups etc
to associate a lat/lng with a trigger and do error handling

*/

#include "config.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
using namespace std;

#include <unistd.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "boinc_db.h"
#include "backend_lib.h"
#include "error_numbers.h"
#include "parse.h"
#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "sched_msgs.h"

#include "server_types.h"

// CMC -- need to include our trigger file from the svn tree
// CMC -- include the source code here from the qcn/server/trigger/qcn_trigger.cpp file
#include "qcn_trigger.cpp"
        
void getMFHXML(const int num, char* strXML);

int main(int argc, char** argv)
{
    DB_MSG_FROM_HOST mfh[5]; // = new DB_MSG_FROM_HOST[10]; // make a variety of messages to test database & maxmind lookups
    mfh[0].db->open("qcntest", "db-private", "qcn", "HAHAHA");

    int retval = 0;

    // CMC -- handle triggers via handle_qcn_trigger
    for (int i = 0; i < 5; i++) { // mysql did 100K inserts in a minute, 1666 trans per sec
        mfh[i].create_time = (long) dtime();
        mfh[i].hostid = 27+i;
        mfh[i].handled = 0;
        //strcpy(mfh[i].variety, "testing");
        strcpy(mfh[i].variety, "trigger");
        getMFHXML(i, mfh[i].xml);
        if (strcmp(mfh[i].variety, "trigger")) {
           retval = mfh[i].insert(); // not a trigger, process as normal
           if (!retval) {
             retval = mfh[i].db->insert_id();
             if (retval) {
               fprintf(stdout, "Insert ID = %d\n", retval);
               retval = 0;
             }
           }
        }
        else { // break at line 67 which is the next line
           retval = handle_qcn_trigger(&mfh[i]);
        }
    }
    return retval;
}

void getMFHXML(const int num, char* strXML)
{
   /* example xml from msg_from_host:

ult_name>qcne_000121_0</result_name>
      <time>1201792021</time>
<vr>2.21</vr>
<sms>1</sms>
<ctime>1201792020.711935</ctime>
<fsig>3.732729</fsig>
<fmag>5.201985</fmag>
<file>qcne_000121_000208_1201792020.zip</file>
<reset>0</reset>
<dt>0.020000</dt>
<tsync>1201791599.474022</tsync>
<toff>0.347541</toff>
<extip>68.81.65.108</extip>

  */

   // make a variety of test XML strings to process in handle_qcn_trigger
   switch(num) {
     case 0: // normal response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>68.81.65.108</extip>\n"
        );
        break;
     case 1: // bad IP but in good format response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>684.821.655.108</extip>\n"
        );
        break;
     case 2: // bad IP, cutoff, next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>68.81</extip>\n"
        );
        break;
     case 3: // Oxford IP with matching host record, response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>112.43.3.31</extip>\n"
        );
        break;
     case 4: // IP address which is in my qcn_host_ipaddr record
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>.</extip>\n"
        );
        break;
     case 5: // normal response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>201.34.65.108</extip>\n"
        );
        break;
     case 6: // normal response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>68.81.65.108</extip>\n"
        );
        break;
     case 7: // normal response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>68.81.65.108</extip>\n"
        );
        break;
     case 8: // normal response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>68.81.65.108</extip>\n"
        );
        break;
     case 9: // normal response next 16 lines
        strcpy(strXML,
          "<result_name>qcne_000121_0</result_name>\n"
          "<time>1201792021</time>\n"
          "<vr>2.21</vr>\n"
          "<sms>1</sms>\n"
          "<ctime>1201792020.711935</ctime>\n"
          "<fsig>3.732729</fsig>\n"
          "<fmag>5.201985</fmag>\n"
          "<file>qcne_000121_000208_1201792020.zip</file>\n"
          "<reset>0</reset>\n"
          "<dt>0.020000</dt>\n"
          "<tsync>1201791599.474022</tsync>\n"
          "<toff>0.347541</toff>\n"
          "<extip>68.81.65.108</extip>\n"
        );
        break;
   }

}


