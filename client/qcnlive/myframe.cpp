/*
 *  myframe.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 2/15/08.
 *  Copyright 2008 Stanford University School of Earth Sciences. All rights reserved.
 *
 */

#include "qcnwx.h"
// these are our toolbar icons in C-array-style XPM format, var names prefixed xpm_icon_
//#include "icons.h"   // 64x64
#include "icons32.h"   // 32x32
#include "dlgsettings.h"
#include "qcn_earth.h"
#include "qcn_2dplot.h"

const int ID_TOOL_ACTION_CAMERA = 999;
const int ID_TOOLBAR = 998;
const int ID_TOOLBAR_2DPLOT = 997;

//#define QCN_TOOLBAR_IMG(mytbimg) wxBitmap(mytbimg, wxBITMAP_TYPE_XPM, 32, 32)
#define QCN_TOOLBAR_IMG(mytbimg) wxBitmap(mytbimg, wxBITMAP_TYPE_XPM)

enum e_menu_file {
   wxID_FILE_SETTINGS = 101
};

enum e_tool_view { 
    ID_TOOL_VIEW_EARTH = 1000,
    ID_TOOL_VIEW_SENSOR_2D,
    ID_TOOL_VIEW_SENSOR_3D,
    ID_TOOL_VIEW_CUBE
};
const int ciNumView = 4;  // the number of tools
	
enum e_tool_action_earth { 
    ID_TOOL_ACTION_EARTH_DAY = 1010,
    ID_TOOL_ACTION_EARTH_NIGHT,
    ID_TOOL_ACTION_EARTH_ROTATE_ON,
    ID_TOOL_ACTION_EARTH_ROTATE_OFF,	
    ID_TOOL_ACTION_EARTH_USGS,
    ID_TOOL_ACTION_EARTH_LATEST
};

enum e_tool_action_sensor { 
    ID_TOOL_ACTION_SENSOR_01 = 1020,
    ID_TOOL_ACTION_SENSOR_10,
    ID_TOOL_ACTION_SENSOR_60,
    ID_TOOL_ACTION_SENSOR_BACK,
    ID_TOOL_ACTION_SENSOR_PAUSE,
    ID_TOOL_ACTION_SENSOR_RESUME,
    ID_TOOL_ACTION_SENSOR_RECORD,
    ID_TOOL_ACTION_SENSOR_FORWARD,
    ID_TOOL_ACTION_SENSOR_ABSOLUTE,
    ID_TOOL_ACTION_SENSOR_SCALED,
	ID_TOOL_ACTION_SENSOR_SCROLLBAR,
	ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_OUT,
	ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_IN,
	ID_TOOL_ACTION_SENSOR_VERT_ZOOM_OUT,
	ID_TOOL_ACTION_SENSOR_VERT_ZOOM_IN,
	ID_TOOL_ACTION_SENSOR_VERT_ZOOM_AUTO
};

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_SIZE(MyFrame::OnSize)
    EVT_CLOSE(MyFrame::OnCloseWindow)

    EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
    EVT_MENU(wxID_FILE_SETTINGS, MyFrame::OnFileSettings)
    //EVT_MENU(wxID_HELP, MyFrame::OnAbout)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)

	EVT_MENU(ID_TOOL_VIEW_EARTH, MyFrame::OnActionView)
	EVT_MENU(ID_TOOL_VIEW_SENSOR_2D, MyFrame::OnActionView)
	EVT_MENU(ID_TOOL_VIEW_SENSOR_3D, MyFrame::OnActionView)
	EVT_MENU(ID_TOOL_VIEW_CUBE, MyFrame::OnActionView)
	
	EVT_MENU(ID_TOOL_ACTION_EARTH_DAY, MyFrame::OnActionEarth)
	EVT_MENU(ID_TOOL_ACTION_EARTH_NIGHT, MyFrame::OnActionEarth)
	EVT_MENU(ID_TOOL_ACTION_EARTH_ROTATE_ON, MyFrame::OnActionEarth)
	EVT_MENU(ID_TOOL_ACTION_EARTH_ROTATE_OFF, MyFrame::OnActionEarth)
	EVT_MENU(ID_TOOL_ACTION_EARTH_USGS, MyFrame::OnActionEarth)
	EVT_MENU(ID_TOOL_ACTION_EARTH_LATEST, MyFrame::OnActionEarth)

	EVT_MENU(ID_TOOL_ACTION_SENSOR_01, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_10, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_60, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_BACK, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_PAUSE, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_RESUME, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_RECORD, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_FORWARD, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_ABSOLUTE, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_SCALED, MyFrame::OnActionSensor)

	EVT_MENU(ID_TOOL_ACTION_SENSOR_SCROLLBAR, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_OUT, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_IN, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_OUT, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_IN, MyFrame::OnActionSensor)
	EVT_MENU(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_AUTO, MyFrame::OnActionSensor)

/*
	EVT_MENU(ID_TOOL_ACTION_2D_01, MyFrame::OnActionPlot)
	EVT_MENU(ID_TOOL_ACTION_2D_10, MyFrame::OnActionPlot)
	EVT_MENU(ID_TOOL_ACTION_2D_60, MyFrame::OnActionPlot)

	EVT_MENU(ID_TOOL_ACTION_3D_01, MyFrame::OnActionPlot)
	EVT_MENU(ID_TOOL_ACTION_3D_10, MyFrame::OnActionPlot)
	EVT_MENU(ID_TOOL_ACTION_3D_60, MyFrame::OnActionPlot)
*/

	EVT_MENU(ID_TOOL_ACTION_CAMERA, MyFrame::OnScreenshot)
	
END_EVENT_TABLE()


MyFrame::MyFrame(const wxRect& rect, MyApp* papp) 
 : wxFrame(NULL, -1,  wxT("QCNLive"), rect.GetPosition(), rect.GetSize()), pMyApp(papp), glPane(NULL), toolBar(NULL), scrollBar2D(NULL)
{

    statusBar = CreateStatusBar();
	m_view = ID_TOOL_VIEW_EARTH;  // set view to 0
	m_ptbBase = NULL; // no toolbar base yet

    bEarthDay = true;
    bEarthRotate = true;
    iSensorAction = 0;
	
    bSensorAbsolute2D = false;
    bSensorAbsolute3D = false;

    // Make a menubar
    menuFile = new wxMenu;
    menuFile->Append(wxID_FILE_SETTINGS, wxString("&Local Settings", wxConvUTF8), 
        wxString("Enter local settings such as station name, latutide, longitude, elevation", wxConvUTF8));
    menuFile->Append(wxID_EXIT, wxString("E&xit", wxConvUTF8), wxString("Quit QCNLive", wxConvUTF8));

    menuHelp = new wxMenu;
//#ifndef __WXMAC__  // Mac's have a default about box btn
    menuHelp->Append(wxID_ABOUT, wxString("&About", wxConvUTF8), wxString("About QCNLive", wxConvUTF8));
//#endif

    menuView = new wxMenu;
    menuOptions = new wxMenu;
    menuBar = new wxMenuBar;
	
    menuBar->Append(menuFile, wxString("&File", wxConvUTF8));
    menuBar->Append(menuView, wxString("&View", wxConvUTF8));
    menuBar->Append(menuOptions, wxString("&Options", wxConvUTF8));
    menuBar->Append(menuHelp, wxString("&Help", wxConvUTF8));

    // Associate the menu bar with the frame
    SetMenuBar(menuBar);

}

void MyFrame::SetupToolbars()
{
	toolBar = CreateToolBar(wxNO_BORDER|wxHORIZONTAL, ID_TOOLBAR);

	if (toolBar) {
		toolBar->SetToolBitmapSize(wxSize(32,32));

		ToolBarView();
		ToolBarEarth(true);
		
		//scrollBar2D = new wxScrollBar(toolBar, ID_TOOL_ACTION_SENSOR_SCROLLBAR, wxDefaultPosition, wxSize(100,10), wxSB_HORIZONTAL, wxDefaultValidator, "Time Scroll");
		if (scrollBar2D) scrollBar2D->Hide();
	}

/*
	if (glPane) {
		toolBar2DPlot = new wxToolBar((wxWindow*) glPane, ID_TOOLBAR_2DPLOT, wxDefaultPosition, wxDefaultSize, 
			wxTB_HORIZONTAL | wxTB_RIGHT | wxTB_BOTTOM, 
			//wxTB_BOTTOM | wxTB_RIGHT | wxTB_HORIZONTAL | wxNO_BORDER, 
			wxT("2D Plot Controls"));
		scrollBar = new wxScrollBar(toolBar2DPlot, ID_TOOL_ACTION_SENSOR_SCROLLBAR);
		toolBar2DPlot->AddControl(scrollBar);
		toolBar2DPlot->Realize();
		toolBar2DPlot->Show(true);
	}
*/
}

void MyFrame::OnCloseWindow(wxCloseEvent& wxc)
{
     if (pMyApp) { // save the current window position & size, in MyApp::OnExit these get written to a prefs xml file
        //pMyApp->SetRect(GetSize(), GetPosition());
        //pMyApp->SetRect(GetRect());   // GetScreenRect()
        pMyApp->SetRect(GetScreenRect());   // GetScreenRect()
      }
     // stop timers and get rid of OpenGL window which causes a hang on Windows
     if (glPane) {
         if (glPane->m_ptimer) {
            glPane->m_ptimer->Stop();
         }
         glPane->Destroy();
     }
	 wxFrame::OnCloseWindow(wxc);
}

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
		   pMyApp->set_qcnlive_prefs(); // saved in KillMainThread 
		   // probably have to kill & restart the main thread too?
			if (pMyApp && myOldSensor != sm->iMySensor) {  // we changed sensors, have to restart main thread?
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

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(evt))
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(evt))
{
	wxAboutDialogInfo myAboutBox;
	//myAboutBox.SetIcon(wxIcon("qcnwin.ico", wxBITMAP_TYPE_ICO));
	myAboutBox.SetVersion(wxString(QCN_VERSION_STRING));
	myAboutBox.SetName(wxT("QCNLive"));
	myAboutBox.SetWebSite(wxT("http://qcn.stanford.edu"), wxT("Quake-Catcher Network Website"));
	myAboutBox.SetCopyright(wxT("(c) 2009 Stanford University")); 
	//myAboutBox.AddDeveloper(wxT("Carl Christensen  (carlgt1@yahoo.com"));
	myAboutBox.SetDescription(wxT("This software is provided free of charge for educational purposes.\n\nPlease visit us on the web:\n"));

    wxAboutBox(myAboutBox);
}

void MyFrame::OnSize(wxSizeEvent& evt)
{
	 if (glPane) glPane->resized(evt);
	 wxFrame::OnSize(evt);
}

void MyFrame::OnActionView(wxCommandEvent& evt)
{
   // get item from event do appropriate action (boinc_key_press!)
   // todo: hook up the other toolbars
  Toggle(m_view, false, true);
  bool bChanged = false;
  switch(evt.GetId())
  {
     case ID_TOOL_VIEW_EARTH:
	     if (bEarthDay) 
                qcn_graphics::earth.SetMapCombined();
	     else 
                qcn_graphics::earth.SetMapNight();
         m_view = evt.GetId();
	     ToolBarEarth();
	     bChanged = true;
	     break;
	 case ID_TOOL_VIEW_SENSOR_2D:
	     qcn_graphics::g_eView = VIEW_PLOT_2D;
		 // note only redraw sensor toolbar if not coming from a sensor view already
         //if (m_view != ID_TOOL_VIEW_SENSOR_2D && m_view != ID_TOOL_VIEW_SENSOR_3D) ToolBarSensor(evt.GetId());
         m_view = evt.GetId();
         ToolBarSensor2D();
         bChanged = true;
		 break;
	 case ID_TOOL_VIEW_SENSOR_3D:
	     qcn_graphics::g_eView = VIEW_PLOT_3D;
		 // note only redraw sensor toolbar if not coming from a sensor view already
         //if (m_view != ID_TOOL_VIEW_SENSOR_2D && m_view != ID_TOOL_VIEW_SENSOR_3D) ToolBarSensor(evt.GetId());
         m_view = evt.GetId();
         ToolBarSensor3D();
         bChanged = true;
		 break;
	 case ID_TOOL_VIEW_CUBE:
	     qcn_graphics::g_eView = VIEW_CUBE;
         m_view = evt.GetId();
		 ToolBarCube();
		 bChanged = true;
		 break;
	}
    if (bChanged) {
      Toggle(m_view, true, true);
    }
    qcn_graphics::ResetPlotArray();
}
	
void MyFrame::OnActionEarth(wxCommandEvent& evt)
{
   // get item from event do appropriate action (boinc_key_press!)
  switch(evt.GetId())
  {
     case ID_TOOL_ACTION_EARTH_DAY:
	     bEarthDay = true;
	     qcn_graphics::earth.SetMapCombined(); 
		 break;
	 case ID_TOOL_ACTION_EARTH_NIGHT:
	     bEarthDay = false;
	     qcn_graphics::earth.SetMapNight(); 
		 break;
     case ID_TOOL_ACTION_EARTH_ROTATE_ON:
	     EarthRotate(true);
		 break;
     case ID_TOOL_ACTION_EARTH_ROTATE_OFF:
	     EarthRotate(false);
		 break;
	case ID_TOOL_ACTION_EARTH_USGS:
	     statusBar->SetStatusText(wxString("Opening USGS website for selected earthquake", wxConvUTF8));
	     qcn_graphics::earth.checkURLClick(true);
	     break;
	case ID_TOOL_ACTION_EARTH_LATEST:
	     if (pMyApp) pMyApp->GetLatestQuakeList();
	     break;
  }
  SetToggleEarth();
}

void MyFrame::OnActionSensor(wxCommandEvent& evt)
{
   // get item from event do appropriate action (boinc_key_press!)
  static int current = evt.GetId();
  switch(evt.GetId())
  {
     case ID_TOOL_ACTION_SENSOR_BACK:
         if (! qcn_graphics::TimeWindowIsStopped()) {
		    iSensorAction = 1;
	        qcn_graphics::TimeWindowStop(); 
		 }
		 qcn_graphics::TimeWindowBack(); 
		 break;
     case ID_TOOL_ACTION_SENSOR_PAUSE:
         if (! qcn_graphics::TimeWindowIsStopped()) {
		    iSensorAction = 1;
	        qcn_graphics::TimeWindowStop(); 
		 }
		 break;
     case ID_TOOL_ACTION_SENSOR_RESUME:
         if (qcn_graphics::TimeWindowIsStopped()) {
            iSensorAction = 0;
	        qcn_graphics::TimeWindowStart();
		 }
		 break;
     case ID_TOOL_ACTION_SENSOR_RECORD:
         if (qcn_graphics::TimeWindowIsStopped()) {
            iSensorAction = 0;
	        qcn_graphics::TimeWindowStart();
		 }
		  if (sm->bSensorFound) {
			  if (sm->bRecording) { // we're turning off recording
				  SetStatusText(wxString("Recording stopped", wxConvUTF8));
			  }
			  else { // we're starting recording
				  SetStatusText(wxString("Recording...", wxConvUTF8));
			  }
			
			 // flip the state
			 sm->bRecording = !sm->bRecording;
		  }
	 break;
		  
     case ID_TOOL_ACTION_SENSOR_FORWARD:
         if (! qcn_graphics::TimeWindowIsStopped()) {
		    iSensorAction = 1;
	        qcn_graphics::TimeWindowStop(); 
		 }
		 qcn_graphics::TimeWindowForward(); 
		 break;
     case ID_TOOL_ACTION_SENSOR_ABSOLUTE:
	     if (m_view == ID_TOOL_VIEW_SENSOR_2D) {
		     bSensorAbsolute2D = true;
		 }
		 else bSensorAbsolute3D = false;
         break;	 
     case ID_TOOL_ACTION_SENSOR_SCALED:
	     if (m_view == ID_TOOL_VIEW_SENSOR_2D)
		     bSensorAbsolute2D = false;
		 else
		     bSensorAbsolute3D = false;
         break;	 
	 case ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_OUT:
		 qcn_graphics::SetTimeWindowWidth(true);
		 break;
	 case ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_IN:
		 qcn_graphics::SetTimeWindowWidth(false);
		 break;
	 case ID_TOOL_ACTION_SENSOR_VERT_ZOOM_OUT:
		 qcn_2dplot::SensorDataZoomOut();
		 break;
	 case ID_TOOL_ACTION_SENSOR_VERT_ZOOM_IN:
		 qcn_2dplot::SensorDataZoomIn();
		 break;
	 case ID_TOOL_ACTION_SENSOR_VERT_ZOOM_AUTO:
		 qcn_2dplot::SensorDataZoomAuto();
		 break;
    }
    current = evt.GetId();
    SetToggleSensor();
}
	  
void MyFrame::OnScreenshot(wxCommandEvent& WXUNUSED(evt))
{
   const char* strSS = qcn_graphics::ScreenshotJPG();
   if (strSS && strSS[0] != 0x00) {
      // we have a valid screenshot filename
         char* statmsg = new char[_MAX_PATH];
		 sprintf(statmsg, "Screenshot file saved to %s", strSS);
	     SetStatusText(wxString(statmsg, wxConvUTF8));
		 delete [] statmsg;
   }
}


void MyFrame::ToolBarView()
{
    if (!toolBar) return; // null toolbar?
    wxString wxsShort[4], wxsLong[4];
    wxsShort[0].assign("&Earthquakes");
    wxsLong[0].assign("Select this view to see the latest earthquakes worldwide");
    wxsShort[1].assign("Sensor &2-dimensional");
    wxsLong[1].assign("Select this view to see your accelerometer output as a 2-dimensional plot");
    wxsShort[2].assign("Sensor &3-dimensional");
    wxsLong[2].assign("Select this to see your accelerometer output as a 3-dimensional plot");
    wxsShort[3].assign("&Cube");
    wxsLong[3].assign("Select this view to see a bouncing cube that responds to your accelerometer");

#ifndef wxUSE_LIBPNG	
    fprintf(stdout, "Error -- you need wxWidgets with PNG support!\n");
	return;
#endif

	wxToolBarToolBase* toolBarView = toolBar->AddRadioTool(ID_TOOL_VIEW_EARTH, 
	   wxsShort[0], 
	   QCN_TOOLBAR_IMG(xpm_icon_earth),
	   wxNullBitmap, wxsLong[0], wxsLong[0]
	);
	
	if (toolBarView) {
	    // add the rest of the view buttons
        toolBar->AddRadioTool(ID_TOOL_VIEW_SENSOR_2D, 
	      wxsShort[1], 
      	   QCN_TOOLBAR_IMG(xpm_icon_twod),
	       wxNullBitmap, 
           wxsLong[1], 
           wxsLong[1]
	    );
	
        toolBar->AddRadioTool(ID_TOOL_VIEW_SENSOR_3D, 
	      wxsShort[2], 
      	   QCN_TOOLBAR_IMG(xpm_icon_threed),
	       wxNullBitmap,
	       wxsLong[2],
	       wxsLong[2]
	    );
	
        toolBar->AddRadioTool(ID_TOOL_VIEW_CUBE, 
	      wxsShort[3], 
      	   QCN_TOOLBAR_IMG(xpm_icon_cube),
	       wxNullBitmap, wxsLong[3], wxsLong[3]
        );
		
	}
	
	toolBar->AddSeparator();

    // now add the menuView
    for (int i = 0; i < ciNumView; i++) 
       menuView->AppendCheckItem(ID_TOOL_VIEW_EARTH+i, wxsShort[i], wxsLong[i]);
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

    if (bFirst) Toggle(ID_TOOL_VIEW_EARTH, true, true);
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
	
	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_EARTH_DAY, 
	   wxsShort[0], 
	   QCN_TOOLBAR_IMG(xpm_icon_sun),
	   wxNullBitmap,
	   wxsShort[0], wxsLong[0]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_EARTH_DAY, wxsShort[0], wxsLong[0]);

	toolBar->AddRadioTool(ID_TOOL_ACTION_EARTH_NIGHT, 
       wxsShort[1],
	   QCN_TOOLBAR_IMG(xpm_icon_moon),
	   wxNullBitmap, wxsShort[1], wxsLong[1]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_EARTH_NIGHT, wxsShort[1], wxsLong[1]);
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();
	
	// stop/start rotation
	toolBar->AddRadioTool(ID_TOOL_ACTION_EARTH_ROTATE_ON, 
	   wxsShort[2], 
	   QCN_TOOLBAR_IMG(xpm_icon_spin),
	   wxNullBitmap,
	   wxsShort[2], wxsLong[2]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_EARTH_ROTATE_ON, wxsShort[2], wxsLong[2]);

	toolBar->AddRadioTool(ID_TOOL_ACTION_EARTH_ROTATE_OFF, 
	   wxsShort[3],
	   QCN_TOOLBAR_IMG(xpm_icon_nospin),
	   wxNullBitmap,
	   wxsShort[3],
	   wxsLong[3]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_EARTH_ROTATE_OFF, wxsShort[3], wxsLong[3]);
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();

	toolBar->AddTool(ID_TOOL_ACTION_EARTH_LATEST, 
	   wxsShort[4],
	   QCN_TOOLBAR_IMG(xpm_icon_quakelist),
	   wxNullBitmap,
	   wxITEM_NORMAL,
       wxsShort[4], wxsLong[4]
	);
    menuOptions->Append(ID_TOOL_ACTION_EARTH_LATEST, wxsShort[4], wxsLong[4]);

	toolBar->AddTool(ID_TOOL_ACTION_EARTH_USGS, 
	   wxsShort[5], 
	   QCN_TOOLBAR_IMG(xpm_icon_usgs),
	   wxNullBitmap,
	   wxITEM_NORMAL, wxsShort[5], wxsLong[5]
	);
    menuOptions->Append(ID_TOOL_ACTION_EARTH_USGS, wxsShort[5], wxsLong[5]);

    AddScreenshotItem();

    SetToggleEarth();
	
	toolBar->Realize();
}

// toggle on off both the menu & the toolbar
void MyFrame::Toggle(const int id, const bool bOn, const bool bView)
{
      toolBar->ToggleTool(id, bOn);
      if (bView)  // use menuView
          menuView->Check(id, bOn);
      else
          menuOptions->Check(id, bOn);
}

void MyFrame::SetToggleSensor()
{

      if (m_view == ID_TOOL_VIEW_SENSOR_2D) {
		  Toggle(ID_TOOL_ACTION_SENSOR_RECORD, (bool)(sm->bRecording));

          toolBar->EnableTool(ID_TOOL_ACTION_SENSOR_ABSOLUTE, true);
          menuOptions->Enable(ID_TOOL_ACTION_SENSOR_ABSOLUTE, true);
	      if (bSensorAbsolute2D) {
             if (qcn_graphics::IsScaled()) qcn_graphics::SetScaled(false);
 		  }
		  else {
             if (!qcn_graphics::IsScaled()) qcn_graphics::SetScaled(true);
		  }		  
         Toggle(ID_TOOL_ACTION_SENSOR_ABSOLUTE, bSensorAbsolute2D);
         Toggle(ID_TOOL_ACTION_SENSOR_SCALED, !bSensorAbsolute2D);

		 Toggle(ID_TOOL_ACTION_SENSOR_RESUME, (bool)(sm->bRecording && iSensorAction == 0));
		 Toggle(ID_TOOL_ACTION_SENSOR_PAUSE, (bool)(sm->bRecording && iSensorAction == 1));
	  }
	  else if (m_view == ID_TOOL_VIEW_SENSOR_3D) { // force to be scaled
	      bSensorAbsolute3D = false;
          if (!qcn_graphics::IsScaled()) qcn_graphics::SetScaled(true);
          toolBar->EnableTool(ID_TOOL_ACTION_SENSOR_ABSOLUTE, false);
          menuOptions->Enable(ID_TOOL_ACTION_SENSOR_ABSOLUTE, false);
          Toggle(ID_TOOL_ACTION_SENSOR_ABSOLUTE, bSensorAbsolute3D);
          Toggle(ID_TOOL_ACTION_SENSOR_SCALED, !bSensorAbsolute3D);
			//Toggle(ID_TOOL_ACTION_SENSOR_01, (bool)(iSensorTimeWindow == 60));
			//Toggle(ID_TOOL_ACTION_SENSOR_10, (bool)(iSensorTimeWindow == 600));
			//Toggle(ID_TOOL_ACTION_SENSOR_60, (bool)(iSensorTimeWindow == 3600));
	  }
}

void MyFrame::SetToggleEarth()
{
      Toggle(ID_TOOL_ACTION_EARTH_DAY, bEarthDay);
      Toggle(ID_TOOL_ACTION_EARTH_NIGHT, !bEarthDay);
      Toggle(ID_TOOL_ACTION_EARTH_ROTATE_OFF, !bEarthRotate);
      Toggle(ID_TOOL_ACTION_EARTH_ROTATE_ON, bEarthRotate);
}

void MyFrame::ToggleStartStop(bool bStart)
{
	Toggle(ID_TOOL_ACTION_SENSOR_RESUME, bStart);
	Toggle(ID_TOOL_ACTION_SENSOR_PAUSE, !bStart);
	if (bStart && qcn_graphics::TimeWindowIsStopped()) {
		qcn_graphics::TimeWindowStart();
	}
	else if (!bStart && !qcn_graphics::TimeWindowIsStopped()) {
		qcn_graphics::TimeWindowStop();
	}
}

void MyFrame::SensorNavButtons()
{
    wxString wxsShort[7], wxsLong[7];
	
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

    wxsShort[5].assign("Set Recording State");
    wxsLong[5].assign("Set Recording State On or Off");

    wxsShort[6].assign("Move Forward");
    wxsLong[6].assign("Move Forward In Time");

	m_ptbBase = toolBar->AddTool(ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_IN, 
	   wxsShort[0], 
	   QCN_TOOLBAR_IMG(xpm_horiz_zoom_in),
	   wxNullBitmap, wxITEM_NORMAL,
	   wxsShort[0], 
	   wxsLong[0]
	);
    menuOptions->Append(ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_IN, wxsShort[0], wxsLong[0]);

	m_ptbBase = toolBar->AddTool(ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_OUT, 
	   wxsShort[1], 
	   QCN_TOOLBAR_IMG(xpm_horiz_zoom_out),
	   wxNullBitmap, wxITEM_NORMAL,
	   wxsShort[1], 
	   wxsShort[1]
	);
    menuOptions->Append(ID_TOOL_ACTION_SENSOR_HORIZ_ZOOM_OUT, wxsShort[1], wxsShort[1]);
	
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();

	m_ptbBase = toolBar->AddTool(ID_TOOL_ACTION_SENSOR_BACK, 
	   wxsShort[2], 
	   QCN_TOOLBAR_IMG(xpm_icon_rw),
	   wxNullBitmap, wxITEM_NORMAL,
	   wxsShort[2],
	   wxsLong[2]
	);
    menuOptions->Append(ID_TOOL_ACTION_SENSOR_BACK, wxsShort[2], wxsLong[2]);

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_PAUSE, 
       wxsShort[3],
	   QCN_TOOLBAR_IMG(xpm_icon_pause),
	   wxNullBitmap, wxsShort[3], wxsLong[3]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_PAUSE, wxsShort[3], wxsLong[3]);

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_RESUME, 
	   wxsShort[4], 
	   QCN_TOOLBAR_IMG(xpm_icon_play),
	   wxNullBitmap, wxsShort[4], wxsLong[4]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_RESUME, wxsShort[4], wxsLong[4]);

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_RECORD, 
	   wxsShort[5], 
	   QCN_TOOLBAR_IMG(xpm_icon_record),
	   wxNullBitmap, wxsShort[5], wxsLong[5]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_RECORD, wxsShort[5], wxsLong[5]);

	m_ptbBase = toolBar->AddTool(ID_TOOL_ACTION_SENSOR_FORWARD, 
	   wxsShort[6], 
	   QCN_TOOLBAR_IMG(xpm_icon_ff),
	   wxNullBitmap, wxITEM_NORMAL,
       wxsShort[6], wxsLong[6]
	);
    menuOptions->Append(ID_TOOL_ACTION_SENSOR_FORWARD, wxsShort[6], wxsLong[6]);

}

void MyFrame::AddScreenshotItem()
{
	toolBar->AddSeparator();
    menuOptions->AppendSeparator();

    toolBar->AddTool(ID_TOOL_ACTION_CAMERA, 
	   wxString("Screenshot", wxConvUTF8), 
	   QCN_TOOLBAR_IMG(xpm_icon_camera),
	   wxNullBitmap,
	   wxITEM_NORMAL,
	   wxString("Make a screenshot", wxConvUTF8),
	   wxString("Make a screenshot (saved in the 'sac' data folder)", wxConvUTF8)
	);
	menuOptions->Append(ID_TOOL_ACTION_CAMERA, wxString("&Screenshot", wxConvUTF8),
        wxString("Make a screenshot (saved in the 'sac' data folder)", wxConvUTF8));
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

	m_ptbBase = toolBar->AddTool(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_AUTO, 
	   wxsShort[0], 
	   QCN_TOOLBAR_IMG(xpm_zoom_auto),
	   wxNullBitmap, wxITEM_NORMAL,
	   wxsShort[0], 
	   wxsShort[0]
	);
    menuOptions->Append(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_AUTO, wxsShort[0], wxsShort[0]);

	m_ptbBase = toolBar->AddTool(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_IN, 
	   wxsShort[1], 
	   QCN_TOOLBAR_IMG(xpm_vert_zoom_in),
	   wxNullBitmap, wxITEM_NORMAL,
	   wxsShort[1], 
	   wxsShort[1]
	);
    menuOptions->Append(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_IN, wxsShort[1], wxsShort[1]);

	m_ptbBase = toolBar->AddTool(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_OUT, 
	   wxsShort[2], 
	   QCN_TOOLBAR_IMG(xpm_vert_zoom_out),
	   wxNullBitmap, wxITEM_NORMAL,
	   wxsShort[2], 
	   wxsShort[2]
	);
    menuOptions->Append(ID_TOOL_ACTION_SENSOR_VERT_ZOOM_OUT, wxsShort[2], wxsShort[2]);

    // scrollbar for back & forth time
	if (scrollBar2D) {
	   toolBar->AddSeparator();
       menuOptions->AppendSeparator();
	   toolBar->AddControl(scrollBar2D);
	}
	
    SensorNavButtons();

	toolBar->AddSeparator();
    menuOptions->AppendSeparator();

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_ABSOLUTE, 
	   wxsShort[5],
	   QCN_TOOLBAR_IMG(xpm_icon_absolute),
	   wxNullBitmap,
	   wxsShort[5], wxsShort[5]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_ABSOLUTE, wxsShort[5], wxsShort[5]);

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_SCALED, 
	   wxsShort[6],
	   QCN_TOOLBAR_IMG(xpm_icon_scaled),
	   wxNullBitmap,
	   wxsShort[6], wxsShort[6]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_SCALED, wxsShort[6], wxsShort[6]);

    AddScreenshotItem();
		
	toolBar->Realize();

    SetToggleSensor();  // put this after realize() because we may enable/disable tools
}

void MyFrame::ToolBarSensor3D()
{
    if (!toolBar) return; // null toolbar?
    RemoveCurrentTools();

    wxString wxsShort[10], wxsLong[10];

    wxsShort[0].assign("&1-minute Window");
    wxsLong[0].assign("1-minute Time Window");

    wxsShort[1].assign("1&0-minute Window");
    wxsLong[1].assign("10-minute Time Window");

    wxsShort[2].assign("&60-minute Window");
    wxsLong[2].assign("60-minute Time Window");

//    wxsShort[3].assign("&Previous Time Window");
//    wxsLong[3].assign("Previous Time Window");

//    wxsShort[4].assign("&Pause Sensor Display");
//    wxsLong[4].assign("Pause Sensor Display");

    wxsShort[3].assign("Zoom In Time Scale");
    wxsShort[4].assign("Zoom Out Time Scale");

    wxsShort[5].assign("&Current Sensor Display");
    wxsLong[5].assign("Current Sensor Display");

    wxsShort[6].assign("&Next Time Window");
	wxsLong[6].assign("Go forwards one 'window' of time");

    wxsShort[7].assign("&Absolute sensor values");
    wxsLong[7].assign("View the absolute sensor values");

    wxsShort[8].assign("S&caled sensor values");
    wxsLong[8].assign("View the scaled sensor values");

    wxsShort[9].assign("&Record sensor output");
    wxsLong[9].assign("Set recording state of the sensor output");

	//toolBar->AddSeparator();

/*
	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_01, 
	   wxsShort[0], 
	   QCN_TOOLBAR_IMG(xpm_icon_one),
	   wxNullBitmap,
	   wxsShort[0], 
	   wxsLong[0]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_01, wxsShort[0], wxsLong[0]);

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_10, 
	   wxsShort[1], 
	   QCN_TOOLBAR_IMG(xpm_icon_ten),
	   wxNullBitmap,
	   wxsShort[1], 
	   wxsLong[1]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_10, wxsShort[1], wxsLong[1]);
	
	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_60, 
	   wxsShort[2], 
	   QCN_TOOLBAR_IMG(xpm_icon_sixty),
	   wxNullBitmap,
	   wxsShort[2], 
	   wxsLong[2]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_60, wxsShort[2], wxsLong[2]);
*/

	SensorNavButtons();
	

	toolBar->AddSeparator();
    menuOptions->AppendSeparator();

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_ABSOLUTE, 
	   wxsShort[7],
	   QCN_TOOLBAR_IMG(xpm_icon_absolute),
	   wxNullBitmap,
	   wxsShort[7], wxsLong[7]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_ABSOLUTE, wxsShort[7], wxsLong[7]);

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_SCALED, 
	   wxsShort[8],
	   QCN_TOOLBAR_IMG(xpm_icon_scaled),
	   wxNullBitmap,
	   wxsShort[8], wxsLong[8]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_SCALED, wxsShort[8], wxsLong[8]);

    AddScreenshotItem();
		
	toolBar->Realize();

//	m_view = iView;
    SetToggleSensor();  // put this after realize() because we may enable/disable tools
}

void MyFrame::ToolBarCube()
{
    if (!toolBar) return; // null toolbar?
    RemoveCurrentTools();
	
    AddScreenshotItem();

	toolBar->Realize();
}

void MyFrame::EarthRotate(bool bAuto)
{
   if (!qcn_graphics::earth.IsShown()) return;  // only matters if we're on the earth view!
   // see if it's rotating and we want to stop, or it's not rotating and we want it to start
   if ( (!bAuto && qcn_graphics::earth.IsAutoRotate())
	|| (bAuto && ! qcn_graphics::earth.IsAutoRotate()))  {
      bEarthRotate = bAuto;
      Toggle(ID_TOOL_ACTION_EARTH_ROTATE_OFF, !bAuto);
      Toggle(ID_TOOL_ACTION_EARTH_ROTATE_ON, bAuto);
      qcn_graphics::earth.AutoRotate(bAuto);
   }
}
 

