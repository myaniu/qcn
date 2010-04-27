/* This program 2will monitor the "live" triggers in memory 
   (the mysql trigmem.qcn_trigger_memory table)

The general idea is that a query is run every few seconds to see if any quakes were
detected by QCN sensors via lat/lng & time etc

(c) 2010  Stanford University School of Earth Sciences

*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
using std::vector;

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"

#define DEFAULT_SCAN_INTERVAL  3.0

#define ENUM_FIRST_PASS     0
#define ENUM_SECOND_PASS    1
#define ENUM_OVER           2

const char* order_clause="";
char mod_select_clause[256];
double scan_interval = DEFAULT_SCAN_INTERVAL;

void signal_handler(int) {
    log_messages.printf(MSG_NORMAL, "Signaled by simulator\n");
    return;
}

void feeder_loop() {
    vector<DB_WORK_ITEM> work_items;
    
    for (int i=0; i<napps; i++) {
        DB_WORK_ITEM* wi = new DB_WORK_ITEM();
        work_items.push_back(*wi);
    }

    while (1) {
        bool action = scan_work_array(work_items);
        ssp->ready = true;
        if (!action) {
#ifdef GCL_SIMULATOR
            continue_simulation("feeder");
            log_messages.printf(MSG_DEBUG, "Waiting for signal\n");
            signal(SIGUSR2, simulator_signal_handler);
            pause();
#else
            log_messages.printf(MSG_DEBUG,
                "No action; sleeping %.2f sec\n", sleep_interval
            );
            boinc_sleep(sleep_interval);
#endif
        } else {
            if (config.job_size_matching) {
                update_stats();
            }
        }

        fflush(stdout);
        check_stop_daemons();
        check_reread_trigger();
    }
}

int main(int argc, char** argv) {
    int i, retval;
    void* p;
    char path[256];
    char* appids=NULL;

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

    log_messages.printf(MSG_NORMAL, "Starting\n");

    if (config.feeder_query_size) {
        enum_limit = config.feeder_query_size;
    }
    if (config.shmem_work_items) {
        num_work_items = config.shmem_work_items;
    }
    strncpy(path, config.project_dir, sizeof(path));
    get_key(path, 'a', sema_key);
    destroy_semaphore(sema_key);
    create_semaphore(sema_key);

    retval = destroy_shmem(config.shmem_key);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't destroy shmem\n");
        exit(1);
    }

    int shmem_size = sizeof(SCHED_SHMEM) + num_work_items*sizeof(WU_RESULT);
    retval = create_shmem(config.shmem_key, shmem_size, 0 /* don't set GID */, &p);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't create shmem\n");
        exit(1);
    }
    ssp = (SCHED_SHMEM*)p;
    ssp->init(num_work_items);

    atexit(cleanup_shmem);
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
    retval = boinc_db.set_isolation_level(READ_UNCOMMITTED);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "boinc_db.set_isolation_level: %d; %s\n", retval, boinc_db.error_string()
        );
    }
    ssp->scan_tables();

    log_messages.printf(MSG_NORMAL,
        "read "
        "%d platforms, "
        "%d apps, "
        "%d app_versions, "
        "%d assignments\n",
        ssp->nplatforms,
        ssp->napps,
        ssp->napp_versions,
        ssp->nassignments
    );
    log_messages.printf(MSG_NORMAL,
        "Using %d job slots\n", ssp->max_wu_results
    );

    app_indices = (int*) calloc(ssp->max_wu_results, sizeof(int));

    // If all_apps is set, make an array saying which array slot
    // is associated with which app
    //
    if (all_apps) {
        napps = ssp->napps;
        enum_sizes = (int*) calloc(ssp->napps, sizeof(int));
        double* weights = (double*) calloc(ssp->napps, sizeof(double));
        int* counts = (int*) calloc(ssp->napps, sizeof(int));
        if (ssp->app_weights == 0) {
            for (i=0; i<ssp->napps; i++) {
                ssp->apps[i].weight = 1;
            }
            ssp->app_weights = ssp->napps;
        }
        for (i=0; i<ssp->napps; i++) {
            weights[i] = ssp->apps[i].weight;
        }
        for (i=0; i<ssp->napps; i++) {
            enum_sizes[i] = (int) floor(0.5 + enum_limit*(weights[i])/(ssp->app_weights));
        }
        weighted_interleave(
            weights, ssp->napps, ssp->max_wu_results, app_indices, counts
        );
    } else {
        napps = 1;
    }

    signal(SIGUSR1, show_state);

    feeder_loop();
}

