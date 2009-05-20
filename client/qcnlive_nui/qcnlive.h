/*
 *  qcnlive.h
 *  qcnwx
 *
 *  Created by Carl Christensen on 2/9/08.
 *  Copyright 2008 Stanford U All rights reserved.
 *
 */
 
#ifndef _QCNLIVE_H_
#define _QCNLIVE_H_

/*
// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/aboutdlg.h>
#include <wx/image.h>
#include <wx/splash.h>

#include <wx/glcanvas.h>
#include <wx/timer.h>
#include <wx/frame.h>
#include <wx/dialog.h>
#include <wx/valtext.h>
*/

#ifndef _WIN32
#include <unistd.h>
#endif

#include "main.h"
#include "qcn_graphics.h"
#include "qcn_curl.h"
#include "myglpane.h"
#include "myframe.h"
#include "qcnlive_define.h"

class MyApp; // defined below
class MyAppTimer; // defined below

// definition for the main app
class MyApp: public wxApp
{
  private:
    virtual bool OnInit();
    virtual int OnExit();

    MyFrame* frame;
	MyAppTimer* myapptimer;
#if wxUSE_LIBPNG
	wxPNGHandler* myPNGHandler;
#endif
#if wxUSE_LIBJPEG
	wxJPEGHandler* myJPEGHandler;
#endif

    wxRect myRect;            // apps screen coordinates

  public:
    //void SetRect(const wxSize& newsize, const wxPoint& newposition);
    void SetRect(const wxRect& rect);
    void GetLatestQuakeList();

    bool get_qcnlive_prefs();
    bool set_qcnlive_prefs();

    void SetPath(const char* strArgv = NULL);
    bool CreateBOINCInitFile();
    bool MainInit();
    void KillSplashScreen(); 
	
    wxSplashScreen* m_psplash;  // the apps splash screen
};

// this time is called every hour to get the earthquake list
class MyAppTimer : public wxTimer
{
   public:
      MyAppTimer(MyApp* papp) { pMyApp = papp; };
   
   private:
      MyApp* pMyApp;
	  void Notify();
};

#endif // ifndef _QCNLIVE_H_
