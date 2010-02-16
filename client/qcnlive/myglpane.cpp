/*
 *  myglpane.cpp
 *  qcn
 *
 *  Created by Carl Christensen on 2/15/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "qcnwx.h"
#include "qcn_earth.h"

// the next line puts in the appropriate headers so I can call wxGetApp.ProcessIdle() below
//DECLARE_APP(MyApp)

// first off do the timer stuff that the GLPane will use
	  
MyGLTimer::MyGLTimer(MyGLPane* pglp) : wxTimer()
{
	 m_pGLPane = pglp;
}

void MyGLTimer::Notify() 
{
	if (m_pGLPane && m_pGLPane->IsShown()) {
		m_pGLPane->Refresh(false);  // paint the window if visible!
    }
	if (sm && sm->strDisplay[0]) { // little trick to display a status message to the GUI from elsewhere in the qcn system i.e. after writing a sac file
		m_pGLPane->m_pframe->SetStatusText(sm->strDisplay);
		memset(sm->strDisplay, 0x00, sizeof(char) * 256);
	}
}


BEGIN_EVENT_TABLE(MyGLPane, wxGLCanvas)
EVT_SIZE(MyGLPane::resized)
EVT_PAINT(MyGLPane::render)
//EVT_IDLE(MyGLPane::OnIdle)

EVT_MOTION(MyGLPane::OnMouseMove)
EVT_LEFT_DOWN(MyGLPane::OnMouseDown)
EVT_LEFT_UP(MyGLPane::OnMouseRelease)
EVT_LEFT_DCLICK(MyGLPane::OnMouseDoubleClick)
EVT_RIGHT_DOWN(MyGLPane::OnMouseDown)
EVT_RIGHT_UP(MyGLPane::OnMouseRelease)
EVT_ERASE_BACKGROUND(MyGLPane::OnEraseBackground)

/*
EVT_RIGHT_DOWN(MyGLPane::rightClick)
EVT_LEAVE_WINDOW(MyGLPane::mouseLeftWindow)
EVT_KEY_DOWN(MyGLPane::keyPressed)
EVT_KEY_UP(MyGLPane::keyReleased)
EVT_MOUSEWHEEL(MyGLPane::mouseWheelMoved)
*/
END_EVENT_TABLE()
 
MyGLPane::MyGLPane(MyFrame* parent, int* aiAttrib) 
  :  wxGLCanvas( (wxWindow*) parent, 
        wxID_ANY, 
        wxDefaultPosition, 
        wxDefaultSize, 
        0, 
        wxT("GLPane"),  
        aiAttrib,
        wxNullPalette)
{
	m_pframe = parent;
	
	// setup & start the timer for repaint evts
        m_ptimer = new MyGLTimer(this);
	if (m_ptimer) {
		m_ptimer->Start(40);  // 40 milliseconds per paint evt, that should be fine (25 frames per second)
	}

    // init the OpenGL graphics from the qcn_graphics namespace
    qcn_graphics::graphics_main(0, NULL);
}

MyGLPane::~MyGLPane()
{ // just stop & destroy the timer
   if (m_ptimer) {
      m_ptimer->Stop();
	  delete m_ptimer;
   }
}

// some useful evts to use
void MyGLPane::OnMouseMove(wxMouseEvent& evt) 
{
   qcn_graphics::MouseMove(evt.GetPosition().x, evt.GetPosition().y, evt.m_leftDown ? 1 : 0, evt.m_middleDown ? 1 : 0, evt.m_rightDown ? 1 : 0);
}

void MyGLPane::OnMouseDoubleClick(wxMouseEvent& evt) 
{
	int which = -1;
	
	//qcn_graphics::MouseButton(evt.GetPosition().x, evt.GetPosition().y, which, 1);
	// ignore double clicks?
	
	switch(qcn_graphics::g_eView) {
		case VIEW_EARTH_DAY:
		case VIEW_EARTH_NIGHT:
			m_pframe->EarthRotate(true);
			break;
		case VIEW_PLOT_2D:
			m_pframe->ToggleStartStop(true);
			break;
		case VIEW_PLOT_3D:
		case VIEW_EARTH_COMBINED:
		case VIEW_CUBE:
			break;
	}

	// force to start
	qcn_graphics::MouseButton(evt.GetPosition().x, evt.GetPosition().y, which, 1);

	SetCursor(wxCursor(wxNullCursor));
}
	
void MyGLPane::OnMouseDown(wxMouseEvent& evt) 
{
   int which = -1;
   switch(evt.GetButton()) {
     case 1:  
        which = GLUT_LEFT_BUTTON;
		break;
     case 2:  
        which = GLUT_MIDDLE_BUTTON;
		break;
     case 3:  
        which = GLUT_RIGHT_BUTTON;
		break;
   }
   m_mouseDown[evt.GetButton()-1] = true;  // the wxwidgets getbutton is one off from our left/mid/right array

	switch(qcn_graphics::g_eView) {
		case VIEW_EARTH_DAY:
		case VIEW_EARTH_NIGHT:
			m_pframe->EarthRotate(false);
			break;
		case VIEW_PLOT_2D:
			m_pframe->ToggleStartStop(false);
			break;
		case VIEW_PLOT_3D:
		case VIEW_EARTH_COMBINED:
		case VIEW_CUBE:		
			break;
	}
	
	SetCursor(wxCursor(wxCURSOR_HAND));	

   qcn_graphics::MouseButton(evt.GetPosition().x, evt.GetPosition().y, which, 1);

/* // show earthquake text on status bar?
   if ((qcn_graphics::earth.IsShown()) && !earth.IsAutoRotate() && earth.IsEarthquakeSelected()) {
      char* strQuake = new char[_MAX_PATH];
	  memset(strQuake, 0x00, sizeof(char) * _MAX_PATH);
      earth.EarthquakeSelectedString(strQuake, _MAX_PATH);
      m_pframe->SetStatusText(strQuake);
	  delete [] strQuake;
   }
   else {
      m_pframe->SetStatusText("");
   }
*/
}

void MyGLPane::OnEraseBackground(wxEraseEvent& evt)
{  // do nothing -- prevents flicker from the timer event triggering repaints every 40 ms...
}

void MyGLPane::OnMouseRelease(wxMouseEvent& evt) 
{ 
   int which = -1;
   switch(evt.GetButton()) {
     case 1:  
        which = GLUT_LEFT_BUTTON;
		break;
     case 2:  
        which = GLUT_MIDDLE_BUTTON;
		break;
     case 3:  
        which = GLUT_RIGHT_BUTTON;
		break;
   }
   m_mouseDown[evt.GetButton()-1] = false;  // the wxwidgets getbutton is one off from our left/mid/right array
   qcn_graphics::MouseButton(evt.GetPosition().x, evt.GetPosition().y, which, 0);
   SetCursor(wxNullCursor);
}

/*
void MyGLPane::mouseWheelMoved(wxMouseEvent& evt) 
{
}

void MyGLPane::rightClick(wxMouseEvent& evt) 
{
}

void MyGLPane::mouseLeftWindow(wxMouseEvent& evt) 
{
}

void MyGLPane::keyPressed(wxKeyEvent& evt) 
{
}

void MyGLPane::keyReleased(wxKeyEvent& evt) 
{
}
*/
 
void MyGLPane::resized(wxSizeEvent& evt)
{
    if (!qcn_graphics::g_bInitGraphics) return;
    wxGLCanvas::OnSize(evt);
    // call the qcn_graphics.cpp resize	
	qcn_graphics::Resize(m_pframe->GetClientSize().GetWidth(), m_pframe->GetClientSize().GetHeight());	
	// tell the earth to regen earthquakes coords
	qcn_graphics::earth.RecalculateEarthquakePositions();
    Refresh(true);
}
 
int MyGLPane::getWidth()
{
    return GetSize().x;
}
 
int MyGLPane::getHeight()
{
    return GetSize().y;
}

void MyGLPane::render( wxPaintEvent& evt )
{
	if(!IsShown()) {
        evt.Skip();
		return;
    }	

   /* eventually will need to use the 4th wxGLCanvas constructor and then call these two methods:
       wxGLContext myglcontext((wxGLCanvas*) this, NULL);
       myglcontext.SetCurrent(*this);
   */
   
    SetCurrent(); // gets the OpenGL device context
    wxPaintDC dc(this);  // get the paint DC, really processes the paint msg, otherwise dialog boxes "hang"

    if (!qcn_graphics::g_bInitGraphics) {  // first time in, need to init OpenGL settings & load bitmaps etc
        qcn_graphics::Init(); 
#ifdef _WIN32  // windows needs a resize kickstart...
        wxSizeEvent sizeevt;
        resized(sizeevt);
#endif
        m_pframe->pMyApp->KillSplashScreen();
    }

	//void Render(int xs, int ys, double time_of_day)
    qcn_graphics::Render(0,0,0);
    SwapBuffers();
}

    /*
void MyGLPane::OnIdle(wxIdleEvent& evt)
{
    static double lasttime = 0.0f;
    double curtime = dtime();
	if (curtime > lasttime + .04f && IsShown()) {
	    // the .04 will make a frame rate of about 25 fps  (25 * .04 = 1 second)
	    lasttime = curtime;
        Refresh(true);
	}
    evt.RequestMore();
}
    */
