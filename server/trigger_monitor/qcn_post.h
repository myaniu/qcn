#ifndef _QCN_POST_H_
#define _QCN_POST_H_

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
using std::string;
using std::vector;

#include "boinc_db.h"
#include "util.h"
#include "str_util.h"

extern DB_CONN boinc_db;
extern DB_CONN trigmem_db;

bool qcn_post_check();
bool qcn_post_setup();

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
    DB_QCN_HOST_IPADDR(DB_CONN* dc=0) :
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
