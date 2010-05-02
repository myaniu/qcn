/* This program 2will monitor the "live" triggers in memory 
   (the mysql trigmem.qcn_trigger_memory table)

The general idea is that a query is run every few seconds to see if any quakes were
detected by QCN sensors via lat/lng & time etc

if there were "hits" in a certain area, then flag this as a quake and put in the qcn_quake table

(c) 2010  Stanford University School of Earth Sciences

*/

#include "trigmon.h"

// important constants for the main query
#define TRIGGER_TIME_INTERVAL 10
#define TRIGGER_COUNT 1

char strSQLTrigger[256];

// first test query - simple just rounts lat/lng to integers, does 10 seconds
void setQuery() {
  sprintf(strSQLTrigger,
    "select "
    "   round(latitude,0) rlat, "
    "   round(longitude,0) rlng, "
    "   count(*) from trigmem.qcn_trigger_memory  "
    "where time_trigger > unix_timestamp()-%d  "
  "group by rlat, rlng "
  "having count(*) > %d ",
     TRIGGER_TIME_INTERVAL,
     TRIGGER_COUNT 
}

double scan_interval = DEFAULT_SCAN_INTERVAL;

void trigmon_loop() {

/*
    vector<DB_WORK_ITEM> work_items;
    
    for (int i=0; i<napps; i++) {
        DB_WORK_ITEM* wi = new DB_WORK_ITEM();
        work_items.push_back(*wi);
    }

    while (1) {
        bool action = scan_work_array(work_items);
        ssp->ready = true;
        if (!action) {
            log_messages.printf(MSG_DEBUG,
                "No action; sleeping %.2f sec\n", sleep_interval
            );
            boinc_sleep(sleep_interval);
        } else {
            if (config.job_size_matching) {
                update_stats();
            }
        }

        fflush(stdout);
        check_stop_daemons();
        //check_reread_trigger();
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
        exit(1);
    }

/*
    for (i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            log_messages.set_debug_level(atoi(argv[++i]));
        } else if (!strcmp(argv[i], "-random_order")) {
            order_clause = "order by r1.random ";
        } else if (!strcmp(argv[i], "-allapps")) {
            all_apps = true;
        } else if (!strcmp(argv[i], "-priority_order")) {
            order_clause = "order by r1.priority desc ";
        } else if (!strcmp(argv[i], "-priority_order_create_time")) {
            order_clause = "order by r1.priority desc, r1.workunitid";
        } else if (!strcmp(argv[i], "-purge_stale")) {
            purge_stale_time = atoi(argv[++i])*60;
        } else if (!strcmp(argv[i], "-appids")) {
           strcat(mod_select_clause, " and workunit.appid in (");
           strcat(mod_select_clause, argv[++i]);
           strcat(mod_select_clause, ")");
        } else if (!strcmp(argv[i], "-mod")) {
            int n = atoi(argv[++i]);
            int j = atoi(argv[++i]);
            sprintf(mod_select_clause, "and r1.id %% %d = %d ", n, j);
        } else if (!strcmp(argv[i], "-wmod")) {
            int n = atoi(argv[++i]);
            int j = atoi(argv[++i]);
            sprintf(mod_select_clause, "and workunit.id %% %d = %d ", n, j);
        } else if (!strcmp(argv[i], "-sleep_interval")) {
            sleep_interval = atof(argv[++i]);
        } else {
            log_messages.printf(MSG_CRITICAL,
                "bad cmdline arg: %s\n", argv[i]
            );
            exit(1);
        }
    }
*/

    //atexit(cleanup_shmem);
    install_stop_signal_handler();

    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.open: %d; %s\n", retval, boinc_db.error_string()
        );
        exit(1);
    }
    retval = boinc_db.set_isolation_level(REPEATABLE_READ);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
        );
    }

    //signal(SIGUSR1, show_state);

    trigmon_loop();
}

