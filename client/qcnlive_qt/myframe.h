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
	
    QWidget* centralWidget;
    QScrollArea* glWidgetArea;
    //QScrollArea* pixmapLabelArea;
    GLWidget* glWidget;
    QLabel* pixmapLabel;
	//QStatusBar* pStatusBar;
	QToolBar* pToolBar;
	QSlider* pTimeSlider;
    //QSlider *xSlider;
    //QSlider *ySlider;
    //QSlider *zSlider;
	
    QMenu *fileMenu;
    QMenu *helpMenu;
    //QAction *grabFrameBufferAct;
    //QAction *renderIntoPixmapAct;
    //QAction *clearPixmapAct;
    QAction *exitAct;
    QAction *aboutAct;
    //QAction *aboutQtAct;
	
public:
	MyFrame(const QRect& rect, MyApp* papp);
		
	private slots:
    //void renderIntoPixmap();
    //void grabFrameBuffer();
    //void clearPixmap();
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

    MyApp* pMyApp;
    //MyGLPane* glPane;
	
	QMenu *menuFile;
        QMenu *menuView;
        QMenu *menuOptions;
        QMenu *menuHelp;
        QMenuBar* menuBar;
 
    bool bEarthDay;
    bool bEarthRotate;
    long iSensorAction;
        bool bSensorAbsolute2D;
        bool bSensorAbsolute3D;

        long m_view;  // holds the current enum ID above
     QToolBar* m_ptbBase;

	/*
    void closeEvent(QCloseEvent* pqc);
    void OnSize(wxSizeEvent& evt);

    // menu events
    void OnAbout(wxCommandEvent& evt);
    void OnQuit(wxCommandEvent& evt);
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
