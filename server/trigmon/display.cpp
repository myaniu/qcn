/* version of qcn_trigmon for Jesse

  This program will monitor the "live" triggers in memory 
   (the mysql trigmem.qcn_trigger_memory table)

The general idea is that a query is run every few seconds to see if any quakes were
detected by QCN sensors via lat/lng & time etc

if there were "hits" in a certain area, then flag this as a quake and put in the qcn_quake table

logic:
  1) check for numerous trickles within a region in a short time period (i.e. 10 seconds) 
  2) if there are numerous trickles - see if this event has been reported in qcnalpha.qcn_quake - lookup by time/lat/lng
  3) if has been reported, use that event to tag trickles; else make an entry in qcn_quake and tag triggers
  4) request uploads from these triggers as appropriate

Example usage:
./jfl_trigmon -d 3 -sleep_interval 10 -time_interval 100

(run every 10 seconds, show all debug (-d 3), triggers in last 100 seconds)

(c) 2010  Stanford University School of Earth Sciences

*/

#include "display.h"

DB_CONN trigmem_db;

double g_dTimeCurrent = 0.0;  // global for current time

// global params for trigger monitoring behavior
double g_dSleepInterval = -1.0;   // number of seconds to sleep between trigmem enumerations
int g_iTriggerTimeInterval = -1;  // number of seconds to check for triggers (i.e. "time width" of triggers for an event)
int g_iTriggerDeleteInterval = -1;  // number of seconds to delete trigmem table array


// keep a global vector of recent QCN quake events, remove them after an hour or so
// this way follup triggers can be matched to a known QCN quake event (i.e. qcn_quake table)
//vector<QCN_QUAKE_EVENT> vQuakeEvent;

void close_db()
{
   log_messages.printf(MSG_DEBUG, "Closing databases.\n");
   boinc_db.close();
   trigmem_db.close();
}

void create_plot() {


    system ("csh /var/www/qcn/rt_image/inc/rt_images.csh &");

}

void do_delete_trigmem()
{
    char strDelete[128];
    sprintf(strDelete,
      "DELETE FROM trigmem.qcn_trigger_memory WHERE time_trigger<(unix_timestamp() - %d"
       ") OR time_trigger>(unix_timestamp()+10.0)",
      g_iTriggerDeleteInterval
    );

    int retval = trigmem_db.do_query(strDelete);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "do_delete_trigmem() error: %s\n", boincerror(retval)
        );
    }
    else {
        log_messages.printf(MSG_DEBUG,
            "do_delete_trigmem(): Removed old triggers from memory\n"
        );
    }
}

void do_display() 
{
   DB_QCN_TRIGGER_MEMORY qtm;
   qtm.clear();
//   fprintf(stdout,"HELLO\n");
//   time_t t_now; time(&t_now);                        // Current time
//   fprintf(stdout,"HELLO2 %s\n",rtfile);
   char* rtfile_ltn = "/var/www/qcn/rt_image/rt_triggers_LTN.xyz";  // real time triggers   
   char* rtfile_dtn = "/var/www/qcn/rt_image/rt_triggers_DTN.xyz";  // real time triggers   
   FILE *fp10; fp10 = fopen(rtfile_ltn,"w+");             // Open output file
   FILE *fp11; fp11 = fopen(rtfile_dtn,"w+");             // Open output file


   int iCtr = -1;
   char strWhere[64];
   sprintf(strWhere, "WHERE time_trigger > (unix_timestamp()-%d)", g_iTriggerTimeInterval);
   while (!qtm.enumerate(strWhere))  {
    iCtr++;
    // just print a line out of trigger info i.e. all fields in qtm
/*     fprintf(stdout, "%d %s %d %d %s %s %f %f %f %f %f %f %f %f %f %d %d %f %d %d %d %d %d\n",
        ++iCtr, qtm.db_name, qtm.triggerid, qtm.hostid, qtm.ipaddr, qtm.result_name, qtm.time_trigger,
        qtm.time_received, qtm.time_sync, qtm.sync_offset, qtm.significance, qtm.magnitude, qtm.latitude,
         qtm.longitude, qtm.levelvalue, qtm.levelid, qtm.alignid, qtm.dt, qtm.numreset, qtm.type_sensor,
         qtm.varietyid, qtm.qcn_quakeid, qtm.posted ); */
//     float dt = t_now-qtm.time_trigger;
    if (qtm.type_sensor < 100) {
     fprintf(fp10,"%f,%f,%f,%d\n",qtm.longitude,qtm.latitude,qtm.magnitude,qtm.hostid);
    } else {
     fprintf(fp11,"%f,%f,%f,%d\n",qtm.longitude,qtm.latitude,qtm.magnitude,qtm.hostid);
    }
    iCtr++;
   }
   create_plot();
   fclose(fp10);                                        // Close output file
   fclose(fp11);                                        // Close output file
}


int main(int argc, char** argv) 
{
    int retval;
    //vQuakeEvent.clear();

 
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
        } else if (!strcmp(argv[i], "-time_interval")) {
            g_iTriggerTimeInterval = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "-delete_interval")) {
            g_iTriggerDeleteInterval = atoi(argv[++i]);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "bad cmdline arg: %s\n\n"
                "Example usage: bin/trigdisplay -d 3 -sleep_interval 3 -time_interval 10 -delete_interval 300\n\n"
             , argv[i]
            );
            return 2;
        }
    }
    if (g_dSleepInterval < 0) g_dSleepInterval = TRIGGER_SLEEP_INTERVAL;
    if (g_iTriggerTimeInterval < 0) g_iTriggerTimeInterval = TRIGGER_TIME_INTERVAL;
    if (g_iTriggerDeleteInterval < 0) g_iTriggerDeleteInterval = TRIGGER_DELETE_INTERVAL;

    install_stop_signal_handler();
    atexit(close_db);

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "trigdisplay: boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        return 3;
    }
    retval = trigmem_db.open(
        config.trigmem_db_name, config.trigmem_db_host, config.trigmem_db_user, config.trigmem_db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "trigdisplay: trigmem_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        return 4;
    }
    retval = boinc_db.set_isolation_level(REPEATABLE_READ);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "trigdisplay: boinc_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
        );
    }

    log_messages.printf(MSG_NORMAL,
            "trigdisplay started with the following options:\n"
            "  -time_interval   = %d\n" 
            "  -sleep_interval  = %f\n",
         g_iTriggerTimeInterval,
         g_dSleepInterval
    ); 

    //signal(SIGUSR1, show_state);
    double dtDelete = 0.0f; // time to delete old triggers from memory
    while (1) {
      g_dTimeCurrent = dtime();
      double dtEnd = g_dTimeCurrent + g_dSleepInterval;
      if (g_dTimeCurrent > dtDelete) {
         do_delete_trigmem();  // get rid of triggers every once in awhile
         dtDelete = g_dTimeCurrent + g_iTriggerDeleteInterval;
      }
      do_display();          // the main trigger monitoring routine
      check_stop_daemons();  // checks for a quit request
      g_dTimeCurrent = dtime();
      if (g_dTimeCurrent < dtEnd && (dtEnd - g_dTimeCurrent) < 60.0) { // sleep a bit if not less than 60 seconds
          log_messages.printf(MSG_DEBUG, "Sleeping %f seconds....\n", dtEnd - g_dTimeCurrent);
          boinc_sleep(dtEnd - g_dTimeCurrent);
      } 
    }
    return 0;
}



