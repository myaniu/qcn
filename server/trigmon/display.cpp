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

    retval = qcn_open_db();
    if (retval) return retval;

    log_messages.printf(MSG_NORMAL,
            "trigdisplay started with the following options:\n"
            "  -time_interval   = %d\n"
            "  -sleep_interval  = %f\n",
         g_iTriggerTimeInterval,
         g_dSleepInterval
    );

    //signal(SIGUSR1, show_state);
    //double dtDelete = 0.0f; // time to delete old triggers from memory
    while (1) {
      g_dTimeCurrent = dtime();
      double dtEnd = g_dTimeCurrent + g_dSleepInterval;
#if 0
    // the qcn_trigmon program which checks against known USGS quakes takes care of deleting
      if (g_dTimeCurrent > dtDelete) {
         do_delete_trigmem();  // get rid of triggers every once in awhile
         dtDelete = g_dTimeCurrent + g_iTriggerDeleteInterval;
      }
#endif
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

