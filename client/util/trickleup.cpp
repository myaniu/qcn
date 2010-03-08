
#ifdef _WIN32
   #include <windows.h>
   #include "config.h.win"
#else
   #include "config.h"
#endif

#include "trickleup.h"
#include "filesys.h"

namespace trickleup {

void qcnTrickleUp(const char* strTrickle, const int iVariety, const char* strWU)
{
#ifdef QCNLIVE
  return; // no trickles on gui!
#else

enum e_trigvariety { TRIGGER_VARIETY_FINALSTATS = -2, TRIGGER_VARIETY_QUAKELIST, TRIGGER_VARIETY_NORMAL, TRIGGER_VARIETY_PING, TRIGGER_VARIETY_CONTINUAL };

   char strVariety[32];
   memset(strVariety, 0x00, 32);
   strcpy(strVariety, "trigger"); break;
   switch (iVariety) {
      case TRIGGER_VARIETY_FINALSTATS:
        strcpy(strVariety, "finalstats"); break;
      case TRIGGER_VARIETY_QUAKELIST:
        strcpy(strVariety, "quakelist"); break;
      case VARIETY_TRIGGER_NORMAL:
        strcpy(strVariety, "trigger"); break;
      case TRIGGER_VARIETY_PING:
        strcpy(strVariety, "ping"); break;
      case TRIGGER_VARIETY_CONTINUAL:
        strcpy(strVariety, "continual"); break;
   }


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

