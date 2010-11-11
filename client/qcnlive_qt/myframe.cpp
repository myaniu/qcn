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

MyFrame::MyFrame(MyApp* papp)
{
	m_pMyApp = papp;
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
	
    QGridLayout *centralLayout = new QGridLayout;
    centralLayout->addWidget(m_glWidgetArea, 0, 0, 1, 2);
    centralLayout->addWidget(m_sliderTime, 1, 0, 1, 2);
    m_centralWidget->setLayout(centralLayout);
	
    m_sliderTime->setValue(100);
	m_sliderTime->hide();
		
    setWindowTitle(tr("QCNLive"));
	statusBar()->showMessage(tr("Ready"), 0);
	
	m_toolbar = new QToolBar(tr("Actions"), m_centralWidget);
	
	QIcon pm[5];
	QToolButton* pTB[5];
	
	pm[0] = QIcon(xpm_icon_absolute);
	pm[1] = QIcon(xpm_icon_spin);
	pm[2] = QIcon(xpm_icon_ff);
	pm[3] = QIcon(xpm_icon_record);
	pm[4] = QIcon(xpm_icon_usgs);
	
	for (int i = 0; i < 5; i++) {
		pTB[i] = new QToolButton(m_toolbar);
		pTB[i]->setIcon(pm[i]);
		m_toolbar->addWidget(pTB[i]);
	}
	
	this->addToolBar(m_toolbar);
	
	// tool bar
	
#ifdef __APPLE_CC__
	setUnifiedTitleAndToolBarOnMac(true);
#endif
	
	// set the size to be the sizes from our saved prefs (or default sizes, either way it's set in myApp)
	//move(m_pMyApp->getX(), m_pMyApp->getY());
    //resize(m_pMyApp->getWidth(), m_pMyApp->getHeight());
	//setGeometry(m_pMyApp->getRect());
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
    connect(m_actionViewEarth, SIGNAL(triggered()), this, SLOT(actionView()));

	m_actionViewSensor2D = new QAction(tr("Sensor &2-dimensional"), this);
	m_actionViewSensor2D->setToolTip(tr("Select this view to see your accelerometer output as a 2-dimensional plot"));
    connect(m_actionViewSensor2D, SIGNAL(triggered()), this, SLOT(actionView()));

	m_actionViewSensor3D = new QAction(tr("Sensor &3-dimensional"), this);
	m_actionViewSensor3D->setToolTip(tr("Select this to see your accelerometer output as a 3-dimensional plot"));
    connect(m_actionViewSensor3D, SIGNAL(triggered()), this, SLOT(actionView()));

	m_actionViewCube = new QAction(tr("&Cube"), this);
	m_actionViewCube->setToolTip(tr("Select this view to see a bouncing cube that responds to your accelerometer"));
    connect(m_actionViewCube, SIGNAL(triggered()), this, SLOT(actionView()));
	
	
	// Option - Earth
	m_actionOptionEarthDay = new QAction(tr("&Day"), this);
	m_actionOptionEarthDay->setToolTip(tr("Show day view global earthquake map"));
	connect(m_actionOptionEarthDay, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));

	m_actionOptionEarthNight = new QAction(tr("&Night"), this);
	m_actionOptionEarthNight->setToolTip(tr("Show night view of global earthquake map"));
	connect(m_actionOptionEarthNight, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthRotateOn = new QAction(tr("&Auto-rotate"), this);
	m_actionOptionEarthRotateOn->setToolTip(tr("Auto-rotate the globe"));
	connect(m_actionOptionEarthRotateOn, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthRotateOff = new QAction(tr("&Stop rotation"), this);
	m_actionOptionEarthRotateOff->setToolTip(tr("Stop rotation of the globe"));
	connect(m_actionOptionEarthRotateOff, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthQuakelist = new QAction(tr("&Get latest earthquakes"), this);
	m_actionOptionEarthQuakelist->setToolTip(tr("Get the latest earthquake list from the USGS"));
	connect(m_actionOptionEarthQuakelist, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	m_actionOptionEarthUSGS = new QAction(tr("&USGS Website"), this);
	m_actionOptionEarthUSGS->setToolTip(tr("Go to the USGS website for the currently selected earthquake"));
	connect(m_actionOptionEarthUSGS, SIGNAL(triggered()), this, SLOT(actionOptionEarth()));
	
	
	// Option - Sensor (2D & 3D)
	m_actionOptionSensorVerticalZoomAuto = new QAction(tr("Auto-Zoom Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomAuto->setToolTip(tr("Auto-Zoom Vertical Scale"));
	connect(m_actionOptionSensorVerticalZoomAuto, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorVerticalZoomIn = new QAction(tr("Zoom In Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomIn->setToolTip(tr("Zoom In Vertical Scale"));
	connect(m_actionOptionSensorVerticalZoomIn, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorVerticalZoomOut = new QAction(tr("Zoom Out Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomOut->setToolTip(tr("Zoom Out Vertical Scale"));
	connect(m_actionOptionSensorVerticalZoomOut, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorHorizontalZoomIn = new QAction(tr("Zoom In Time Scale"), this);
	m_actionOptionSensorHorizontalZoomIn->setToolTip(tr("Zoom In Time Scale"));
	connect(m_actionOptionSensorHorizontalZoomIn, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorHorizontalZoomOut = new QAction(tr("Zoom Out Time Scale"), this);
	m_actionOptionSensorHorizontalZoomOut->setToolTip(tr("Zoom Out Time Scale"));
	connect(m_actionOptionSensorHorizontalZoomOut, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorBack = new QAction(tr("Move Back"), this);
	m_actionOptionSensorBack->setToolTip(tr("Move Back In Time"));
	connect(m_actionOptionSensorBack, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorPause = new QAction(tr("Pause Display"), this);
	m_actionOptionSensorPause->setToolTip(tr("Pause Sensor Display"));
	connect(m_actionOptionSensorPause, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorResume = new QAction(tr("Start Display"), this);
	m_actionOptionSensorResume->setToolTip(tr("Start Sensor Display"));
	connect(m_actionOptionSensorResume, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorRecordStart = new QAction(tr("Start Recording"), this);
	m_actionOptionSensorRecordStart->setToolTip(tr("Start Recording Sensor Time Series"));
	connect(m_actionOptionSensorRecordStart, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorRecordStop = new QAction(tr("Stop Recording"), this);
	m_actionOptionSensorRecordStop->setToolTip(tr("Stop Recording Sensor Time Series"));
	connect(m_actionOptionSensorRecordStop, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorForward = new QAction(tr("Move Forward"), this);
	m_actionOptionSensorForward->setToolTip(tr("Move Forward In Time"));
	connect(m_actionOptionSensorForward, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorAbsolute = new QAction(tr("&Absolute sensor values"), this);
	m_actionOptionSensorAbsolute->setToolTip(tr("Absolute sensor values"));
	connect(m_actionOptionSensorAbsolute, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	
	m_actionOptionSensorScaled = new QAction(tr("S&caled sensor values"), this);
	m_actionOptionSensorScaled->setToolTip(tr("Scaled sensor values"));
	connect(m_actionOptionSensorScaled, SIGNAL(triggered()), this, SLOT(actionOptionSensor()));
	

	// Option action for all (Screenshot / Logo)
	m_actionOptionScreenshot = new QAction(tr(""), this);
	m_actionOptionScreenshot->setToolTip(tr(""));
	connect(m_actionOptionScreenshot, SIGNAL(triggered()), this, SLOT(actionOptionScreenshot()));
	
	m_actionOptionLogo = new QAction(tr(""), this);
	m_actionOptionLogo->setToolTip(tr(""));
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
	//m_menuFile->addSeparator();
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
	m_menuHelp->addAction(m_actionHelpAbout);
#ifndef __APPLE_CC__
	m_menuHelp->addSeparator();  // on Mac the Help/About goes on the left-most system menu, so don't need a separator
#endif
	m_menuHelp = menuBar()->addMenu(tr("&Help"));
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

void MyFrame::SetupToolbars()
{
	toolBar = CreateToolBar(wxNO_BORDER|wxHORIZONTAL, ID_TOOLBAR);
	
	if (toolBar) {
		toolBar->SetToolBitmapSize(wxSize(32,32));
		
		ToolBarView();
		ToolBarEarth(true);
		
		//if (scrollBar2D) scrollBar2D->Hide();
	}
}

void MyFrame::actionView()
{
	// get item from event do appropriate action (boinc_key_press!)
	// todo: hook up the other toolbars
	Toggle(qcn_graphics::g_eView, false, true);
	
	// figure out who called this, i.e. get the pointer of the QAction* from trigger and compare
	QAction *pAction = qobject_cast<QAction*>(QObject::sender());
	bool bChanged = false;
	if (pAction == m_actionViewSensor2D)
	{
		qcn_graphics::g_eView = VIEW_PLOT_2D;
		// note only redraw sensor toolbar if not coming from a sensor view already
		//if (qcn_graphics::g_eView != VIEW_PLOT_2D && cn_graphics::g_eView != VIEW_PLOT_3D) ToolBarSensor(evt.GetId());
		ToolBarSensor2D();
		bChanged = true;
	}
	else if (pAction == m_actionViewSensor3D)
	{
		qcn_graphics::g_eView = VIEW_PLOT_3D;
		// note only redraw sensor toolbar if not coming from a sensor view already
		//if (qcn_graphics::g_eView != VIEW_PLOT_2D && cn_graphics::g_eView != VIEW_PLOT_3D) ToolBarSensor(evt.GetId());
		ToolBarSensor3D();
		bChanged = true;
	}
	else if (pAction == m_actionViewCube)
	{
		qcn_graphics::g_eView = VIEW_CUBE;
		ToolBarCube();
		bChanged = true;
	}
	else {
		if (m_bEarthDay) {
			qcn_graphics::g_eView = VIEW_EARTH_DAY;
			qcn_graphics::earth.SetMapCombined();
		}
		else {
			qcn_graphics::g_eView = VIEW_EARTH_NIGHT;
			qcn_graphics::earth.SetMapNight();
		}
		ToolBarEarth();
		bChanged = true;
	}

	qcn_graphics::FaderOn();
    if (bChanged) {
		Toggle(qcn_graphics::g_eView, true, true);
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
	bool bChanged = false;
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
	if (!toolBar) return; // null toolbar?
	
#ifndef wxUSE_LIBPNG	
	fprintf(stdout, "Error -- you need wxWidgets with PNG support!\n");
	return;
#endif
	
	wxToolBarToolBase* toolBarView = toolBar->AddRadioTool(m_actionViewEarth, 
														   wxsShort[0], 
														   QCN_TOOLBAR_IMG(xpm_icon_earth),
														   wxNullBitmap, wxsLong[0], wxsLong[0]
														   );
	
	if (toolBarView) {
		// add the rest of the view buttons
		toolBar->AddRadioTool(m_actionViewSensor2D, 
							  wxsShort[1], 
							  QCN_TOOLBAR_IMG(xpm_icon_twod),
							  wxNullBitmap, 
							  wxsLong[1], 
							  wxsLong[1]
							  );
		
		toolBar->AddRadioTool(m_actionViewSensor3D, 
							  wxsShort[2], 
							  QCN_TOOLBAR_IMG(xpm_icon_threed),
							  wxNullBitmap,
							  wxsLong[2],
							  wxsLong[2]
							  );
		
		toolBar->AddRadioTool(m_actionViewCube, 
							  wxsShort[3], 
							  QCN_TOOLBAR_IMG(xpm_icon_cube),
							  wxNullBitmap, wxsLong[3], wxsLong[3]
							  );
		
	}
	
	toolBar->AddSeparator();
	
	// now add the menuView
	for (int i = 0; i < ciNumView; i++) 
		menuView->AppendCheckItem(m_actionViewEarth+i, wxsShort[i], wxsLong[i]);
}

void MyFrame::RemoveCurrentTools()
{  // remove the current "action" tools if any  
	// delete tools and separators after our view tool position (0-3)
	int i;
	for (i = (int)toolBar->GetToolsCount() - 1; i > ciNumView; i--) toolBar->DeleteToolByPos(i);
	i = (int)menuOptions->GetMenuItemCount()-1;
	while (i>=0) {
		wxMenuItem* wxmi = menuOptions->FindItemByPosition(i);
		if (wxmi) menuOptions->Delete(wxmi);  // for submenus use Remove
		i = (int)menuOptions->GetMenuItemCount()-1;
	}
}

void MyFrame::ToolBarEarth(bool bFirst)
{
    if (!toolBar) return; // null toolbar?
	
    if (bFirst) Toggle(m_actionViewEarth, true, true);
    wxString wxsShort[6], wxsLong[6];
	
    wxsShort[0].assign("&Day");
    wxsLong[0].assign("Show day view global earthquake map");
	
    wxsShort[1].assign("&Night");
    wxsLong[1].assign("Show night view of global earthquake map");
	
    wxsShort[2].assign("&Auto-rotate");
	wxsLong[2].assign("Auto-rotate the globe");
	
    wxsShort[3].assign("&Stop rotation");
	wxsLong[3].assign("Stop rotation of the globe");
	
    wxsShort[4].assign("&Get latest earthquakes");
    wxsLong[4].assign("Get the latest earthquake list from the USGS");
	
    wxsShort[5].assign("&USGS Website");
    wxsLong[5].assign("Go to the USGS website for the currently selected earthquake");
	
#ifndef wxUSE_LIBPNG	
    fprintf(stdout, "Error -- you need wxWidgets with PNG support!\n");
	return;
#endif
	
    if (!bFirst)
		RemoveCurrentTools();
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionEarthDay, 
									  wxsShort[0], 
									  QCN_TOOLBAR_IMG(xpm_icon_sun),
									  wxNullBitmap,
									  wxsShort[0], wxsLong[0]
									  );
    menuOptions->AppendCheckItem(m_actionOptionEarthDay, wxsShort[0], wxsLong[0]);
	
	toolBar->AddRadioTool(m_actionOptionEarthNight, 
						  wxsShort[1],
						  QCN_TOOLBAR_IMG(xpm_icon_moon),
						  wxNullBitmap, wxsShort[1], wxsLong[1]
						  );
    menuOptions->AppendCheckItem(m_actionOptionEarthNight, wxsShort[1], wxsLong[1]);
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();
	
	// stop/start rotation
	toolBar->AddRadioTool(m_actionOptionEarthRotateOn, 
						  wxsShort[2], 
						  QCN_TOOLBAR_IMG(xpm_icon_spin),
						  wxNullBitmap,
						  wxsShort[2], wxsLong[2]
						  );
    menuOptions->AppendCheckItem(m_actionOptionEarthRotateOff, wxsShort[2], wxsLong[2]);
	
	toolBar->AddRadioTool(m_actionOptionEarthRotateOff, 
						  wxsShort[3],
						  QCN_TOOLBAR_IMG(xpm_icon_nospin),
						  wxNullBitmap,
						  wxsShort[3],
						  wxsLong[3]
						  );
    menuOptions->AppendCheckItem(m_actionOptionEarthRotateOn, wxsShort[3], wxsLong[3]);
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();
	
	toolBar->AddTool(m_actionOptionEarthQuakelist, 
					 wxsShort[4],
					 QCN_TOOLBAR_IMG(xpm_icon_quakelist),
					 wxNullBitmap,
					 wxITEM_NORMAL,
					 wxsShort[4], wxsLong[4]
					 );
    menuOptions->Append(m_actionOptionEarthQuakelist, wxsShort[4], wxsLong[4]);
	
	toolBar->AddTool(m_actionOptionEarthUSGS, 
					 wxsShort[5], 
					 QCN_TOOLBAR_IMG(xpm_icon_usgs),
					 wxNullBitmap,
					 wxITEM_NORMAL, wxsShort[5], wxsLong[5]
					 );
    menuOptions->Append(m_actionOptionEarthUSGS, wxsShort[5], wxsLong[5]);
	
    AddScreenshotItem();
	
    SetToggleEarth();
	
	toolBar->Realize();
}

// toggle on off both the menu & the toolbar
void MyFrame::Toggle(const QAction* pqa, const bool bOn, const bool bView);
{
	toolBar->ToggleTool(id, bOn);
	if (bView)  // use menuView
		menuView->Check(id, bOn);
	else
		menuOptions->Check(id, bOn);
}

void MyFrame::SetToggleSensor()
{
	
	if (qcn_graphics::g_eView == VIEW_PLOT_2D) {
		toolBar->EnableTool(m_actionOptionSensorRecordStart, !sm->bRecording);
		toolBar->EnableTool(m_actionOptionSensorRecordStop, sm->bRecording);
		//Toggle(m_actionOptionSensorRecordStart, sm->bRecording);
		//Toggle(m_actionOptionSensorRecordStop, !sm->bRecording);
		
		toolBar->EnableTool(m_actionOptionSensorAbsolute, true);
		menuOptions->Enable(m_actionOptionSensorAbsolute, true);
		if (bSensorAbsolute2D) {
			if (qcn_graphics::IsScaled()) qcn_graphics::SetScaled(false);
		}
		else {
			if (!qcn_graphics::IsScaled()) qcn_graphics::SetScaled(true);
		}		  
		Toggle(m_actionOptionSensorAbsolute, bSensorAbsolute2D);
		Toggle(m_actionOptionSensorScaled, !bSensorAbsolute2D);
		
		Toggle(m_actionOptionSensorResume, (bool)(sm->bRecording && m_iSensorAction == 0));
		Toggle(m_actionOptionSensorPause, (bool)(sm->bRecording && m_iSensorAction == 1));
	}
	else if (qcn_graphics::g_eView == VIEW_PLOT_3D) { // force to be scaled
		bSensorAbsolute3D = false;
		if (!qcn_graphics::IsScaled()) qcn_graphics::SetScaled(true);
		toolBar->EnableTool(m_actionOptionSensorAbsolute, false);
		menuOptions->Enable(m_actionOptionSensorAbsolute, false);
		Toggle(m_actionOptionSensorAbsolute, bSensorAbsolute3D);
		Toggle(m_actionOptionSensorScaled, !bSensorAbsolute3D);
	}
}

void MyFrame::SetToggleEarth()
{
	Toggle(m_actionOptionEarthDay, bEarthDay);
	Toggle(m_actionOptionEarthNight, !bEarthDay);
	Toggle(m_actionOptionEarthRotateOff, !bEarthRotate);
	Toggle(m_actionOptionEarthRotateOn, bEarthRotate);
}


void MyFrame::SensorNavButtons()
{
    wxString wxsShort[8], wxsLong[8];
	
    wxsShort[0].assign("Zoom In Time Scale");
    wxsLong[0].assign("Zoom In Time Scale");
	
    wxsShort[1].assign("Zoom Out Time Scale");
    wxsLong[1].assign("Zoom Out Time Scale");
	
    wxsShort[2].assign("Move Back");
    wxsLong[2].assign("Move Back In Time");
	
    wxsShort[3].assign("Pause Display");
    wxsLong[3].assign("Pause Sensor Display");
	
    wxsShort[4].assign("Start Display");
    wxsLong[4].assign("Start Sensor Display");
	
    wxsShort[5].assign("Start Recording");
    wxsLong[5].assign("Start Recording Sensor Time Series");
	
    wxsShort[6].assign("Move Forward");
    wxsLong[6].assign("Move Forward In Time");
	
    wxsShort[7].assign("Stop Recording");
    wxsLong[7].assign("Stop Recording Sensor Time Series");
	
	m_ptbBase = toolBar->AddTool(m_actionOptionSensorHorizontalZoomIn, 
								 wxsShort[0], 
								 QCN_TOOLBAR_IMG(xpm_horiz_zoom_in),
								 wxNullBitmap, wxITEM_NORMAL,
								 wxsShort[0], 
								 wxsLong[0]
								 );
    menuOptions->Append(m_actionOptionSensorHorizontalZoomIn, wxsShort[0], wxsLong[0]);
	
	m_ptbBase = toolBar->AddTool(m_actionOptionSensorHorizontalZoomOut, 
								 wxsShort[1], 
								 QCN_TOOLBAR_IMG(xpm_horiz_zoom_out),
								 wxNullBitmap, wxITEM_NORMAL,
								 wxsShort[1], 
								 wxsShort[1]
								 );
    menuOptions->Append(m_actionOptionSensorHorizontalZoomIn, wxsShort[1], wxsShort[1]);
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();
	
	m_ptbBase = toolBar->AddTool(m_actionOptionSensorHorizontalBack, 
								 wxsShort[2], 
								 QCN_TOOLBAR_IMG(xpm_icon_rw),
								 wxNullBitmap, wxITEM_NORMAL,
								 wxsShort[2],
								 wxsLong[2]
								 );
    menuOptions->Append(m_actionOptionSensorHorizontalBack, wxsShort[2], wxsLong[2]);
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorPause, 
									  wxsShort[3],
									  QCN_TOOLBAR_IMG(xpm_icon_pause),
									  wxNullBitmap, wxsShort[3], wxsLong[3]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorPause, wxsShort[3], wxsLong[3]);
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorResume, 
									  wxsShort[4], 
									  QCN_TOOLBAR_IMG(xpm_icon_play),
									  wxNullBitmap, wxsShort[4], wxsLong[4]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorResume, wxsShort[4], wxsLong[4]);
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorRecordStart, 
									  wxsShort[5], 
									  QCN_TOOLBAR_IMG(xpm_icon_record),
									  wxNullBitmap, wxsShort[5], wxsLong[5]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorRecordStart, wxsShort[5], wxsLong[5]);
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorRecordStop, 
									  wxsShort[7], 
									  QCN_TOOLBAR_IMG(xpm_icon_stop),
									  wxNullBitmap, wxsShort[7], wxsLong[7]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorRecordStop, wxsShort[7], wxsLong[7]);
	
	m_ptbBase = toolBar->AddTool(m_actionOptionSensorForward, 
								 wxsShort[6], 
								 QCN_TOOLBAR_IMG(xpm_icon_ff),
								 wxNullBitmap, wxITEM_NORMAL,
								 wxsShort[6], wxsLong[6]
								 );
    menuOptions->Append(m_actionOptionSensorForward, wxsShort[6], wxsLong[6]);
	
}

void MyFrame::AddScreenshotItem()
{
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();
	
    toolBar->AddTool(m_actionOptionScreenshot, 
					 wxString("Screenshot", wxConvUTF8), 
					 QCN_TOOLBAR_IMG(xpm_icon_camera),
					 wxNullBitmap,
					 wxITEM_NORMAL,
					 wxString("Make a screenshot", wxConvUTF8),
					 wxString("Make a screenshot (saved in the 'sac' data folder)", wxConvUTF8)
					 );
	menuOptions->Append(m_actionOptionScreenshot, wxString("&Screenshot", wxConvUTF8),
						wxString("Make a screenshot (saved in the 'sac' data folder)", wxConvUTF8));
	
#ifdef QCNLIVE_DEMO  
	// add a function to cycle through ad images i.e. science museum logos
	menuOptions->Append(m_actionOptionLogo, wxString("Next &Logo", wxConvUTF8),
						wxString("Cycle Through Logos", wxConvUTF8));
#endif
	
}

void MyFrame::ToolBarSensor2D()
{
    if (!toolBar) return; // null toolbar?
    RemoveCurrentTools();
	
    wxString wxsShort[8];
	
    wxsShort[0].assign("Auto-Zoom Vertical Scale");
    wxsShort[1].assign("Zoom In Vertical Scale");
    wxsShort[2].assign("Zoom Out Vertical Scale");
    wxsShort[3].assign("Zoom In Time Scale");
    wxsShort[4].assign("Zoom Out Time Scale");
	
    wxsShort[5].assign("&Absolute sensor values");
    wxsShort[6].assign("S&caled sensor values");
	
    wxsShort[7].assign("&Record sensor output");
	
	// vertical zoom
	//toolBar->AddSeparator();
    //menuOptions->AppendSeparator();
	
	m_ptbBase = toolBar->AddTool(m_actionOptionSensorVerticalZoomAuto, 
								 wxsShort[0], 
								 QCN_TOOLBAR_IMG(xpm_zoom_auto),
								 wxNullBitmap, wxITEM_NORMAL,
								 wxsShort[0], 
								 wxsShort[0]
								 );
    menuOptions->Append(m_actionOptionSensorVerticalZoomAuto, wxsShort[0], wxsShort[0]);
	
	m_ptbBase = toolBar->AddTool(m_actionOptionSensorVerticalZoomIn, 
								 wxsShort[1], 
								 QCN_TOOLBAR_IMG(xpm_vert_zoom_in),
								 wxNullBitmap, wxITEM_NORMAL,
								 wxsShort[1], 
								 wxsShort[1]
								 );
    menuOptions->Append(m_actionOptionSensorVerticalZoomAuto, wxsShort[1], wxsShort[1]);
	
	m_ptbBase = toolBar->AddTool(m_actionOptionSensorVerticalZoomOut, 
								 wxsShort[2], 
								 QCN_TOOLBAR_IMG(xpm_vert_zoom_out),
								 wxNullBitmap, wxITEM_NORMAL,
								 wxsShort[2], 
								 wxsShort[2]
								 );
    menuOptions->Append(m_actionOptionSensorVerticalZoomOut, wxsShort[2], wxsShort[2]);
	
    // scrollbar for back & forth time
	if (scrollBar2D) {
		toolBar->AddSeparator();
		menuOptions->AppendSeparator();
		toolBar->AddControl(scrollBar2D);
	}
	
    SensorNavButtons();
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorAbsolute, 
									  wxsShort[5],
									  QCN_TOOLBAR_IMG(xpm_icon_absolute),
									  wxNullBitmap,
									  wxsShort[5], wxsShort[5]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorAbsolute, wxsShort[5], wxsShort[5]);
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorScaled, 
									  wxsShort[6],
									  QCN_TOOLBAR_IMG(xpm_icon_scaled),
									  wxNullBitmap,
									  wxsShort[6], wxsShort[6]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorScaled, wxsShort[6], wxsShort[6]);
	
    AddScreenshotItem();
	
	toolBar->Realize();
	
    SetToggleSensor();  // put this after realize() because we may enable/disable tools
}

void MyFrame::ToolBarSensor3D()
{
    if (!toolBar) return; // null toolbar?
    RemoveCurrentTools();
	
    wxString wxsShort[10], wxsLong[10];
	
	
	SensorNavButtons();
	
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorAbsolute, 
									  wxsShort[7],
									  QCN_TOOLBAR_IMG(xpm_icon_absolute),
									  wxNullBitmap,
									  wxsShort[7], wxsLong[7]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorAbsolute, wxsShort[7], wxsLong[7]);
	
	m_ptbBase = toolBar->AddRadioTool(m_actionOptionSensorScaled, 
									  wxsShort[8],
									  QCN_TOOLBAR_IMG(xpm_icon_scaled),
									  wxNullBitmap,
									  wxsShort[8], wxsLong[8]
									  );
    menuOptions->AppendCheckItem(m_actionOptionSensorScaled, wxsShort[8], wxsLong[8]);
	
    AddScreenshotItem();
	
	toolBar->Realize();
	
	//	qcn_graphics::g_eView = iView;
    SetToggleSensor();  // put this after realize() because we may enable/disable tools
}

void MyFrame::ToolBarCube()
{
    if (!toolBar) return; // null toolbar?
    RemoveCurrentTools();
	
    AddScreenshotItem();
	
	toolBar->Realize();
}

