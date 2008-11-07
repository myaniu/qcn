/*
 *  qcn_util.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *
 */

#include "main.h"
#include "qcn_util.h"
#include "parse.h"  // BOINC parse functions

#ifndef ZIPARCHIVE
 #include "boinc_zip.h"
#endif

using std::string;

#ifdef GRAPHICS_PROGRAM
// this is in boinc/api/gutil.cpp - used for reading an RGB file as an alpha channel for QCN multitexture blending
extern unsigned * read_rgb_texture(const char *name, int *width, int *height, int *components);
#endif

namespace qcn_util 
{

char cPathSeparator()
{
// make it simple, only _WIN32 return \ path
//    return (sm && strchr((const char*) sm->strPathTrigger, '\\')) ? '\\' : '/';
#ifdef _WIN32
      return '\\';
#else
      return '/';
#endif
}

void FormatElapsedTime(const double& dTime, char* strFormat, int iLen)
{
    char *strTemp = new char[_MAX_PATH];
    double dRemain=dTime;
    int iDay=0, iHour=0, iMinute=0, iSecond=0, iMillisecond=0;
    memset(strFormat, 0x00, iLen);
    memset(strTemp, 0x00, _MAX_PATH);
    if (dRemain > 86400.0f) { // show days
        iDay = (int)(dRemain/86400.0f);
        dRemain -= (float)(iDay * 84600.0f);
    }
    if (dRemain > 3600.0f) { // show hours 
        iHour = (int)(dRemain/3600.0f);
        dRemain -= (float)(iHour * 3600.0f);
    }
    if (dRemain > 60.0f) { // show minute 
        iMinute = (int)(dRemain/60.0f);
        dRemain -= (float)(iMinute * 60.0f);
    }
    iSecond = (int) dRemain;
    dRemain -= (float)(iSecond);
    iMillisecond = (int)(dRemain * 100.0f);

    if (iDay)
       sprintf(strTemp, "%i:%02i:%02i:%02i.%02i", iDay, iHour, iMinute, iSecond, iMillisecond);
    else
       sprintf(strTemp, "%02i:%02i:%02i.%02i", iHour, iMinute, iSecond, iMillisecond);

    strlcpy(strFormat, strTemp, iLen); // copy no more than the max length
    delete [] strTemp;

}

float mod(float x1, float y1)
{
    // Determines the remainder of x1/y1
    float div11 = x1/y1;
    int   idiv1 = (int) div11;
    div11 = div11-(float)idiv1;
    return div11;
}

e_endian check_endian(void) {
  int x = 1;
  if(*(char *)&x == 1)
    return ENDIAN_LITTLE;
  else
    return ENDIAN_BIG;
}

void dtime_to_string(const double dtime, const char cType, char* strTime)
{ // this sets strTime to a character string of UTC time from the given double, strTime at least 26 chars!

    if (dtime <= 0) { // if time not set
        strcpy(strTime, "");
        return;
    }

    struct tm tmp;
    time_t tt = (time_t) dtime;  // note that this truncates the microseconds i.e. decimal part of dtime   
#ifdef _WIN32 
        gmtime_s(&tmp, &tt);  // note the "secure" Microsoft version flips the args around!
#else
        gmtime_r(&tt, &tmp);
#endif
   
    if (cType == 'd') // print out day
       sprintf(strTime, "%04d_%02d_%02d",
         tmp.tm_year+1900, tmp.tm_mon+1, tmp.tm_mday);
    else if (cType == 'a')  // print out all
       sprintf(strTime, "%02d/%02d/%04d %02d:%02d:%02d",
         tmp.tm_mon+1, tmp.tm_mday, tmp.tm_year+1900,
         tmp.tm_hour, tmp.tm_min, tmp.tm_sec);  // don't need microseconds! , dtime - (double) tt);
    else if (cType == 'h') // print out horu portion
       sprintf(strTime, "%02d:%02d:%02d",
         tmp.tm_hour, tmp.tm_min, tmp.tm_sec);  // don't need microseconds! , dtime - (double) tt);
}

// return time today 0:00 in seconds since 1970 as a double
//
double qcn_dday(double dNow) {
    return (dNow-fmod(dNow, SECONDS_PER_DAY));
}

#ifndef QCN_USB
void retrieveProjectPrefs(APP_INIT_DATA* pdataBOINC)
{
      boinc_parse_init_data_file(); // parse the file for BOINC
      boinc_get_init_data_p((APP_INIT_DATA*) pdataBOINC);
      if (pdataBOINC->project_preferences) { // copy proj prefs over to a separate struct since it is unstable in shared mem (malloc'd by BOINC)
         memset((char*) sm->strProjectPreferences, 0x00, sizeof(char) * MAX_PROJPREFS);
         strlcpy((char*) sm->strProjectPreferences, pdataBOINC->project_preferences, MAX_PROJPREFS);
         free(pdataBOINC->project_preferences); // explicitly free the boinc ref so it doesn't get memory leak
         pdataBOINC->project_preferences = NULL;
      }
}

void getBOINCInitData(const e_where eWhere)
{
    if (!sm) return;
    APP_INIT_DATA dataBOINC;  // useful BOINC user prefs, i.e. for graphics
    // just needed on startup and when we get proj prefs
    fprintf(stderr, "qcn_util::getBOINCInitData requested at %f\n", dtime());
    // workunit will never change during program
    if (eWhere == WHERE_MAIN_STARTUP)  { // only the main.cpp (i.e. not the thread) can set this
      sm->setTriggerLock();
      retrieveProjectPrefs(&dataBOINC);

      // set trigger path -- note sm->strPathTrigger will NOT have appropriate \ or / at the end
      // in main.cpp -- the program checks for and creates this path if it doesn't exist (i.e. ../../sac or ../../projects/qcn/triggers)
      char cTerm = dataBOINC.project_dir[strlen((const char*) dataBOINC.project_dir)-1];
      char cOK = 0x00;
      if (cTerm != '\\' && cTerm != '/') {
        // Look for Windows \ dir marker, if found set that in cTerm
        // note all strPath Trigger's will have \ or / at the end for convenience
        cTerm = strchr((const char*) dataBOINC.project_dir, '\\') ? '\\' : '/';
        cOK = cTerm; // for the first char below
      }
#ifndef QCNDEMO // just an odd quirk of the qcndemo program, bypass this section because of the refs to strPathTrigger qcndemo doesn't know about
      if (qcn_main::g_bDemo) {
#ifdef QCNLIVE
        sprintf((char*) qcn_main::g_strPathTrigger, "..%csac", qcn_util::cPathSeparator());
        sprintf((char*) sm->strPathImage, "..%cimages", qcn_util::cPathSeparator());
#else
        sprintf((char*) qcn_main::g_strPathTrigger,
           "..%c..%csac", cTerm, cTerm
        );
        sprintf((char*) sm->strPathImage,
           "..%c..%cimages", cTerm, cTerm
        );
#endif
      }
      else {
            // check if has a \ or / directory terminator character
        sprintf((char*) qcn_main::g_strPathTrigger,
           "%s%c%s",
           dataBOINC.project_dir,
                   cOK,
           DIR_TRIGGER
        );
        sprintf((char*) sm->strPathImage,
           "%s%cimages",
           dataBOINC.project_dir,
                   cOK
        );
      }

      // other important stuff to transfer from dataBOINC to sm-> now that project_prefs & img & trigger paths set
    strcpy(sm->user_name, dataBOINC.user_name);
    strcpy(sm->team_name, dataBOINC.team_name);
    strcpy(sm->project_dir, dataBOINC.project_dir);
    strcpy(sm->boinc_dir, dataBOINC.boinc_dir);
    strcpy(sm->wu_name, dataBOINC.wu_name);

    sm->userid = dataBOINC.userid;
    sm->hostid = dataBOINC.hostid;
    sm->teamid = dataBOINC.teamid;

      sm->releaseTriggerLock();
      fprintf(stdout, "Trigger SAC Files will be stored in \n%s\nand removed after a month.\n", qcn_main::g_strPathTrigger);
      fprintf(stdout, "Image Files will be stored in %s\n", sm->strPathImage);
      fflush(stdout);
#endif // ndef QCNDEMO
    }
    else if (eWhere == WHERE_MAIN_PROJPREFS) {  // just want to get new projprefs, if any
       sm->setTriggerLock();
       retrieveProjectPrefs(&dataBOINC);
       sm->releaseTriggerLock();
    }
}
#endif // QCN_USB - doesn't need boinc prefs update

#if (!defined(QCNDEMO) && !defined(QCN_USB) && !defined(GRAPHICS_PROGRAM)) || defined(QCNLIVE)

void ResetCounter(const e_where eWhere, const int iNumResetInitial)
{    // set some important values here

    if (qcn_main::g_iStop) return;   // check for stop signal

    fprintf(stdout, "qcn_util::ResetCounter(%d)\n", eWhere);
    fflush(stdout);

    if (eWhere == WHERE_MAIN_STARTUP) {
        sm->clear(true);   // clear out shared mem unless we read it in a file above!
    }

    // if in sm->bDemo mode, i.e. the 10-minute SAC dumps,
    // make sure triggers are all processed (10 seconds should be enough)
    double dTimeStart = dtime();
    bool bFinishedDemoTrigger = false;
    bool bHaveDemoTrigger = false;
    while (!qcn_main::g_vectTrigger.empty() && !bFinishedDemoTrigger && (dtime() - dTimeStart) < 10.0f)  {
       for (unsigned int i = 0; i < qcn_main::g_vectTrigger.size(); i++) {
         STriggerInfo& ti = qcn_main::g_vectTrigger.at(i);
         if (ti.bDemo) { // we're just concerned with demo triggers
            bHaveDemoTrigger = true;
            break;
         }
         bFinishedDemoTrigger = true;  // if we get to this line, no demo triggers left, so can split...
       }
       boinc_sleep(0.25f); // wait 1/4 second up to 10 seconds for triggers to empty
    } 
    if (bHaveDemoTrigger) fprintf(stdout, "Waited %f seconds for triggers to finish to disk\n", dtime() - dTimeStart);

    if (eWhere != WHERE_MAIN_STARTUP) {
       // we must have had a timing error, check the number of resets, if we broke 30 (SLUGGISH_MACHINE_THRESHOLD)
       // if not in startup, increment the reset counter and save to our prefs file
       // if ResetCounter not called from main startup, we're here due to an error - a time reset error or suspend/resume
       // if from a suspend/resume, we decremented iNumReset in the main loop, so it's incrementation here
       // "cancels out" and it's not really an increase in our error count
       if (qcn_main::g_iStop) { // don't count if stopping, as that could have triggered resetcounter
         fprintf(stderr, "qcn_util::ResetCounter(%d) - qcn_main::g_iStop signal caught, not incrementing sm->iNumReset (%d) at %f\n", 
            (int) eWhere, sm->iNumReset, dtime());
       }
       else {
         sm->iNumReset++; // this is the one and only place iNumReset gets incremented (it can get decremented in qcn_main on a "false alarm" i.e. BOINC suspend
         fprintf(stderr, "qcn_util::ResetCounter(%d) - Incrementing iNumReset to %d at %f\n", (int) eWhere, sm->iNumReset, dtime());
       }

       qcn_util::set_qcn_counter();  // save our current counters

       sm->clear();   // clear out shared mem unless we read it in a file above!  it saves important vars such as iNumReset though

       // bump up the DT value (.1 vs .02) -- note MAXI now holds 4.2 hours of time not just 1 hour!
       if ((sm->iNumReset - iNumResetInitial) > SLUGGISH_MACHINE_THRESHOLD) 
         sm->dt = g_DT_SLOW;  // too many failures this session (10), try a lower dt
       else
         sm->dt = g_DT;
    }
    sm->lOffset = 0;              //  LOOP BACK THRU DATA SERIES
    sm->iWindow = (int) (g_cfTimeWindow / (sm->dt > 0 ? sm->dt : g_DT));  // number of points in time window

    // this resets the t0check & t0active, call right before you start accessing the sensor for mean_xyz
    sm->resetSampleClock();   

    qcn_main::g_vectTrigger.clear(); // erase all trigger elements if any

    // shared memory setup, so get the init data into the shared mem struct
    // never call this from the thread
    getBOINCInitData(eWhere); 

    // get the trigger counters etc
    qcn_util::get_qcn_counter();

    // we're restarting the array so set wrapped to off
    //sm->bWrapped = false;

    // set flag to get the time sync with server
    //sm->bTimeCheck = true;
}

void getLastTrigger(double& dTime, long& lTime)
{
    int iOpen;
    for (iOpen = MAX_TRIGGER_LAST-1; iOpen >=0; iOpen--)  {
        if (sm->lTriggerLastOffset[iOpen] != 0L) break; // this is a valid one
    }
    if (iOpen == -1)  { // couldn't find, ergo no trigger
       dTime = 0.0f;
       lTime = 0L;
    }
    else {  // last trigger is at iOpen
       dTime = sm->dTriggerLastTime[iOpen];
       lTime = sm->lTriggerLastOffset[iOpen];
    }
}

void setLastTrigger(const double dTime, const long lTime)
{  // set trigger at the end of the array, pushout out beginning element if necessary
    int iOpen;
    for (iOpen = 0; iOpen < MAX_TRIGGER_LAST; iOpen++) {
        if (sm->lTriggerLastOffset[iOpen] == 0L) break;
    }
    if (iOpen == MAX_TRIGGER_LAST) { // we ran out of slots and have to shift the array down and then set this at MAX_TIME_ARRAY-1
        for (iOpen = 0; iOpen < MAX_TRIGGER_LAST - 1; iOpen++) {
          sm->lTriggerLastOffset[iOpen] = sm->lTriggerLastOffset[iOpen+1];
          sm->dTriggerLastTime[iOpen]   = sm->dTriggerLastTime[iOpen+1];
        }
        sm->lTriggerLastOffset[MAX_TRIGGER_LAST-1] = lTime;
        sm->dTriggerLastTime[MAX_TRIGGER_LAST-1]   = dTime;
    }
    else { // we have an open slot at iOpen
        sm->lTriggerLastOffset[iOpen] = lTime;
        sm->dTriggerLastTime[iOpen]   = dTime;
    }
}

// get rid of trigger files over two weeks old
void removeOldTriggers(const char* strPathTrigger)
{
    // get a list of all files in strPathTrigger
    long lTimeDelete, lTimeTest;
    double dTimeCurrent;
    std::string::size_type locUnderscore, locPeriod;

    dTimeCurrent = dtime();
    //getTimeOffset((const double*) sm->dTimeServerTime, (const double*) sm->dTimeServerOffset, dTimeCurrent, dTimeOffset, dTimeOffsetTime);
    lTimeDelete = (long) (dTimeCurrent - TIME_FILE_DELETE + qcn_main::g_dTimeOffset); // delete files with a timestamp older than two weeks

    ZipFileList zfl;

#ifdef ZIPARCHIVE
    boinc_filelist(strPathTrigger, "", &zfl); //, SORT_NAME | SORT_ASCENDING, true); // this makes a vector of strings of filenames
#else
    boinc_filelist(strPathTrigger, "", &zfl, SORT_NAME | SORT_ASCENDING, true); // this makes a vector of strings of filenames
#endif
    for (unsigned int i = 0; i < zfl.size(); i++)
    {
        // just do an rfind to find the last _, the digits between that and . are the adjusted timestamp, if less than lTimeDelete, erase file
        locUnderscore = zfl[i].rfind('_');
        locPeriod     = zfl[i].rfind('.');
        if (locUnderscore == std::string::npos || locPeriod == std::string::npos || locUnderscore >= locPeriod) continue; // invalid values

        // the time is the bit between the _ and .
        lTimeTest = atol( zfl[i].substr( locUnderscore + 1, locPeriod - locUnderscore - 1).c_str() );

#ifdef _DEBUG
        fprintf(stdout, "File %04d=%s  lTimeDelete=%ld  lTimeTest=%ld  chartime=%s\n", 
          i+1, zfl[i].c_str(), lTimeDelete, lTimeTest, zfl[i].substr( locUnderscore + 1, locPeriod - locUnderscore - 1).c_str());
        fflush(stdout);
#endif

        if (lTimeTest <= lTimeDelete)  { // delete this file!
            fprintf(stdout, "Deleting old trigger file %s\n", zfl[i].c_str());
            fflush(stdout);
            boinc_delete_file(zfl[i].c_str());
        }
    }
}

bool set_trigger_file(char* strTrigger, const char* strWU, const int iTrigger, const long lTime, bool bReal)
{
     if (bReal) {
        sprintf(strTrigger, "%s_%06d_%ld.zip", strWU, iTrigger, lTime);
     }
     else  {// add subdirectory of trig number date for demo mode
        char strDir[_MAX_PATH], strDay[16];
        dtime_to_string((double) lTime, 'd', strDay);
        sprintf(strDir, "%s%c%s", 
           (const char*) qcn_main::g_strPathTrigger, 
           qcn_util::cPathSeparator(),
           strDay
        );
        // need full path without trailing / or \ to make the trigger dir
        if (! boinc_file_exists((const char*) strDir) ) {
           // now open dir to see if exists
           if (boinc_mkdir((const char*) strDir)) {
              // OK, now it's a fatal error     
              fprintf(stderr, "QCN exiting, can't make directory %s\n", strDir);
              fprintf(stdout, "QCN exiting, can't make directory %s\n", strDir);
              fflush(stdout); 
              return false;
           }
        }
        sprintf(strDir, "%s%c", 
           strDay,
           qcn_util::cPathSeparator()
        );
        sprintf(strTrigger, "%s%s_%06d_%ld.zip", 
            strDir, 
            strWU, iTrigger, lTime);
     }
     return true;
}

/*
void getTimeOffset(const double* dTimeServerTime, const double* dTimeServerOffset, const double t0, double& dTimeOffset, double& dTimeOffsetTime)
{
   // t[j] = sm->t0[lOff];
   // CMC note:  we have to add on the offset for this particular time -- stored in shared mem as an array of 10 w/ corresponding time
   //    double dTimeServerOffset[10];  // the time offset between client & server, +/- in seconds difference from server
   //    double dTimeServerTime[10];    // the (unadjusted client) time this element was retrieved
   dTimeOffset = 0.0f;
   dTimeOffsetTime = 0.0f;
   // the time offset array is in shared memory and in order of time, i.e. 0 is the earliest sync, MAX_TIME_ARRAY-1 is the latest sync (if all values used)
   for (int i = MAX_TIME_ARRAY-1; i >=0; i--)  {
     if (dTimeServerTime[i] > 0.0f && (dTimeServerTime[i] < t0 || i==0)) {
       // the latest time is less than this point or is the only entry (startup sync), so use this offset value and break out of loop
       dTimeOffset = dTimeServerOffset[i];
       dTimeOffsetTime = dTimeServerTime[i];
       break;
     }
   }
}
*/

bool get_qcn_counter()
{  
    if (!sm) return false;
    // read in the saved trigger count
    FILE* fp; 
    char strRead[_MAX_PATH], strSearch[16];
    memset(strRead, 0x00, _MAX_PATH);

    if (!boinc_file_exists(XML_PREFS_FILE)) return false; // don't bother if doesn't exist!

    if ( (fp = fopen(XML_PREFS_FILE, "r")) == NULL) {
       fprintf(stdout, "Error opening file %s\n", XML_PREFS_FILE);
       return false;
    }
    fread(strRead, sizeof(char), _MAX_PATH, fp);
    fclose(fp);
  
    sprintf(strSearch, "<%s>", XML_TRIGGER_COUNT);
    if (!parse_int(strRead, strSearch, (int&) sm->iNumTrigger))
        sm->iNumTrigger = 0;

    sprintf(strSearch, "<%s>", XML_UPLOAD_COUNT);
    if (!parse_int(strRead, strSearch, (int&) sm->iNumUpload))
        sm->iNumUpload = 0;

    sprintf(strSearch, "<%s>", XML_RESET_COUNT);
    if (!parse_int(strRead, strSearch, (int&) sm->iNumReset))
        sm->iNumReset = 0;

    sprintf(strSearch, "<%s>", XML_CLOCK_TIME);
    if (!parse_double(strRead, strSearch, (double&) sm->clock_time))
        sm->clock_time = 0.0f;

    sprintf(strSearch, "<%s>", XML_CPU_TIME);
    if (!parse_double(strRead, strSearch, (double&) sm->cpu_time))
        sm->cpu_time = 0.0f;
    return true;
}

bool set_qcn_counter()
{
    // write out the trigger count
    // if trigger count is too high (>9999999), i.e. sac field only holds 7 chars, then reset
    if (sm->iNumTrigger > 9999999L) sm->iNumTrigger = 0L;

    FILE* fp; 
    if ( (fp = fopen(XML_PREFS_FILE, "w")) == NULL) {
       fprintf(stdout, "Error opening file %s\n", XML_PREFS_FILE);
       return false;
    }
    boinc_begin_critical_section();
    fprintf(fp, "<wu>\n"
      "<%s>%d</%s>\n"
      "<%s>%d</%s>\n"
      "<%s>%d</%s>\n"
      "<%s>%.2f</%s>\n"
      "<%s>%.2f</%s>\n"
     "</wu>\n", 
        XML_TRIGGER_COUNT, sm->iNumTrigger, XML_TRIGGER_COUNT,
        XML_UPLOAD_COUNT, sm->iNumUpload, XML_UPLOAD_COUNT,
        XML_RESET_COUNT, sm->iNumReset, XML_RESET_COUNT,
        XML_CLOCK_TIME, sm->clock_time, XML_CLOCK_TIME,
        XML_CPU_TIME, sm->cpu_time, XML_CPU_TIME
    );

    fclose(fp);
    boinc_end_critical_section();
    return true;
}

#endif // if !defined(GUI) & !defined(GRAPHICS_PROGRAM)

#ifdef GRAPHICS_PROGRAM

// screenshot grab found on the web, modified by CMC 

bool ScreenshotJPG(const unsigned int iWidth, const unsigned int iHeight, const char *strFile, const int iQuality) // note quality defaults to 90 in header
{
         bool ret=false;
         struct jpeg_compress_struct cinfo; // the JPEG OBJECT
         struct jpeg_error_mgr jerr; // error handler struct
         unsigned char *row_pointer[1]; // pointer to JSAMPLE row[s]
         GLubyte *pixels=0, *flip=0;
         FILE* shot;
         int row_stride; // iWidth of row in image buffer
         if((shot=fopen(strFile, "wb"))!=NULL) { // jpeg file
            // initialization
            cinfo.err = jpeg_std_error(&jerr); // error handler
            jpeg_create_compress(&cinfo); // compression object
            jpeg_stdio_dest(&cinfo, shot); // tie stdio object to JPEG object
            row_stride = iWidth * 3;
       
            pixels = new GLubyte[iWidth*iHeight*3];  //(GLubyte *)malloc(sizeof(GLubyte)*iWidth*iHeight*3);
            flip   = new GLubyte[iWidth*iHeight*3];  //(GLubyte *)malloc(sizeof(GLubyte)*iWidth*iHeight*3);
       
            if (pixels && flip) {
               // save the screen shot into the buffer
               //glReadBuffer(GL_FRONT_LEFT);
               glPixelStorei(GL_PACK_ALIGNMENT, 1);
               glReadPixels(0, 0, iWidth, iHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
       
               // give some specifications about the image to save to libjpeg
               cinfo.image_width = iWidth;
               cinfo.image_height = iHeight;
               cinfo.input_components = 3; // 3 for R, G, B
               cinfo.in_color_space = JCS_RGB; // type of image
       
               jpeg_set_defaults(&cinfo);
               jpeg_set_quality(&cinfo, iQuality, TRUE);
               jpeg_start_compress(&cinfo, TRUE);
       
               // OpenGL writes from bottom to top.
               // libjpeg goes from top to bottom.
               // flip lines.
               for (unsigned int y=0;y<iHeight;y++) {
                  for (unsigned int x=0;x<iWidth;x++) {
                     flip[(y*iWidth+x)*3] = pixels[((iHeight-1-y)*iWidth+x)*3];
                     flip[(y*iWidth+x)*3+1] = pixels[((iHeight-1-y)*iWidth+x)*3+1];
                     flip[(y*iWidth+x)*3+2] = pixels[((iHeight-1-y)*iWidth+x)*3+2];
                  }
               }
               // write the lines
               while (cinfo.next_scanline < cinfo.image_height) {
                  row_pointer[0] = &flip[cinfo.next_scanline * row_stride];
                  jpeg_write_scanlines(&cinfo, row_pointer, 1);
               }
       
               ret=true;
               // finish up and free resources
               jpeg_finish_compress(&cinfo);
               jpeg_destroy_compress(&cinfo);
            }
            fclose(shot);
         }
         
         if (pixels)
            delete [] pixels;

         if (flip)
            delete [] flip;
       
         return ret;
}

#if 0  // not using right now

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

int CreateTextureJPG(const char* strFileName, qcn_util::tImageJPG** ppImage) 
{
	if(!strFileName) return -1;
    *ppImage = LoadJPG(strFileName);			// Load the image and store the data
	if (ppImage == NULL || *ppImage == NULL) return -1;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        GLuint* pglui = &((*ppImage)->id);
	glGenTextures(1, (GLuint*) pglui);
	glBindTexture(GL_TEXTURE_2D, (*ppImage)->id);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, (*ppImage)->sizeX, (*ppImage)->sizeY, GL_RGB, GL_UNSIGNED_BYTE, (*ppImage)->data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

/* CMC - we want to keep this around
	if (pImage) {
		if (pImage->data) {
			free(pImage->data);
		}
		free(pImage);
	}
*/
    return 0;
}

qcn_util::tImageJPG* LoadJPG(const char* filename) 
{
	struct jpeg_decompress_struct cinfo;
    qcn_util::tImageJPG *pImageData = NULL;
	FILE *pFile;
#if defined(HAVE_ALLOCA) || defined(_WIN32)
	alloca(16);  // Force a frame pointer even when compiled with 
                 // -fomit-frame-pointer
#endif

	if((pFile = boinc_fopen(filename, "rb")) == NULL) {
		fprintf(stderr,"Unable to load JPG File %s\n", filename);
		return NULL;
	}


    struct qcn_util::my_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = qcn_util::my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
       jpeg_destroy_decompress(&cinfo);
       fclose(pFile);
       return NULL;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, pFile);
    pImageData = (qcn_util::tImageJPG*)malloc(sizeof(qcn_util::tImageJPG));
        if (!pImageData) {
           jpeg_destroy_decompress(&cinfo);
           fclose(pFile);
           fprintf(stderr, "out of mem in LoadJPG");
           return NULL;
        }
	DecodeJPG(&cinfo, pImageData);
	jpeg_destroy_decompress(&cinfo);
	fclose(pFile);
	return pImageData;
}

void DecodeJPG(jpeg_decompress_struct* cinfo, qcn_util::tImageJPG *pImageData) 
{
	jpeg_read_header(cinfo, TRUE);
	jpeg_start_decompress(cinfo);
	//int rem = cinfo->output_width%4;
	pImageData->rowSpan = cinfo->output_width * cinfo->output_components;
	pImageData->sizeX   = cinfo->output_width;
	pImageData->sizeY   = cinfo->output_height;

	pImageData->data = new unsigned char[pImageData->rowSpan * pImageData->sizeY];

	unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];
	for (int i = 0; i < pImageData->sizeY; i++)
		rowPtr[i] = &(pImageData->data[i*pImageData->rowSpan]);

	int rowsRead = 0;

	while (cinfo->output_scanline < cinfo->output_height) {
		rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}
	delete [] rowPtr;
	jpeg_finish_decompress(cinfo);
}

#endif // not using some of these right now, just use the ones in gutil.cpp

// based on boinc/api/gutil.cpp -- but makes a GL_ALPHA out of an RGB file (sums values each point to make the alpha)
// the basic idea is you can make a simple image that can translate to a complex map -- white values (1) "pass" and black values (0) "block"
GLuint CreateRGBAlpha(const char* strFileName) 
{
        if(!strFileName) return 0;
        GLuint uiTexture = 0;
        int sizeX;
        int sizeY;
        int sizeZ;
        // Load the image and store the data
        unsigned int *pImage = read_rgb_texture(strFileName,&sizeX,&sizeY,&sizeZ);
        if(pImage == NULL) return 0;
        if (sizeZ > 1) { // error - just want a 1-D for alpha levels
            fprintf(stderr, "Improper RGB Image for Alpha: %s needs just 1 level, this file has %d\n", strFileName, sizeZ);
            free(pImage);
            return 0;
        }
        unsigned char* pByte = new unsigned char[sizeX*sizeY*sizeZ];
        memset(pByte, 0x00, sizeX*sizeY*sizeZ);
        for (int i = 0 ; i < (sizeX*sizeY*sizeZ) ; i++)  {
            pByte[i] = (unsigned char) *(pImage+i) & ~0xffffff00;   // get the final char from masking the higher bits
        }
        free(pImage); // don't need the image data, may as well free it

        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glGenTextures(1, &uiTexture);
        glBindTexture(GL_TEXTURE_2D, uiTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, sizeX, sizeY, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pByte);

        //gluBuild2DMipmaps(GL_TEXTURE_2D, 3, sizeX, sizeY, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        delete [] pByte;  // free the temporary byte array
        return uiTexture;
}

#endif  // GRAPHICS_PROGRAM


// boinc_filelist is defined in boinc_zip
#ifdef ZIPARCHIVE  
// -------------------------------------------------------------------
//
// Function: bool boinc_filelist(const std::string directory
//                                              const std::string pattern,
//                                              ZipFileList* pList)
//
// Description: Supply a directory and a pattern as arguments, along
//              with a FileList variable to hold the result list; sorted by name.
//              Returns a vector of files in the directory which match the
//              pattern.  Returns true for success, or false if there was a problem.
//
// CMC Note: this is a 'BOINC-friendly' implementation of "old" CPDN code
//           the "wildcard matching"/regexp is a bit crude, it matches substrings in 
//           order in a file; to match all files such as *.pc.*.x4.*.nc" you would send in
//                       ".pc|.x4.|.nc" for 'pattern'
//
// --------------------------------------------------------------------

bool boinc_filelist(const std::string directory,
                  const std::string pattern,
                  ZipFileList* pList,
                  const unsigned char ucSort, const bool bClear)
{	
        std::string strFile;
        // at most three |'s may be passed in pattern match
        int iPos[3], iFnd, iCtr, i, lastPos;  
        std::string strFullPath;
        char strPart[3][32];
        std::string spattern = pattern;
        std::string strDir = directory;
        std::string strUserDir = directory;
        int iLen = strUserDir.size();

    if (!pList) return false;

        // wildcards are blank!
        if (pattern == "*" || pattern == "*.*") spattern.assign("");

        if (bClear)
                pList->clear();  // removes old entries that may be in pList

        // first tack on a final slash on user dir if required
        if (strUserDir[iLen-1] != '\\' 
                && strUserDir[iLen] != '/')
        {
                // need a final slash, but what type?
                // / is safe on all OS's for CPDN at least
                // but if they already used \ use that
                // well they didn't use a backslash so just use a slash
                if (strUserDir.find("\\") == string::npos)
                        strUserDir += "/";
                else
                        strUserDir += "\\";
        }

        // transform strDir to either all \\ or all /
        int j;
        for (j=0; j<(int)directory.size(); j++)  {
                // take off final / or backslash
                if (j == ((int)directory.size()-1) 
                 && (strDir[j] == '/' || strDir[j]=='\\'))
                        strDir.resize(directory.size()-1);
                else {
#ifdef _WIN32  // transform paths appropriate for OS
           if (directory[j] == '/')
                                strDir[j] = '\\';
#else
           if (directory[j] == '\\')
                                strDir[j] = '/';
#endif
                }
        }

        DirScanner dirscan(strDir);
        memset(strPart, 0x00, 3*32);
        while (dirscan.scan(strFile))
        {
                iCtr = 0;
                lastPos = 0;
                iPos[0] = -1;
                iPos[1] = -1;
                iPos[2] = -1;
                // match the whole filename returned against the regexp to see if it's a hit
                // first get all the |'s to get the pieces to verify
                while (iCtr<3 && (iPos[iCtr] = (int) spattern.find('|', lastPos)) > -1)
                {
                        if (iCtr==0)  {
                                strncpy(strPart[0], spattern.c_str(), iPos[iCtr]);
                        }
                        else  {
                                strncpy(strPart[iCtr], spattern.c_str()+lastPos, iPos[iCtr]-lastPos);
                        }
                        lastPos = iPos[iCtr]+1;

                        iCtr++;
                }
                if (iCtr>0)  // found a | so need to get the part from lastpos onward
                {
                        strncpy(strPart[iCtr], spattern.c_str()+lastPos, spattern.length() - lastPos);
                }

                // check no | were found at all
                if (iCtr == 0)
                {
                        strcpy(strPart[0], spattern.c_str());
                        iCtr++; // fake iCtr up 1 to get in the loop below
                }

                bool bFound = true;
                for (i = 0; i <= iCtr && bFound; i++)
                {
                        if (i==0)  {
                                iFnd = (int) strFile.find(strPart[0]);
                                bFound = (bool) (iFnd > -1);
                        }
                        else  {
                                // search forward of the old part found
                                iFnd = (int) strFile.find(strPart[i], iFnd+1);
                                bFound = bFound && (bool) (iFnd > -1);
                        }
                }

                if (bFound)
                {
                        // this pattern matched the file, add to vector
                        // NB: first get stat to make sure it really is a file
                        strFullPath = strUserDir + strFile;
                        // only add if the file really exists (i.e. not a directory)
                        if (is_file(strFullPath.c_str())) {
                                pList->push_back(strFullPath);
                        }
                }

        }

    // sort by file creation time
    //if (pList->size()>1)  { // sort if list is greather than 1
    //   std::sort(pList->begin(), pList->end(), StringVectorSort);  // may as well sort it?
    //}
    return true;
}
#endif //ZIPARCHIVE

} // namespace util
