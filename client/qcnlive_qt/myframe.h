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
#include <QDockWidget>

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
    QWidget* m_centralWidget;
    QScrollArea* m_glWidgetArea;
    //QScrollArea* pixmapLabelArea;
    GLWidget* m_glWidget;
	//QStatusBar* m_statusbar;
	QDockWidget* m_dockWidgetView;
	QDockWidget* m_dockWidgetOption;
	QToolBar* m_toolBarView;
	QToolBar* m_toolBarOption;
	QMenuBar* m_menuBar;
	QSlider* m_sliderTime;

	// menu objects
	QMenu* m_menuFile;
	QMenu* m_menuView;
	QMenu* m_menuOptions;
	QMenu* m_menuHelp;
	
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
	
	QAction* m_actionCurrent; // save the current action
	
	// pointer to the base app instance
    MyApp* m_pMyApp;
	
    bool m_bEarthDay;
    bool m_bEarthRotate;
    long m_iSensorAction;
	bool m_bSensorAbsolute2D;
	bool m_bSensorAbsolute3D;
	
	QToolBar* m_ptbBase;
	
	//std::vector<QAction*> m_vqaSeparator; // a vector of separators so we can remove them as needed i.e. when redrawing toolbars
	void AddToolBarSeparator();  // function to keep track of separators for easy removal
	
public:
	MyFrame(MyApp* papp);

	bool Init();
	void EarthRotate(bool bAuto = true);
	void ToggleStartStop(bool bStart);
	
private slots:
	void actionView();
	
	void actionOptionEarth();
	void actionOptionSensor();	
	void actionOptionScreenshot();
	void actionOptionLogo();

    void actionHelp();
			
  private:
	// inherited events
    void closeEvent(QCloseEvent* pqc);
	//void resizeEvent(QResizeEvent* prs);
	//void moveEvent (QMoveEvent* pme);

	// utility functions for toolbars menus etc
    void createActions();
    void createMenus();
	void createToolbar();
    
	//void setPixmap(const QPixmap &pixmap);
    QSize getSize();

    void ToolBarView();
	void Toggle(QAction* pqa, const bool bCheck = true, const bool bEnable = true);
    void ToolBarEarth(bool bFirst = false);
    void ToolBarSensor2D();
    void ToolBarSensor3D();
    void ToolBarCube();
    void AddScreenshotItem();
    void SetToggleEarth();
    void SetToggleSensor();
	void SensorNavButtons();
    void RemoveCurrentTools();

};

#endif // _MYFRAME_H_
