/*

   this will check for demo or other triggers that need to be posted back to a server

*/

#include "qcn_post.h"

vector<DB_QCN_POST> vQCN_Post;

#define XML_FORMAT \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<Quake>\n"
  "<QuakeMagnitude>%f</QuakeMagnitude>\n"
  "<QuakeLon>%f</QuakeLon>\n"
  "<QuakeLat>%f</QuakeLat>\n"
  "<QuakeDepth>%f</QuakeDepth>
  "<QuakeOriginTime>%ld</QuakeOriginTime>
  "<QuakeID>%s</QuakeID>\n"
  "</Quake>\n"


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

   char strQuery[512];

   vector<DB_QCN_POST>::iterator itPost;
   for (it = vQCN_Post.begin(); it < vQCN_Post.end(); it++) {
      // enumerate through our vector of qcn_post entries (i.e. servers we should send XML Post triggers to)
     // try the where statement
        QCN_TRIGGER_MEMORY qtm;
        qtm.clear();
         sprintf(strQuery,
             "SELECT * "
/*
             "db_name, triggerid, time_trigger, time_sync, sync_offset, 
              significance, magnitude, latitude, longitude, type_sensor, qcn_quakeid "
*/
//CMC HERE
             "FROM trigmem.qcn_trigger_memory "
             "WHERE (posted = 0) AND (%s)",
           it->where_clause
         );

         MYSQL_ROW trow;
         MYSQL_RES* trp;
         int tret = boinc_db.do_query(strQuery);
         if (tret) {
            log_messages.printf(MSG_CRITICAL,
              "qcn_post_check() strQuery error: %s - %s\n", "Query Error", boincerror(retval)
            );
            return false; ////exit(10);
         }
         trp = mysql_store_result(boinc_db.mysql);
         while ((trow = mysql_fetch_row(trp))) {

             // for each db_name & triggerid need to update trigmem table with qcn_quakeid as well as db_name.qcn_trigger
             char strUpdate[256], strDBName[17];
             int iTriggerID = atoi(trow[1]);
             memset(strUpdate, 0x00, 256);
             memset(strDBName, 0x00, 17);
             strcpy2(strDBName, trow[0]);
             if (iTriggerID) {
               sprintf(strUpdate, "UPDATE trigmem.qcn_trigger_memory SET posted=1 WHERE db_name='%s' AND triggerid=%d",
                   iQuakeID, strDBName, iTriggerID
               );
               tret = trigmem_db.do_query(strUpdate);
               if (tret) {
                  log_messages.printf(MSG_CRITICAL,
                    "do_trigmon() strUpdate error: %s - %s\n", strUpdate, boincerror(retval)
                  );
               }

               sprintf(strUpdate, "UPDATE %s.qcn_trigger SET qcn_quakeid=%d WHERE id=%d",
                   strDBName, iQuakeID, iTriggerID
               );
               tret = boinc_db.do_query(strUpdate);
               if (tret) {
                  log_messages.printf(MSG_CRITICAL,
                    "do_trigmon() strUpdate error: %s - %s\n", strUpdate, boincerror(retval)
                  );
               }
             } // iTriggerID
         } // while trow
         mysql_free_result(trp);
      } // if iQuakeID
    } // outer while
    mysql_free_result(rp);

    //trigmem.update_field("posted=1");

    return true;
}

