#include "boinc_db.h"
#include "backend_lib.h"
#include "sched_config.h"
#include "sched_util.h"
#include "str_util.h"
#include "util.h"
#include "parse.h"
#include "filesys.h"

#define APP_SENSOR    "qcnsensor"
#define APP_CONTINUAL "qcncontinual"

int main(int argc, char** argv) {
    DB_APP app;
    DB_WORKUNIT wu;
    char strWU[64];
    char strApp[64];
    long int lNumWU;
    char* wu_template = NULL;
    //char* infiles[3] = {"qcn_t1", "qcn_t2", "qcn_t3"};
    char* infileA[1] = {"qcn_t1"};
    char* infileB[1] = {"qcn_t2"};
    char* infileC[1] = {"qcn_t3"};
    char* infileD[1] = {"qcn_t4"};
    char path[1024];

    if (argc!=4) {
       fprintf(stdout, "Usage: bin/qcn_workgen wu_prefix num_wu appname\n"
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
    char strWrite[64];
  /*
    sprintf(path, "%s/%s", config.download_dir, infiles[0]);
    FILE* f = fopen(path, "w");
    strcpy(strWrite, "Test QCN Input");
    fwrite(strWrite, 1+strlen(strWrite), sizeof(char), f);
    fclose(f);
  */

    float fSigCutoff = 3.0f;

    sprintf(path, "%s/%s", config.download_dir, infileA[0]);
    FILE* f = fopen(path, "w");
    sprintf(strWrite, "<fsig>%.2f</fsig>\n<fsta>%.2f</fsta>\n", fSigCutoff, 0.00f);
    fwrite(strWrite, 1+strlen(strWrite), sizeof(char), f);
    fclose(f);

    sprintf(path, "%s/%s", config.download_dir, infileB[0]);
    f = fopen(path, "w");
    sprintf(strWrite, "<fsig>%.1f</fsig>\n<fsta>%.2f</fsta>\n", fSigCutoff, 1.00f);
    fwrite(strWrite, 1+strlen(strWrite), sizeof(char), f);
    fclose(f);

    sprintf(path, "%s/%s", config.download_dir, infileC[0]);
    f = fopen(path, "w");
    sprintf(strWrite, "<fsig>%.2f</fsig>\n<fsta>%.2f</fsta>\n", fSigCutoff, 2.00f);
    fwrite(strWrite, 1+strlen(strWrite), sizeof(char), f);
    fclose(f);

    sprintf(path, "%s/%s", config.download_dir, infileD[0]);
    f = fopen(path, "w");
    sprintf(strWrite, "<fsig>%.2f</fsig>\n<fsta>%.2f</fsta>\n", fSigCutoff, 3.00f);
    fwrite(strWrite, 1+strlen(strWrite), sizeof(char), f);
    fclose(f);


    // read the template
    if (!strcmp(strApp, APP_CONTINUAL))
      read_file_malloc("templates/qcn_input_continual.xml", wu_template);
    else
      read_file_malloc("templates/qcn_input.xml", wu_template);

    float fShortTermAvg = 3.0f;
    char **inFileUse = NULL;

    for (long int i = 0L; i < lNumWU; i++)  {
       wu.clear();     // zeroes all fields

       switch(i%4) {
          case 0:
            fShortTermAvg = 0.0f;
            inFileUse = infileA;
            break;
          case 1:
            fShortTermAvg = 1.0f;
            inFileUse = infileB;
            break;
          case 2:
            fShortTermAvg = 2.0f;
            inFileUse = infileC;
            break;
          case 3:
            fShortTermAvg = 3.0f;
            inFileUse = infileD;
            break;
       }

    if (!strcmp(strApp, APP_CONTINUAL))
       sprintf(wu.name, "continual_%s_%06ld", strWU, i);
    else
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

    if (!strcmp(strApp, APP_CONTINUAL))
       create_work(
          wu,
          wu_template,
          "templates/qcn_output_continual.xml",
          "templates/qcn_output_continual.xml",
          (const char**) inFileUse,
          1,
          config,
          NULL,
          NULL
      );
     else
       create_work(
          wu,
          wu_template,
          "templates/qcn_output.xml",
          "templates/qcn_output.xml",
          (const char**) inFileUse,
          1,
          config,
          NULL,
          NULL
      );
    }
    if (wu_template) free(wu_template);
}

