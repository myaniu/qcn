#ifndef _UTIL_H_
#define _UTIL_H_
/*
 *  main.h -- main header file for the Quake Catcher Network
 *  qcn
 *
 *  Created by Carl Christensen on 08/11/2007.
 *  Copyright 2007 Stanford University. All rights reserved.
 *
 */

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "define.h"

#ifdef __APPLE_CC__
  #include <IOKit/IOKitLib.h>
  #include <CoreFoundation/CoreFoundation.h>
#endif

/*
#ifdef QCNLIVE   // get wxRect def
#include <wx/gdicmn.h>
#endif
*/

namespace qcn_util 
{

// useful functions for all
void FormatElapsedTime(const double& dTime, char* strFormat, int iLen);
char cPathSeparator(); // returns path sep, i.e. for strPathTrigger
void dtime_to_string(const double dtime, const char cType, char* strTime);
double qcn_dday(double dNow);
void string_tidy(char* strIn, int length);
e_endian check_endian(void);
float mod(float x1, float y1);

// useful functions just for the main program & qcnlive
#ifndef QCN_USB
void getBOINCInitData(const e_where eWhere);
#endif

#if (!defined(QCNDEMO) && !defined(QCN_USB) && !defined(GRAPHICS_PROGRAM)) || defined(QCNLIVE)
void ResetCounter(const e_where eWhere, const int iNumResetInitial = -1);
void removeOldTriggers(const char* strPathTrigger);
//void getTimeOffset(const double* dTimeServerTime, const double* dTimeServerOffset, const double t0, double& dTimeOffset, double& dTimeOffsetTime);
bool set_trigger_file(char* strTrigger, const char* strWU, const int iTrigger, const long lTime, bool bReal);
bool get_qcn_counter();
bool set_qcn_counter();
void setLastTrigger(const double dTime, const long lTime);
void getLastTrigger(double& dTime, long& lTime);
void sendIntermediateUpload(std::string strLogicalName, std::string strFullPath);
#endif

// useful functions for graphics
#ifdef GRAPHICS_PROGRAM
  // mostly JPEG utils, some taken from the boinc/api/gutil.cpp 
  #undef HAVE_STDLIB_H  // this is (re)defined in jpeglib.h below
  extern "C" {
          #include "jpeglib.h"
	}

#if 0
  #define _CSETJMP_  // bypass the csetjmp
  #include <setjmp.h>

  struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
  };

  typedef struct my_error_mgr * my_error_ptr;

  struct tImageJPG {
        GLuint id;
	    int rowSpan;
	    int sizeX;
	    int sizeY;
	    unsigned char *data;
  };

  void DecodeJPG(jpeg_decompress_struct* cinfo, qcn_util::tImageJPG *pImageData); 
  qcn_util::tImageJPG* LoadJPG(const char* filename); 
  int CreateTextureJPG(const char* strFileName, qcn_util::tImageJPG** ppImage);
#endif // not using now
  bool ScreenshotJPG(const unsigned int iWidth, const unsigned int iHeight, const char *strFile, const int iQuality = 90);
  GLuint CreateRGBAlpha(const char* strFileName);
#endif // graphics prog

#ifdef ZIPARCHIVE
 bool boinc_filelist(const std::string directory,
                  const std::string pattern,
                  ZipFileList* pList,
                  const unsigned char ucSort = 0x00, const bool bClear = true);
#endif
}

#endif //_UTIL_H_
