#ifndef _QCN_GRAPHICS_H_
#define _QCN_GRAPHICS_H_

#include <string>
#include <vector>

#ifdef _WIN32
  #include <windows.h>
  #include "boinc_win.h"
#else
  #include <math.h>
#endif

#include "boinc_gl.h"
#include "gutil.h"

#include "reduce.h"  // boinc includes -- reduce, parse -- parses xml files, util & gutil & gl & graphics stuff etc
#include "parse.h"
#include "qcn_util.h"
#include "util.h"  // this is boinc/lib/util
#include "app_ipc.h"
#include "boinc_api.h"
#include "graphics2.h"

#include "txf_util.h"

#include "qcn_shmem.h"
#include "qcn_util.h"

#ifdef QCNLIVE
   #include "main.h"
#endif

using std::string;
using std::vector;

extern CQCNShMem* volatile sm;                    // the main shared memory pointer

extern GLfloat white[4];
extern GLfloat red[4];
extern GLfloat green[4];
extern GLfloat yellow[4];
extern GLfloat cyan[4];
extern GLfloat magenta[4];
extern GLfloat blue[4];
extern GLfloat purple[4];
extern GLfloat dark_blue[4];
extern GLfloat dark_green[4];
extern GLfloat black[4];
extern GLfloat trans_red[4];
extern GLfloat trans_yellow[4];
extern GLfloat grey[4];
extern GLfloat grey_trans[4];
extern GLfloat light_blue[4];
extern GLfloat orange[4];

// structures
struct Point3f {
        float x, y, z;
};

struct Mapping2f {
        float u, v;
};

// struct for earthquake info
struct SQuake
{
   public:
      float magnitude;
      float latitude;
      float longitude;
      float depth_km;
      int num;
      int year;
      int month;
      int day;
      int hour;
      int minute;
      int second;
      string strDesc;
      string strURL;
      Point3f v;
      float radius;
      bool bActive;
      e_quake eType;

      SQuake() { memset(this, 0x00, sizeof(this)); };
      ~SQuake() { };
};

class CEarth;

//#ifndef QCNLIVE
// boinc stuff -- need to keep out of the namespace as boinc calls these exact named functions
extern void app_graphics_init();
extern void app_graphics_render(int xs, int ys, double time_of_day);
extern void app_graphics_resize(int w, int h);
extern void boinc_app_mouse_move(int x, int y, int left, int middle, int right);
extern void boinc_app_mouse_button(int x, int y, int which, int is_down);
extern void boinc_app_key_press(int k1, int k2);
extern void boinc_app_key_release(int k1, int k2);
extern void app_graphics_reread_prefs();
//#endif

namespace qcn_graphics {

extern void Cleanup();

extern void Init();
extern void Render(int xs, int ys, double time_of_day);
extern void Resize(int w, int h);
extern void MouseMove(int x, int y, int left, int middle, int right);
extern void MouseButton(int x, int y, int which, int is_down);
extern void KeyDown(int k1, int k2);
extern void KeyUp(int k1, int k2);

extern vector<SQuake> vsq; // a vector of earthquake data struct
extern bool g_bFullScreen;
extern int g_width, g_height;
extern CEarth earth;

extern e_view g_eView;  // default to 3d plots unless user prefs override below
#ifdef QCNLIVE
  extern bool g_bThreadGraphics;
  extern bool g_bInitGraphics;
  // declare thread handles
#ifdef _WIN32
   extern HANDLE thread_handle_graphics;
#else
   extern pthread_t thread_handle_graphics;
#endif  // _WIN32
#endif

extern const float xax[2];
extern const float yax[4];
extern const float xax_qcnlive[3];
extern const float yax_qcnlive[5]; // note the last is the very top of sig, so it's 15 + .5 padding for the sig axis which is .5 above next line

extern const float Y_TRIGGER_LAST[2]; // the Y of the trigger & timer tick line

extern int  iFullScreenView;  // user preferred view, can be set on cmd line

extern GLfloat* colorsPlot[4];
// time of the latest trigger, so we don't have them less than a second away, note unadjusted wrt server time!
// the "LastRebin" will be the actual displayed array offset position after the rebin
extern double dTriggerLastTime[MAX_TRIGGER_LAST];    
extern long lTriggerLastOffset[MAX_TRIGGER_LAST];
extern long lTimeLast[MAX_TICK_MARK];    
extern long lTimeLastOffset[MAX_TICK_MARK];
extern int g_iTimeCtr;
extern int g_iZoomLevel;

// an array of x/y/z for screensaver moving
//GLfloat jiggle[3] = {0., 0., 0.};

extern double dtw[2]; // time window

extern bool mouse_down;
extern int mouseX, mouseY;
extern int mouseSX, mouseSY;

extern double pitch_angle[4]; 
extern double roll_angle[4]; 
extern double viewpoint_distance[4];

extern float g_fmax[4], g_fmin[4];

//extern float color[4] = {.7, .2, .5, 1};

extern TEXTURE_DESC logo;  // customized version of the boinc/api/gutil.h TEXTURE_DESC
extern RIBBON_GRAPH rgx, rgy, rgz, rgs; // override the standard boinc/api ribbon graph draw as it's making the earth red!

#ifndef QCNLIVE
extern bool bFirstShown;             // flags that the view hasn't been shown yet
#endif
extern bool bScaled;             // scaled is usually for 3D pics, but can also be done on 2D in the QCNLIVE
extern char* g_strFile;           // optional file of shared memory serialization
extern bool bResetArray;          // reset our plot memory array, otherwise it will just try to push a "live" point onto the array
extern float aryg[4][PLOT_ARRAY_SIZE];   // the data points for plotting -- DS DX DY DZ
extern float g_fAvg[4];   // running avg for plot scale
	
// current view is an enum i.e. { VIEW_PLOT_3D = 1, VIEW_PLOT_2D, VIEW_EARTH_DAY, VIEW_EARTH_NIGHT, VIEW_EARTH_COMBINED, VIEW_CUBE }; 
extern char g_strJPG[_MAX_PATH];
extern int g_iJPG;

// end of namespace variable declarations

// forward declarations for functions
extern void getProjectPrefs();
extern const char* ScreenshotJPG();
extern void ResetPlotArray();
extern const int   GetTimeWindowWidth();
extern const int   SetTimeWindowWidth(bool bUp = true);
extern const long  TimeWindowBack();
extern const long  TimeWindowStop();
extern const long  TimeWindowStart();
extern const long  TimeWindowForward();
extern const bool TimeWindowIsStopped();
extern void SetScaled(bool scaleit);
extern const bool IsScaled();

extern int graphics_main(int argc, char** argv);

int getLastTrigger(const long lTriggerCheck, const int iWinSizeArray, const int iRebin);
void getSharedMemory();
void set_viewpoint(double dist);
void init_camera(double dist = 10.0f, double field = 45.0f);
void init_lights();
void draw_logo();
void draw_text_user();
void draw_text_plot();
void draw_text_sensor();
bool setupPlotMemory(long llOff);
void draw_triggers();
void draw_plots_3d();
void parse_quake_info(char* strQuake, int ctr, e_quake eType);
void parse_project_prefs();
}  // namespace graphics_plot

#endif

