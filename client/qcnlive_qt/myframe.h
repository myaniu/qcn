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
	QStatusBar* m_statusbar;
	QToolBar* m_toolbar;
	QSlider* m_sliderTime;
	
    QAction* m_actionFileExit;
	QAction* m_actionFileDlgSettings;	
	QAction* m_actionFileMakeQuake;
	
    QAction* m_actionHelpAbout;
	QAction* m_actionHelpManual;
	QAction* m_actionHelpWebQCN;
	QAction* m_actionHelpWebQCNLive;
	QAction* m_actionHelpWebEarthquakes;
	QAction* m_actionHelpWebLessons;
	QAction* m_actionHelpWebRequestSensor;
	QAction* m_actionHelpWebGlossary;
	
    MyApp* m_pMyApp;
	
	QMenu* m_menuFile;
	QMenu* m_menuView;
	QMenu* m_menuOptions;
	QMenu* m_menuHelp;
	QMenuBar* m_menuBar;
	
    bool m_bEarthDay;
    bool m_bEarthRotate;
    long m_iSensorAction;
	bool m_bSensorAbsolute2D;
	bool m_bSensorAbsolute3D;
	
	long m_view;  // holds the current enum ID above
	QToolBar* m_ptbBase;
	
public:
	MyFrame(MyApp* papp);
		
	private slots:
    void about();

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
	void SetupToolbars();
	void ToggleStartStop(bool bStart);
*/
	
  private:
    //void closeEvent(QCloseEvent* pqc);
	void resizeEvent(QResizeEvent* prs);

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
