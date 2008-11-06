#include "boinc_db.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "sched_util.h"
#include "str_util.h"
#include "util.h"
#include "parse.h"
#include "filesys.h"

int main(int argc, char** argv) {
    DB_APP app;
    DB_WORKUNIT wu;
    char strWU[32];
    char strApp[16];
    long int lNumWU;
    char* wu_template;
    char* infiles[1] = {"qcn_input_file"};
    char path[1024];

    if (argc!=4) {
       fprintf(stdout, "Usage: ./workgen wu_prefix num_wu appname\n"
         "  where wu_template is the prefix for each workunit (e.g. 'qcna')\n"
         "  and num_wu is the total number of workunits to create\n"
         "  and appname is the BOINC app name (e.g. 'qcnalpha')\n"
       );
       fflush(stdout);
       return 1;
    }

    strcpy(strWU, argv[1]);
    lNumWU = atol(argv[2]);
    strcpy(strApp, argv[3]);

    SCHED_CONFIG config;
    config.parse_file();

    boinc_db.open(config.db_name, config.db_host, config.db_user, config.db_passwd);
    char strLookup[64];
    sprintf(strLookup, "where name='%s'", strApp);
    app.lookup(strLookup);
    if (!app.id) {
       fprintf(stdout, "Error -- app ID for %s not found\n\n"
         "Usage: ./workgen wu_prefix num_wu appname\n"
         "  where wu_template is the prefix for each workunit\n"
         "  and num_wu is the total number of workunits to create\n"
         "  and appname is the BOINC app name (i.e. 'qcnalpha')\n",
         strApp
       );
       fflush(stdout);
       return 2;
    }

    // write input file in the download directory
    //
    sprintf(path, "%s/%s", config.download_dir, infiles[0]);
    FILE* f = fopen(path, "w");
    fwrite("test qcn input", 10, sizeof(char), f);
    fclose(f);
    read_file_malloc("templates/qcn_input.xml", wu_template);

    for (long int i = 0L; i < lNumWU; i++)  {
       wu.clear();     // zeroes all fields

       sprintf(wu.name, "%s_%06ld", strWU, i);
       wu.appid = app.id;
       wu.min_quorum = 1;
       wu.target_nresults = 1;
       wu.max_error_results = 1;
       wu.max_total_results = 1;
       wu.max_success_results = 1;
       wu.rsc_fpops_est = 1e14;
       wu.rsc_fpops_bound = 1e15;
       wu.rsc_memory_bound = 1e8;
       wu.rsc_disk_bound = 1e9;
       wu.delay_bound = 14*86400L;

       create_work(
          wu,
          wu_template,
          "templates/qcn_output.xml",
          "templates/qcn_output.xml",
          (const char**) infiles,
          1,
          config,
          NULL,
          NULL
      );
    }
}

