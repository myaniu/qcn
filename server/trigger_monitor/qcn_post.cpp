/*

   this will check for demo or other triggers that need to be posted back to a server

*/

#include "qcn_post.h"

vector <DB_QCN_POST> vQCN_Post;

// setup the vector
// just call qcn_post_check when qcn_trigmon is starting up to get a list of servers to post to (if any)
bool qcn_post_check()
{

   return true;
}

// ideally this should be a multithreaded asychronous curl post to the servers,
// and set the post state in qcn_trigger_memory
bool qcn_post_check()
{

   char strQuery[256];
   sprintf(strQuery,
      "SELECT "
      "  ROUND(latitude,0) rlat, "
      "  ROUND(longitude,0) rlng, "
      "  COUNT(distinct hostid) ctr, "
      "  MIN(time_trigger) time_min, "
      "  MAX(time_trigger) time_max "
      "FROM trigmem.qcn_trigger_memory "
      "WHERE time_trigger > (unix_timestamp() - %d) AND qcn_quakeid=0 "
      "GROUP BY rlat, rlng "
      "HAVING ctr > %d",
         g_iTriggerTimeInterval,
         g_iTriggerCount
    );

    int retval = trigmem_db.do_query(strQuery);
    if (retval) { // big error, should probably quit as may have lost database connection
        log_messages.printf(MSG_CRITICAL,
            "do_trigmon() error: %s - %s\n", "Query Error", boincerror(retval)
        );
        exit(10);
    }

    MYSQL_ROW row;
    MYSQL_RES* rp;
    int numRows = 0;

    rp = mysql_store_result(trigmem_db.mysql);
    while ((row = mysql_fetch_row(rp))) {
      // if we have rows from the above query, then we have detected an event
      // so need to see if this is in our current event array, and add it if it isn't

      numRows++;
      double dLat = atof(row[0]);

      double dLng = atof(row[1]);
      int iCtr = atoi(row[2]);
      double dTimeMin = atof(row[3]);
      double dTimeMax = atof(row[4]);
      log_messages.printf(MSG_DEBUG,
          "  #%d  %f - (%f, %f) - %d distinct hosts from %f to %f\n",
           numRows, g_dTimeCurrent, dLat, dLng, iCtr, dTimeMin, dTimeMax
      );

      int iQuakeID = getQCNQuakeID(dLat, dLng, iCtr, dTimeMin, dTimeMax);
      if (iQuakeID) {
         log_messages.printf(MSG_DEBUG,
           "do_trigmon() processing QCN Quake # %d - %d hosts\n", iQuakeID, iCtr);

         // get matching triggers and update appropriate qcn_trigger table (qcnalpha and/or continual)
         char strTrigs[512];
         sprintf(strTrigs,
             "SELECT db_name, triggerid "
             "FROM trigmem.qcn_trigger_memory "
             "WHERE ROUND(latitude,0)=ROUND(%f,0) AND ROUND(longitude,0)=ROUND(%f,0) "
             " AND time_trigger BETWEEN FLOOR(%f) AND CEIL(%f) AND qcn_quakeid=0 ",
           dLat, dLng, dTimeMin, dTimeMax
         );

         MYSQL_ROW trow;
         MYSQL_RES* trp;
         int tret = boinc_db.do_query(strTrigs);
         if (tret) {
            log_messages.printf(MSG_CRITICAL,
              "do_trigmon() strTrigs error: %s - %s\n", "Query Error", boincerror(retval)
            );
            exit(10);
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
               sprintf(strUpdate, "UPDATE trigmem.qcn_trigger_memory SET qcn_quakeid=%d WHERE db_name='%s' AND triggerid=%d",
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

