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

#include "qcn_earth.h"
#include "qcn_cube.h"

#ifdef QCNLIVE
   #include "main.h"
#endif

using std::string;
using std::vector;

extern CQCNShMem* volatile sm;                    // the main shared memory pointer

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


// forward declarations for functions
extern void getProjectPrefs();
extern const char* ScreenshotJPG();
extern void ResetPlotArray();
extern const long  TimeWindowWidth(int minutes);
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
bool setupPlotMemory(long llOff);
void draw_triggers();
void draw_plots_2d();
void draw_plots_3d();
void parse_quake_info(char* strQuake, int ctr, e_quake eType);
void parse_project_prefs();
}  // namespace graphics_plot

#endif

