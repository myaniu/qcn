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

#include <QtGui>
#include <QtOpenGL>

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

class GLWidget;
class MyFrame;


// definition for the main app
class MyApp: public QApplication
{
  private:
	QTimer* m_timer;  // slow timer for getting quakes every half hour
	MyFrame* m_frame;
    QRect m_rect;            // apps screen coordinates
    QSplashScreen* m_psplash;  // the apps splash screen
	bool m_bInit;  // flag to see if we are initialized

  public:
    MyApp(int& argc, char** argv);
	
	bool Init();
    int Exit();

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
	
	int getWidth() {return m_rect.width(); }
	int getHeight() {return m_rect.height(); }
	int getX() {return m_rect.x(); }
	int getY() {return m_rect.y(); }
	void setRect(const QRect& rect) {  m_rect = rect; }
	const QRect& getRect() { return m_rect; }
	
};

#endif // ifndef _QCNLIVE_H_
