#ifndef _QCN_POST_H_
#define _QCN_POST_H_

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <csignal>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
using std::string;
using std::vector;

#include <curl/curl.h>

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"

#include "../trigger/qcn_trigger.h"

extern DB_CONN boinc_db;
extern DB_CONN trigmem_db;

bool qcn_post_check();
bool qcn_post_setup();
bool qcn_post_xml_http(const DB_QCN_TRIGGER_MEMORY& qtm, const char* strURL);

//formatted time
void utc_timestamp(double dt, char* p);

// decl for curl wrapper function
bool qcn_post_curl(const char* strURL, char* strPost, const int iLenPost);

// decl for curl write and read functions
//size_t qcn_post_curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream);
size_t qcn_post_curl_read_data(void *ptr, size_t size, size_t nmemb, void *stream);

struct QCN_POST
{
    int id;
    char where_clause[256];
    char url[256];
    bool active;
    void clear() {memset(this, 0x00, sizeof(QCN_POST));}
};

class DB_QCN_POST: public DB_BASE, public QCN_POST
{
public:
    DB_QCN_POST(DB_CONN* dc=0) :
          DB_BASE("qcn_post", dc ? dc : &boinc_db)  { }

    int get_id() {return id;}

    void db_print(char* buf)
    {
      sprintf(buf,
        "id=%d,"
        "where_clause='%s',"
        "url='%s',"
        "active=%d",
        id, where_clause, url, active ? 1 : 0
       );
    }
    void db_parse(MYSQL_ROW& r)
    {
      int i=0;
      clear();
      id = safe_atoi(r[i++]);
      strcpy2(where_clause, r[i++]);
      strcpy2(url, r[i++]);
      active = (bool) safe_atoi(r[i++]);
    }
};


#endif // _QCN_POST_H_
