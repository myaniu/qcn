/* This program 2will monitor the "live" triggers in memory 
   (the mysql trigmem.qcn_trigger_memory table)

The general idea is that a query is run every few seconds to see if any quakes were
detected by QCN sensors via lat/lng & time etc

if there were "hits" in a certain area, then flag this as a quake and put in the qcn_quake table

logic:
  1) check for numerous trickles within a region in a short time period (i.e. 10 seconds) -- see g_strSQLTrigger
  2) if there are numerous trickles - see if this event has been reported in qcnalpha.qcn_quake - lookup by time/lat/lng
  3) if has been reported, use that event to tag trickles; else make an entry in qcn_quake and tag triggers
  4) request uploads from these triggers as appropriate


(c) 2010  Stanford University School of Earth Sciences

*/

#include "qcn_trigmon.h"

DB_CONN trigmem_db;

char g_strSQLTrigger[256];
double g_dSleepInterval = -1.0;
int g_iTriggerTimeInterval = -1;
int g_iTriggerCount = -1;
int g_iTriggerDeleteInterval = -1;

void close_db()
{
   log_messages.printf(MSG_DEBUG, "Closing databases.\n");
   boinc_db.close();
   trigmem_db.close();
}

void do_delete_trigmem()  
{
    char strQueryDelete[512];
    sprintf(strQueryDelete, 
      "delete from trigmem.qcn_trigger_memory where time_trigger<(unix_timestamp() - %d) OR time_trigger>(unix_timestamp()+1.0)", TRIGGER_DELETE_INTERVAL);
    trigmem_db.do_query(strQueryDelete);
}

// first test query - simple just rounts lat/lng to integers, does 10 seconds
void setQuery() 
{
  sprintf(g_strSQLTrigger,
    "select "
    "   round(latitude,0) rlat, "
    "   round(longitude,0) rlng, "
    "   count(distinct hostid) ctr "
    "from trigmem.qcn_trigger_memory "
    "where time_trigger > unix_timestamp()-%d  "
  "group by rlat, rlng "
  "having ctr > %d ",
     g_iTriggerTimeInterval, 
     g_iTriggerCount
   );
}

void do_trigmon() 
{
   vector<DB_QCN_TRIGGER_MEMORY> vqtm;

   // first get a vector of potential matchups by region i.e. lat/lnt/count/time

/*    
    for (int i=0; i<napps; i++) {
        DB_WORK_ITEM* wi = new DB_WORK_ITEM();
        work_items.push_back(*wi);
    }
*/
}

int main(int argc, char** argv) {
    int retval;
 
    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        return 1;
    }

    for (int i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-sleep_interval")) {
            g_dSleepInterval = atof(argv[++i]);
        } else if (!strcmp(argv[i], "-count")) {
            g_iTriggerCount = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-time_interval")) {
            g_iTriggerTimeInterval = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-delete_interval")) {
            g_iTriggerDeleteInterval = atoi(argv[++i]);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "bad cmdline arg: %s\n", argv[i]
            );
            return 2;
        }
    }
    if (g_dSleepInterval < 0) g_dSleepInterval = TRIGGER_SLEEP_INTERVAL;
    if (g_iTriggerTimeInterval < 0) g_iTriggerTimeInterval = TRIGGER_TIME_INTERVAL;
    if (g_iTriggerCount < 0) g_iTriggerCount = TRIGGER_COUNT;
    if (g_iTriggerDeleteInterval < 0) g_iTriggerDeleteInterval = TRIGGER_DELETE_INTERVAL;

    install_stop_signal_handler();
    atexit(close_db);

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        return 3;
    }
    retval = trigmem_db.open(
        DB_TRIGMEM, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "trigmem_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        return 4;
    }
    retval = boinc_db.set_isolation_level(REPEATABLE_READ);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
        );
    }

    log_messages.printf(MSG_NORMAL,
            "qcn_trigmon started with the following options:\n"
            "  -count           = %d\n" 
            "  -time_interval   = %d\n" 
            "  -sleep_interval  = %f\n" 
            "  -delete_interval = %d\n\n",
         g_iTriggerCount,
         g_iTriggerTimeInterval,
         g_dSleepInterval,
         g_iTriggerDeleteInterval
    ); 

    do_delete_trigmem();  // get rid of triggers every once in awhile, i.e. if daemon just starting, it may have a lot of old triggers

    //signal(SIGUSR1, show_state);
    double dtDelete = dtime() + TRIGGER_DELETE_INTERVAL; // delete every TRIGGER_DELETE_INTERVAL (probably a minute when live, otherwise 30 minutes in the map_trigger.sh
    while (1) {
      double dtEnd = dtime() + TRIGGER_SLEEP_INTERVAL, dtCheck = 0.0;
      do_trigmon();
      dtCheck = dtime();
      check_stop_daemons();
      if (dtCheck > dtDelete) {
         do_delete_trigmem();  // get rid of triggers every once in awhile
      }
      if (dtCheck < dtEnd && (dtEnd - dtCheck) < 60.0) { // sleep a bit if not less than 60 seconds
          log_messages.printf(MSG_DEBUG, "Sleeping %f seconds....\n", dtEnd - dtCheck);
          boinc_sleep(dtEnd - dtCheck);
      } 
    }
    return 0;
}

