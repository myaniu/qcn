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
	     if (m_pMyApp) m_pMyApp->GetLatestQuakeList();
	     break;
  }
  SetToggleEarth();
}

void MyFrame::OnActionHelp(wxCommandEvent& evt)
{
	static int current = evt.GetId();
	wxString wxstrURL = "";
	switch(evt.GetId())
	{
		case ID_TOOL_HELP_WEB_QCN:
			wxstrURL = _("http://qcn.stanford.edu");
			break;
		case ID_TOOL_HELP_WEB_QCNLIVE:
			wxstrURL = _("http://qcn.stanford.edu/learning/software.php");
			break;
		case ID_TOOL_HELP_WEB_MANUAL:
			wxstrURL = _("http://qcn.stanford.edu/downloads/QCNLive_User_Manual.pdf");
			break;
		case ID_TOOL_HELP_WEB_EARTHQUAKES:
			wxstrURL = _("http://qcn.stanford.edu/learning/earthquakes.php");
			break;
		case ID_TOOL_HELP_WEB_LESSONS:
			wxstrURL = _("http://qcn.stanford.edu/learning/lessons.php");
			break;
		case ID_TOOL_HELP_WEB_REQUEST_SENSOR:
			wxstrURL = _("http://qcn.stanford.edu/learning/requests.php");
			break;
		case ID_TOOL_HELP_WEB_GLOSSARY:
			wxstrURL = _("http://qcn.stanford.edu/learning/glossary.php");
			break;
	}
	if (!wxstrURL.empty())  qcn_util::launchURL(wxstrURL.c_str());
	current = evt.GetId();
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
     case ID_TOOL_ACTION_SENSOR_RECORD_START:
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
	  case ID_TOOL_ACTION_SENSOR_RECORD_STOP:
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

void MyFrame::OnLogoChange(wxCommandEvent& vet)
{
#ifdef QCNLIVE_DEMO
	qcn_graphics::demo_switch_ad();
#endif
}



void MyFrame::ToolBarView()
{
    if (!toolBar) return; // null toolbar?
 
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
          toolBar->EnableTool(ID_TOOL_ACTION_SENSOR_RECORD_START, !sm->bRecording);
          toolBar->EnableTool(ID_TOOL_ACTION_SENSOR_RECORD_STOP, sm->bRecording);
		  //Toggle(ID_TOOL_ACTION_SENSOR_RECORD_START, sm->bRecording);
		  //Toggle(ID_TOOL_ACTION_SENSOR_RECORD_STOP, !sm->bRecording);

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

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_RECORD_START, 
	   wxsShort[5], 
	   QCN_TOOLBAR_IMG(xpm_icon_record),
	   wxNullBitmap, wxsShort[5], wxsLong[5]
	);
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_RECORD_START, wxsShort[5], wxsLong[5]);

	m_ptbBase = toolBar->AddRadioTool(ID_TOOL_ACTION_SENSOR_RECORD_STOP, 
									  wxsShort[7], 
									  QCN_TOOLBAR_IMG(xpm_icon_stop),
									  wxNullBitmap, wxsShort[7], wxsLong[7]
									  );
    menuOptions->AppendCheckItem(ID_TOOL_ACTION_SENSOR_RECORD_STOP, wxsShort[7], wxsLong[7]);
	
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
	
#ifdef QCNLIVE_DEMO  
	// add a function to cycle through ad images i.e. science museum logos
	menuOptions->Append(ID_TOOL_ACTION_AD, wxString("Next &Logo", wxConvUTF8),
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

 
*/

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
	
	
    m_sliderTime = createSlider(SIGNAL(TimePositionChanged(const double&)),
								SLOT(setTimePosition(const double&)));
	
	m_view = VIEW_EARTH_DAY;  // set view to 0
	m_ptbBase = NULL; // no toolbar base yet
	
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
	connect(m_actionOptionEarthDay, SIGNAL(triggered()), this, SLOT(actionOptionEarthDay()));

	m_actionOptionEarthNight = new QAction(tr("&Night"), this);
	m_actionOptionEarthNight->setToolTip(tr("Show night view of global earthquake map"));
	connect(m_actionOptionEarthNight, SIGNAL(triggered()), this, SLOT(actionOptionEarthNight()));
	
	m_actionOptionEarthRotateOn = new QAction(tr("&Auto-rotate"), this);
	m_actionOptionEarthRotateOn->setToolTip(tr("Auto-rotate the globe"));
	connect(m_actionOptionEarthRotateOn, SIGNAL(triggered()), this, SLOT(actionOptionEarthRotateOn()));
	
	m_actionOptionEarthRotateOff = new QAction(tr("&Stop rotation"), this);
	m_actionOptionEarthRotateOff->setToolTip(tr("Stop rotation of the globe"));
	connect(m_actionOptionEarthRotateOff, SIGNAL(triggered()), this, SLOT(actionOptionEarthRotateOff()));
	
	m_actionOptionEarthQuakelist = new QAction(tr("&Get latest earthquakes"), this);
	m_actionOptionEarthQuakelist->setToolTip(tr("Get the latest earthquake list from the USGS"));
	connect(m_actionOptionEarthQuakelist, SIGNAL(triggered()), this, SLOT(actionOptionEarthQuakelist()));
	
	m_actionOptionEarthUSGS = new QAction(tr("&USGS Website"), this);
	m_actionOptionEarthUSGS->setToolTip(tr("Go to the USGS website for the currently selected earthquake"));
	connect(m_actionOptionEarthUSGS, SIGNAL(triggered()), this, SLOT(actionOptionEarthUSGS()));
	
	
	// Option - Sensor (2D & 3D)
	m_actionOptionSensorVerticalZoomAuto = new QAction(tr("Auto-Zoom Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomAuto->setToolTip(tr("Auto-Zoom Vertical Scale"));
	connect(m_actionOptionSensorVerticalZoomAuto, SIGNAL(triggered()), this, SLOT(actionOptionSensorVerticalZoomAuto()));
	
	m_actionOptionSensorVerticalZoomIn = new QAction(tr("Zoom In Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomIn->setToolTip(tr("Zoom In Vertical Scale"));
	connect(m_actionOptionSensorVerticalZoomIn, SIGNAL(triggered()), this, SLOT(actionOptionSensorVerticalZoomIn()));
	
	m_actionOptionSensorVerticalZoomOut = new QAction(tr("Zoom Out Vertical Scale"), this);
	m_actionOptionSensorVerticalZoomOut->setToolTip(tr("Zoom Out Vertical Scale"));
	connect(m_actionOptionSensorVerticalZoomOut, SIGNAL(triggered()), this, SLOT(actionOptionSensorVerticalZoomOut()));
	
	m_actionOptionSensorHorizontalZoomIn = new QAction(tr("Zoom In Time Scale"), this);
	m_actionOptionSensorHorizontalZoomIn->setToolTip(tr("Zoom In Time Scale"));
	connect(m_actionOptionSensorHorizontalZoomIn, SIGNAL(triggered()), this, SLOT(actionOptionSensorHorizontalZoomIn()));
	
	m_actionOptionSensorHorizontalZoomOut = new QAction(tr("Zoom Out Time Scale"), this);
	m_actionOptionSensorHorizontalZoomOut->setToolTip(tr("Zoom Out Time Scale"));
	connect(m_actionOptionSensorHorizontalZoomOut, SIGNAL(triggered()), this, SLOT(actionOptionSensorHorizontalZoomOut()));
	
	m_actionOptionSensorBack = new QAction(tr("Move Back"), this);
	m_actionOptionSensorBack->setToolTip(tr("Move Back In Time"));
	connect(m_actionOptionSensorBack, SIGNAL(triggered()), this, SLOT(actionOptionSensorBack()));
	
	m_actionOptionSensorPause = new QAction(tr("Pause Display"), this);
	m_actionOptionSensorPause->setToolTip(tr("Pause Sensor Display"));
	connect(m_actionOptionSensorPause, SIGNAL(triggered()), this, SLOT(actionOptionSensorPause()));
	
	m_actionOptionSensorResume = new QAction(tr("Start Display"), this);
	m_actionOptionSensorResume->setToolTip(tr("Start Sensor Display"));
	connect(m_actionOptionSensorResume, SIGNAL(triggered()), this, SLOT(actionOptionSensorResume()));
	
	m_actionOptionSensorRecordStart = new QAction(tr("Start Recording"), this);
	m_actionOptionSensorRecordStart->setToolTip(tr("Start Recording Sensor Time Series"));
	connect(m_actionOptionSensorRecordStart, SIGNAL(triggered()), this, SLOT(actionOptionSensorRecordStart()));
	
	m_actionOptionSensorRecordStop = new QAction(tr("Stop Recording"), this);
	m_actionOptionSensorRecordStop->setToolTip(tr("Stop Recording Sensor Time Series"));
	connect(m_actionOptionSensorRecordStop, SIGNAL(triggered()), this, SLOT(actionOptionSensorRecordStop()));
	
	m_actionOptionSensorForward = new QAction(tr("Move Forward"), this);
	m_actionOptionSensorForward->setToolTip(tr("Move Forward In Time"));
	connect(m_actionOptionSensorForward, SIGNAL(triggered()), this, SLOT(actionOptionSensorForward()));
	
	m_actionOptionSensorAbsolute = new QAction(tr("&Absolute sensor values"), this);
	m_actionOptionSensorAbsolute->setToolTip(tr("Absolute sensor values"));
	connect(m_actionOptionSensorAbsolute, SIGNAL(triggered()), this, SLOT(actionOptionSensorAbsolute()));
	
	m_actionOptionSensorScaled = new QAction(tr("S&caled sensor values"), this);
	m_actionOptionSensorScaled->setToolTip(tr("Scaled sensor values"));
	connect(m_actionOptionSensorScaled, SIGNAL(triggered()), this, SLOT(actionOptionSensorScaled()));
	

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
    connect(m_actionHelpAbout, SIGNAL(triggered()), this, SLOT(actionHelpAbout()));

	m_actionHelpManual = new QAction(tr("&Manual (PDF) for QCNLive"), this);
	m_actionHelpManual->setToolTip(tr("Download/View Manual (PDF) for QCNLive"));
    connect(m_actionHelpManual, SIGNAL(triggered()), this, SLOT(actionHelpManual()));

	m_actionHelpWebQCN = new QAction(tr("&QCN Website"), this);
	m_actionHelpWebQCN->setToolTip(tr("Visit the main QCN website"));
    connect(m_actionHelpWebQCN, SIGNAL(triggered()), this, SLOT(actionHelpWebQCN()));
	
	m_actionHelpWebQCNLive = new QAction(tr("QCN&Live Website"), this);
	m_actionHelpWebQCNLive->setToolTip(tr("Visit the QCNLive website"));
    connect(m_actionHelpWebQCNLive, SIGNAL(triggered()), this, SLOT(actionHelpWebQCNLive()));
	
	m_actionHelpWebEarthquakes = new QAction(tr("&Earthquake Information"), this);
	m_actionHelpWebEarthquakes->setToolTip(tr("Visit QCN's website for earthquakes"));
    connect(m_actionHelpWebEarthquakes, SIGNAL(triggered()), this, SLOT(actionHelpWebEarthquakes()));
	
	m_actionHelpWebLessons = new QAction(tr("Lessons and &Activities"), this);
	m_actionHelpWebLessons->setToolTip(tr("Lessons and Activities website"));
    connect(m_actionHelpWebLessons, SIGNAL(triggered()), this, SLOT(actionHelpWebLessons()));
	
	m_actionHelpWebRequestSensor = new QAction(tr("&Request a Sensor"), this);
	m_actionHelpWebRequestSensor->setToolTip(tr("Request/Purchase a sensor to use with QCN"));
    connect(m_actionHelpWebRequestSensor, SIGNAL(triggered()), this, SLOT(actionHelpWebRequestSensor()));
	
	m_actionHelpWebGlossary = new QAction(tr("&Glossary"), this);
	m_actionHelpWebGlossary->setToolTip(tr("Online Glossary"));
    connect(m_actionHelpWebGlossary, SIGNAL(triggered()), this, SLOT(actionHelpWebGlossary()));

	
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
	m_menuHelp = menuBar()->addMenu(tr("&Help"));
	m_menuHelp->addAction(m_actionHelpManual);
	m_menuHelp->addAction(m_actionHelpWebQCN);
	m_menuHelp->addAction(m_actionHelpWebQCNLive);
	m_menuHelp->addAction(m_actionHelpWebEarthquakes);
	m_menuHelp->addAction(m_actionHelpWebLessons);
	m_menuHelp->addAction(m_actionHelpWebRequestSensor);
	m_menuHelp->addAction(m_actionHelpWebGlossary);
	m_menuHelp->addSeparator();
	m_menuHelp->addAction(m_actionHelpAbout);
	
		
}

QSlider *MyFrame::createSlider(const char *changedSignal,
                                  const char *setterSlot)
{
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    connect(slider, SIGNAL(valueChanged(int)), m_glWidget, setterSlot);
    connect(m_glWidget, changedSignal, slider, SLOT(setValue(int)));
    return slider;
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
	/*
	// see if it's rotating and we want to stop, or it's not rotating and we want it to start
	if ( (!bAuto && qcn_graphics::earth.IsAutoRotate())
		|| (bAuto && ! qcn_graphics::earth.IsAutoRotate()))  {
		bEarthRotate = bAuto;
		Toggle(ID_TOOL_ACTION_EARTH_ROTATE_OFF, !bAuto);
		Toggle(ID_TOOL_ACTION_EARTH_ROTATE_ON, bAuto);
		qcn_graphics::earth.AutoRotate(bAuto);
	}
	*/
}

void MyFrame::ToggleStartStop(bool bStart)
{
	/*
	Toggle(ID_TOOL_ACTION_SENSOR_RESUME, bStart);
	Toggle(ID_TOOL_ACTION_SENSOR_PAUSE, !bStart);
	if (bStart && qcn_graphics::TimeWindowIsStopped()) {
		qcn_graphics::TimeWindowStart();
	}
	else if (!bStart && !qcn_graphics::TimeWindowIsStopped()) {
		qcn_graphics::TimeWindowStop();
	}
	*/
}

void MyFrame::SetupToolbars()
{
	/* CMC
	 toolBar = CreateToolBar(wxNO_BORDER|wxHORIZONTAL, ID_TOOLBAR);
	
	if (toolBar) {
		toolBar->SetToolBitmapSize(wxSize(32,32));
		
		ToolBarView();
		ToolBarEarth(true);
		
		//scrollBar2D = new wxScrollBar(toolBar, ID_TOOL_ACTION_SENSOR_SCROLLBAR, wxDefaultPosition, wxSize(100,10), wxSB_HORIZONTAL, wxDefaultValidator, "Time Scroll");
		if (scrollBar2D) scrollBar2D->Hide();
	}
	 */
}

void MyFrame::actionView()
{
	// get item from event do appropriate action (boinc_key_press!)
	// todo: hook up the other toolbars
	// CMC Toggle(m_view, false, true);
	
	// figure out who called this
	QAction *pAction = qobject_cast<QAction*>(QObject::sender());
	bool bChanged = false;
	if (pAction == m_actionViewSensor2D)
	{
		qcn_graphics::g_eView = VIEW_PLOT_2D;
		// note only redraw sensor toolbar if not coming from a sensor view already
		//if (m_view != ID_TOOL_VIEW_SENSOR_2D && m_view != ID_TOOL_VIEW_SENSOR_3D) ToolBarSensor(evt.GetId());
		m_view = VIEW_PLOT_2D;
		// CMC ToolBarSensor2D();
		bChanged = true;
	}
	else if (pAction == m_actionViewSensor3D)
	{
		qcn_graphics::g_eView = VIEW_PLOT_3D;
		// note only redraw sensor toolbar if not coming from a sensor view already
		//if (m_view != ID_TOOL_VIEW_SENSOR_2D && m_view != ID_TOOL_VIEW_SENSOR_3D) ToolBarSensor(evt.GetId());
		m_view = VIEW_PLOT_3D;
		// CMC ToolBarSensor3D();
		bChanged = true;
	}
	else if (pAction == m_actionViewCube)
	{
		qcn_graphics::g_eView = VIEW_CUBE;
		m_view = VIEW_CUBE;
		// CMC ToolBarCube();
		bChanged = true;
	}
	else {
		if (m_bEarthDay)
			qcn_graphics::earth.SetMapCombined();
		else
			qcn_graphics::earth.SetMapNight();
		m_view = VIEW_EARTH_DAY;
		// CMC ToolBarEarth();
		bChanged = true;
	}

	qcn_graphics::FaderOn();
    if (bChanged) {
		// CMC Toggle(m_view, true, true);
    }
    qcn_graphics::ResetPlotArray();
}

/*
void MyFrame::actionViewEarth()
{
	// CMC Toggle(m_view, false, true);
	if (m_bEarthDay)  {
		m_view = VIEW_EARTH_DAY;
		qcn_graphics::earth.SetMapCombined();
	}
	else  { 
		m_view = VIEW_EARTH_NIGHT;
		qcn_graphics::earth.SetMapNight();
	}
	//CMC			ToolBarEarth();
}

void MyFrame::actionViewSensor2D()
{
	// CMC Toggle(m_view, false, true);
	qcn_graphics::g_eView = VIEW_PLOT_2D;
	m_view = qcn_graphics::g_eView;	
	//CMC			ToolBarSensor2D();
	qcn_graphics::FaderOn();
	// CMC Toggle(m_view, true, true);
    qcn_graphics::ResetPlotArray();
}

void MyFrame::actionViewSensor3D()
{
	// CMC Toggle(m_view, false, true);
	qcn_graphics::g_eView = VIEW_PLOT_3D;
	m_view = qcn_graphics::g_eView;	
	//CMC			ToolBarSensor3D();
	qcn_graphics::FaderOn();
	// CMC Toggle(m_view, true, true);
    qcn_graphics::ResetPlotArray();
}

void MyFrame::actionViewCube()
{
	// CMC Toggle(m_view, false, true);
	qcn_graphics::g_eView = VIEW_CUBE;
	//CMC			ToolBarCube();
}
*/

void MyFrame::actionHelpAbout()
{
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

void MyFrame::actionOptionScreenshot()
{
}

void MyFrame::actionOptionLogo()
{
}

void MyFrame::actionOptionEarthDay()
{
}

void MyFrame::actionOptionEarthNight()
{
}

void MyFrame::actionOptionEarthRotateOn()
{
}

void MyFrame::actionOptionEarthRotateOff()
{
}

void MyFrame::actionOptionEarthUSGS()
{
}

void MyFrame::actionOptionEarthQuakelist()
{
}


void MyFrame::actionOptionSensorVerticalZoomAuto()
{
}

void MyFrame::actionOptionSensorVerticalZoomIn()
{
}

void MyFrame::actionOptionSensorVerticalZoomOut()
{
}

void MyFrame::actionOptionSensorHorizontalZoomIn()
{
}

void MyFrame::actionOptionSensorHorizontalZoomOut()
{
}

void MyFrame::actionOptionSensorBack()
{
}

void MyFrame::actionOptionSensorPause()
{
}

void MyFrame::actionOptionSensorResume()
{
}

void MyFrame::actionOptionSensorRecordStart()
{
}

void MyFrame::actionOptionSensorRecordStop()
{
}

void MyFrame::actionOptionSensorForward()
{
}

void MyFrame::actionOptionSensorAbsolute()
{
}

void MyFrame::actionOptionSensorScaled()
{
}



void MyFrame::actionHelpManual()
{
}

void MyFrame::actionHelpWebQCN()
{
}

void MyFrame::actionHelpWebQCNLive()
{
}

void MyFrame::actionHelpWebEarthquakes()
{
}

void MyFrame::actionHelpWebLessons()
{
}

void MyFrame::actionHelpWebRequestSensor()
{
}

void MyFrame::actionHelpWebGlossary()
{
}
