/*
 *  myframe.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 2/15/08.
 *  Copyright 2008 Stanford University School of Earth Sciences. All rights reserved.
 *
 */

#include "qcnqt.h"

// these are our toolbar icons in C-array-style XPM format, var names prefixed xpm_icon_
//#include "icons.h"   // 64x64
#include "icons32.h"   // 32x32

// CMC #include "dlgsettings.h"
#include "qcn_earth.h"
#include "qcn_2dplot.h"

/* canonical list of all icons used for toolbar:
 
 xpm_icon_earth
 xpm_icon_twod
 xpm_icon_threed
 xpm_icon_cube
 
 xpm_icon_sun
 xpm_icon_moon
 xpm_icon_spin
 xpm_icon_nospin
 
 xpm_icon_quakelist
 xpm_icon_usgs
 
 xpm_horiz_zoom_in
 xpm_horiz_zoom_out
 xpm_icon_rw
 xpm_icon_pause
 xpm_icon_play
 xpm_icon_record
 xpm_icon_stop
 xpm_icon_ff
 xpm_icon_camera
 
 xpm_zoom_auto
 xpm_vert_zoom_in
 xpm_vert_zoom_out
 
 xpm_icon_absolute
 xpm_icon_scaled
 
 xpm_icon_camera

 */

MyFrame::MyFrame(MyApp* papp)
     : m_toolBar(NULL), m_menuBar(NULL), m_actionCurrent(NULL), m_pMyApp(papp)
{
	m_vqaSeparator.clear();
}

bool MyFrame::Init()
{
	QSettings settings(SET_COMPANY, SET_APP);
	restoreGeometry(settings.value("geometry").toByteArray());
	
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
	
    m_glWidget = new GLWidget(this);
    //pixmapLabel = new QLabel;
	
    m_glWidgetArea = new QScrollArea;
    m_glWidgetArea->setWidget(m_glWidget);
    m_glWidgetArea->setWidgetResizable(true);
    m_glWidgetArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_glWidgetArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_glWidgetArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_glWidgetArea->setMinimumSize(50, 50);
	
	// time slider for sensor views
    QSlider* createSlider(const char *changedSignal, const char *setterSlot);
    m_sliderTime = new QSlider(Qt::Horizontal);
    m_sliderTime->setRange(0, 100);
    m_sliderTime->setSingleStep(5);
    m_sliderTime->setPageStep(20);
    m_sliderTime->setTickInterval(10);
    m_sliderTime->setTickPosition(QSlider::TicksRight);
    connect(m_glWidget, SIGNAL(TimePositionChanged(const double&)), m_sliderTime, SLOT(setTimePosition(const double&)));
	
	m_ptbBase = NULL; // no toolbar base yet

	// initial view is the earth
	qcn_graphics::g_eView = VIEW_EARTH_DAY;  // set view to 0
    m_bEarthDay = true;
    m_bEarthRotate = true;
    m_iSensorAction = 0;
	
    m_bSensorAbsolute2D = false;
    m_bSensorAbsolute3D = false;
	
    createActions();
    createMenus();
	createToolbar();
	
    QGridLayout *centralLayout = new QGridLayout;
    centralLayout->addWidget(m_glWidgetArea, 0, 0, 1, 2);
    centralLayout->addWidget(m_sliderTime, 1, 0, 1, 2);
    m_centralWidget->setLayout(centralLayout);
	
    m_sliderTime->setValue(100);
	m_sliderTime->hide();
		
    setWindowTitle(tr("QCNLive"));
	statusBar()->showMessage(tr("Ready"), 0);
		
	return true;
}

/*
 void MyFrame::renderIntoPixmap()
 {
 QSize size = getSize();
 if (size.isValid()) {
 QPixmap pixmap = glWidget->renderPixmap(size.width(), size.height());
 setPixmap(pixmap);
 }
 }
 
 void MyFrame::grabFrameBuffer()
 {
 QImage image = glWidget->grabFrameBuffer();
 setPixmap(QPixmap::fromImage(image));
 }
 
 void MyFrame::clearPixmap()
 {
 setPixmap(QPixmap());
 }
*/


void MyFrame::createActions()
{
	// setup the actions of the various menu bar and toggle buttons
	
	// File actions
    m_actionFileExit = new QAction(tr("E&xit"), this);
	m_actionFileExit->setToolTip(tr("Quit QCNLive"));
    m_actionFileExit->setShortcuts(QKeySequence::Quit);
    connect(m_actionFileExit, SIGNAL(triggered()), this, SLOT(close()));	
	
	m_actionFileDlgSettings = new QAction(tr("&Local Settings"), this);
	m_actionFileDlgSettings->setShortcut(tr("Ctrl+F"));
	m_actionFileDlgSettings->setToolTip(tr("Enter local settings such as station name, latutide, longitude, elevation"));
	//connect(m_actionFileDlgSettings, SIGNAL(triggered()), this, SLOT(fileDlgSettings()));

	m_actionFileMakeQuake = new QAction(tr("&Make Earthquake"), this);
	m_actionFileMakeQuake->setToolTip(tr("Make and Print Your Own Earthquake"));
	m_actionFileMakeQuake->setShortcut(tr("Ctrl+M")); 
	//connect(m_actionFileMakeQuake, SIGNAL(triggered()), this, SLOT(makeEarthquake()));
	
	// View actions
	m_actionViewEarth = new QAction(tr("&Earthquakes"), this);
	m_actionViewEarth->setToolTip(tr("Select this view to see the latest and historical earthquakes worldwide"));
	m_actionViewEarth->setCheckable(true);
	m_actionViewEarth->setIcon(QIcon(xpm_icon_earth));
    connect(m_actionViewEarth, SIGNAL(triggered()), this, SLOT(actionView()));

	m_actionViewSensor2D = new QAction(tr("Sensor &2-dimensional"), this);
	m_actionViewSensor2D->setToolTip(tr("Select this view to see your accelerometer output as a 2-dimensional plot"));
	m_actionViewSensor2D->setCheckable(true);
	m_actionViewSensor2D->setIcon(QIcon(xpm_icon_twod));
    connect(m_actionViewSensor2D, SIGNAL(triggered()), this, SLOT(actionView()));

	m_actionViewSensor3D = new QAction(tr("Sensor &3-dimensional"), this);
	m_actionViewSensor3D->setToolTip(tr("Select this to see your accelerometer output as a 3-dimensional plot"));
	m_actionViewSensor3D->setCheckable(true);
	m_actionViewSensor3D->setIcon(QIcon(xpm_icon_threed));
    connect(m_actionViewSensor3D, SIGNAL(triggered()), this, SLOT(actionView()));

	m_actionViewCube = new QAction(tr("&Cube"), this);
	m_actionViewCube->setToolTip(tr("Select this view to see a bouncing cube that responds to your accelerometer"));
	m_actionViewCube->setCheckable(true);
	m_actionViewCube->setIcon(QIcon(xpm_icon_cube));
    connect(m_actionViewCube, SIGNAL(triggered()), this, SLOT(actionView()));
	
	
	// Option - Earth
	m_actionOptionEarthDay = new QAction(tr("&Day"), this);
	m_actionOptionEarthDay->setToolTip(tr("Show day view of global earthquake map"));
	m_actionOptionEarthDay->setCheckable(true);
	m_actionOptionEarthDay->setIcon(QIcon(xpm_icon_sun));
	connect(m_actionOptionEarthDay, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));

	m_actionOptionEarthNight = new QAction(tr("&Night"), this);
	m_actionOptionEarthNight->setToolTip(tr("Show night view of global earthquake map"));
	m_actionOptionEarthNight->setCheckable(true);
	m_actionOptionEarthNight->setIcon(QIcon(xpm_icon_moon));
	connect(m_actionOptionEarthNight, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthRotateOn = new QAction(tr("&Auto-rotate"), this);
	m_actionOptionEarthRotateOn->setToolTip(tr("Auto-rotate the globe"));
	m_actionOptionEarthRotateOn->setCheckable(true);
	m_actionOptionEarthRotateOn->setIcon(QIcon(xpm_icon_spin));
	connect(m_actionOptionEarthRotateOn, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthRotateOff = new QAction(tr("&Stop rotation"), this);
	m_actionOptionEarthRotateOff->setToolTip(tr("Stop rotation of the globe"));
	m_actionOptionEarthRotateOff->setCheckable(true);
	m_actionOptionEarthRotateOff->setIcon(QIcon(xpm_icon_nospin));
	connect(m_actionOptionEarthRotateOff, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthQuakelist = new QAction(tr("&Get latest earthquakes"), this);
	m_actionOptionEarthQuakelist->setToolTip(tr("Get the latest earthquake list from the USGS"));
	m_actionOptionEarthQuakelist->setCheckable(true);
	m_actionOptionEarthQuakelist->setIcon(QIcon(xpm_icon_quakelist));
	connect(m_actionOptionEarthQuakelist, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthUSGS = new QAction(tr("&USGS Website"), this);
	m_actionOptionEarthUSGS->setToolTip(tr("Go to the USGS website for the currently selected earthquake"));
	m_actionOptionEarthUSGS->setCheckable(true);
	m_actionOptionEarthUSGS->setIcon(QIcon(xpm_icon_usgs));
	connect(m_actionOptionEarthUSGS, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	
	// Option - Sensor (2D & 3D)
	m_actionOptionSensorVerticalZoomAuto = new QAction(tr("Auto-Zoom Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomAuto->setToolTip(tr("Auto-Zoom Vertical Scale"));
	m_actionOptionSensorVerticalZoomAuto->setIcon(QIcon(xpm_zoom_auto));
	m_actionOptionSensorVerticalZoomAuto->setCheckable(true);
	connect(m_actionOptionSensorVerticalZoomAuto, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorVerticalZoomIn = new QAction(tr("Zoom In Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomIn->setToolTip(tr("Zoom In Vertical Scale"));
	m_actionOptionSensorVerticalZoomIn->setIcon(QIcon(xpm_vert_zoom_in));
	m_actionOptionSensorVerticalZoomIn->setCheckable(true);
	connect(m_actionOptionSensorVerticalZoomIn, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorVerticalZoomOut = new QAction(tr("Zoom Out Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomOut->setToolTip(tr("Zoom Out Vertical Scale"));
	m_actionOptionSensorVerticalZoomOut->setIcon(QIcon(xpm_vert_zoom_out));
	m_actionOptionSensorVerticalZoomOut->setCheckable(true);
	connect(m_actionOptionSensorVerticalZoomOut, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorHorizontalZoomIn = new QAction(tr("Zoom In Time Scale"), this);
	m_actionOptionSensorHorizontalZoomIn->setToolTip(tr("Zoom In Time Scale"));
	m_actionOptionSensorHorizontalZoomIn->setIcon(QIcon(xpm_horiz_zoom_in));
	m_actionOptionSensorHorizontalZoomIn->setCheckable(true);
	connect(m_actionOptionSensorHorizontalZoomIn, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorHorizontalZoomOut = new QAction(tr("Zoom Out Time Scale"), this);
	m_actionOptionSensorHorizontalZoomOut->setToolTip(tr("Zoom Out Time Scale"));
	m_actionOptionSensorHorizontalZoomOut->setIcon(QIcon(xpm_horiz_zoom_out));
	m_actionOptionSensorHorizontalZoomOut->setCheckable(true);
	connect(m_actionOptionSensorHorizontalZoomOut, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorBack = new QAction(tr("Move Back"), this);
	m_actionOptionSensorBack->setToolTip(tr("Move Back In Time"));
	m_actionOptionSensorBack->setIcon(QIcon(xpm_icon_rw));
	m_actionOptionSensorBack->setCheckable(true);
	connect(m_actionOptionSensorBack, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorPause = new QAction(tr("Pause Display"), this);
	m_actionOptionSensorPause->setToolTip(tr("Pause Sensor Display"));
	m_actionOptionSensorPause->setIcon(QIcon(xpm_icon_pause));
	m_actionOptionSensorPause->setCheckable(true);
	connect(m_actionOptionSensorPause, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorResume = new QAction(tr("Start Display"), this);
	m_actionOptionSensorResume->setToolTip(tr("Start Sensor Display"));
	m_actionOptionSensorResume->setIcon(QIcon(xpm_icon_play));
	m_actionOptionSensorResume->setCheckable(true);
	connect(m_actionOptionSensorResume, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorRecordStart = new QAction(tr("Start Recording"), this);
	m_actionOptionSensorRecordStart->setToolTip(tr("Start Recording Sensor Time Series"));
	m_actionOptionSensorRecordStart->setIcon(QIcon(xpm_icon_record));
	m_actionOptionSensorRecordStart->setCheckable(true);
	connect(m_actionOptionSensorRecordStart, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorRecordStop = new QAction(tr("Stop Recording"), this);
	m_actionOptionSensorRecordStop->setToolTip(tr("Stop Recording Sensor Time Series"));
	m_actionOptionSensorRecordStop->setIcon(QIcon(xpm_icon_stop));
	m_actionOptionSensorRecordStop->setCheckable(true);
	connect(m_actionOptionSensorRecordStop, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorForward = new QAction(tr("Move Forward"), this);
	m_actionOptionSensorForward->setToolTip(tr("Move Forward In Time"));
	m_actionOptionSensorForward->setIcon(QIcon(xpm_icon_ff));
	m_actionOptionSensorForward->setCheckable(true);
	connect(m_actionOptionSensorForward, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorAbsolute = new QAction(tr("&Absolute sensor values"), this);
	m_actionOptionSensorAbsolute->setToolTip(tr("Absolute sensor values"));
	m_actionOptionSensorAbsolute->setIcon(QIcon(xpm_icon_absolute));
	m_actionOptionSensorAbsolute->setCheckable(true);
	connect(m_actionOptionSensorAbsolute, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorScaled = new QAction(tr("S&caled sensor values"), this);
	m_actionOptionSensorScaled->setToolTip(tr("Scaled sensor values"));
	m_actionOptionSensorScaled->setIcon(QIcon(xpm_icon_scaled));
	m_actionOptionSensorScaled->setCheckable(true);
	connect(m_actionOptionSensorScaled, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	

	// Option action for all (Screenshot / Logo)
	m_actionOptionScreenshot = new QAction(tr("Screenshot"), this);
	m_actionOptionScreenshot->setToolTip(tr("Make a screenshot and save as a JPG file in data directory"));
	m_actionOptionScreenshot->setIcon(QIcon(xpm_icon_camera));
	connect(m_actionOptionScreenshot, SIGNAL(triggered()), this, SLOT(actionOptionScreenshot()));
	
	m_actionOptionLogo = new QAction(tr(""), this);
	m_actionOptionLogo->setToolTip(tr(""));
	//m_actionOptionLogo->setIcon(QIcon(xpm_icon_scaled));
	connect(m_actionOptionLogo, SIGNAL(triggered()), this, SLOT(actionOptionLogo()));
		
	
	// Help actions
    m_actionHelpAbout = new QAction(tr("&About"), this);
	m_actionHelpAbout->setToolTip(tr("About QCNLive"));
    connect(m_actionHelpAbout, SIGNAL(triggered()), this, SLOT(actionHelp()));

	m_actionHelpManual = new QAction(tr("&Manual (PDF) for QCNLive"), this);
	m_actionHelpManual->setToolTip(tr("Download/View Manual (PDF) for QCNLive"));
    connect(m_actionHelpManual, SIGNAL(triggered()), this, SLOT(actionHelp()));

	m_actionHelpWebQCN = new QAction(tr("&QCN Website"), this);
	m_actionHelpWebQCN->setToolTip(tr("Visit the main QCN website"));
    connect(m_actionHelpWebQCN, SIGNAL(triggered()), this, SLOT(actionHelp()));
	
	m_actionHelpWebQCNLive = new QAction(tr("QCN&Live Website"), this);
	m_actionHelpWebQCNLive->setToolTip(tr("Visit the QCNLive website"));
    connect(m_actionHelpWebQCNLive, SIGNAL(triggered()), this, SLOT(actionHelp()));
	
	m_actionHelpWebEarthquakes = new QAction(tr("&Earthquake Information"), this);
	m_actionHelpWebEarthquakes->setToolTip(tr("Visit QCN's website for earthquakes"));
    connect(m_actionHelpWebEarthquakes, SIGNAL(triggered()), this, SLOT(actionHelp()));
	
	m_actionHelpWebLessons = new QAction(tr("Lessons and &Activities"), this);
	m_actionHelpWebLessons->setToolTip(tr("Lessons and Activities website"));
    connect(m_actionHelpWebLessons, SIGNAL(triggered()), this, SLOT(actionHelp()));
	
	m_actionHelpWebRequestSensor = new QAction(tr("&Request a Sensor"), this);
	m_actionHelpWebRequestSensor->setToolTip(tr("Request/Purchase a sensor to use with QCN"));
    connect(m_actionHelpWebRequestSensor, SIGNAL(triggered()), this, SLOT(actionHelp()));
	
	m_actionHelpWebGlossary = new QAction(tr("&Glossary"), this);
	m_actionHelpWebGlossary->setToolTip(tr("Online Glossary"));
    connect(m_actionHelpWebGlossary, SIGNAL(triggered()), this, SLOT(actionHelp()));

	
}

void MyFrame::createMenus()
{
    // Make a menubar
	
	// File
    m_menuFile = menuBar()->addMenu(tr("&File"));
    m_menuFile->addAction(m_actionFileDlgSettings);
    m_menuFile->addAction(m_actionFileMakeQuake);
    m_menuFile->addAction(m_actionFileExit);
	
	// View
	m_menuView = menuBar()->addMenu(tr("&View"));
	m_menuView->addAction(m_actionViewEarth);
	m_menuView->addAction(m_actionViewSensor2D);
	m_menuView->addAction(m_actionViewSensor3D);
	m_menuView->addAction(m_actionViewCube);
	
	// Options - these change based on the View
	m_menuOptions = menuBar()->addMenu(tr("&Options"));
	

	// Help
	m_menuHelp = menuBar()->addMenu(tr("&Help"));
	m_menuHelp->addAction(m_actionHelpAbout);
#ifndef __APPLE_CC__
	m_menuHelp->addSeparator();  // on Mac the Help/About goes on the left-most system menu, so don't need a separator
#endif
	m_menuHelp->addAction(m_actionHelpManual);
	m_menuHelp->addAction(m_actionHelpWebQCN);
	m_menuHelp->addAction(m_actionHelpWebQCNLive);
	m_menuHelp->addAction(m_actionHelpWebEarthquakes);
	m_menuHelp->addAction(m_actionHelpWebLessons);
	m_menuHelp->addAction(m_actionHelpWebRequestSensor);
	m_menuHelp->addAction(m_actionHelpWebGlossary);
	
		
}

QSize MyFrame::getSize()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("QCNLive"),
                                         tr("Enter pixmap size:"),
                                         QLineEdit::Normal,
                                         tr("%1 x %2").arg(m_glWidget->width())
										 .arg(m_glWidget->height()),
                                         &ok);
    if (!ok)
        return QSize();
	
    QRegExp regExp(tr("([0-9]+) *x *([0-9]+)"));
    if (regExp.exactMatch(text)) {
        int width = regExp.cap(1).toInt();
        int height = regExp.cap(2).toInt();
        if (width > 0 && width < 2048 && height > 0 && height < 2048)
            return QSize(width, height);
    }
	
    return m_glWidget->size();
}

void MyFrame::closeEvent(QCloseEvent* pqc)
{
	QSettings settings(SET_COMPANY, SET_APP);
	settings.setValue("geometry", saveGeometry());
	QWidget::closeEvent(pqc);
}

/*
void MyFrame::moveEvent (QMoveEvent* pme)
{
}

void MyFrame::resizeEvent(QResizeEvent* prs)
{
}
*/

void MyFrame::EarthRotate(bool bAuto)
{
	if (!qcn_graphics::earth.IsShown()) return;  // only matters if we're on the earth view!
	// see if it's rotating and we want to stop, or it's not rotating and we want it to start
	if ( (!bAuto && qcn_graphics::earth.IsAutoRotate())
		|| (bAuto && ! qcn_graphics::earth.IsAutoRotate()))  {
		m_bEarthRotate = bAuto;
		Toggle(m_actionOptionEarthRotateOff, !bAuto);
		Toggle(m_actionOptionEarthRotateOn, bAuto);
		qcn_graphics::earth.AutoRotate(bAuto);
	}
}

void MyFrame::ToggleStartStop(bool bStart)
{
	Toggle(m_actionOptionSensorResume, bStart);
	Toggle(m_actionOptionSensorPause, !bStart);
	if (bStart && qcn_graphics::TimeWindowIsStopped()) {
		qcn_graphics::TimeWindowStart();
	}
	else if (!bStart && !qcn_graphics::TimeWindowIsStopped()) {
		qcn_graphics::TimeWindowStop();
	}
}

void MyFrame::createToolbar()
{
	m_toolBar = new QToolBar(tr("Actions"), m_centralWidget);	
	if (m_toolBar) {
		this->addToolBar(m_toolBar);
		m_toolBar->setIconSize(QSize(32,32));		
		ToolBarView();
		ToolBarEarth(true);
#ifdef __APPLE_CC__
		setUnifiedTitleAndToolBarOnMac(true);
#endif
	}
}

void MyFrame::actionView()
{
	// get item from event do appropriate action (boinc_key_press!)
	// todo: hook up the other toolbars
	if (m_actionCurrent) Toggle(m_actionCurrent, false, false);
	
	// figure out who called this, i.e. get the pointer of the QAction* from trigger and compare
	QAction *pAction = qobject_cast<QAction*>(QObject::sender());
	bool bChanged = false;
	if (pAction == m_actionViewSensor2D)
	{
		// note only redraw sensor toolbar if not coming from a sensor view already
		//if (qcn_graphics::g_eView != VIEW_PLOT_2D && cn_graphics::g_eView != VIEW_PLOT_3D) ToolBarSensor(evt.GetId());
		ToolBarSensor2D();
		qcn_graphics::g_eView = VIEW_PLOT_2D; // note set graphics after the toolbar build since the RemoveToolBar needs to know current view/options available
		bChanged = true;
	}
	else if (pAction == m_actionViewSensor3D)
	{
		// note only redraw sensor toolbar if not coming from a sensor view already
		//if (qcn_graphics::g_eView != VIEW_PLOT_2D && cn_graphics::g_eView != VIEW_PLOT_3D) ToolBarSensor(evt.GetId());
		ToolBarSensor3D();
		qcn_graphics::g_eView = VIEW_PLOT_3D;
		bChanged = true;
	}
	else if (pAction == m_actionViewCube)
	{
		ToolBarCube();
		qcn_graphics::g_eView = VIEW_CUBE;
		bChanged = true;
	}
	else {
		ToolBarEarth();
		if (m_bEarthDay) {
			qcn_graphics::g_eView = VIEW_EARTH_DAY;
			qcn_graphics::earth.SetMapCombined();
		}
		else {
			qcn_graphics::g_eView = VIEW_EARTH_NIGHT;
			qcn_graphics::earth.SetMapNight();
		}
		bChanged = true;
	}

	m_actionCurrent = pAction;
	
	qcn_graphics::FaderOn();
    if (bChanged && m_actionCurrent) {
		Toggle(m_actionCurrent, true, true);
    }
    qcn_graphics::ResetPlotArray();
}


void MyFrame::actionOptionScreenshot()
{
	const char* strSS = qcn_graphics::ScreenshotJPG();
	if (strSS && strSS[0] != 0x00) {
		// we have a valid screenshot filename
		char* statmsg = new char[_MAX_PATH];
		sprintf(statmsg, "Screenshot file saved to %s", strSS);
		statusBar()->showMessage(tr(statmsg), 5000);
		delete [] statmsg;
	}
}

void MyFrame::actionOptionLogo()
{
#ifdef QCNLIVE_DEMO
	qcn_graphics::demo_switch_ad();
#endif
}

/*
void MyFrame::OnFileSettings(wxCommandEvent& WXUNUSED(evt))
{
	CDialogSettings* pcds = new CDialogSettings(this, wxID_FILE_SETTINGS);
	if (pcds) {
		int myOldSensor = sm->iMySensor;
	    if (pcds->ShowModal() == wxID_OK)  {
			// accept values
			//statusBar->SetStatusText(wxString("Saving your settings and updating earthquake list", wxConvUTF8));
			pcds->SaveValues();  // save to the global variables
			// call our save function to write values to disk
			m_pMyApp->set_qcnlive_prefs(); // saved in KillMainThread 
			// probably have to kill & restart the main thread too?
			if (m_pMyApp && myOldSensor != sm->iMySensor) {  // we changed sensors, have to restart main thread?
				// put up a message box to quit and restart
				if (::wxMessageBox(_("You have changed your preferred sensor.\n\nPlease restart to use your new preferred USB sensor choice.\n\nClick 'OK' to quit now.\nClick 'Cancel' to continue this session of QCNLive."), 
								   _("Restart Required"), 
								   wxOK | wxCANCEL | wxICON_EXCLAMATION, this) == wxOK)
					Close();
			}
		}
	    pcds->Destroy();
	    delete pcds;
	}
}
*/

void MyFrame::actionOptionEarth()
{
	// figure out who called this, i.e. get the pointer of the QAction* from trigger and compare
	QAction *pAction = qobject_cast<QAction*>(QObject::sender());
	if (pAction == m_actionOptionEarthDay) {
		m_bEarthDay = true;
		qcn_graphics::earth.SetMapCombined(); 	}
	else if (pAction == m_actionOptionEarthNight) {
		m_bEarthDay = false;
		qcn_graphics::earth.SetMapNight();	}
	else if (pAction == m_actionOptionEarthRotateOn) {
			EarthRotate(true);	
	}
	else if (pAction == m_actionOptionEarthRotateOff) {
			EarthRotate(false);	
	}
	else if (pAction == m_actionOptionEarthUSGS) {
		statusBar()->showMessage(tr("Opening USGS website for selected earthquake"), 5000);
		qcn_graphics::earth.checkURLClick(true);	}
	else if (pAction == m_actionOptionEarthQuakelist) {
			if (m_pMyApp) m_pMyApp->GetLatestQuakeList();
	}
	SetToggleEarth();
}

void MyFrame::actionHelp()
{
	QAction *pAction = qobject_cast<QAction*>(QObject::sender());
	std::string strURL("");
	if (pAction == m_actionHelpManual) {
		strURL = "http://qcn.stanford.edu/downloads/QCNLive_User_Manual.pdf";
	}
	else if (pAction == m_actionHelpWebQCN) {
		strURL = "http://qcn.stanford.edu";
	}
	else if (pAction == m_actionHelpWebQCNLive) {
		strURL = "http://qcn.stanford.edu/learning/software.php";
	}
	else if (pAction == m_actionHelpWebEarthquakes) {
		strURL = "http://qcn.stanford.edu/learning/earthquakes.php";
	}
	else if (pAction == m_actionHelpWebLessons) {
		strURL = "http://qcn.stanford.edu/learning/lessons.php";
	}
	else if (pAction == m_actionHelpWebRequestSensor) {
		strURL = "http://qcn.stanford.edu/learning/requests.php";
	}
	else if (pAction == m_actionHelpWebGlossary) {
		strURL = "http://qcn.stanford.edu/learning/glossary.php";
	}
	else if (pAction == m_actionHelpAbout) {
		QMessageBox::about(this, tr("About QCNLive"),
						   tr("<b>QCNLive</b> is provided by the <BR> Quake-Catcher Network Project <BR><BR>http://qcn.stanford.edu<BR><BR>(c) 2010 Stanford University"));
		
		/*
		 wxAboutDialogInfo myAboutBox;
		 //myAboutBox.SetIcon(wxIcon("qcnwin.ico", wxBITMAP_TYPE_ICO));
		 myAboutBox.SetVersion(wxString(QCN_VERSION_STRING));
		 myAboutBox.SetName(wxT("QCNLive"));
		 myAboutBox.SetWebSite(wxT("http://qcn.stanford.edu"), wxT("Quake-Catcher Network Website"));
		 myAboutBox.SetCopyright(wxT("(c) 2009 Stanford University")); 
		 //myAboutBox.AddDeveloper(wxT("Carl Christensen  (carlgt1@yahoo.com"));
		 myAboutBox.SetDescription(wxT("This software is provided free of charge for educational purposes.\n\nPlease visit us on the web:\n"));
		 
		 wxAboutBox(myAboutBox);
		 
		 QDialog* dlgAbout = new QDialog(this);
		 dlgAbout->setModal(true);
		 dlgAbout->exec();
		 
		 */		
	}
	if (!strURL.empty())  qcn_util::launchURL(strURL.c_str());
}

void MyFrame::actionOptionSensor() 
{
	// get item from event do appropriate action (boinc_key_press!)
	QAction *pAction = qobject_cast<QAction*>(QObject::sender());
	if (pAction == m_actionOptionSensorBack) {
		if (! qcn_graphics::TimeWindowIsStopped()) {
			m_iSensorAction = 1;
			qcn_graphics::TimeWindowStop(); 
		}
		qcn_graphics::TimeWindowBack(); 
	}
	else if (pAction == m_actionOptionSensorPause) {
		if (! qcn_graphics::TimeWindowIsStopped()) {
			m_iSensorAction = 1;
			qcn_graphics::TimeWindowStop(); 
		}
	}
	else if (pAction == m_actionOptionSensorResume) {
		if (qcn_graphics::TimeWindowIsStopped()) {
			m_iSensorAction = 0;
			qcn_graphics::TimeWindowStart();
		}
	}
	else if (pAction == m_actionOptionSensorRecordStart) {
		if (qcn_graphics::TimeWindowIsStopped()) {
			m_iSensorAction = 0;
			qcn_graphics::TimeWindowStart();
		}
		if (sm->bSensorFound) {
			if (sm->bRecording) { // we're turning off recording
				statusBar()->showMessage(tr("Recording stopped"), 5000);
			}
			else { // we're starting recording
				statusBar()->showMessage(tr("Recording..."), 0);
			}
			
			// flip the state
			sm->bRecording = !sm->bRecording;
		}
	}
	else if (pAction == m_actionOptionSensorRecordStop) {
		if (qcn_graphics::TimeWindowIsStopped()) {
			m_iSensorAction = 0;
			qcn_graphics::TimeWindowStart();
		}
		if (sm->bSensorFound) {
			if (sm->bRecording) { // we're turning off recording
				statusBar()->showMessage(tr("Recording stopped"), 5000);
			}
			else { // we're starting recording
				statusBar()->showMessage(tr("Recording..."), 0);
			}
			
			// flip the state
			sm->bRecording = !sm->bRecording;
		}		
	}
	else if (pAction == m_actionOptionSensorForward) {
		if (! qcn_graphics::TimeWindowIsStopped()) {
			m_iSensorAction = 1;
			qcn_graphics::TimeWindowStop(); 
		}
		qcn_graphics::TimeWindowForward(); 
	}
	else if (pAction == m_actionOptionSensorAbsolute) {
		if (qcn_graphics::g_eView == VIEW_PLOT_2D) {
			m_bSensorAbsolute2D = true;
		}
		else {
			m_bSensorAbsolute3D = false;
		}
	}
	else if (pAction == m_actionOptionSensorScaled) {
		if (qcn_graphics::g_eView == VIEW_PLOT_2D) {
			m_bSensorAbsolute2D = false;
		}
		else {
			m_bSensorAbsolute3D = false;
		}
	}
	else if (pAction == m_actionOptionSensorHorizontalZoomOut) {
		qcn_graphics::SetTimeWindowWidth(true);
	}
	else if (pAction == m_actionOptionSensorHorizontalZoomIn) {
		qcn_graphics::SetTimeWindowWidth(false);
	}
	else if (pAction == m_actionOptionSensorVerticalZoomOut) {
		qcn_2dplot::SensorDataZoomOut();
	}
	else if (pAction == m_actionOptionSensorVerticalZoomIn) {
		qcn_2dplot::SensorDataZoomIn();
	}
	else if (pAction == m_actionOptionSensorVerticalZoomAuto) {
		qcn_2dplot::SensorDataZoomAuto();
	}

    SetToggleSensor();
}


void MyFrame::ToolBarView()
{
	if (!m_toolBar) return; // null toolbar?  should have already been created in Init()

	// just set the actions which were already created and map to the menu
	m_toolBar->addAction(m_actionViewEarth); 
	m_toolBar->addAction(m_actionViewSensor2D); 
	m_toolBar->addAction(m_actionViewSensor3D); 
	m_toolBar->addAction(m_actionViewCube); 
	m_toolBar->addSeparator();	 // don't need to use our function as we want to keep this separator permanently
}

void MyFrame::RemoveCurrentTools()
{   // remove the current "Option" tools if any 
	// it depends on what the current view is so don't switch the view before calling this function!
	
	// first get rid of any separators
	vector<QAction*>::iterator itAction = m_vqaSeparator.begin();
	while (itAction != m_vqaSeparator.end()) {
		if (!*itAction) m_toolBar->removeAction(*itAction); // check for null pointer
		itAction++;
	}
	m_vqaSeparator.clear(); // clear the separator vector
	
	switch (qcn_graphics::g_eView)
	{
		case VIEW_EARTH_DAY:
		case VIEW_EARTH_NIGHT:
			m_toolBar->removeAction(m_actionOptionEarthDay);
			m_toolBar->removeAction(m_actionOptionEarthNight);
			m_toolBar->removeAction(m_actionOptionEarthRotateOn);
			m_toolBar->removeAction(m_actionOptionEarthRotateOff);
			m_toolBar->removeAction(m_actionOptionEarthUSGS);
			m_toolBar->removeAction(m_actionOptionEarthQuakelist);
			break;
		case VIEW_PLOT_2D:
			m_toolBar->removeAction(m_actionOptionSensorVerticalZoomAuto);
			m_toolBar->removeAction(m_actionOptionSensorVerticalZoomIn);
			m_toolBar->removeAction(m_actionOptionSensorVerticalZoomOut);
			m_toolBar->removeAction(m_actionOptionSensorHorizontalZoomIn);
			m_toolBar->removeAction(m_actionOptionSensorHorizontalZoomOut);
			m_toolBar->removeAction(m_actionOptionSensorBack);
			m_toolBar->removeAction(m_actionOptionSensorPause);
			m_toolBar->removeAction(m_actionOptionSensorResume);
			m_toolBar->removeAction(m_actionOptionSensorRecordStart);
			m_toolBar->removeAction(m_actionOptionSensorRecordStop);
			m_toolBar->removeAction(m_actionOptionSensorForward);
			m_toolBar->removeAction(m_actionOptionSensorAbsolute);
			m_toolBar->removeAction(m_actionOptionSensorScaled);
			break;
		case VIEW_PLOT_3D:
			m_toolBar->removeAction(m_actionOptionSensorHorizontalZoomIn);
			m_toolBar->removeAction(m_actionOptionSensorHorizontalZoomOut);
			m_toolBar->removeAction(m_actionOptionSensorBack);
			m_toolBar->removeAction(m_actionOptionSensorPause);
			m_toolBar->removeAction(m_actionOptionSensorResume);
			m_toolBar->removeAction(m_actionOptionSensorRecordStart);
			m_toolBar->removeAction(m_actionOptionSensorRecordStop);
			m_toolBar->removeAction(m_actionOptionSensorForward);
			m_toolBar->removeAction(m_actionOptionSensorAbsolute);
			m_toolBar->removeAction(m_actionOptionSensorScaled);
			break;
		default:
			break;
	}
	
	// we always need to get rid of the screenshot icon
	m_toolBar->removeAction(m_actionOptionScreenshot);
	
	m_menuOptions->clear(); // can just clear the options menu
}

void MyFrame::AddToolBarSeparator()
{
	QAction* pqa = m_toolBar->addSeparator();
	m_menuOptions->addSeparator();
	if (pqa) {
		m_vqaSeparator.push_back(pqa);
	}
}

void MyFrame::ToolBarEarth(bool bFirst)
{
    if (!m_toolBar) return; // null toolbar?
	
    if (bFirst) Toggle(m_actionViewEarth, true, true);

    if (!bFirst)
		RemoveCurrentTools();

	// menu & toolbar options were cleared, so have to add actions again

	// day & night view
	m_toolBar->addAction(m_actionOptionEarthDay); 
	m_menuOptions->addAction(m_actionOptionEarthDay);

	m_toolBar->addAction(m_actionOptionEarthNight); 
	m_menuOptions->addAction(m_actionOptionEarthNight);

	AddToolBarSeparator(); // use our function to keep track of separators so they can be easily removed	

	// rotate on/off
	m_toolBar->addAction(m_actionOptionEarthRotateOn); 
	m_menuOptions->addAction(m_actionOptionEarthRotateOn);
	
	m_toolBar->addAction(m_actionOptionEarthRotateOff); 
	m_menuOptions->addAction(m_actionOptionEarthRotateOff);
	
	AddToolBarSeparator(); // use our function to keep track of separators so they can be easily removed
	
	m_toolBar->addAction(m_actionOptionEarthQuakelist); 
	m_menuOptions->addAction(m_actionOptionEarthQuakelist);

	m_toolBar->addAction(m_actionOptionEarthUSGS); 
	m_menuOptions->addAction(m_actionOptionEarthUSGS);
	
    AddScreenshotItem();
	
    SetToggleEarth();
}

// toggle on off both the menu & the toolbar
void MyFrame::Toggle(QAction* pqa, const bool bCheck, const bool bEnable)
{
	pqa->setChecked(bCheck);
	pqa->setEnabled(bEnable);
}

void MyFrame::SetToggleSensor()
{
	
	if (qcn_graphics::g_eView == VIEW_PLOT_2D) {
		Toggle(m_actionOptionSensorRecordStart, !sm->bRecording);
		Toggle(m_actionOptionSensorRecordStop, sm->bRecording);
		
		m_actionOptionSensorAbsolute->setEnabled(true);
		if (m_bSensorAbsolute2D) {
			if (qcn_graphics::IsScaled()) qcn_graphics::SetScaled(false);
		}
		else {
			if (!qcn_graphics::IsScaled()) qcn_graphics::SetScaled(true);
		}		  
		Toggle(m_actionOptionSensorAbsolute, m_bSensorAbsolute2D);
		Toggle(m_actionOptionSensorScaled, !m_bSensorAbsolute2D);
		
		Toggle(m_actionOptionSensorResume, (bool)(sm->bRecording && m_iSensorAction == 0));
		Toggle(m_actionOptionSensorPause, (bool)(sm->bRecording && m_iSensorAction == 1));
	}
	else if (qcn_graphics::g_eView == VIEW_PLOT_3D) { // force to be scaled
		m_bSensorAbsolute3D = false;
		if (!qcn_graphics::IsScaled()) qcn_graphics::SetScaled(true);
		//toolBar->EnableTool(m_actionOptionSensorAbsolute, false);
		//menuOptions->Enable(m_actionOptionSensorAbsolute, false);
		Toggle(m_actionOptionSensorAbsolute, m_bSensorAbsolute3D);
		Toggle(m_actionOptionSensorScaled, !m_bSensorAbsolute3D);
	}
}

void MyFrame::SetToggleEarth()
{
	Toggle(m_actionOptionEarthDay, m_bEarthDay);
	Toggle(m_actionOptionEarthNight, !m_bEarthDay);
	Toggle(m_actionOptionEarthRotateOff, !m_bEarthRotate);
	Toggle(m_actionOptionEarthRotateOn, m_bEarthRotate);
}


void MyFrame::SensorNavButtons()
{  // common buttons/menu choices for 2 or 3-d sensor plot view
	
	m_toolBar->addAction(m_actionOptionSensorHorizontalZoomIn);
	m_menuOptions->addAction(m_actionOptionSensorHorizontalZoomIn);

	m_toolBar->addAction(m_actionOptionSensorHorizontalZoomOut);
	m_menuOptions->addAction(m_actionOptionSensorHorizontalZoomOut);
	
	AddToolBarSeparator(); // use our function to keep track of separators so they can be easily removed
	
	m_toolBar->addAction(m_actionOptionSensorBack);
	m_menuOptions->addAction(m_actionOptionSensorBack);
	
	m_toolBar->addAction(m_actionOptionSensorPause);
	m_menuOptions->addAction(m_actionOptionSensorPause);
	
	m_toolBar->addAction(m_actionOptionSensorResume);
	m_menuOptions->addAction(m_actionOptionSensorResume);
	
	m_toolBar->addAction(m_actionOptionSensorRecordStart);
	m_menuOptions->addAction(m_actionOptionSensorRecordStart);
	
	m_toolBar->addAction(m_actionOptionSensorRecordStop);
	m_menuOptions->addAction(m_actionOptionSensorRecordStop);
	
	m_toolBar->addAction(m_actionOptionSensorForward);
	m_menuOptions->addAction(m_actionOptionSensorForward);
	
}

void MyFrame::AddScreenshotItem()
{
	AddToolBarSeparator(); // use our function to keep track of separators so they can be easily removed

	m_toolBar->addAction(m_actionOptionScreenshot);
	m_menuOptions->addAction(m_actionOptionScreenshot);
	
#ifdef QCNLIVE_DEMO  
	// add a function to cycle through ad images i.e. science museum logos
	m_menuOptions->addAction(m_actionOptionLogo);
#endif
	
}

void MyFrame::ToolBarSensor2D()
{
    if (!m_toolBar) return; // null toolbar?
    RemoveCurrentTools();
	
	m_toolBar->addAction(m_actionOptionSensorVerticalZoomAuto);
	m_menuOptions->addAction(m_actionOptionSensorVerticalZoomAuto);

	m_toolBar->addAction(m_actionOptionSensorVerticalZoomIn);
	m_menuOptions->addAction(m_actionOptionSensorVerticalZoomIn);

	m_toolBar->addAction(m_actionOptionSensorVerticalZoomOut);
	m_menuOptions->addAction(m_actionOptionSensorVerticalZoomOut);

		
    // scrollbar for back & forth time
	/*
	if (scrollBar2D) {
    	 AddToolBarSeparator(); // use our function to keep track of separators so they can be easily removed
		toolBar->AddControl(scrollBar2D);
	}
	*/
	
	SensorNavButtons();
	
	AddToolBarSeparator(); // use our function to keep track of separators so they can be easily removed
	
	m_toolBar->addAction(m_actionOptionSensorAbsolute);
	m_menuOptions->addAction(m_actionOptionSensorAbsolute);
	
	m_toolBar->addAction(m_actionOptionSensorScaled);
	m_menuOptions->addAction(m_actionOptionSensorScaled);
	
    AddScreenshotItem();
	
    SetToggleSensor();  // put this after realize() because we may enable/disable tools
}

void MyFrame::ToolBarSensor3D()
{
    if (!m_toolBar) return; // null toolbar?
    RemoveCurrentTools();
	
	SensorNavButtons();

	AddToolBarSeparator(); // use our function to keep track of separators so they can be easily removed
	
	m_toolBar->addAction(m_actionOptionSensorAbsolute);
	m_menuOptions->addAction(m_actionOptionSensorAbsolute);
	
	m_toolBar->addAction(m_actionOptionSensorScaled);
	m_menuOptions->addAction(m_actionOptionSensorScaled);
	
    AddScreenshotItem();
	
	//	qcn_graphics::g_eView = iView;
    SetToggleSensor();  // put this after realize() because we may enable/disable tools
}

void MyFrame::ToolBarCube()
{
    if (!m_toolBar) return; // null toolbar?
    RemoveCurrentTools();
	
    AddScreenshotItem();
}

