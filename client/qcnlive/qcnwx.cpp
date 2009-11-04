/* qcnwx -- main QCN GUI (using wxWidgets cross-platform windowing toolkit) */

#include "qcnwx.h"
#include "qcn_graphics.h"
#include "qcn_curl.h"

#ifdef _WIN32  // Win fn ptr
  DWORD WINAPI QCNThreadMain(LPVOID /* lpParameter */)
#else  // Mac & Linux thread function ptr
  void* QCNThreadMain(void*)
#endif
{ 
    qcn_main::qcn_main(0, NULL);
    return 0;
}

// the next two will be used in the main thread, but declare here (outside the thread)
CQCNShMem* volatile sm = NULL;

void MyAppTimer::Notify() 
{ // get the earthquake list every hour
    if (pMyApp) pMyApp->GetLatestQuakeList();
}

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
	
    // CMC - start init QCN/BOINC stuff -- this gets the latest quake data and creates a boinc-style init_data.xml file
	CreateBOINCInitFile();

    qcn_main::g_bDemo = true;
    qcn_util::ResetCounter(WHERE_MAIN_STARTUP);  // this is the one and only place ResetCounter is called outside of the sensor thread, so it's safe
    qcn_main::parseArgs(0, NULL); // parse args has to be done early in startup, right after the first ResetCounter usually

    get_qcnlive_prefs();  // this sets the myRect among other things

    return StartMainThread();  
}

bool MyApp::get_qcnlive_prefs()
{
    // read in the saved trigger count
    FILE *fp; 
    char strRead[_MAX_PATH];
	
    memset(strRead, 0x00, _MAX_PATH);
	
    // basic defaults
    myRect.x      =  MY_RECT_DEFAULT_POS_X;
    myRect.y      =  MY_RECT_DEFAULT_POS_Y;
    myRect.width  =  MY_RECT_DEFAULT_WIDTH;
    myRect.height =  MY_RECT_DEFAULT_HEIGHT;

    sm->dMyLatitude = NO_LAT;
    sm->dMyLongitude = NO_LNG; 
    memset((char*) sm->strMyStation, 0x00, SIZEOF_STATION_STRING);;

    sm->dMyElevationMeter = 0.0f; 
    sm->iMyElevationFloor = 0; 
	sm->iMySensor = -1;

    if (!boinc_file_exists(QCNGUI_XML_PREFS_FILE)) return false; // don't bother if doesn't exist!

    if ( (fp = fopen(QCNGUI_XML_PREFS_FILE, "r")) == NULL) {
       fprintf(stdout, "Error opening file %s\n", QCNGUI_XML_PREFS_FILE);
       return false;
    }
    fread(strRead, sizeof(char), _MAX_PATH, fp);
    fclose(fp);

    // get the current screen dimensions so we can reset to a sensible size
    // in case they are at a lower resolution now than when they saved in the past etc
    wxSize wxsize = ::wxGetDisplaySize();

    // parse the settings, but default to sensible sizes in case they don't make sense
    char strParse[16];  // make the tag from the define which don't have the <>
    sprintf(strParse, "<%s>", XML_X);
    if (!parse_int(strRead, strParse, myRect.x) || myRect.x<0)
        myRect.x =  MY_RECT_DEFAULT_POS_X;
    sprintf(strParse, "<%s>", XML_Y);
    if (!parse_int(strRead, strParse, myRect.y) || myRect.y<0)
        myRect.y =  MY_RECT_DEFAULT_POS_Y;
    sprintf(strParse, "<%s>", XML_WIDTH);
    if (!parse_int(strRead, strParse, myRect.width) || myRect.width<100 || myRect.width > wxsize.GetWidth())
        myRect.width =  MY_RECT_DEFAULT_WIDTH;
    sprintf(strParse, "<%s>", XML_HEIGHT);
    if (!parse_int(strRead, strParse, myRect.height) || myRect.height<100 || myRect.height > wxsize.GetHeight())
        myRect.height =  MY_RECT_DEFAULT_HEIGHT;

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
                        ,
                    XML_X, myRect.x, XML_X,
                    XML_Y, myRect.y, XML_Y, 
                    XML_WIDTH, myRect.width, XML_WIDTH, 
#ifdef __WXMAC__  // bizarre Mac problem -- seems to add 39 to the height from the GetRect(), probably a toolbar fudge that they don't report elsewhere?
                    XML_HEIGHT, myRect.height - 39, XML_HEIGHT,
#else
                    XML_HEIGHT, myRect.height, XML_HEIGHT,
#endif
                    XML_LATITUDE, sm->dMyLatitude, XML_LATITUDE,
                    XML_LONGITUDE, sm->dMyLongitude, XML_LONGITUDE,
                    XML_STATION, sm->strMyStation, XML_STATION,
                    XML_ELEVATION, sm->dMyElevationMeter, XML_ELEVATION,
                    XML_FLOOR, sm->iMyElevationFloor, XML_FLOOR,
					XML_SENSOR, sm->iMySensor, XML_SENSOR
    );

    fclose(fp);
    return true;
}

IMPLEMENT_APP(MyApp) 

void MyApp::KillSplashScreen()
{  // used in a wx.CallAfter from the initial myglpane Render() call so that we can get rid of the splash screen just as soon 
   // as the final init (the graphics files loaded etc) is completed!
   if (m_psplash) {
	   m_psplash->Close();
	   delete m_psplash;
	   m_psplash = NULL;
   }
}
	
void MyApp::SetRect(const wxRect& rect)
{
   myRect = rect;
   //myRect.SetSize(newsize);
   //myRect.SetPosition(newposition);
}

bool MyApp::OnInit()
{
    m_psplash = NULL; // init the splash screen
    SetPath();  // go to the init/ directory

    // note that since we're all in one process (yet with multiple threads, we can just build our shmem struct on the heap with new
	// it will get deleted in MyApp::OnExit, so we don't need the boinc call below (which I think never destroys the shared mem segment!)
    //	sm = (CQCNShMem*) boinc_graphics_make_shmem(QCNGUI_SHMEM, sizeof(CQCNShMem));
	// init the graphics stuff, i.e. memory pointer
    // clear memory and setup important vars below

    sm = new CQCNShMem();
    if (!sm) {
        fprintf(stderr, "failed to create shared mem segment %s, exiting\n", QCNGUI_SHMEM);
        return false;
    }
    strcpy(sm->dataBOINC.wu_name, "qcnlive");

    myRect.x      = MY_RECT_DEFAULT_POS_X;
    myRect.y      = MY_RECT_DEFAULT_POS_Y;
    myRect.width  = MY_RECT_DEFAULT_WIDTH;
    myRect.height = MY_RECT_DEFAULT_HEIGHT;

    frame = new MyFrame(myRect, this);
    if (!frame) return false;  // big error if can't make the window frame!

#if wxUSE_LIBJPEG
    myJPEGHandler = new wxJPEGHandler();
    if (myJPEGHandler) 
        wxImage::AddHandler(myJPEGHandler);
#endif

#if wxUSE_LIBPNG	
    myPNGHandler = new wxPNGHandler();
	if (myPNGHandler)  {
      wxImage::AddHandler(myPNGHandler);
	  wxBitmap bitmap;
#ifndef _DEBUG // no splash screen on debug, gets in the way!
      const char cstrSplash[] = {"splash.png"};
	  if (boinc_file_exists(cstrSplash) 
		  && bitmap.LoadFile(wxString(cstrSplash, wxConvUTF8), wxBITMAP_TYPE_PNG))
	  {
          m_psplash = new wxSplashScreen(bitmap,
              wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
              60000, frame, wxID_ANY, wxDefaultPosition, wxDefaultSize,
              wxSIMPLE_BORDER|wxSTAY_ON_TOP);
      }
#endif
    }
#endif // wxUSE_LIBPNG

#ifdef _WIN32   // load the icons in init/qcnwin.ico, not we're in init/ dir by now
	if (boinc_file_exists("qcnwin.ico")) {
        frame->SetIcon(wxIcon("qcnwin.ico", wxBITMAP_TYPE_ICO));
	}
#endif

    if (!MainInit()) return false;  // this does a lot of init stuff such as get the latest quake list via curl etc

    // if here then the main thread was launched & init

	// final setup of the frame and the OpenGL canvas
    // note all the graphics init stuff is in the myGLPane constructor

    int aiAttrib[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0 };

    frame->glPane = new MyGLPane(frame, aiAttrib);
    frame->sizer = new wxBoxSizer(wxHORIZONTAL);
    frame->sizer->Add(frame->glPane, 1, wxEXPAND);
    frame->SetSizer(frame->sizer);
    frame->SetAutoLayout(true);	
   	
    // set size that was loaded into myRect earlier in the Init()
    frame->SetSize(myRect.GetSize());
    frame->SetPosition(myRect.GetPosition());

    frame->SetStatusText(wxString("Ready", wxConvUTF8));

    // setup the toolbar controls for the 2D Plot, i.e. a horiz scrollbar, buttons for scaling etc
    frame->SetupToolbars();

        // send a resized event
	//qcn_graphics::Resize(frame->GetClientSize().GetWidth(), frame->GetClientSize().GetHeight());	
	// tell the earth to regen earthquakes coords
	//qcn_graphics::earth.RecalculateEarthquakePositions();
    frame->Layout();

	frame->Show();

	// setup & start the timer for getting the next earthquake list from the qcn server
	myapptimer = new MyAppTimer(this);
	if (myapptimer) {
	  myapptimer->Start(3600000L);  // this will get the earthquake list every hour
	}

    //KillSplashScreen();
    return true;
} 

void MyApp::GetLatestQuakeList()
{
    frame->statusBar->SetStatusText(wxString("Getting recent earthquake list...", wxConvUTF8));
    // this sequence will get the latest earthquake list
    if (! CreateBOINCInitFile()) {
	    frame->statusBar->SetStatusText(wxString("Failed to get the latest earthquake list, try again later!", wxConvUTF8));
        return; // may as well split
    }
    qcn_graphics::getProjectPrefs(); // we have the strProjectPrefs so can get earthquake data now, this is in qcn_graphics.cpp
    frame->statusBar->SetStatusText(wxString("Recent earthquake data updated", wxConvUTF8));
}

int MyApp::OnExit()
{
    // just in case we're exiting early and the splash screen still up!
	if (m_psplash)  {
	   m_psplash->Close();
	   delete m_psplash;
           m_psplash = NULL;
	}
	
	if (myapptimer) {
		myapptimer->Stop();
		delete myapptimer;
		myapptimer = NULL;
	}
	
    KillMainThread();
	
	if (sm) { // try to remove the global shared mem?  that would be nice...
	   fprintf(stdout, "Freeing shared memory segment\n");
	   delete sm;
	   sm = NULL; // paranoid!  but who knows maybe the last microsecond the graphics will try to access sm...
	}
	
	fflush(stdout);

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
	// CMC -- QCN cleanup stuff, especially the main threads, was bombing out in the wxApp::OnExit
	//   which seems to somehow clobber sm or do cleanup that screws up QCN stuff somehow?
	qcn_main::doMainQuit(); // qcn_main::g_iStop = TRUE; // try and sleep a little to give the threads a chance to stop, a second should suffice
	set_qcnlive_prefs();  // save graphics prefs
	
	//if (myJPEGHandler) delete myJPEGHandler;
	//if (myPNGHandler) delete myPNGHandler;
	// free proj prefs just to be safe, destructor for APP_INIT_DATA should do it
	if (sm->dataBOINC.project_preferences) {
		free(sm->dataBOINC.project_preferences);
		sm->dataBOINC.project_preferences = NULL;
	}
	
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
