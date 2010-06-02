/*

   this will check for demo or other triggers that need to be posted back to a server

*/

#include "qcn_post.h"

static vector<DB_QCN_POST> vQCN_Post;
static long g_curlBytes = 0L;

#define XML_FORMAT \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
  "<Quake>\n" \
  "<QuakeMagnitude>%f</QuakeMagnitude>\n" \
  "<QuakeLon>%f</QuakeLon>\n" \
  "<QuakeLat>%f</QuakeLat>\n" \
  "<QuakeDepth>%f</QuakeDepth>\n" \
  "<QuakeOriginTime>%f</QuakeOriginTime>\n" \
  "<QuakeID>%d</QuakeID>\n" \
  "</Quake>\n\n"

// for curl post
struct WriteThis {
  const char *readptr;
  int sizeleft;
};

// setup the vector
// just call qcn_post_check when qcn_trigmon is starting up to get a list of servers to post to (if any)
bool qcn_post_setup()
{  // databases should be open by now, so get fill vector with qcn_post entries
   DB_QCN_POST qp;
   vQCN_Post.clear();
   while (!qp.enumerate("WHERE active=1")) {  // active=1 means it is a server record we want to use i.e. post XML to
      vQCN_Post.push_back(qp);
   } 
   log_messages.printf(MSG_DEBUG, "%d Server(s) to Post XML triggers to\n", (int) vQCN_Post.size());
   return true;
}

// ideally this should be a multithreaded asychronous curl post to the servers,
// and set the post state in qcn_trigger_memory (posted = 1)
bool qcn_post_check()
{
   if (!vQCN_Post.size()) return true;

   vector<DB_QCN_POST>::iterator it;
   for (it = vQCN_Post.begin(); it < vQCN_Post.end(); it++) {
      // enumerate through our vector of qcn_post entries (i.e. servers we should send XML Post triggers to)
     // try the where statement
        DB_QCN_TRIGGER_MEMORY qtm;
        char strWhere[284], strUpdate[256];
        qtm.clear();
        if (!it->where_clause || !it->url) continue;  // go to next record if no where_clause for some reason
        sprintf(strWhere, "WHERE posted=0 AND (%s)", it->where_clause);
        while (!qtm.enumerate(strWhere)) { 
           // OK we have all the unused info and must have had a match

           // CMC: note this isn't thread-safe in that we want an asynchronous HTTP POST 
           //      to the server URL but we're setting it posted=1 here -- if the HTTP
           //      fails they will never get the message, also the trigmem.qcn_trigger_memory
           //      only has a lifetime of a few minutes so they may never get the message past
           //      that time on a retry anyway (which may be alright if we just care about 
           //      "instant messages"

            qcn_post_xml_http(qtm);  // send the XML to the server in question

            // mark this record as processed (note the db_name & triggerid required)
            sprintf(strUpdate, "UPDATE trigmem.qcn_trigger_memory "
                                  "SET posted=1 "
                                  "WHERE db_name='%s' AND triggerid=%d",
                        qtm.db_name, qtm.triggerid);

            int tret = trigmem_db.do_query(strUpdate);
            if (tret) { // on error print a message and return false
                  log_messages.printf(MSG_CRITICAL,
                    "do_trigmon() strUpdate error: %s - %s\n", strUpdate, boincerror(tret)
                  );
                  return false;
            }
        }
    }
    return true; // processed OK
}

bool qcn_post_xml_http(const DB_QCN_TRIGGER_MEMORY& qtm)
{
    char strXML[512];
    // provide magnitude, longitude, latitude, depth_km, time_trigger, qcn_quakeid
    
    sprintf(strXML, XML_FORMAT, qtm.magnitude, qtm.longitude, qtm.latitude, 0.0f, qtm.time_trigger, rand());

    log_messages.printf(MSG_DEBUG,
          strXML
    );

    return true;
}


// CMC - as noted above, eventually do this as async curl calls
bool qcn_post_curl(const char* strURL, char* strPost, const int iLenPost)
{
   // easycurl should be fine, just send a request to maxmind (strURL has the key & ip etc),
   CURLcode cc;
   struct WriteThis wt;
 
   wt.readptr = strPost;
   wt.sizeleft = strlen(strPost);

   CURL* curlHandle = curl_easy_init();

   g_curlBytes = 0L;  // reset long num of curl bytes read
   long lResponse = 0L;

   if (!curlHandle) return false;  // problem with init

   cc = curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 0L);
   cc = curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1L);
   cc = curl_easy_setopt(curlHandle, CURLOPT_URL, strURL);
   //cc = curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, strReply);
   //cc = curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, qcn_post_curl_write_data);
   cc = curl_easy_setopt(curlHandle, CURLOPT_READDATA, &wt);
   cc = curl_easy_setopt(curlHandle, CURLOPT_READFUNCTION, qcn_post_curl_read_data);
   cc = curl_easy_setopt(curlHandle, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);
   cc = curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, strlen(strPost));
   cc = curl_easy_setopt(curlHandle, CURLOPT_POST, 1L);

   cc = curl_easy_perform(curlHandle);

   // get response code, should be 200
   curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &lResponse);

   curl_easy_cleanup(curlHandle);

   return (bool) (cc == 0 && lResponse == 200);  // 0 is good CURLcode
}

size_t qcn_post_curl_read_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
/*
   int iLeft = BYTESIZE_CURL - g_curlBytes - 1;
   if (iLeft > 0 && size > 0) { // we have some room left to write
      strlcat((char*) stream, (char*) ptr, BYTESIZE_CURL);
   }
   g_curlBytes += (size * nmemb);
   return size * nmemb;
*/
  struct WriteThis *pwt= (struct WriteThis *)stream;
 
  if(size*nmemb < 1)
    return 0;
 
  if(pwt->sizeleft) {
    *(char *)ptr = pwt->readptr[0]; /* copy one single byte */ 
    pwt->readptr++;                 /* advance pointer */ 
    pwt->sizeleft--;                /* less data left */ 
    return 1;                        /* we return 1 byte at a time! */ 
  }
 
  return 0;                          /* no more data left to deliver */ 

}

/*
size_t qcn_post_curl_write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
   int iLeft = BYTESIZE_CURL - g_curlBytes - 1;
   if (iLeft > 0 && size > 0) { // we have some room left to write
      strlcat((char*) stream, (char*) ptr, BYTESIZE_CURL);
   }
   g_curlBytes += (size * nmemb);
   return size * nmemb;
}
*/


