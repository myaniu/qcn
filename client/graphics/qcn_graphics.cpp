// CMC -- OpenGL Graphics for QCN project
// (c) 2007 Stanford University

#ifdef _WIN32
  #pragma warning( disable : 4244 )  // Disable warning messages for double to float conversion
#endif

#include "qcn_graphics.h"
#ifndef QCNLIVE
    #include "qcn_signal.h"
    CQCNShMem* volatile sm = NULL;             // the main shared memory pointer
#endif

using std::string;
using std::vector;

// note the colors are extern'd in define.h, so keep outside the namespace qcn_graphics
GLfloat white[4] = {1., 1., 1., 1.};
GLfloat red[4] = {1., 0., 0., 1.};
GLfloat green[4] = {0., 1., 0., 1.};
//GLfloat yellow[4] = {1., 1., 0., 1.};
GLfloat yellow[4] = {1., .5, 0., 1.};
GLfloat orange[4] = {1., .5, 0., 1.};
GLfloat blue[4] = {0., 0., 1., 1.};
GLfloat magenta[4] = {1., 0., 1., 1.};
GLfloat cyan[4] = {0., 1., 1., 1.};
GLfloat dark_blue[4] = {  1.0f/255.f,   1.0f/255.f, 101.0f/255.f, 1.0f};
GLfloat dark_green[4] = { 32.0f/255.f, 101.0f/255.f,   8.0f/255.f, 1.0f};
GLfloat black[4] = {0., 0., 0., 1.};
GLfloat black_trans[4] = {0., 0., 0., .8};
GLfloat trans_red[4] = {1., 0., 0., .5};
GLfloat trans_yellow[4] = {1., 1., 0., .5};
GLfloat grey[4] = {.4,.4,.4,.3};
GLfloat grey_trans[4] = {.6,.6,.6,.6};
GLfloat white_trans[4] = {1., 1., 1., 0.50f};
GLfloat light_blue[4] = {0., 0., .5f, .5f};

#ifndef QCNLIVE

void qcn_graphics_exit()
{
   qcn_graphics::Cleanup();
}

// install signal handlers
void graphics_signal_handler(int iSignal)
{
   fprintf(stderr, "Signal %d received, exiting...\n", iSignal);
   qcn_graphics::Cleanup();
   _exit(EXIT_SIGNAL);
}

#endif

// the following are required by BOINC
void app_graphics_render(int xs, int ys, double time_of_day) 
{
    qcn_graphics::Render(xs, ys, time_of_day);
}

void app_graphics_resize(int w, int h)
{
    qcn_graphics::Resize(w, h);
}

void app_graphics_reread_prefs() 
{
   fprintf(stderr, "BOINC Graphics app_graphics_reread_prefs() request to reread project prefs at %f\n", dtime());
   qcn_graphics::getProjectPrefs();
}

// mouse drag w/ left button rotates 3D objects;
// mouse draw w/ right button zooms 3D objects
//
void boinc_app_mouse_move(int x, int y, int left, int middle, int right) 
{
    qcn_graphics::MouseMove(x, y, left, middle, right);
}

void boinc_app_mouse_button(int x, int y, int which, int is_down) 
{
    qcn_graphics::MouseButton(x, y, which, is_down);
}

void boinc_app_key_press(int k1, int k2)
{ 
   qcn_graphics::KeyDown(k1, k2);
}

void boinc_app_key_release(int k1, int k2)
{
   qcn_graphics::KeyUp(k1, k2);
}

void app_graphics_init() 
{
    qcn_graphics::Init();
}
//#endif

static const float xax[2] = { -15.0, 44.0 };
static const float yax[4] = { -25.0, -10.0, 8.0, 21.0 };
static const float xax_qcnlive[3] = { -47.0, 44.0, 49.0 };
static const float yax_qcnlive[5] = { -28.5, -13.5, 1.5, 16.5, 32.0 }; // note the last is the very top of sig, so it's 15 + .5 padding for the sig axis which is .5 above next line

static const float Y_TRIGGER_LAST[2] = { -30.0, 32.0 }; // the Y of the trigger & timer tick line

static GLfloat* colorsPlot[4] = { green, yellow, blue, red };
// time of the latest trigger, so we don't have them less than a second away, note unadjusted wrt server time!
// the "LastRebin" will be the actual displayed array offset position after the rebin
static double dTriggerLastTime[MAX_TRIGGER_LAST];    
static long lTriggerLastOffset[MAX_TRIGGER_LAST];
static long lTimeLast[MAX_TRIGGER_LAST];    
static long lTimeLastOffset[MAX_TRIGGER_LAST];
static int g_iTimeCtr = 0;
static int g_iZoomLevel = 0;


static int  iFullScreenView = 0;  // user preferred view, can be set on cmd line

// an array of x/y/z for screensaver moving
//GLfloat jiggle[3] = {0., 0., 0.};

static double dtw[2]; // time window

// an hour seems to be the max to vis without much delay
static long awinsize[MAX_KEY_WINSIZE+1]; 
static int key_winsize = 0; // points to an element of the above array to get the winsize (# of dt points i.e. 100 = 10 sec 600 = minute, 36000 = hour)
static int key_press = 0;
static int key_press_alt = 0;
static int key_up = 0;
static int key_up_alt = 0;

static long g_lSnapshotPoint = 0L;
static long g_lSnapshotPointOriginal = 0L;
static long g_lSnapshotTimeBackMinutes = 0L;  // the minutes back in time we've gone for snapshot
static bool g_bSnapshot = false;
static bool g_bSnapshotArrayProcessed = false;

static bool mouse_down = false;
static int mouseX, mouseY;
static int mouseSX, mouseSY;

static double pitch_angle[4] = {0.0, 0.0, 0.0, 0.0}; 
static double roll_angle[4] = {0.0, 0.0, 0.0, 0.0}; 
static double viewpoint_distance[4]={ 10.0, 10.0, 10.0, 10.0};

static float l_fmax[4], l_fmin[4];

//static float color[4] = {.7, .2, .5, 1};

static TEXTURE_DESC logo;  // customized version of the boinc/api/gutil.h TEXTURE_DESC
static RIBBON_GRAPH rgx, rgy, rgz, rgs; // override the standard boinc/api ribbon graph draw as it's making the earth red!

#ifndef QCNLIVE
static bool bFirstShown = false;             // flags that the view hasn't been shown yet
#endif
static bool bScaled = false;             // scaled is usually for 3D pics, but can also be done on 2D in the QCNLIVE
static char* g_strFile = NULL;           // optional file of shared memory serialization
static bool bResetArray = true;          // reset our plot memory array, otherwise it will just try to push a "live" point onto the array
static float aryg[4][PLOT_ARRAY_SIZE];   // the data points for plotting -- DS DX DY DZ

// current view is an enum i.e. { VIEW_PLOT_3D = 1, VIEW_PLOT_2D, VIEW_EARTH_DAY, VIEW_EARTH_NIGHT, VIEW_EARTH_COMBINED, VIEW_CUBE }; 
static char g_strJPG[_MAX_PATH];
static int g_iJPG = 0;

static int g_TimerTick = 5;

static int g_iScaleSigOffset = 3;
static int g_iScaleAxesOffset = 3;
static float g_fScaleSig[4] = { 1.0f, 2.5f, 5.0f, 10.0f }; // default scale for sig is 10
static float g_fScaleAxes[4] = { 2.0f, 4.9f, 9.8f, 19.6f };

#ifdef QCNLIVE
	static bool g_b2DPlotWhite = true;
#else
	static bool g_b2DPlotWhite = false;
#endif

#ifndef QCNLIVE  
// the main entry point for BOINC standalone graphics (i.e. separate executable)
int main(int argc, char** argv) 
{
#ifndef _DEBUG
   atexit(qcn_graphics_exit);
   qcn_signal::InstallHandlers(graphics_signal_handler);   // note this is set to ignore SIGPIPE by default
#endif
   return qcn_graphics::graphics_main(argc, argv);
}

/*
#ifdef _WIN32   // transfer WinMain call in Windows to our regular main()
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode) {
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
	return qcn_graphics::graphics_main(argc, argv);
}
#endif  // _WIN32
*/
#endif  // QCNLIVE

namespace qcn_graphics {

bool g_bThreadGraphics = false;
bool g_bInitGraphics   = false;

int g_width, g_height;      // window dimensions

void Cleanup()
{
   bool bInHere = false;
   if (bInHere) return; // in case called more than once 
   fprintf(stderr, "Cleaning up graphics objects...\n");
  // free project prefs
   if (sm && sm->dataBOINC.project_preferences) {
	   free(sm->dataBOINC.project_preferences);
	   sm->dataBOINC.project_preferences = NULL;
	}
   earth.Cleanup();
   vsq.clear();
   bInHere = true;
}

#ifdef QCNLIVE
// declare thread handles
#ifdef _WIN32
    HANDLE thread_handle_graphics = NULL;
#else
    pthread_t thread_handle_graphics = NULL;
#endif  // _WIN32
#endif

    /* below no longer necessary
 // below this comment will not be available externally
#ifdef _WIN32
  static HWND g_hWnd = NULL;   // need to get the handle of this window for screen coord/conversions
  inline void WinGetCoords(int& mx, int& my); // no longer necessary
#endif
*/

e_view g_eView = VIEW_PLOT_3D;  // default to 3d plots unless user prefs override below

bool g_bFullScreen = false;         // bool to denote if we're running in fullscreen (screensaver) mode
CEarth earth;   // earth object
CCube cube;     // cube object
vector<SQuake> vsq; // a vector of earthquake data struct (see qcn_graphics.h)

void getProjectPrefs()
{
    static bool bInHere = false;
    if (!sm || bInHere) return;
    bInHere = true;
    qcn_util::getBOINCInitData(WHERE_MAIN_PROJPREFS);
    qcn_graphics::parse_project_prefs();
    // tell the earth to regen earthquakes coords
    qcn_graphics::earth.RecalculateEarthquakePositions();
    qcn_graphics::earth.ResetEarthquakeNumber();
    bInHere = false;
}

int getLastTrigger(const long lTriggerCheck, const int iWinSizeArray, const int iRebin, const bool bFirst)  // we may need to see if this trigger lies within a "rebin" range for aryg
{  // this sets up the vertical views, i.e. trigger & time marker
  static long lStartTime = 0L;
  static long lCheckTime = 0L;
  int iRet = 0;
  int i;
  bool bProc = false;

  if (bFirst) lStartTime = 0L;

  for (i = 0; i < MAX_TRIGGER_LAST; i++)  {
     // check if this offset matches a trigger and is within a rounding error for the time
     if (sm->lTriggerLastOffset[i] == lTriggerCheck && fabs(sm->dTriggerLastTime[i] - sm->t0[lTriggerCheck])<.05f) break;
  }
  if (i < MAX_TRIGGER_LAST)  { 
     // it must have matched, so add this to our graphics last trigger list
     iRet = iWinSizeArray / iRebin;  // iWinSize points into the graphics array, but have to divide by iRebin to plot onto PLOT_ARRAY_SIZE
     lTriggerLastOffset[i] = iRet;  // note have to factor in the bNewPoint decrement for pushing onto the aryg array
     dTriggerLastTime[i] = sm->dTriggerLastTime[i];
  }

  if (g_eView == VIEW_PLOT_2D) { // only need the timing markers on 2d view
	// use a mod of the time interval with time - sm->dTimeStart
    // first point is never a boundary, but mark second for next time
    //long lTimeTest = (long)(sm->t0[lTriggerCheck]);
	if (lStartTime == 0L) {  // it's our first time in and our point is a valid start time
	   //long lMult = (long)(sm->t0[lTriggerCheck] - sm->dTimeStart) / g_TimerTick;
	   long lMod = (long)(sm->t0[lTriggerCheck] - sm->dTimeStart) % g_TimerTick;
	   if (sm->t0[lTriggerCheck] >= sm->dTimeStart 
		  && lMod == 0 
		  && (sm->t0[lTriggerCheck] - (float((long) sm->t0[lTriggerCheck]))) < 0.10f ) { 
           // get the even increment of sm->t0 from dTimeStart
		   //long lMult = (sm->t0[lTriggerCheck] - sm->dTimeStart) / g_TimerTick;
		   lCheckTime = sm->t0[lTriggerCheck];
	       lTimeLast[g_iTimeCtr] = lCheckTime;
		   lStartTime = lCheckTime;
		   bProc = true;
	   }
	}
	else {
	   if (sm->t0[lTriggerCheck] > lCheckTime && g_iTimeCtr < MAX_TRIGGER_LAST) {
	       lTimeLast[g_iTimeCtr] = lCheckTime;
		   bProc = true;
	   }
	}
	if (bProc && g_iTimeCtr < MAX_TRIGGER_LAST) {  // we hit a timer interval, so setup the array
	   lTimeLastOffset[g_iTimeCtr] = iWinSizeArray / iRebin;
	   g_iTimeCtr++;
       lCheckTime += g_TimerTick;  // bump up to check next second
	}
  }
  
  return iRet;

}

void getSharedMemory()
{
#ifndef QCNLIVE   // we don't need this in the GUI everything is "all in one" so sm is already setup by qcn_main
        if (sm) return; // already setup?
        // boinc_graphics_get_shmem() must be called after 
        // boinc_parse_init_data_file()
        boinc_parse_init_data_file();  // important to call this before the graphics shmem call
        sm = static_cast<CQCNShMem*>(boinc_graphics_get_shmem((char*) QCN_SHMEM));
/*
        if (sm && g_strFile && g_strFile[0] != 0x00) {
               strcpy((char*) sm->strCurFile, g_strFile);
               sm->deserialize(sm, sizeof(CQCNShMem), (const char*) sm->strCurFile);
               sm->bReadOnly = true; // put this in readonly mode
               fprintf(stdout, "QCN memory read from file %s\n", sm->strCurFile);
               fflush(stdout);
        }
*/
#endif
}

void set_viewpoint(double dist) 
{
    double x, y, z;
    x = 0.0;
    y = (g_eView == VIEW_PLOT_2D ? 0.0 : 3.0*dist); // note if in pseudo-2D mode don't shift the Y axis
    z = 11.0*dist;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
          x, y, z,        // eye position
          0,-.8,0,        // where we're looking
          0.0, 1.0, 0.    // up is in positive Y direction
    );

    if (g_eView==VIEW_PLOT_3D) { // don't pitch or roll on the 2d view
      glRotated(pitch_angle[g_eView], 1., 0., 0);
      glRotated(roll_angle[g_eView], 0., 1., 0);    
    }
}

void init_camera(double dist, double field) 
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
        field,       // field of view in degree
        1.0,        // aspect ratio
        1.0,        // Z near clip
        1000.0      // Z far
    );
    set_viewpoint(dist);
}

// set up lighting model
//
void init_lights() 
{
   GLfloat posl0[4] = {-13.0, 6.0, 20.0, 1.0};
   GLfloat dir[] = {-1, -.5, -3, 1.0};

   GLfloat ambient[] = {.2, .2, .2, 1.0};
   GLfloat diffuse[] = {.8, .8, .8, 1.0};
   GLfloat specular[]= {.0, .0, .0, 1.0};

   GLfloat shine = 1.f;
   GLfloat attenuation = 1.f;

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   glLightfv(GL_LIGHT0, GL_POSITION, posl0);
   
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

   glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

   glLightfv(GL_LIGHT0, GL_CONSTANT_ATTENUATION, &attenuation);
   glMaterialfv(GL_FRONT, GL_SHININESS, &shine);

   // no two-sided polygons
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

   // local viewer for specular light?
   glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

   // enable smoooth shading (multi-coloured polygons)
   glShadeModel(GL_SMOOTH);

   // fill polygons
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LESS);
}

void draw_logo() 
{
    if (logo.id) {
        mode_unshaded();
        mode_ortho();

        float pos[3] = {0.0, .5, 0};
/* CMC note -- the shake/jiggle sucks, don't bother
        if (bFullScreen && sm && (g_eView == VIEW_PLOT_3D || g_eView == VIEW_PLOT_2D)) { // shake on normal view
          float size[3] = { .21-(.01*aryg[E_DX][PLOT_ARRAY_SIZE-1]), 
                            .21-(.01*aryg[E_DY][PLOT_ARRAY_SIZE-1]), 
                                (.01*aryg[E_DZ][PLOT_ARRAY_SIZE-1]) };
          logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER);
        }
        else {
*/
          float size[3] = {.21, .21, 0};
          logo.draw(pos, size, ALIGN_CENTER, ALIGN_CENTER);
//      }
        ortho_done();
    }
}

void draw_text_user() 
{
   char buf[128];
   // draw text on top
   mode_unshaded();
   mode_ortho();

    if (!sm) {
       txf_render_string(.1, 0, 0, 0, 800, red, TXF_HELVETICA, (char*) "No shared memory, QCN not running?");
       return;
    }

/*
    sprintf(buf, "mouse x=%d  y=%d", mouseSX, mouseSY);
    txf_render_string(.1, 0, .04, 0, MSG_SIZE_NORMAL, red, TXF_HELVETICA, buf);
*/

    // user info
#ifdef QCNLIVE
   if (strlen((const char*) sm->strMyStation)>0) {
      sprintf(buf, "Station: %s", (const char*) sm->strMyStation);
      txf_render_string(.1, 0, .12, 0, MSG_SIZE_BIG, green, TXF_HELVETICA, buf);
   }

   if (sm && earth.IsShown() && sm->dMyLatitude != NO_LAT && sm->dMyLongitude != NO_LNG
	    && sm->dMyLatitude != 0.0f && sm->dMyLongitude != 0.0f) {
       sprintf(buf, "Location: %.4f, %.4f", sm->dMyLatitude, sm->dMyLongitude);
       txf_render_string(.1, 0, .09, 0, MSG_SIZE_BIG, green, TXF_HELVETICA, buf);
   }

   if (sm) {
      char strTime[32];
      qcn_util::FormatElapsedTime((const double&) sm->clock_time, strTime, 32);
      sprintf(buf, "Run Time: %s", strTime);
      txf_render_string(.1, 0, 0.06, 0, MSG_SIZE_NORMAL, white, TXF_HELVETICA, buf);

      qcn_util::FormatElapsedTime((const double&) sm->cpu_time, strTime, 32);
      sprintf(buf, "CPU Time: %s", strTime);
      txf_render_string(.1, 0, 0.04, 0, MSG_SIZE_NORMAL, white, TXF_HELVETICA, buf);
    }
#else
    if (sm) {
		txf_render_string(.1, 0, .125, 0, MSG_SIZE_NORMAL, white, TXF_HELVETICA, (char*) sm->dataBOINC.user_name);
      //txf_render_string(.1, 0, 0.10, 0, MSG_SIZE_NORMAL, white, TXF_HELVETICA, (char*) sm->dataBOINC.team_name);

      sprintf(buf, "WU #: %s", sm->dataBOINC.wu_name);
      txf_render_string(.1, 0, 0.105, 0, MSG_SIZE_NORMAL, white, 0, buf);

      char strTime[32];
      qcn_util::FormatElapsedTime((const double&) sm->clock_time, strTime, 32);
      sprintf(buf, "Run Time: %s", strTime);
      txf_render_string(.1, 0, 0.085, 0, MSG_SIZE_NORMAL, white, TXF_HELVETICA, buf);

      qcn_util::FormatElapsedTime((const double&) sm->cpu_time, strTime, 32);
      sprintf(buf, "CPU Time: %s", strTime);
      txf_render_string(.1, 0, 0.065, 0, MSG_SIZE_NORMAL, white, TXF_HELVETICA, buf);

      sprintf(buf, "%.2f Percent Complete", 100.0f * sm->fraction_done);
      txf_render_string(.1, 0, 0.045, 0, MSG_SIZE_NORMAL, white, TXF_HELVETICA, buf);

      if (sm && earth.IsShown() && sm->dMyLatitude != NO_LAT && sm->dMyLongitude != NO_LNG
  	    && sm->dMyLatitude != 0.0f && sm->dMyLongitude != 0.0f) {
         sprintf(buf, "Home Map Location: %.3f, %.3f", sm->dMyLatitude, sm->dMyLongitude);
         txf_render_string(.1, 0, .025, 0, MSG_SIZE_NORMAL, green, TXF_HELVETICA, buf);
      }
    }
#endif

#ifdef KEYVIEW
    sprintf(buf, "keys:  dn=%d  dnalt=%d  up=%d upalt=%d", key_press, key_press_alt, key_up, key_up_alt);
    txf_render_string(.1, 0, 0, 0, 800, red, TXF_HELVETICA, buf);
#endif

//#ifdef QCNLIVE
//   int isize = MSG_SIZE_NORMAL;
//#else
   int isize = MSG_SIZE_SMALL;
//#endif

    if (sm) {
        if (!sm->bSensorFound) {
            txf_render_string(.1, 0.003, 0.01,0, isize, red, TXF_HELVETICA, (char*) "Demo Mode - Sensor Not Found");
        } else if (sm->lOffset >=0 && sm->lOffset < sm->iWindow ) {  // we're in our calibration window
            sprintf(buf, "%s sensor calibration in progress (Reset %d)", sm->strSensor, sm->iNumReset);
            txf_render_string(.1, 0.003, 0.01, 0, isize, red, TXF_HELVETICA, buf);
        } else if (sm->strSensor[0] != 0x00) {
            sprintf(buf, "Using %s Accelerometer (Reset %d)", sm->strSensor, sm->iNumReset);
            txf_render_string(.1, 0.003, 0.01, 0, isize, red, TXF_HELVETICA, buf);
        } else if (dtime()-sm->update_time > 5) {
            txf_render_string(.1, 0.003, 0.01, 0, isize, red, TXF_HELVETICA, (char*) "QCN Not Running");
        } else if (sm->statusBOINC.suspended) {
            txf_render_string(.1, 0.003, 0.01, 0, isize, red, TXF_HELVETICA, (char*) "QCN Suspended");
		}
    } 

//#ifndef QCNLIVE  // QCNLIVE writes to the status bar on the window
      // if we wrote a JPG file, display a message for a little bit (200 frame refreshes ~ 7 seconds)
      if (++g_iJPG < 200 && g_strJPG[0] != 0x00) { // we have written a JPG file
        sprintf(buf, "Screenshot saved to: %s", g_strJPG);
        txf_render_string(.1, 0.003, 0.03, 0, MSG_SIZE_SMALL, yellow, TXF_HELVETICA, buf);
      }
//#endif
    ortho_done();
}

void draw_text_plot_qcnlive() 
{
   // draw text on top
   mode_unshaded();
   mode_ortho();

   // now draw time text at the bottom
   char strTime[16];
   //txf_render_string(.1, fWhere, Y_TRIGGER_LAST[0] - 3.0f, 0, 800, blue, TXF_HELVETICA, (char*) strTime);
    for (int i = 0; i < g_iTimeCtr; i++) {
       if (lTimeLast[i] > 0) { // there's a marker to place here
	     float fWhere = (float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE;
		 qcn_util::dtime_to_string((const double) lTimeLast[i], 'h', strTime);
		 txf_render_string(.1, fWhere - 0.042f, 0.030f, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? light_blue : grey_trans, TXF_HELVETICA, (char*) strTime);
	   }
	}

	// labels for each axis
	const float fAxisLabel = 1.061f;
	const float fVertLabel = 0.988f;
	const float fYOffset = 0.016f;

    txf_render_string(.1, fAxisLabel, 0.60f - fYOffset, 0, MSG_SIZE_NORMAL, red, TXF_HELVETICA, "Significance", 90.0f);
    txf_render_string(.1, fAxisLabel, 0.46f - fYOffset, 0, MSG_SIZE_NORMAL, blue, TXF_HELVETICA, "Z Axis", 90.0f);
    txf_render_string(.1, fAxisLabel, 0.30f - fYOffset, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, "Y Axis", 90.0f);
    txf_render_string(.1, fAxisLabel, 0.14f - fYOffset, 0, MSG_SIZE_NORMAL, green, TXF_HELVETICA, "X Axis", 90.0f);

	// labels for significance
    txf_render_string(.1, fVertLabel, 0.578f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " 0.00", 0.0f);
    txf_render_string(.1, fVertLabel, 0.604f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " 3.33", 0.0f);
    txf_render_string(.1, fVertLabel, 0.632f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " 6.67", 0.0f);
    txf_render_string(.1, fVertLabel, 0.659f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "10.00", 0.0f);
    txf_render_string(.1, fVertLabel, 0.686f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "13.33", 0.0f);
    txf_render_string(.1, fVertLabel, 0.713f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "16.67", 0.0f);
    txf_render_string(.1, fVertLabel, 0.735f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "20.00", 0.0f);

	// labels for Z axis
    txf_render_string(.1, fVertLabel, 0.413f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-19.62", 0.0f);

	// labels for Y axis
    txf_render_string(.1, fVertLabel, 0.248f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-19.62", 0.0f);

	// labels for X axis
    txf_render_string(.1, fVertLabel, 0.084f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-19.62", 0.0f);
    txf_render_string(.1, fVertLabel, 0.105f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-13.08", 0.0f);
    txf_render_string(.1, fVertLabel, 0.133f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " -6.54", 0.0f);
    txf_render_string(.1, fVertLabel, 0.161f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "  0.00", 0.0f);
    txf_render_string(.1, fVertLabel, 0.188f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " +6.54", 0.0f);
    txf_render_string(.1, fVertLabel, 0.215f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "+13.08", 0.0f);
    txf_render_string(.1, fVertLabel, 0.235f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "+19.62", 0.0f);

	// units label (meters per second per second
    txf_render_string(.1, fVertLabel, 0.066f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " m/s/s", 0.0f);

	ortho_done();
}

void draw_text_plot() 
{  // this draws the seismic sensor/accelerometer labels
    char buf[128];

   // draw text on top
   mode_unshaded();
   mode_ortho();

   // the following uncommented out will let the text bounce around!
    //static float x=0, y=0;
    //static float dx=0.0003, dy=0.0007;
    //x += dx;
    //y += dy;
    //if (x < 0 || x > .5) dx *= -1;
    //if (y < 0 || y > .5) dy *= -1;

    // left of window informative text

    // help messages
#ifndef QCNLIVE
    if (!g_bFullScreen) {
 	if (g_bSnapshot)  {
		sprintf(buf, "Press 'S' for live view"); 
		txf_render_string(.1, 0, .4, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, buf);
		sprintf(buf, "Use '<' & '>' keys to pan");
		txf_render_string(.1, 0, .38, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, buf);
	}
	else {
		sprintf(buf, "Press 'S' for snapshot view"); 
		txf_render_string(.1, 0, .4, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, buf);
	}

	//sprintf(buf, "Press 'C' for bouncy cube"); 
	//txf_render_string(.1, 0, .34, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, buf);
       
	sprintf(buf, "Press 'Q' for world earthquake map"); 
	txf_render_string(.1, 0, .34, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, buf);
       
	sprintf(buf, "Press 'L' to toggle 2D/3D Plot"); 
	txf_render_string(.1, 0, .32, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, buf);
       
	sprintf(buf, "Press +/- to change time window");
	txf_render_string(.1, 0, .30, 0, MSG_SIZE_NORMAL, yellow, TXF_HELVETICA, buf);
   }
#endif

    const float fTop[4] = { 0.09, 0.28, 0.47, 0.62 };

	// graph labels
	sprintf(buf, "Significance");
	txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DS], 0, MSG_SIZE_MEDIUM, colorsPlot[E_DS], TXF_HELVETICA, buf);
        if (sm) {
           sprintf(buf, " max=%5.2f", sm->fmax[E_DS]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DS] - 0.02, 0, MSG_SIZE_SMALL, colorsPlot[E_DS], TXF_HELVETICA, buf);
           sprintf(buf, " min=%5.2f", sm->fmin[E_DS]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DS] - 0.04, 0, MSG_SIZE_SMALL, colorsPlot[E_DS], TXF_HELVETICA, buf);
        }

	sprintf(buf, "Z-amp");
	txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DZ], 0, MSG_SIZE_MEDIUM, colorsPlot[E_DZ], TXF_HELVETICA, buf);
        if (sm) {
           //sprintf(buf, " max=%5.2f", sm->fmax[E_DZ] < 0 ? 0.0 : sm->fmax[E_DZ]);
           sprintf(buf, " max=%5.2f", sm->fmax[E_DZ]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DZ] - 0.02, 0, MSG_SIZE_SMALL, colorsPlot[E_DZ], TXF_HELVETICA, buf);
           sprintf(buf, " min=%5.2f", sm->fmin[E_DZ]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DZ] - 0.04, 0, MSG_SIZE_SMALL, colorsPlot[E_DZ], TXF_HELVETICA, buf);
        }

	sprintf(buf, "Y-amp");
	txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DY], 0, MSG_SIZE_MEDIUM, colorsPlot[E_DY], TXF_HELVETICA, buf);
        if (sm) {
           //sprintf(buf, " max=%5.2f", sm->fmax[E_DY] < 0 ? 0.0 : sm->fmax[E_DY]);
           sprintf(buf, " max=%5.2f", sm->fmax[E_DY]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DY] - 0.02, 0, MSG_SIZE_SMALL, colorsPlot[E_DY], TXF_HELVETICA, buf);
           sprintf(buf, " min=%5.2f", sm->fmin[E_DY]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DY] - 0.04, 0, MSG_SIZE_SMALL, colorsPlot[E_DY], TXF_HELVETICA, buf);
        }

	sprintf(buf, "X-amp");
	txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DX], 0, MSG_SIZE_MEDIUM, colorsPlot[E_DX], TXF_HELVETICA, buf);
        if (sm) {
           //sprintf(buf, " max=%5.2f", sm->fmax[E_DX] < 0 ? 0.0 : sm->fmax[E_DX]);
           sprintf(buf, " max=%5.2f", sm->fmax[E_DX]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DX] - 0.02, 0, MSG_SIZE_SMALL, colorsPlot[E_DX], TXF_HELVETICA, buf);
           sprintf(buf, " min=%5.2f", sm->fmin[E_DX]);
           txf_render_string(.05, TEXT_PLOT_LEFT_AXES, fTop[E_DX] - 0.04, 0, MSG_SIZE_SMALL, colorsPlot[E_DX], TXF_HELVETICA, buf);
        }

   if (g_eView == VIEW_PLOT_2D) {
/*
      switch (key_winsize) {
        case 0:
          txf_render_string(.1, 0, 0.18, 0, MSG_SIZE_SMALL, white, 0, (char*) "Small Tick Mark = 1 Second");
          txf_render_string(.1, 0, 0.16, 0, MSG_SIZE_SMALL, white, 0, (char*) "Large Tick Mark = 10 Seconds");
          break;
        case 1:
          txf_render_string(.1, 0, 0.18, 0, MSG_SIZE_SMALL, white, 0, (char*) "Small Tick Mark = 10 Seconds");
          txf_render_string(.1, 0, 0.16, 0, MSG_SIZE_SMALL, white, 0, (char*) "Large Tick Mark = 1 Minute");
          break;
        case 2:
          txf_render_string(.1, 0, 0.18, 0, MSG_SIZE_SMALL, white, 0, (char*) "Small Tick Mark = 1 Minute");
          txf_render_string(.1, 0, 0.16, 0, MSG_SIZE_SMALL, white, 0, (char*) "Large Tick Mark = 10 Minutes");
          break;
	  }
*/
	}

	char strt[2][32];
	if (g_bSnapshot) {
     	memset(strt, 0x00, sizeof(char) * 64);
	    qcn_util::dtime_to_string((const double) dtw[0], 'h', strt[0]);
	    qcn_util::dtime_to_string((const double) dtw[1], 'h', strt[1]);
        txf_render_string(.1, 0, 0.20, 0, 1500, white, TXF_HELVETICA, (char*) "Time In Hour UTC");
    }
	
    for (int jj = 0; jj < 4; jj++)  {
      if (g_bSnapshot) {  //snapshot time
     	  txf_render_string(.05, TEXT_PLOT_LEFT_AXES + 0.715f, fTop[jj] - 0.03, 0, MSG_SIZE_SMALL, white_trans, TXF_HELVETICA, strt[1]);		
       	  txf_render_string(.05, TEXT_PLOT_LEFT_AXES + 0.070f, fTop[jj] - 0.03, 0, MSG_SIZE_SMALL, white_trans, TXF_HELVETICA, strt[0]);
      }
	  else { // current time
     	  txf_render_string(.05, TEXT_PLOT_LEFT_AXES + 0.735f, fTop[jj] - 0.03, 0, MSG_SIZE_SMALL, white_trans, TXF_HELVETICA, (char*) "Now");		
		  switch(key_winsize) {
		      case 0:
			     sprintf(buf, "1 minute ago");
			     break;
		      case 1:
			     sprintf(buf, "10 minutes ago");
			     break;
		      case 2:
			     sprintf(buf, "1 hour ago");
			     break;
		  }
       	  txf_render_string(.05, TEXT_PLOT_LEFT_AXES + 0.090f, fTop[jj] - 0.03, 0, MSG_SIZE_SMALL, white_trans	, TXF_HELVETICA, buf);
	  }
	}
    ortho_done();
}

bool setupPlotMemory(const long lOffset)  
{
    static bool bInHere = false;
    static long lLastDrawnOffset = -MAXI;
	
    if (bInHere || !sm) return false;

	if (g_bSnapshot && !g_bSnapshotArrayProcessed) {
	    lLastDrawnOffset = -MAXI;  // flag to force the regeneration of the aryg data array
		g_bSnapshotArrayProcessed = true;
    }
		
	bInHere = true;

    int iRebin;
    long ii, jj;
    float fAvg[4];
    switch(key_winsize) {
      case 0: // 1 minute = 3000 pts, if PLOT_ARRAY_SIZE=600 we avg 5 points to 1
         iRebin = (int) ceil(60.0 / sm->dt) / PLOT_ARRAY_SIZE;  // for dt=.02, 3000 points,  for dt=.1, 600 pts, div by 500 iRebin = 6 or 1
         break;
      case 1: // 10 minutes = 30000 pts
         iRebin = (int) ceil(600.0 / sm->dt) / PLOT_ARRAY_SIZE; // for dt=.02, 30000 points,  for dt=.1, 6000 pts, iRebin = 60 or 12
        break;
      case 2: // 1 hour = 180000 pts 
         iRebin = (int) ceil(3600.0 / sm->dt) / PLOT_ARRAY_SIZE; // for dt=.02, 180000 points,  for dt=.1, 36000 pts, iRebin = 360 or 72
         break;
      default:  iRebin = 10; // should never get here!
    }

	if (iRebin < 1) {
	  // shouldn't happen but just in case
	  bInHere = false;
      return false;
	}

    // it would be easiest if we shift our array on iRebin-divisable boundaries -- that way each recalc will have the same values
    // in each rebin "section" i.e. 0-5 / 6-11 / 12-17 / etc.  That way we shouldn't have to bother with the bNewPoint logic as the
    // previous rebin values will appear again identically to the previous rendering
    // return if lOffset isn't evenly divisible by iRebin, i.e. we haven't gone to a new point
    // CMC - we're getting sampling problems if the frame-rate of the rendering is too low,
	// we're not getting on the proper boundary of the rebin, so if not snapshot then override
	long lDrawableOffset = (lOffset / iRebin) * iRebin; // this is the "ideal" offset, i.e. the closet point in the rebin to what we want to view at the end
	
	// of course it's possible our lOffset in question matches this, if so we're OK below
	// we want to return if: 1) we aren't reforcing the array (bResetArray) and 2) lLastDrawn != -MAXI and  3) last drawn offset matches our lDrawableOffset above
	// #3 holds whether it was live or from a snapshot (not to be confused with the JPG "SnapShot"!)

    if (!bResetArray && lLastDrawnOffset != -MAXI && lLastDrawnOffset == lDrawableOffset) {  
	    bInHere = false;
	    return false; // if offset mod iRebin isn't 0, we return (unless we're in a snapshot view of course!)
    }

    // if we made it here then we are either doing a bResetArray, or at a new offset/rebin point
	// set our lastdrawn to this point
	lLastDrawnOffset = lDrawableOffset;
  
    long lOff = lDrawableOffset - awinsize[key_winsize];
    // set timestamps for window to be displayed in the draw_text
    dtw[1] = sm->t0[lOffset];   // timestamp for end of the window (which is the current point)
    earth.SetTime(dtw[1]);

    float *af[4] = {NULL,NULL,NULL,NULL};
    for (ii = 0; ii < 4; ii++) {
      af[ii] = new float[awinsize[key_winsize]];
    }
    if (!af[E_DX] || !af[E_DY] || !af[E_DZ] || !af[E_DS]) { 
      if (af[E_DX]) delete [] af[E_DX];
      if (af[E_DY]) delete [] af[E_DY];
      if (af[E_DZ]) delete [] af[E_DZ];
      if (af[E_DS]) delete [] af[E_DS];
	  bInHere = false;
      return false;
    }

    for (ii = 0; ii < 4; ii++) {
      memset(af[ii], 0x00, sizeof(float) * awinsize[key_winsize]);
      memset((void*) aryg[ii], 0x00, sizeof(float) * PLOT_ARRAY_SIZE);
    }

    // reset our trigger list & timer values
    memset(dTriggerLastTime, 0x00, sizeof(double) * MAX_TRIGGER_LAST);
    memset(lTriggerLastOffset, 0x00, sizeof(long) * MAX_TRIGGER_LAST);
    memset(lTimeLast, 0x00, sizeof(long) * MAX_TRIGGER_LAST);
    memset(lTimeLastOffset, 0x00, sizeof(long) * MAX_TRIGGER_LAST);
    g_iTimeCtr = 0;
	
    if (lOff > 0)  {  // all points exist in our window, so we can just go into lOffset - winsize and copy/scale from there
      dtw[0] = sm->t0[lOff];  // this will be the timestamp for the beginning of the window, i.e. "awinsize[key_winsize] ticks ago"

      for (ii = 0; ii < awinsize[key_winsize]; ii++) { 
        getLastTrigger(lOff, ii, iRebin, (const bool) (ii==0));  // we may need to see if this trigger lies within a "rebin" range for aryg
#ifdef QCNLIVE   // use the bScaled value, defaults to normal 2D/absolute & 3D/scaled, but user can change
        if (! bScaled) {
#else   // 2D is always absolute, 3D is scaled
        if (g_eView == VIEW_PLOT_2D || g_eView == VIEW_CUBE) {
#endif
          af[E_DX][ii] = sm->x0[lOff]; 
          af[E_DY][ii] = sm->y0[lOff]; 
          af[E_DZ][ii] = sm->z0[lOff]; 
        }
        else {
          af[E_DX][ii] = sm->x0[lOff] - sm->xa[lOff];   
          af[E_DY][ii] = sm->y0[lOff] - sm->ya[lOff];  
		  af[E_DZ][ii] = sm->z0[lOff] - sm->za[lOff];
        }
        af[E_DS][ii] = sm->fsig[lOff];

        lOff++;
      }
    }
    else { // we are wrapping around the array, lOff <= 0
      long lStart = MAXI + lOff + 1;   // start here, wrap around to 1+(awinsize-lStart) (skip 0 as that's baseline?)
      if (lStart >= MAXI || lStart == 0) lStart = 1;
      dtw[0] = sm->t0[lStart];  // this will be the timestamp for the beginning of the window, i.e. "awinsize[key_winsize] ticks ago"

      // Note that if the array (MAXI) is close to an hour, there's problems with overlapping points
      // i.e. the "live sensor" lOffset can get ahead of the graphics and start writing over times etc!
      // so make the MAXI 2 hours or more to be safe
      //fprintf(stdout, "lOffset = %ld  lstart = %ld   dtw0 = %f\n", lOffset, lStart, dtw[0]);
      //fflush(stdout);

      for (ii = 0; ii < awinsize[key_winsize]; ii++) {
        getLastTrigger(lStart, ii, iRebin, (const bool) (ii==0));  // we may need to see if this trigger lies within a "rebin" range for aryg
#ifdef QCNLIVE   // use the bScaled value, defaults to normal 2D/absolute & 3D/scaled, but user can change
        if (! bScaled) {
#else   // 2D is always absolute, 3D is scaled
        if (g_eView == VIEW_PLOT_2D || g_eView == VIEW_CUBE) {
#endif
            af[E_DX][ii] = sm->x0[lStart]; 
            af[E_DY][ii] = sm->y0[lStart];
            af[E_DZ][ii] = sm->z0[lStart]; 
        }
        else {
          // now we can scale each axis according to fmin = 0 and fmax = 1
          af[E_DX][ii] = sm->x0[lStart] - sm->xa[lStart];  
		  af[E_DY][ii] = sm->y0[lStart] - sm->ya[lStart];   
          af[E_DZ][ii] = sm->z0[lStart] - sm->za[lStart];   
        }
        af[E_DS][ii] = sm->fsig[lStart];

        if (++lStart >= MAXI || lStart == 0) lStart = 1;
      }
    }

	// note: setup a max min range per axis per rebin -- this doesn't really correspond to absolute max/min sm->fmax/fmin values, but for display purposes
	l_fmax[E_DX] = l_fmax[E_DY] = l_fmax[E_DZ] = l_fmax[E_DS] = -10000;
	l_fmin[E_DX] = l_fmin[E_DY] = l_fmin[E_DZ] = l_fmin[E_DS] = 10000;

    // now try a simple averaging rebinning to get the array down to manageable size (1000 pts)
    for (ii = 0; ii < awinsize[key_winsize]/iRebin; ii++) {
      fAvg[E_DX] = fAvg[E_DY] = fAvg[E_DZ] = fAvg[E_DS]  = 0.0f;
	  if (iRebin == 1) {
			fAvg[E_DX] = af[E_DX][ii];
			fAvg[E_DY] = af[E_DY][ii];
			fAvg[E_DZ] = af[E_DZ][ii];
			fAvg[E_DS] = af[E_DS][ii];
	  }
	  else {
		  for (jj = 0; jj < iRebin; jj++) {
			fAvg[E_DX] += af[E_DX][(ii*iRebin)+jj];
			fAvg[E_DY] += af[E_DY][(ii*iRebin)+jj];
			fAvg[E_DZ] += af[E_DZ][(ii*iRebin)+jj];
			fAvg[E_DS] += af[E_DS][(ii*iRebin)+jj];
		  }
	  }
/*
        fAvg[E_DX] += (af[E_DX][(ii*iRebin)+jj] * 
          (g_eView == VIEW_PLOT_2D && !bScaled && key_winsize > 0 ? (af[E_DX][(ii*iRebin)+jj] > 0.0f ? 5.0f : 1.0f) : 1.0f)); 
        fAvg[E_DY] += (af[E_DY][(ii*iRebin)+jj] * 
          (g_eView == VIEW_PLOT_2D && !bScaled && key_winsize > 0 ? (af[E_DY][(ii*iRebin)+jj] > 0.0f ? 5.0f : 1.0f) : 1.0f)); 
        fAvg[E_DZ] += (af[E_DZ][(ii*iRebin)+jj] * 
          (g_eView == VIEW_PLOT_2D && !bScaled && key_winsize > 0 ? (af[E_DZ][(ii*iRebin)+jj] > 0.0f ? 5.0f : 1.0f) : 1.0f)); 
        fAvg[E_DS] += (af[E_DS][(ii*iRebin)+jj] * 
          (g_eView == VIEW_PLOT_2D && !bScaled && key_winsize > 0 ? (af[E_DS][(ii*iRebin)+jj] > 0.0f ? 5.0f : 1.0f) : 1.0f)); 
*/
      aryg[E_DX][ii] = fAvg[E_DX] / (float) iRebin;
      aryg[E_DY][ii] = fAvg[E_DY] / (float) iRebin;
      aryg[E_DZ][ii] = fAvg[E_DZ] / (float) iRebin;
      aryg[E_DS][ii] = fAvg[E_DS] / (float) iRebin;
	  
	  // note: setup a max min range per axis per rebin -- this doesn't really correspond to absolute max/min sm->fmax/fmin values, but for display purposes
      for (int qq = 0; qq < 4; qq++)  {
        if (aryg[qq][ii] < l_fmin[qq]) l_fmin[qq] = aryg[qq][ii];
        else if (aryg[qq][ii] > l_fmax[qq]) l_fmax[qq] = aryg[qq][ii];
	  }

    }

    if (af[E_DX]) delete [] af[E_DX];
    if (af[E_DY]) delete [] af[E_DY];
    if (af[E_DZ]) delete [] af[E_DZ];
    if (af[E_DS]) delete [] af[E_DS];
   
    bResetArray = false; 
    bInHere = false;

    return true;
}

void draw_tick_marks_qcnlive()
{  // draw vertical blue lines every 1/10/60/600 seconds depending on view size
	  // note the labels underneath are drawn in draw_text_plot_qcnlive
		 
    // show the time markers, if any
    glPushMatrix();
    for (int i = 0; i < g_iTimeCtr; i++) {
       if (lTimeLast[i] > 0) { // there's a marker to place here
	     float fWhere;
	     if (g_eView == VIEW_PLOT_2D) {
            fWhere = xax_qcnlive[0] + ( ((float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0]));
		 }
		 else  {
            fWhere = xax[0] + ( ((float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax[1]-xax[0]));
         }
         //fprintf(stdout, "%d  dTriggerLastTime=%f  lTriggerLastOffset=%ld  fWhere=%f\n",
         //    i, dTriggerLastTime[i], lTriggerLastOffset[i], fWhere);
         //fflush(stdout);
         glColor4fv((GLfloat*) g_b2DPlotWhite ? light_blue : grey_trans);
         glLineWidth(1);
         //glLineStipple(4, 0xAAAA);
         //glEnable(GL_LINE_STIPPLE);
         glBegin(GL_LINE_STRIP);
         glVertex2f(fWhere, Y_TRIGGER_LAST[0]);
         glVertex2f(fWhere, Y_TRIGGER_LAST[1]);
         glEnd();
         //glDisable(GL_LINE_STIPPLE);
       }
    }
    glPopMatrix();
}

void draw_triggers()
{
    // show the triggers, if any
    glPushMatrix();
    for (int i = 0; i < MAX_TRIGGER_LAST; i++) {
       if (dTriggerLastTime[i] > 0.0f) { // there's a trigger here
	     float fWhere;
	     if (g_eView == VIEW_PLOT_2D) {
            fWhere = xax_qcnlive[0] + ( ((float) (lTriggerLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0]));
		 }
		 else  {
            fWhere = xax[0] + ( ((float) (lTriggerLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax[1]-xax[0]));
         }
		 
         //fprintf(stdout, "%d  dTriggerLastTime=%f  lTriggerLastOffset=%ld  fWhere=%f\n",
         //    i, dTriggerLastTime[i], lTriggerLastOffset[i], fWhere);
         //fflush(stdout);
         glColor4fv((GLfloat*) magenta);
         glLineWidth(2);
         glLineStipple(4, 0xAAAA);
         glEnable(GL_LINE_STIPPLE);
         glBegin(GL_LINE_STRIP);
         glVertex2f(fWhere, Y_TRIGGER_LAST[0]);
         glVertex2f(fWhere, Y_TRIGGER_LAST[1]);
         glEnd();
         glDisable(GL_LINE_STIPPLE);
       }
    }
    glPopMatrix();
    glFlush();
}


void draw_plots_2d_qcnlive() 
{

/*
- boxes should be even, as well as plotting since all +/- 19.6 m/s2, sig 0 - 10
- bouncing ball at "tip" where drawn
- S/X/Y/Z on right side with vertical axis
- toggle background colors black/white?
*/

    if (!sm) return; // not much point in continuing if shmem isn't setup!

    init_camera(viewpoint_distance[g_eView], 45.0f);
    init_lights();
    scale_screen(g_width, g_height);

    // should just be simple draw each graph in 2D using the info in dx/dy/dz/ds?
    
  //  glPushMatrix();
    mode_unshaded();

	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth(2);

    float* fdata = NULL;

    // each plot section is 15 units high

	float xmin = xax_qcnlive[0] - 0.1f;
	float xmax = xax_qcnlive[1] + 0.1f;
	float ymin = yax_qcnlive[E_DX] - 5.0f;
    float ymax = yax_qcnlive[4]; // + 15.0f;

    for (int ee = E_DX; ee <= E_DS; ee++)  {
	
         switch(ee) {
            case E_DX:  fdata = (float*) aryg[E_DX]; break;
            case E_DY:  fdata = (float*) aryg[E_DY]; break;
            case E_DZ:  fdata = (float*) aryg[E_DZ]; break;
            case E_DS:  fdata = (float*) aryg[E_DS]; break;
         }

         // first draw the axes
		 // draw 2 above & 2 below and one in the middle
		 const float yfactor = 2.50f, xfactor = 0.00f;
		 for (int j = -2; j <= 3; j++) {
			 if (ee == E_DS) {
				 if (j == -2)
					 glLineWidth(3);
				 else
					 glLineWidth(1);
			 }
			 else {
				 if (j == 0)
					 glLineWidth(3);
				 else
					 glLineWidth(1);
			 }

			 glColor4fv(grey);
			 glBegin(GL_LINES);

			 if (ee == E_DS) {
				 glVertex2f(xax_qcnlive[0], yax_qcnlive[ee] + .5f + (yfactor * (float) (j+2)));
				 glVertex2f(xax_qcnlive[1] + xfactor, yax_qcnlive[ee] + .5f + (yfactor * (float) (j+2)));
			 }
			 else { 
				 if (j<3) { // only sig E_DS get's the j=3 line
					 glVertex2f(xax_qcnlive[0], yax_qcnlive[ee] + 7.5f + (yfactor * (float) j));
					 glVertex2f(xax_qcnlive[1] + xfactor, yax_qcnlive[ee] + 7.5f + (yfactor * (float) j));
				 }
			 }

			 glEnd();
		 }

         // x/y/z data points are +/- 19.6 m/s2 -- significance is 0-? make it 0-10		 		 

/*
		 float fmaxfactor;  // scale y -axes
		 if (fabs(l_fmax[ee]) > fabs(l_fmin[ee]))
		      fmaxfactor = fabs(l_fmax[ee]);
         else
		      fmaxfactor = fabs(l_fmin[ee]);
		 
		 if (fmaxfactor == 0) 
		     fmaxfactor = 1.0f;
		 else
		     fmaxfactor = MAX_PLOT_HEIGHT_QCNLIVE / fmaxfactor;
*/
		 
         glLineWidth(1);
 	     glColor4fv((GLfloat*) colorsPlot[ee]);  // set the color for data
		 glLineWidth(2.0f);
         glBegin(GL_LINE_STRIP);

         for (int i=0; i<PLOT_ARRAY_SIZE; i++) {
          if (fdata[i] != 0.0f)  {
			if (ee == E_DS && fdata[i] > 20.0f) fdata[i] = 20.0f; // clip significance at 20 for display
            glVertex2f(
              xax_qcnlive[0] + (((float) i / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0])), 
              yax_qcnlive[ee] + ( ee == E_DS ? .5f : 7.5f) + ( fdata[i] * ( 7.5f / ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] ) ))
            );
			
		   }
         }
         glEnd();

		 // plot a "colored pointer" at the end for ease of seeing current value?
		if (fdata[PLOT_ARRAY_SIZE-1] != 0.0f)  {
			float x1 = xax_qcnlive[0] + (xax_qcnlive[1]-xax_qcnlive[0]);
			float y1 = yax_qcnlive[ee] + ( ee == E_DS ? .5f : 7.5f) + ( fdata[PLOT_ARRAY_SIZE-1] * ( 7.5f / ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] ) ));
			const float fRadius = 1.4f;
			float fAngle = PI/8.0f;

			glBegin(GL_TRIANGLE_FAN);
			glVertex2f(x1, y1);
			glVertex2f(x1 + (cos(fAngle) * fRadius), y1 + (sin(fAngle) * fRadius));
			glVertex2f(x1 + (cos(-fAngle) * fRadius), y1 + (sin(-fAngle) * fRadius));
			glEnd();
		} // colored pointer
	}


	
	// draw boxes around the plots

	 const float fExt = 7.05f;
	 const float fFudge = 0.02f;

	 glColor4fv((GLfloat*) g_b2DPlotWhite ? black : grey_trans);
	 glLineWidth(2);

	 glBegin(GL_LINES);	   // really top line!
     glVertex2f(xmin, ymax+fFudge); 
     glVertex2f(xmax + fExt, ymax+fFudge);  
     glEnd();

	 glBegin(GL_LINES);	   // left side
     glVertex2f(xmin+fFudge, yax_qcnlive[E_DX]); 
     glVertex2f(xmin+fFudge, ymax+fFudge);  
     glEnd();

	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DS]);  // top line (ds)
     glVertex2f(xmax + fExt, yax_qcnlive[E_DS]);  
     glEnd();
	 		 
	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DZ]);  // z
     glVertex2f(xmax + fExt, yax_qcnlive[E_DZ]);  
     glEnd();

	 // right line
	 glBegin(GL_LINES);	 
     glVertex2f(xmax + fExt, yax_qcnlive[E_DX]);  // z
     glVertex2f(xmax + fExt, ymax+fFudge);  
     glEnd();
	 		 
	 // bottom section
	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DY]);  // y
     glVertex2f(xmax + fExt, yax_qcnlive[E_DY]); 
     glEnd();

	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DX]);  // x
     glVertex2f(xmax + fExt, yax_qcnlive[E_DX]); 
     glEnd();

 	draw_tick_marks_qcnlive();

	 glColor4fv((GLfloat*) grey);
	 glRectf(xmin, yax_qcnlive[E_DX], xmax+fExt, ymin);  // bottom rectangle (timer ticks)

	 //right side rectangular region
	 glRectf(xmax, ymax, xmax+fExt, yax_qcnlive[E_DX]);
		 
//    glPopMatrix();    
		
    glFlush();
}

void draw_plots_3d() 
{
    // setup arrays for the plots, as we'll have to pre-process the raw data from the sensor
    if (!sm) return; // not much point in continuing if shmem isn't setup!

    // use jiggle array to move around graph in x/y/z depending on values

    init_camera(viewpoint_distance[g_eView]);

/* CMC note -- the shake/jiggle sucks, don't bother
    if (g_bFullScreen && sm->lOffset > (60.0f/sm->dt)) { // probably just do this in screensaver mode after first minute?
      jiggle[E_DX] = aryg[E_DX][PLOT_ARRAY_SIZE-1];
      jiggle[E_DY] = aryg[E_DY][PLOT_ARRAY_SIZE-1];
      jiggle[E_DZ] = aryg[E_DZ][PLOT_ARRAY_SIZE-1];
      glRotated(jiggle[E_DX], 1., 0., 0.);
      glRotated(jiggle[E_DY], 0., 1., 0.);
      glRotated(jiggle[E_DZ], 0., 0., 1.);
    }
*/

    init_lights();
    scale_screen(g_width, g_height);

    rgx.draw((float*) aryg[E_DX], PLOT_ARRAY_SIZE, false);
    rgy.draw((float*) aryg[E_DY], PLOT_ARRAY_SIZE, false);
    rgz.draw((float*) aryg[E_DZ], PLOT_ARRAY_SIZE, false);
    rgs.draw((float*) aryg[E_DS], PLOT_ARRAY_SIZE, false);

    /* TODO -- get plot labels in the 3d camera view/
    // graph labels
    char buf[8];
    sprintf(buf, "Significance");
    txf_render_string(.05, -10.0, -32.0, 0, 1500, colorsPlot[E_DS], TXF_HELVETICA, buf);

    sprintf(buf, "Z-amp");
    txf_render_string(.05, -10.0, -28.0, 0, 1500, colorsPlot[E_DZ], TXF_HELVETICA, buf);

    sprintf(buf, "Y-amp");
    txf_render_string(.05, -10.0, -24.0, 0, 1500, colorsPlot[E_DY], TXF_HELVETICA, buf);

    sprintf(buf, "X-amp");
    txf_render_string(.05, -10.0, -21.0, 0, 1500, colorsPlot[E_DX], TXF_HELVETICA, buf);
    */
}

extern void ResetPlotArray()
{
   // must have switched view -- reset the graphics bool
   bResetArray = true;
   g_bSnapshotArrayProcessed = false;
}

// CMC Note --- change path to an "images" directory not the trigger directory
const char* ScreenshotJPG()
{
   double dblTime;
   if (sm && sm->strPathImage) {
	  dblTime = dtime();
      g_iJPG = 0;
      memset(g_strJPG, 0x00, sizeof(char) * _MAX_PATH);
	  sprintf(g_strJPG, "%s%c%ld_%ld_%d.jpg",
                sm->strPathImage,
                qcn_util::cPathSeparator(),
	         (long) dblTime, 
		      (long) ( (double) (dblTime - (long) dblTime) * 1000000.0f),
		      (int) g_eView			  
	 );
     if (! qcn_util::ScreenshotJPG(g_width, g_height, g_strJPG, 100)) {
        memset(g_strJPG, 0x00, sizeof(char) * _MAX_PATH);
        g_iJPG = 1000;
     }
   }
   return g_strJPG;
}

void parse_quake_info(char* strQuake, int ctr, e_quake eType)
{
   // each line of quake info is delimited by pipe (|) char -- 5 max
   // first bit is the magnitude of the quake
   // second is the UTC time
   // third is the latitude
   // fourth is the longitude
   // fifth is the depth in km
   // sixth is a text description
 
 
   const int iLenBuf = 256;
   const int iMax = 7; // max # of sections
   if ((int) strlen(strQuake) <= iMax) return; // must be an error, string too short!

   const char delim = '|'; // the pipe char is the delimiter
   // search point of next delim
   char* strWhere[iMax+1] = { strQuake, NULL, NULL, NULL, NULL, NULL, NULL, strQuake+strlen(strQuake) }; 
   SQuake sq; // a temp struct to use for the vector assign
   sq.num = ctr;
   char* buf = new char[iLenBuf];

/*
5.2|2007/11/21 08:09:15          |-32.901       | -179.221 |    53.9     |SOUTH OF THE KERMADEC ISLANDS
*/

   for (int i = 1; i <= iMax; i++)  {
     if (i<iMax)
        strWhere[i] = (char*) strchr((const char*) strWhere[i-1]+1, (int) delim);	
  
     if (strWhere[i-1]) { 
        memset(buf,0x00,iLenBuf);
        strncpy(buf, strWhere[i-1] + ((i==1) ? 0 : 1), strWhere[i] - strWhere[i-1] - ((i==1) ? 0 : 1));
        //fprintf(stdout, "buf[%d] = %s\n", i, buf);

        switch (i) {
          case 1: // mag
            sq.magnitude = atof(buf);
            break;
          case 2: // time in format YYYY/MM/DD HH:MI:SS  e.g. 2007/11/21 15:05:21
            //sq.utc_time.assign(buf); 
            if (strlen(buf) < 19) break;
            char strTmp[8];
            memset(strTmp, 0x00, 8);
            strncpy(strTmp, buf, 4);
            sq.year = atoi(strTmp);

            memset(strTmp, 0x00, 8);
            strncpy(strTmp, buf+5, 2);
            sq.month = atoi(strTmp);

            memset(strTmp, 0x00, 8);
            strncpy(strTmp, buf+8, 2);
            sq.day = atoi(strTmp);

            memset(strTmp, 0x00, 8);
            strncpy(strTmp, buf+11, 2);
            sq.hour = atoi(strTmp);

            memset(strTmp, 0x00, 8);
            strncpy(strTmp, buf+14, 2);
            sq.minute = atoi(strTmp);

            memset(strTmp, 0x00, 8);
            strncpy(strTmp, buf+17, 2);
            sq.second= atoi(strTmp);
            break;
          case 3: // lat
            sq.latitude  = atof(buf);
            break;
          case 4: // lon
            sq.longitude = atof(buf);
            break;
          case 5: // depth
            sq.depth_km  = atof(buf);
            break;
          case 6: // desc
            sq.strDesc.assign(buf);
            break;
          case 7: // URL
            sq.strURL.assign(buf);
            break;
        }
     }
   }
   sq.bActive = false;  // not actively selected yet!
   sq.eType = eType;

   if (sq.magnitude) { // we have magnitude, so we should add this
      vsq.push_back(sq);
   }
   delete [] buf;
}

const long TimeWindowWidth(int minutes)
{
    switch(minutes) {
	   case 1:
	       key_winsize = 0;
		   break;
	   case 10:
	       key_winsize = 1;
		   break;
	   case 60:
	       key_winsize = 2;
		   break;
	}
	bResetArray = true;
	g_bSnapshotArrayProcessed = false;
	return key_winsize;
}

const long TimeWindowBack()
{
	if (g_lSnapshotTimeBackMinutes == TIME_BACK_MINUTES_MAX) return TIME_BACK_MINUTES_MAX; // we've already gone back an hour, don't go back any more as the array may be wrapping!

    if (!g_bSnapshot) TimeWindowStop();
	if (g_bSnapshot) {

       switch(key_winsize) {
          case 0:
		     g_lSnapshotTimeBackMinutes += 1; break;
          case 1:
		     g_lSnapshotTimeBackMinutes += 10; break;
          case 2:
		     g_lSnapshotTimeBackMinutes += 60; break;
	   }
	   if (key_winsize == 2) { // if on hour view only ever just show the past hour
	       g_lSnapshotPoint = g_lSnapshotPointOriginal;
	   }
	   else {
	     if (g_lSnapshotTimeBackMinutes > TIME_BACK_MINUTES_MAX)  {
	       // go to one hour previous (approximately due to timing/rounding errors that may have occurred)
		   g_lSnapshotPoint = g_lSnapshotPointOriginal - awinsize[2]; // (3600.0f/sm->dt);  // e.g. 3600/.02 = 180000 points
		   if (g_lSnapshotPoint < 0) g_lSnapshotPoint += MAXI;
	       g_lSnapshotTimeBackMinutes = TIME_BACK_MINUTES_MAX;
	     }
	     else { // less than an hour, just go back normally;
		   g_lSnapshotPoint -= awinsize[key_winsize];
		   if (g_lSnapshotPoint < 0) g_lSnapshotPoint += MAXI;
	     }
       }
       bResetArray = true;
       g_bSnapshotArrayProcessed = false;
    }
	return g_lSnapshotPoint;
}

const long TimeWindowForward()
{
    if (!g_bSnapshot) TimeWindowStop();
	if (g_bSnapshot) {  // note we can't go further forward than our sm->lOffset!
       switch(key_winsize) {
          case 0:
		     g_lSnapshotTimeBackMinutes -= 1; break;
          case 1:
		     g_lSnapshotTimeBackMinutes -= 10; break;
          case 2:
		     g_lSnapshotTimeBackMinutes -= 60; break;
	   }
	   
       g_lSnapshotPoint += awinsize[key_winsize];
       if (sm && g_lSnapshotPoint > sm->lOffset) {
	      g_lSnapshotPoint = sm->lOffset-2;  // at the end!
		  g_lSnapshotPointOriginal = g_lSnapshotPoint;
		  g_lSnapshotTimeBackMinutes = 0L;
	   }
       bResetArray = true;
       g_bSnapshotArrayProcessed = false;
    }
	return g_lSnapshotPoint;
}

const long TimeWindowStop()
{
     g_bSnapshot = true;
	 if (sm && g_bSnapshot) {
	     g_lSnapshotTimeBackMinutes = 0L;
         g_lSnapshotPoint = sm->lOffset-2;  // -2 gives us some clearance we're not reading from end of array which sensor is writing
         g_lSnapshotPointOriginal = g_lSnapshotPoint; // save the original point so we can see back into the array
         if (earth.IsShown()) {
             // if in earth mode, send it back to regular seismic view, non-snapshot
             g_bSnapshot = false;
	 }
         g_bSnapshotArrayProcessed = false;
     }
	 bResetArray = true;
	 return g_lSnapshotPoint;
}

const long TimeWindowStart()
{
     g_bSnapshot = false;
     g_bSnapshotArrayProcessed = false;
     bResetArray = true;
	 return 0L;
}

const bool TimeWindowIsStopped()
{
    return g_bSnapshot;
}

const bool IsScaled()
{
    return bScaled;
}

void SetScaled(bool scaleit)
{
    bScaled = scaleit;
	bResetArray = true;
}

void parse_project_prefs()
{
   // this is where we can get a list of earthquakes 
   if (!sm || !strlen(sm->strProjectPreferences)) return;

   int iTotal = 0;
   int iLenQuake = 512; 
   char* quake = new char[iLenQuake];
   char strTmp[16];
   int ctr = 0; 
   bool bGo = true;

   // first do the current quakes
   vsq.clear();

/*#ifdef _DEBUG // just show one quake for now
   strcpy(quake, "8.7|1755/11/01 10:16:00|  36.0|-11.0|   0.0|near Lisbon, Portugal|http://earthquake.usgs.gov/regional/world/events/1755_11_01.php");
   parse_quake_info(quake, 1, QUAKE_WORLD85);
#else
*/
   while (bGo) {
      sprintf(strTmp, "<qu%03d>", ++ctr);
	  memset(quake, 0x00, iLenQuake);
      if (parse_str((const char*) sm->strProjectPreferences, strTmp, quake, iLenQuake)) {
        //fprintf(stdout, "%s\n", quake);
        iTotal++;
        parse_quake_info(quake, ctr, QUAKE_CURRENT);
      }
      else bGo = false;
   }

   // now do the historical (world quakes >=8.5 magnitude)
   bGo = true; // reset bGo but not ctr!
   ctr = 0;
   while (bGo) {
      sprintf(strTmp, "<wequ%03d>", ++ctr);
	  memset(quake, 0x00, iLenQuake);
      if (parse_str((const char*) sm->strProjectPreferences, strTmp, quake, iLenQuake)) {
        //fprintf(stdout, "%s\n", quake);
        iTotal++;
        parse_quake_info(quake, iTotal, QUAKE_WORLD85);
      }
      else bGo = false;
   }

   delete [] quake;

#ifndef QCNLIVE  // don't change in qcnlive mode - user selects the screen!
   // get station lat/lng (in QCNLive this is done elsewhere)
   parse_double((const char*) sm->strProjectPreferences, "<lat>", (double&) sm->dMyLatitude);  
   parse_double((const char*) sm->strProjectPreferences, "<lng>", (double&) sm->dMyLongitude);  
   parse_str((const char*) sm->strProjectPreferences, "<stn>", (char*) sm->strMyStation, SIZEOF_STATION_STRING); 
#endif

   // now get preferred view screensaver prefs if not set on cmd line
   if (!iFullScreenView) { // cmd line overrides prefs
     parse_int((const char*) sm->strProjectPreferences, "<ssp>", iFullScreenView);  
   }

#ifndef QCNLIVE  // don't change in qcnlive mode - user selects the screen!
     if (iFullScreenView) { // just maps to a number in the enum
        g_eView = (e_view) iFullScreenView;
     }
     else {
#ifdef _DEBUG
         g_eView = earth.ViewCombined();
#else
	   if (sm && sm->bSensorFound) {
		  g_eView = VIEW_PLOT_3D;
	   }
	   else {
		  g_eView = earth.ViewCombined();
	   }
#endif  // debug view
     }
#endif  // not qcnlive
}

int graphics_main(int argc, char** argv) 
{
    g_bThreadGraphics = false;
    g_bInitGraphics = false;
#if (defined(__APPLE__) && defined(_DEBUG))
    // Provide a way to get error messages from system in Debug builds only. 
    // In Deployment builds, ownership violates sandbox security (unless we 
    // gave it an explicit path outside the BOINC Data directory). 
    freopen("gfx_stderr.txt", "w", stderr);
#endif
    // if started with an argument, it's probably the shared mem file
    for (int i=0; i<argc; i++) {
        if (!strcmp(argv[i], "--dump") && (i+1)<argc && (argv[i+1]))
        {
            g_strFile = argv[i+1];
        }
        if (!strcmp(argv[i], "--fullscreen"))
        {
            g_bFullScreen = true;
        }
        if (!strcmp(argv[i], "--sleep"))
        {
            boinc_sleep(1.0f);  // sleep a little if starting up in demo mode
        }
        if (!strcmp(argv[i], "--view") && (i+1)<argc && (argv[i+1]))
        {
            iFullScreenView = atoi(argv[i+1]);
        }
    }
    memset(g_strJPG, 0x00, sizeof(char) * _MAX_PATH);
    g_iJPG = 0;

    getSharedMemory();

    g_bThreadGraphics = true;

#ifndef QCNLIVE
    boinc_graphics_loop(argc, argv);
    g_bThreadGraphics = false;
#endif
    return 0;
}

//}  // namespace qcn_graphics

/*
#ifdef _WIN32
// it looks like windows gives MouseX & Y absolute to the screen size,
// so we have to further adjust
inline void WinGetCoords(int& mx, int& my)
{

	if (!g_hWnd && sm) { // need to get the hWnd for this window
		char* strTitle = new char[_MAX_PATh];
		memset(strTitle, 0x00, sizeof(char) * _MAX_PATH);
		get_window_title(strTitle, _MAX_PATH);
		g_hWnd = ::FindWindow(NULL, strTitle);
		delete [] strTitle;
    }
	POINT pt;
	pt.x = mx;
	pt.y = my;
	::ScreenToClient(g_hWnd, &pt);
	mx = pt.x;
	my = pt.y;
}
#endif 
*/

void Init()
{
    static bool bFirstIn = false;
    if (bFirstIn || g_bInitGraphics) return; // return if already init
    bFirstIn = true;

    char path[_MAX_PATH];

    getProjectPrefs();

    //fprintf(stderr, "QCN Graphics Init() at %f\n", sm ? sm->update_time : 0.0f);

    // setup the window widths depending on sm->dt
    // note sm->dt could possibly be 0, if so use the DT constant (.02)
    float fdt = (sm && sm->dt) ? sm->dt : g_DT;
    awinsize[0] = (long) (60.0/ fdt);    // 1 minute = 60 seconds / dt // 3000 pts
    awinsize[1] = (long) (600.0/fdt);   // 10 minutes = 60 seconds / dt // 30000 pts
    awinsize[2] = (long) (3600.0/fdt);  // 1 minute = 60 seconds / dt // 180000 pts

    // enable hidden-surface-removal
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

// RIBBON GRAPH

    // setup ribbon graph stuff
    float size[3] = {xax[1] - xax[0], 10.0, 5.0};
    float pos[3];

    pos[0] = xax[0];
    pos[1] = yax[E_DS];
    pos[2] = 0.0;
    rgs.init(pos, size, colorsPlot[E_DS], colorsPlot[E_DS]);

    pos[0] = xax[0];
    pos[1] = yax[E_DZ];
    pos[2] = 0.0;
    rgz.init(pos, size, colorsPlot[E_DZ], colorsPlot[E_DZ]);

    pos[0] = xax[0];
    pos[1] = yax[E_DY];
    pos[2] = 0.0;
    rgy.init(pos, size, colorsPlot[E_DY], colorsPlot[E_DY]);

    pos[0] = xax[0];
    pos[1] = yax[E_DX];
    pos[2] = 0.0;
    rgx.init(pos, size, colorsPlot[E_DX], colorsPlot[E_DX]);

    // load logo & fonts
    txf_load_fonts((char*) ".");
#ifdef QCNLIVE
    strcpy(path, IMG_LOGO);
#else
    boinc_resolve_filename(IMG_LOGO, path, sizeof(path));
#endif

    if (logo.CreateTextureJPG(path)) { // can use load_image_file but we know it's a JPG so just use that
       fprintf(stderr, "Error loading QCN logo file %s\n", path); 
    }

    // get the CEarth object setup...
    earth.Init();
    //cube.Init();  // doing the cube init would probably be redundant and/or conflict with the earth init

    // everything is loaded, so set flag to true to notify main QCNLive window frame
    // and so don't come into this Init routine again (which shouldn't happen anyway of course)

#ifdef QCNLIVE
     earth.SetMapCombined();
#endif
    g_bInitGraphics = true;
}

void Render(int xs, int ys, double time_of_day)
{
    // Put this in the main loop to allow retries if the 
    // worker application has not yet created shared memory
    //
    static bool bInHere = false;
    static double dTimeCheck = sm ? sm->update_time + 3600.0f : 0.0f; // check for new prefs hourly
    static int iCounter = 0;

    if (bInHere) return; // currently rendering
    bInHere = true;

    if (!sm) { // try to get shared mem every few seconds, i.e. 100 frames
       if (!(++iCounter % 100)) {
            iCounter = 0;
            fprintf(stderr, "No shared memory found, trying to attach...\n");  // this should only be required in qcndemo mode
            getSharedMemory();
            if (sm) 
               sm->update_time = dtime();  // set update time so prefs will be read below 
       }
    }

    if (sm && (sm->update_time > dTimeCheck)) {
        //fprintf(stderr, "QCN Graphics Render() request to reread project prefs at %f\n", sm->update_time);
        dTimeCheck = sm->update_time + 3600.0f;  // check in an hour
        getProjectPrefs();   // get new prefs
    }

/*
#ifdef QCNLIVE
    if ((qcn_main::g_iStop || !sm) && qcn_graphics::g_bThreadGraphics) {
        qcn_graphics::g_bThreadGraphics = false; // mark the thread as being done
    #ifdef _WIN32
        _endthread();
    #endif
        _exit(0);
    }
#endif
*/

#ifndef QCNLIVE
    static double dTimeLast = 0.0f;
    // note don't switch views if they have a preferred view (iFullScreenView) as set on their prefs page
    if (g_bFullScreen && !iFullScreenView && sm && sm->bSensorFound)  {  //cycle through various screensaver views, not for QCNLIVE obviously
       if (sm->update_time > dTimeLast + 30.0f) {  // change at a minimum every 30 seconds
          dTimeLast = sm->update_time; // reset the timer so it will switch a view, intersperse the plot views with the quake/earth & cube
          if (bFirstShown) {
            switch (g_eView) {
               case VIEW_PLOT_3D: // if on 3d plot go to earth
                   g_eView = earth.ViewCombined(); 
                   break;
               case VIEW_EARTH_NIGHT:
               case VIEW_EARTH_COMBINED:
               case VIEW_EARTH_DAY: // if on earth go to 3d view
                   g_eView = VIEW_PLOT_2D;
                   break;
               case VIEW_PLOT_2D: // if on 2d plot go to bouncy cube
                   g_eView = VIEW_CUBE;
                   break;
               case VIEW_CUBE: // if on cube go to 3d plot
                   g_eView = VIEW_PLOT_3D;
                   break;
               default:  // should never get here
                   g_eView = earth.ViewCombined();
                   break;
            }
          }
          bFirstShown = true;             // flags that the first view hasn't been shown yet
       }
    }
#endif

    if (! earth.IsShown()) { // if we're in a "plot" view pass in either our snapshot point or current offset to setup graphics memory from sensor data
           setupPlotMemory(g_bSnapshot ? g_lSnapshotPoint : (sm->lOffset-2));  // note we use the next-to-last live point as current point is being written
    }

    // from here on is the plots or earth or cube scene
    
    // draw logo first - it's in background

    switch (g_eView) {
       case VIEW_PLOT_2D:
				//
			if (g_b2DPlotWhite)  // white background for qcnlive
				glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
			else   // black background for screensaver
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		  //    draw_logo();
          draw_plots_2d_qcnlive();
          draw_triggers();
          draw_text_plot_qcnlive();
          break;
       case VIEW_PLOT_3D:
	      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          draw_logo();
          draw_plots_3d();
          draw_triggers();
          draw_text_plot();
          draw_text_user();
          break;
       case VIEW_EARTH_DAY:
       case VIEW_EARTH_NIGHT:
       case VIEW_EARTH_COMBINED:
	      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          draw_logo();
          earth.RenderScene(g_width, g_height, viewpoint_distance[g_eView], pitch_angle[g_eView], roll_angle[g_eView]);
          earth.RenderText();
          draw_text_user();
          break;
       case VIEW_CUBE:
	      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          draw_logo();
          cube.RenderScene(g_width, g_height, viewpoint_distance[g_eView], pitch_angle[g_eView], roll_angle[g_eView]);
		  cube.RenderText();
          draw_text_user();
		  break;
    }

    glFinish();

    bInHere = false;
}

void Resize(int w, int h)
{
    g_width = w;
    g_height = h;

    glViewport(0, 0, w, h);
    if (earth.IsShown()) {
        earth.Resize(g_width, g_height);
    }
    else if (g_eView==VIEW_CUBE) {
        cube.Resize(g_width, g_height);
    }
}

void MouseMove(int x, int y, int left, int middle, int right)
{
	// swap colors on 2d view
	if (g_eView == VIEW_PLOT_2D && left && right) {
		g_b2DPlotWhite = !g_b2DPlotWhite;
	}

	if (g_eView != VIEW_EARTH_DAY && g_eView != VIEW_EARTH_NIGHT && g_eView != VIEW_EARTH_COMBINED) return;

	mouseSX = x;
    mouseSY = y;

    if (earth.IsShown()) {
      earth.MouseMotion(mouseSX, mouseSY, left, middle, right);
    }
    //else if (g_eView==VIEW_CUBE) {
    //  cube.MouseMotion(mouseSX, mouseSY, left, middle, right);
    //}
    else {
      if (left) {
          pitch_angle[g_eView] += (mouseSY-mouseY)*.1;
          roll_angle[g_eView] += (mouseSX-mouseX)*.1;
          mouseX = mouseSX;
          mouseY = mouseSY;
      } else if (right) {
          double d = (mouseSY-mouseY);
          viewpoint_distance[g_eView] *= exp(d/100.);
          mouseX = mouseSX;
          mouseY = mouseSY;
      } else {
          mouse_down = false;
      }
    }
}

void MouseButton(int x, int y, int which, int is_down)
{
	if (g_eView != VIEW_EARTH_DAY && g_eView != VIEW_EARTH_NIGHT && g_eView != VIEW_EARTH_COMBINED) return;

	if (sm) sm->dTimeInteractive = dtime();

	mouseX = x;
    mouseY = y;

    if (earth.IsShown()) {
      earth.MouseButton(mouseX, mouseY, which, is_down);
    }
    else if (g_eView==VIEW_CUBE) {
      cube.MouseButton(mouseX, mouseY, which, is_down);
    }
    else {
        mouse_down = is_down ? true : false;
        /*if (mouse_down) {  // save coords when mouse down
          mouseX = x;
          mouseY = y;
        }*/
    }
}

void KeyDown(int k1, int k2)
{
   // 27=esc  81/113 = Q, 83/115 = S, 52 = left arrow, 54 = right arrow, 45 = minus, 61 = plus
   key_press = k1;
   key_press_alt = k2;

   if (sm) sm->dTimeInteractive = dtime();
   if (key_press == 27) { // immediate quit on escape key
      qcn_graphics::Cleanup();
      _exit(0);
   }

   switch(key_press)
   { 
                case 'q':
                case 'Q':
                        // hit Q so toggle earth view/recent quake display
                        if (g_eView == VIEW_PLOT_3D || g_eView == VIEW_PLOT_2D) {
                                g_eView = earth.ViewCombined();
                        }
                        else {
                                g_eView = VIEW_PLOT_3D;
                        }
                        break;
                case 'p':
                case 'P': // screenshot
                    qcn_graphics::ScreenshotJPG();
                        break;
                case 'c':
                case 'C':
                   g_eView = VIEW_CUBE;
                   break;
                case 'l':
                case 'L':  // limit (scale) the amplitude
                        if (g_eView == VIEW_PLOT_2D) {
                           g_eView = VIEW_PLOT_3D;
                        }
                        else if (g_eView == VIEW_PLOT_3D) {
                           g_eView = VIEW_PLOT_2D;
                        }
                        bResetArray = true;
                        break;
                case 's':
                case 'S':  // hit S so toggle static display
                        g_bSnapshot = !g_bSnapshot;
                        if (sm && g_bSnapshot) {
                           g_lSnapshotPoint = sm->lOffset-2;  // -2 gives us some clearance we're not reading from end of array which sensor is writing
                           g_bSnapshotArrayProcessed = false;
                           if (earth.IsShown()) {
                                  // if in earth mode, send it back to regular seismic view, non-snapshot
                                  g_bSnapshot = false;
                           }
                        }
                        bResetArray = true;
                        break;
   }

   if (earth.IsShown()) {
       earth.KeyDown(key_press, key_press_alt);
       return;
   }
   //if (g_eView == VIEW_CUBE)  { 
   //    cube.KeyDown(key_press, key_press_alt);
   //    return;
   //}


   switch(key_press)
   {
#ifdef _WIN32
				case 188: // < on Windows, or at least on the Lenovo Thinkpad keyboard!
#endif
                case 44:  // <
                case 60:  // ,
                case 52:  // left arrow key so move snapshot point over a winsize
                        TimeWindowBack();
                        break;
#ifdef _WIN32
				case 190: // > on Windows, or at least on the Lenovo Thinkpad keyboard!
#endif
                case 46:  // >
                case 62:  // .
                case 54:  // right arrow key so move snapshot point over
                        TimeWindowForward();
                        break;
#ifdef _WIN32
				case 189: // minus on Windows, or at least on the Lenovo Thinkpad keyboard!
#endif
				case 45: // minus -- decrease the winsize 
                        if (key_winsize > 0) { // not already at the minimum 
                                key_winsize--;
                g_bSnapshotArrayProcessed = false;
                                bResetArray = true;
                        }
                        break;
#ifdef _WIN32
				case 187: // plus on Windows, or at least on the Lenovo Thinkpad keyboard!
#endif
                case 61: // plus 
                        if (key_winsize < MAX_KEY_WINSIZE) { // not already at the maximum
                                key_winsize++;
                                g_bSnapshotArrayProcessed = false;
                                bResetArray = true;
                        }
                        break;
   }
}

void KeyUp(int k1, int k2)
{
   key_up = k1;
   key_up_alt = k2;
   if (earth.IsShown()) earth.KeyUp(k1,k2);
   //else if (g_eView == VIEW_CUBE)  cube.KeyUp(k1, k2);
}

}  // namespace qcn_graphics
