/* trigdisplay  (c) 2011 Stanford University

   this program will dump out the latest triggers from memory to a file every
   few seconds and then use gmt et al to plot out the latest triggers

*/

#include "display.h"

DB_CONN trigmem_db;

double g_dTimeCurrent = 0.0;  // global for current time

// global params for trigger monitoring behavior
double g_dSleepInterval = -1.0;   // number of seconds to sleep between trigmem enumerations
int g_iTriggerTimeInterval = -1;  // number of seconds to check for triggers (i.e. "time width" of triggers for an event)
int g_iTriggerDeleteInterval = -1;  // number of seconds to delete trigmem table array

void create_plot()
{
  // CMC note: not a good idea to run this in the background as it's such a lengthy script
  // it's possible to loop around and overwrite files in the middle of being used by the script etc
    system (CSH_PLOT_CMD);
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

   FILE* fp[2];
   fp[0] = fopen(FILE_NAME_TRIGGER_LAPTOP,"w");             // Open output file
   fp[1] = fopen(FILE_NAME_TRIGGER_DESKTOP,"w");             // Open output file

   int iCtr = 0;
   char strWhere[64];
   sprintf(strWhere, "WHERE time_trigger > (unix_timestamp()-%d)", g_iTriggerTimeInterval);
   while (fp[0] && fp[1] && !qtm.enumerate(strWhere))  {
    //iCtr++;
    // just print a line out of trigger info i.e. all fields in qtm
/*     fprintf(stdout, "%d %s %d %d %s %s %f %f %f %f %f %f %f %f %f %d %d %f %d %d %d %d %d\n",
        ++iCtr, qtm.db_name, qtm.triggerid, qtm.hostid, qtm.ipaddr, qtm.result_name, qtm.time_trigger,
        qtm.time_received, qtm.time_sync, qtm.sync_offset, qtm.significance, qtm.magnitude, qtm.latitude,
         qtm.longitude, qtm.levelvalue, qtm.levelid, qtm.alignid, qtm.dt, qtm.numreset, qtm.qcn_sensorid,
         qtm.varietyid, qtm.qcn_quakeid, qtm.posted ); */
//     float dt = t_now-qtm.time_trigger;
     fprintf( qtm.qcn_sensorid < ID_USB_SENSOR_START ? fp[0] : fp[1],
          "%f,%f,%f,%d\n",
           qtm.longitude,qtm.latitude,qtm.magnitude,qtm.hostid
     );
     iCtr++;
   }
   log_messages.printf(MSG_DEBUG,
      "  # of Active Triggers: %d\n", iCtr
   );
   if (fp[0]) fclose(fp[0]);
   if (fp[1]) fclose(fp[1]);
   if (iCtr) create_plot();   // create_plot does a lot, so just do if there are any trigs
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

    atexit(QCN_DBClose);

    retval = QCN_DBOpen();
    if (retval) return retval;

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
    // the qcn_trigmon program which checks against known USGS quakes takes care of deleting
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
    QCN_DBClose();
    return 0;
}

