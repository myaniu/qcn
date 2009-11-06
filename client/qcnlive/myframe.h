#ifndef _MYFRAME_H_
#define _MYFRAME_H_

#include "qcnwx.h"

class MyGLPane;
class MyApp;

class MyFrame : public wxFrame
{
   public:
    MyFrame(const wxRect& rect, MyApp* papp);

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

    MyApp* pMyApp;
    MyGLPane* glPane;
    wxBoxSizer* sizer;
    wxToolBar* toolBar;
    wxScrollBar* scrollBar2D;
    //wxButtonBar* toolBar;
    wxStatusBar* statusBar;

  private:

        wxMenu *menuFile;
        wxMenu *menuView;
        wxMenu *menuOptions;
        wxMenu *menuHelp;
        wxMenuBar* menuBar;
 
    bool bEarthDay;
    bool bEarthRotate;
    long iSensorAction;
        bool bSensorAbsolute2D;
        bool bSensorAbsolute3D;

        long m_view;  // holds the current enum ID above
        wxToolBarToolBase* m_ptbBase;

    void OnCloseWindow(wxCloseEvent& wxc);
    void OnSize(wxSizeEvent& evt);

    // menu events
    void OnAbout(wxCommandEvent& evt);
    void OnQuit(wxCommandEvent& evt);
    void OnFileSettings(wxCommandEvent& evt);

    void OnActionView(wxCommandEvent& evt);
    void OnActionEarth(wxCommandEvent& evt);
    void OnActionSensor(wxCommandEvent& evt);
    void OnScreenshot(wxCommandEvent& vet);

    void RemoveCurrentTools();

    DECLARE_EVENT_TABLE()
};

#endif // _MYFRAME_H_
