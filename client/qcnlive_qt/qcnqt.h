/*
 *  qcnqt.h
 *  qcnqt
 *
 *  Created by Carl Christensen on 2/9/08.
 *  Copyright 2008 Stanford U All rights reserved.
 *
 */
 
#ifndef _QCNLIVE_H_
#define _QCNLIVE_H_

#include <QApplication>
#include <QMainWindow>
#include <QSplashScreen>
#include <QTimer>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "main.h"
#include "qcn_graphics.h"
#include "qcn_curl.h"
#include "myframe.h"
#include "qcnlive_define.h"


QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QSlider;
QT_END_NAMESPACE
class GLWidget;

class MyApp; // defined below
class MyAppTimer; // defined below

// definition for the main app
class MyApp: public QApplication
{
  private:
    virtual bool OnInit();
    virtual int OnExit();

    MyFrame* frame;
	MyAppTimer* myapptimer;
	
	/*
#if wxUSE_LIBPNG
	wxPNGHandler* myPNGHandler;
#endif
#if wxUSE_LIBJPEG
	wxJPEGHandler* myJPEGHandler;
#endif
	*/

    QRect myRect;            // apps screen coordinates

  public:
	MyApp(int& argc, char** argv)  : QApplication(argc, argv) {};
	
    //void SetRect(const wxSize& newsize, const wxPoint& newposition);
    void SetRect(const QRect& rect);
    void GetLatestQuakeList();

    bool get_qcnlive_prefs();
    bool set_qcnlive_prefs();

    void SetPath(const char* strArgv = NULL);
    bool CreateBOINCInitFile();
    bool MainInit();
    void KillSplashScreen(); 
	bool KillMainThread();
	bool StartMainThread();
	
    QSplashScreen* m_psplash;  // the apps splash screen
};

// this time is called every hour to get the earthquake list
class MyAppTimer : public QTimer
{
   public:
      MyAppTimer(MyApp* papp) { pMyApp = papp; };
   
   private:
      MyApp* pMyApp;
	  void Notify();
};

#endif // ifndef _QCNLIVE_H_
