#ifndef _MYFRAME_H_
#define _MYFRAME_H_

#include "qcnqt.h"
#include "glwidget.h"

#include <QApplication>
#include <QMainWindow>
#include <QSplashScreen>
#include <QTimer>
#include <QSlider>
#include <QScrollArea>
#include <QLabel>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QSlider;
QT_END_NAMESPACE

class GLWidget;
class MyApp;

class MyFrame : public QMainWindow
{

    Q_OBJECT
	
private:
    void createActions();
    void createMenus();
    QSlider* createSlider(const char *changedSignal, const char *setterSlot);
    //void setPixmap(const QPixmap &pixmap);
    QSize getSize();
	
    QWidget* m_centralWidget;
    QScrollArea* m_glWidgetArea;
    //QScrollArea* pixmapLabelArea;
    GLWidget* m_glWidget;
	//QStatusBar* m_statusbar;
	QToolBar* m_toolbar;
	QSlider* m_sliderTime;

	// menu objects
	QMenu* m_menuFile;
	QMenu* m_menuView;
	QMenu* m_menuOptions;
	QMenu* m_menuHelp;
	QMenuBar* m_menuBar;
	
	
	// actions for menu & toggle buttons
    QAction* m_actionFileExit;
	QAction* m_actionFileDlgSettings;	
	QAction* m_actionFileMakeQuake;
	
	QAction* m_actionViewEarth;
	QAction* m_actionViewSensor2D;
	QAction* m_actionViewSensor3D;
	QAction* m_actionViewCube;
	
	QAction* m_actionOptionEarthDay;
	QAction* m_actionOptionEarthNight;
	QAction* m_actionOptionEarthRotateOn;
	QAction* m_actionOptionEarthRotateOff;
	QAction* m_actionOptionEarthUSGS;
	QAction* m_actionOptionEarthQuakelist;

	//QAction* m_actionOptionSensor01;
	//QAction* m_actionOptionSensor10;
	//QAction* m_actionOptionSensor60;
	QAction* m_actionOptionSensorVerticalZoomAuto;
	QAction* m_actionOptionSensorVerticalZoomIn;
	QAction* m_actionOptionSensorVerticalZoomOut;
	QAction* m_actionOptionSensorHorizontalZoomIn;
	QAction* m_actionOptionSensorHorizontalZoomOut;
	QAction* m_actionOptionSensorBack;
	QAction* m_actionOptionSensorPause;
	QAction* m_actionOptionSensorResume;
	QAction* m_actionOptionSensorRecordStart;
	QAction* m_actionOptionSensorRecordStop;
	QAction* m_actionOptionSensorForward;
	QAction* m_actionOptionSensorAbsolute;
	QAction* m_actionOptionSensorScaled;
	//QAction* m_actionOptionSensorScrollbar;

	
    QAction* m_actionHelpAbout;
	QAction* m_actionHelpManual;
	QAction* m_actionHelpWebQCN;
	QAction* m_actionHelpWebQCNLive;
	QAction* m_actionHelpWebEarthquakes;
	QAction* m_actionHelpWebLessons;
	QAction* m_actionHelpWebRequestSensor;
	QAction* m_actionHelpWebGlossary;
	QAction* m_actionOptionScreenshot;
	QAction* m_actionOptionLogo;
	
	// pointer to the base app instance
    MyApp* m_pMyApp;
	
    bool m_bEarthDay;
    bool m_bEarthRotate;
    long m_iSensorAction;
	bool m_bSensorAbsolute2D;
	bool m_bSensorAbsolute3D;
	
	e_view m_view;  // holds the current enum ID above
	QToolBar* m_ptbBase;
	
public:
	MyFrame(MyApp* papp);
	bool Init();
	void EarthRotate(bool bAuto);
	void SetupToolbars();
	void ToggleStartStop(bool bStart);
		
private slots:
	void actionView();
/*
	void actionViewEarth();
	void actionViewSensor2D();
	void actionViewSensor3D();
	void actionViewCube();
*/
	
	void actionOptionEarthDay();
	void actionOptionEarthNight();
	void actionOptionEarthRotateOn();
	void actionOptionEarthRotateOff();
	void actionOptionEarthUSGS();
	void actionOptionEarthQuakelist();
	
	void actionOptionSensorVerticalZoomAuto();
	void actionOptionSensorVerticalZoomIn();
	void actionOptionSensorVerticalZoomOut();
	void actionOptionSensorHorizontalZoomIn();
	void actionOptionSensorHorizontalZoomOut();
	void actionOptionSensorBack();
	void actionOptionSensorPause();
	void actionOptionSensorResume();
	void actionOptionSensorRecordStart();
	void actionOptionSensorRecordStop();
	void actionOptionSensorForward();
	void actionOptionSensorAbsolute();
	void actionOptionSensorScaled();
	
	void actionOptionScreenshot();
	void actionOptionLogo();

    void actionHelpAbout();
	void actionHelpManual();
	void actionHelpWebQCN();
	void actionHelpWebQCNLive();
	void actionHelpWebEarthquakes();
	void actionHelpWebLessons();
	void actionHelpWebRequestSensor();
	void actionHelpWebGlossary();
		
/*

    void ToolBarView();
    void Toggle(const int id, const bool bOn  = true, const bool bView = false);
    void ToolBarEarth(bool bFirst = false);
    void ToolBarSensor2D();
    void ToolBarSensor3D();
    void ToolBarCube();
    void EarthRotate(bool bAuto = true);
    void AddScreenshotItem();
    void SetToggleEarth();
    void SetToggleSensor();
	void SensorNavButtons();
*/
	
  private:
    void closeEvent(QCloseEvent* pqc);
	//void resizeEvent(QResizeEvent* prs);
	//void moveEvent (QMoveEvent* pme);
	
	/*
    // menu events
    void OnFileSettings(wxCommandEvent& evt);

    void OnActionView(wxCommandEvent& evt);
    void OnActionEarth(wxCommandEvent& evt);
    void OnActionSensor(wxCommandEvent& evt);
    void OnActionHelp(wxCommandEvent& evt);
    void OnScreenshot(wxCommandEvent& vet);
    void OnLogoChange(wxCommandEvent& vet);
	*/
	
    void RemoveCurrentTools();

};

#endif // _MYFRAME_H_
