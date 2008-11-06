
#ifdef _WIN32
   #include <windows.h>
   #include "config.h.win"
#else
   #include "config.h"
#endif

#include "trickleup.h"
#include "filesys.h"

namespace trickleup {

void qcnTrickleUp(const char* strTrickle, const char* strVariety, const char* strWU)
{
#ifdef QCNLIVE
  return; // no trickles on gui!
#else
        // BOINC adds the appropriate workunit/resultid etc and posts to trickle_up table in MySQL
        static bool bInHere = false;
        if (bInHere) return;
        bInHere = true;

        // CMC let's print out trickles in standalone mode so I can see something!
        if (boinc_is_standalone()) {
           char szTFile[32];
           static unsigned long iNum = 0L;
           iNum++;
           sprintf(szTFile, "trickle_%09lu_%s.xml", (unsigned long) iNum, strVariety);
           FILE* fTrickle = boinc_fopen(szTFile, "w");
           if (fTrickle) {
             fwrite(strTrickle, 1, strlen(strTrickle), fTrickle);
             fclose(fTrickle);
           }
        }
        else {
           boinc_send_trickle_up((char*) strVariety, (char*) strTrickle);
        }
        bInHere = false;
#endif
}

} // namespace trickleup

