/* qcnqt -- main QCN GUI (using Qt cross-platform windowing toolkit) */

#include "qcnqt.h"
#include "qcn_graphics.h"
#include "qcn_curl.h"

#include "glwidget.h"
#include "qcnqt.h"

// main program for Qt window
int main(int argc, char *argv[])
{
	int iReturn = 1; // default to error status exit
    MyApp myApp(argc, argv); // the constructor of MyApp does all the necessary initialization, splash screens etc (destructor does the cleanup of course)
    if (myApp.Init()) { // if this is false then an error on startup
		iReturn = myApp.exec();
	}
	myApp.Exit();
    return iReturn;
}

MyApp::MyApp(int& argc, char** argv)  
  : QApplication(argc, argv), 
    m_timer(NULL), m_frame(NULL)
{
}

#ifdef _WIN32  // Win fn ptr
  DWORD WINAPI QCNThreadMain(LPVOID /* lpParameter */)
#else  // Mac & Linux thread function ptr
  void* QCNThreadMain(void*)
#endif
{ 
    return 
#ifdef _WIN32
	(DWORD)
#else 
	(void*)
#endif 
	     qcn_main::qcn_main(0, NULL);
}

// the next two will be used in the main thread, but declare here (outside the thread)
CQCNShMem* volatile sm = NULL;

void MyApp::SetPath(const char* strArgv)
{
	char* strPath = new char[_MAX_PATH];
    memset(strPath, 0x00, _MAX_PATH);
	
	if (strArgv)  {	
	  // look for the app name in the full path in the executable name
      char* strLast = strstr((char*) strArgv, QCNGUI_APP_NAME);
      if (strLast) 
         strncpy(strPath, strArgv, strLast - strArgv);
  	  else 
	     strcpy(strPath, "");  // something wrong if no path, I guess they're in root dir?
    }
	else {
	  getcwd(strPath, _MAX_PATH);
	  strPath[strlen(strPath)] = qcn_util::cPathSeparator();  // put the path char here
	}
	
    strlcat(strPath, QCNGUI_INIT_DIR, _MAX_PATH);
    _chdir(strPath); // first off, move to the init directory, this is where the boinc startup stuff is etc

      // set icon - FILENAME_LOGO is set in qcnlive_define.h for the appropriate Mac/Win version
	if (boinc_file_exists(FILENAME_LOGO)) 
		setWindowIcon(QIcon(FILENAME_LOGO));
	
	
    delete [] strPath;
}

bool MyApp::CreateBOINCInitFile()
{
    qcn_curl::g_curlBytes = 0;
	bool bInit = true;
	
	// create the init_data.xml file including the curl lookup to the website

	char* strQuake = new char[qcn_curl::BYTESIZE_CURL];
	memset(strQuake, 0x00, sizeof(char) * qcn_curl::BYTESIZE_CURL);	

    // OK, first off let's try to do a boinc curl connection to get strQuake
	if (!qcn_curl::execute_curl(QCNGUI_URL_QUAKE, strQuake, qcn_curl::BYTESIZE_CURL)) {
	    // Our curl lookup of recent quake data failed, so 
		// copy over historical quake data so we have something
	    memset(strQuake, 0x00, sizeof(char) * qcn_curl::BYTESIZE_CURL);
	    strcpy(strQuake, QCNGUI_QUAKE_HISTORICAL);
		bInit = false;	
	}

    // write a new file if we got the curl data above or file doesn't exist already
    if (bInit || !boinc_file_exists(QCNGUI_BOINC_INIT_FILE)) {
       FILE* fInitFile = fopen(QCNGUI_BOINC_INIT_FILE, "w");
  	   if (fInitFile) {
	      fprintf(fInitFile, QCNGUI_INIT_1, (int)(atof(QCN_VERSION_STRING)*100.0f), "qcnlive");
	      fprintf(fInitFile, "%s", strQuake);
	      fprintf(fInitFile, QCNGUI_INIT_2, "qcnlive");
	      fclose(fInitFile);
	      fInitFile = NULL;
	   }
	   else {
	      bInit = false;
	   }
	}
	delete [] strQuake;
	
	return bInit;
}

// this inits a lot of things and gets ready & launches the main thread etc
bool MyApp::MainInit()
{
	// delete old junk files
	//boinc_delete_file(XML_PREFS_FILE);
	boinc_delete_file(STDERR_FILE);
	boinc_delete_file("boinc_finished");
	boinc_delete_file("boinc_lockfile");
	
	// freopen stdout to stdout.txt
	if (!freopen("stdout.txt", "w", stdout)) {
           fprintf(stderr, "Can't redirect stdout for qcnwx!\n");
	}
	
	processEvents(); // give the app time to process mouse events since we're before the event loop

    // start init QCN/BOINC stuff -- this gets the latest quake data and creates a boinc-style init_data.xml file
	if (m_psplash) m_psplash->showMessage(tr("Retrieving latest earthquakes..."), Qt::AlignRight | Qt::AlignBottom, Qt::black);
	
	CreateBOINCInitFile();

    qcn_main::g_bDemo = false;
	qcn_graphics::g_bFader = false; // no fader required, just in screensaver mode
    qcn_util::ResetCounter(WHERE_MAIN_STARTUP);  // this is the one and only place ResetCounter is called outside of the sensor thread, so it's safe
    qcn_main::parseArgs(0, NULL); // parse args has to be done early in startup, right after the first ResetCounter usually

	processEvents(); // give the app time to process mouse events since we're before the event loop
	if (m_psplash) m_psplash->showMessage(tr("Getting initial settings..."), Qt::AlignRight | Qt::AlignBottom, Qt::black);
    get_qcnlive_prefs();  // this sets the m_rect among other things

    return StartMainThread();  
}

bool MyApp::get_qcnlive_prefs()
{
    // read in the saved trigger count
    FILE *fp; 
    char strRead[_MAX_PATH];
	
    memset(strRead, 0x00, _MAX_PATH);
	
    // basic defaults
    m_rect.setX(MY_RECT_DEFAULT_POS_X);
    m_rect.setY(MY_RECT_DEFAULT_POS_Y);
    m_rect.setWidth(MY_RECT_DEFAULT_WIDTH);
    m_rect.setHeight(MY_RECT_DEFAULT_HEIGHT);

    sm->dMyLatitude = NO_LAT;
    sm->dMyLongitude = NO_LNG; 
    memset((char*) sm->strMyStation, 0x00, SIZEOF_STATION_STRING);;

    sm->dMyElevationMeter = 0.0f; 
    sm->iMyElevationFloor = 0; 
	sm->iMySensor = -1;
	sm->bMyContinual = false;  // default to no continual recording (i.e. user has to start/stop recording via the button)
	sm->bMyOutputSAC = false;  // default to csv text output i.e. not sac

    if (!boinc_file_exists(QCNGUI_XML_PREFS_FILE)) return false; // don't bother if doesn't exist!

    if ( (fp = fopen(QCNGUI_XML_PREFS_FILE, "r")) == NULL) {
       fprintf(stdout, "Error opening file %s\n", QCNGUI_XML_PREFS_FILE);
       return false;
    }
    fread(strRead, sizeof(char), _MAX_PATH, fp);
    fclose(fp);

    // get the current screen dimensions so we can reset to a sensible size
    // in case they are at a lower resolution now than when they saved in the past etc
    QSize qsize(this->desktop()->size());

    // parse the settings, but default to sensible sizes in case they don't make sense
    char strParse[16];  // make the tag from the define which don't have the <>
	int iTemp = -1;
    sprintf(strParse, "<%s>", XML_X);
    if (parse_int(strRead, strParse, iTemp) && iTemp >= 0)
		m_rect.setX(iTemp);
	else
        m_rect.setX(MY_RECT_DEFAULT_POS_X);
	
	iTemp = -1;
    sprintf(strParse, "<%s>", XML_Y);
    if (parse_int(strRead, strParse, iTemp) && iTemp >= 0)
		m_rect.setY(iTemp);
	else
        m_rect.setY(MY_RECT_DEFAULT_POS_Y);

	iTemp = -1;
    sprintf(strParse, "<%s>", XML_WIDTH);
    if (parse_int(strRead, strParse, iTemp) && iTemp >= 100 && iTemp <= qsize.width())
		m_rect.setWidth(iTemp);
	else
        m_rect.setWidth(MY_RECT_DEFAULT_WIDTH);
    
	iTemp = -1;
	sprintf(strParse, "<%s>", XML_HEIGHT);
    if (parse_int(strRead, strParse, iTemp) && iTemp >= 100 && iTemp <= qsize.height())
		m_rect.setHeight(iTemp);
	else
        m_rect.setHeight(MY_RECT_DEFAULT_HEIGHT);

    // get preferred sensor if any
    sprintf(strParse, "<%s>", XML_SENSOR);
    if (!parse_int(strRead, strParse, sm->iMySensor) || sm->iMySensor <= 0)
        sm->iMySensor = -1;
	
    // check for valid lat/lng range
    sprintf(strParse, "<%s>", XML_LATITUDE);
    if (parse_double(strRead, strParse, (double&) sm->dMyLatitude)) {
      if (sm->dMyLatitude < -90.0f || sm->dMyLatitude > 90.0f) {
        sm->dMyLatitude = NO_LAT;
      }
    }
    else {
        sm->dMyLatitude = NO_LAT;
    }

    sprintf(strParse, "<%s>", XML_LONGITUDE);
    if (parse_double(strRead, strParse, (double&) sm->dMyLongitude)) {
      if (sm->dMyLongitude < -180.0f || sm->dMyLongitude > 180.0f) {
        sm->dMyLongitude = NO_LNG;
      }
    }
    else {
        sm->dMyLongitude = NO_LNG;
    }

    sprintf(strParse, "<%s>", XML_STATION);
    if (!parse_str(strRead, strParse, (char*) sm->strMyStation, SIZEOF_STATION_STRING))
       memset((char*) sm->strMyStation, 0x00, SIZEOF_STATION_STRING);

    if (strlen(sm->strMyStation)>0) {
		strcpy(sm->dataBOINC.wu_name, sm->strMyStation);
    }

    // elevation data
    sprintf(strParse, "<%s>", XML_ELEVATION);
    parse_double(strRead, strParse, (double&) sm->dMyElevationMeter);
    sprintf(strParse, "<%s>", XML_FLOOR);
    parse_int(strRead, strParse, (int&) sm->iMyElevationFloor);

    // continual
	int iTmp = 0;
    sprintf(strParse, "<%s>", XML_CONTINUAL);
    parse_int(strRead, strParse, iTmp);
	sm->bMyContinual = (bool)(iTmp > 0);
		
    // sac format
	iTmp = 0;
    sprintf(strParse, "<%s>", XML_SACFORMAT);
    parse_int(strRead, strParse, iTmp);
	sm->bMyOutputSAC = (bool)(iTmp > 0);

    return true;
}

bool MyApp::set_qcnlive_prefs()
{
    FILE *fp; 
	if (strlen(sm->strMyStation)>0) strcpy(sm->dataBOINC.wu_name, sm->strMyStation); // copy station name to workunit name
    if ( (fp = fopen(QCNGUI_XML_PREFS_FILE, "w")) == NULL) {
       fprintf(stdout, "Error opening file %s\n", QCNGUI_XML_PREFS_FILE);
       return false;
    }
	
    // just save a string of pseudo-XML	
    fprintf(fp, "<%s>%d</%s>\n"
                "<%s>%d</%s>\n"
                "<%s>%d</%s>\n"
                "<%s>%d</%s>\n"
                "<%s>%f</%s>\n"
                "<%s>%f</%s>\n"
                "<%s>%s</%s>\n"
                "<%s>%f</%s>\n"
                "<%s>%d</%s>\n"
   			    "<%s>%d</%s>\n"
				"<%s>%d</%s>\n"
				"<%s>%d</%s>\n"
                        ,
                    XML_X, m_rect.x(), XML_X,
                    XML_Y, m_rect.y(), XML_Y, 
                    XML_WIDTH, m_rect.width(), XML_WIDTH, 
                    XML_HEIGHT, m_rect.height(), XML_HEIGHT,
                    XML_LATITUDE, sm->dMyLatitude, XML_LATITUDE,
                    XML_LONGITUDE, sm->dMyLongitude, XML_LONGITUDE,
                    XML_STATION, sm->strMyStation, XML_STATION,
                    XML_ELEVATION, sm->dMyElevationMeter, XML_ELEVATION,
                    XML_FLOOR, sm->iMyElevationFloor, XML_FLOOR,
					XML_SENSOR, sm->iMySensor, XML_SENSOR,
					XML_CONTINUAL, (sm->bMyContinual ? 1 : 0), XML_CONTINUAL,
					XML_SACFORMAT, (sm->bMyOutputSAC ? 1 : 0), XML_SACFORMAT
    );

    fclose(fp);
    return true;
}

void MyApp::KillSplashScreen()
{  // used in a wx.CallAfter from the initial myglpane Render() call so that we can get rid of the splash screen just as soon 
   // as the final init (the graphics files loaded etc) is completed!
   if (m_psplash) {
	   m_psplash->close();
	   delete m_psplash;
	   m_psplash = NULL;
   }
}
	
void MyApp::SetRect(const QRect& rect)
{
   m_rect = rect;
}

bool MyApp::Init()
{
	m_bInit = false;
	// do splash screen until the mainwin show	
    m_psplash = NULL; // init the splash screen
	
    SetPath();  // go to the init/ directory
	
	// splash screen
	if (boinc_file_exists(FILENAME_SPLASH)) {
		m_psplash = new QSplashScreen(QPixmap(FILENAME_SPLASH));

	}
	if (m_psplash) {
		m_psplash->show();
		m_psplash->showMessage(tr("Starting up..."), Qt::AlignRight | Qt::AlignBottom, Qt::black);
	}
	
    // note that since we're all in one process (yet with multiple threads, we can just build our shmem struct on the heap with new
	// it will get deleted in MyApp::Exit, so we don't need the boinc call below (which I think never destroys the shared mem segment!)
    //	sm = (CQCNShMem*) boinc_graphics_make_shmem(QCNGUI_SHMEM, sizeof(CQCNShMem));
	// init the graphics stuff, i.e. memory pointer
    // clear memory and setup important vars below

	processEvents(); // give the app time to process mouse events since we're before the event loop
	
    sm = new CQCNShMem();
    if (!sm) {
        fprintf(stderr, "failed to create shared mem segment %s, exiting\n", QCNGUI_SHMEM);
        return false;
    }
    strcpy(sm->dataBOINC.wu_name, "qcnlive");

	/*
    m_rect.setX(MY_RECT_DEFAULT_POS_X);
    m_rect.setY(MY_RECT_DEFAULT_POS_Y);
    m_rect.setWidth(MY_RECT_DEFAULT_WIDTH);
    m_rect.setHeight(MY_RECT_DEFAULT_HEIGHT);
	*/

    if (!MainInit()) return false;  // this does a lot of init stuff such as get the latest quake list via curl etc
	
    // if here then the main thread was launched & init
	processEvents(); // give the app time to process mouse events since we're before the event loop

    // setup the toolbar controls for the 2D Plot, i.e. a horiz scrollbar, buttons for scaling etc
    // CMC frame->SetupToolbars();

	// setup & start the timer for getting the next earthquake list from the qcn server
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotGetLatestQuakeList()));
    m_timer->start(1800000L);  // in milliseconds, so a half hour is a lot!
	
	processEvents(); // give the app time to process mouse events since we're before the event loop
	if (m_psplash) m_psplash->showMessage(tr("Preparing graphics engine..."), Qt::AlignRight | Qt::AlignBottom, Qt::black);
	m_frame = new MyFrame(this);  // construct the window frame
	if (m_frame) {
		m_frame->Init();
		processEvents(); // give the app time to process mouse events since we're before the event loop
		m_frame->show();  // show the main window frame
		if (m_psplash) m_psplash->finish(m_frame);  // this will sto the m_psplash after the main window is shown
		KillSplashScreen();
		m_bInit = true;
	}
	KillSplashScreen(); // just in case the myframe construction failed

    return m_bInit;
} 

// a private slot to launch the quakelist
void MyApp::slotGetLatestQuakeList()
{
	GetLatestQuakeList();
}

void MyApp::GetLatestQuakeList()
{
	m_frame->statusBar()->showMessage(tr("Getting recent earthquake list..."), 5000);

    // this sequence will get the latest earthquake list
    if (! CreateBOINCInitFile()) {
		m_frame->statusBar()->showMessage(tr("Failed to get the latest earthquake list, try again later!"), 5000);
        return; // may as well split
    }
    qcn_graphics::getProjectPrefs(); // we have the strProjectPrefs so can get earthquake data now, this is in qcn_graphics.cpp
	m_frame->statusBar()->showMessage(tr("Earthquake list updated"), 5000);
}

int MyApp::Exit()
{
	if (m_timer) {
		m_timer->stop();
		delete m_timer;
		m_timer = NULL;
	}
	if (m_frame) {
		delete m_frame; // necessary?
		m_frame = NULL;
	}

    KillMainThread();
	
	qcn_graphics::Cleanup();
	
	if (sm) { // try to remove the global shared mem?  that would be nice...
	   fprintf(stdout, "Freeing shared memory segment\n");
	   delete sm;
	   sm = NULL; // paranoid!  but who knows maybe the last microsecond the graphics will try to access sm...
	}
	
	fflush(stdout);

    // just in case we're exiting early and the splash screen still up!	
	KillSplashScreen();

    return 0;
}

bool MyApp::StartMainThread()
{
	qcn_main::g_threadMain = new CQCNThread(QCNThreadMain);
	if (qcn_main::g_threadMain) qcn_main::g_threadMain->Start();  // note returns whether main thread was created & started OK
	return (bool) (qcn_main::g_threadMain != NULL);
}

bool MyApp::KillMainThread()
{
	qcn_main::doMainQuit(); // qcn_main::g_iStop = TRUE; // try and sleep a little to give the threads a chance to stop, a second should suffice
	set_qcnlive_prefs();  // save graphics prefs
	
	int iCtr = 0;
	if (qcn_main::g_threadMain && qcn_main::g_threadMain->IsRunning())  { // the main thread is running, so kill it
		fprintf(stdout, "qcnwx: stopping main monitoring thread\n");
		while (qcn_main::g_threadMain->IsRunning() && iCtr < 3000) {
			iCtr++;
			usleep(1000);
		}
		fprintf(stdout, "qcnwx: main thread quit within %f seconds...\n", (float) iCtr * .001f);
	}
	else {
		fprintf(stdout, "qcnwx: main thread stopped\n");
	}
	
	if (qcn_main::g_threadMain) { // free main thread resources
		qcn_main::g_threadMain->Stop();
		delete qcn_main::g_threadMain;
		qcn_main::g_threadMain = NULL;
	}
	return true;
}
