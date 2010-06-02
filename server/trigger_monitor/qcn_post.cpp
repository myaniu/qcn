/*

   this will check for demo or other triggers that need to be posted back to a server

*/

#include "qcn_post.h"

vector<DB_QCN_POST> vQCN_Post;

#define XML_FORMAT \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
  "<Quake>\n" \
  "<QuakeMagnitude>%f</QuakeMagnitude>\n" \
  "<QuakeLon>%f</QuakeLon>\n" \
  "<QuakeLat>%f</QuakeLat>\n" \
  "<QuakeDepth>%f</QuakeDepth>\n" \
  "<QuakeOriginTime>%ld</QuakeOriginTime>\n" \
  "<QuakeID>%d</QuakeID>\n" \
  "</Quake>\n\n"


// setup the vector
// just call qcn_post_check when qcn_trigmon is starting up to get a list of servers to post to (if any)
bool qcn_post_start()
{  // databases should be open by now, so get fill vector with qcn_post entries
   DB_QCN_POST qp;
   vQCN_Post.clear();
   while (!qp.enumerate("WHERE active=1")) {  // active=1 means it is a server record we want to use i.e. post XML to
      vQCN_Post.push_back(qp);
   } 
   log_messages.printf(MSG_DEBUG, "%d Server(s) to Post XML triggers to\n", vQCN_Post.size());
   return true;
}

// ideally this should be a multithreaded asychronous curl post to the servers,
// and set the post state in qcn_trigger_memory (posted = 1)
bool qcn_post_check()
{
   if (!vQCN_Post.size()) return true;

   vector<DB_QCN_POST>::iterator itPost;
   for (it = vQCN_Post.begin(); it < vQCN_Post.end(); it++) {
      // enumerate through our vector of qcn_post entries (i.e. servers we should send XML Post triggers to)
     // try the where statement
        DB_QCN_TRIGGER_MEMORY qtm;
        char strWhere[284], strUpdate[256];
        qtm.clear();
        if (!it->where_clause || !it->url) continue;  // go to next record if no where_clause for some reason
        sprintf(strWhere, "WHERE posted=0 AND (%s)", it->where_clause);
        while (!qtm.enumerate(strWhere)) { 
           // OK we have all the unused info and must have had a match

           // CMC: note this isn't thread-safe in that we want an asynchronous HTTP POST 
           //      to the server URL but we're setting it posted=1 here -- if the HTTP
           //      fails they will never get the message, also the trigmem.qcn_trigger_memory
           //      only has a lifetime of a few minutes so they may never get the message past
           //      that time on a retry anyway (which may be alright if we just care about 
           //      "instant messages"

            qcn_post_xml_http(qtm);  // send the XML to the server in question

            // mark this record as processed (note the db_name & triggerid required)
            sprintf(strUpdate, "UPDATE trigmem.qcn_trigger_memory "
                                  "SET posted=1 "
                                  "WHERE db_name='%s' AND triggerid=%d",
                        qtm.db_name, qtm.triggerid);

            int tret = trigmem_db.do_query(strUpdate);
            if (tret) { // on error print a message and return false
                  log_messages.printf(MSG_CRITICAL,
                    "do_trigmon() strUpdate error: %s - %s\n", strUpdate, boincerror(retval)
                  );
                  return false;
            }
        }
    }
    return true; // processed OK
}

bool qcn_post_xml_httpd(const DB_QCN_TRIGGER_MEMORY& qtm)
{
    char strXML[512];
    // provide magnitude, longitude, latitude, depth_km, time_trigger, qcn_quakeid
    
    sprintf(strXML, XML_FORMAT, qtm.magnitude, qtm.longitude, qtm.latitude, 0.0f, qtm.time_trigger, (int) (rand()*12345.));

    log_messages.printf(MSG_DEBUG,
          strXML
    );

    return true;
}


