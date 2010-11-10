// the main OpenGL widget for Qt

#include <QtGui>
#include <QtOpenGL>

#include <math.h>

#include "glwidget.h"
#include "qcn_earth.h"

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
{
	m_pframe = (MyFrame*) parent;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(animate()));
    m_timer->start(40);  // 40 milliseconds = 25 frames per second, a decent animation rate
	
	// init the OpenGL graphics vars from the qcn_graphics namespace
	qcn_graphics::graphics_main(0, NULL);
}

GLWidget::~GLWidget()
{
	m_timer->stop();
    makeCurrent();
}

void GLWidget::setTimePosition(const double& dTime)
{
	emit TimePositionChanged(dTime);
	updateGL();
}

void GLWidget::initializeGL()
{
    if (!qcn_graphics::g_bInitGraphics) {  // first time in, need to init OpenGL settings & load bitmaps etc
        qcn_graphics::Init(); 
        // m_pframe->pMyApp->KillSplashScreen();
    }	
}

void GLWidget::paintGL()
{
    qcn_graphics::Render(0,0,0);
}

void GLWidget::resizeGL(int width, int height)
{
    // call the qcn_graphics.cpp resize	
	qcn_graphics::Resize(width, height);	
	// tell the earth to regen earthquakes coords
	qcn_graphics::earth.RecalculateEarthquakePositions();
	
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	int which = whichGLUTButton(event);
	switch(qcn_graphics::g_eView) {
		case VIEW_EARTH_DAY:
		case VIEW_EARTH_NIGHT:
		//CMC	m_pframe->EarthRotate(true);
			break;
		case VIEW_PLOT_2D:
		//CMC	m_pframe->ToggleStartStop(true);
			break;
		case VIEW_PLOT_3D:
		case VIEW_EARTH_COMBINED:
		case VIEW_CUBE:
			break;
	}
	
	// force to start
	qcn_graphics::MouseButton(event->x(), event->y(), which, 1);
	
	//CMC  SetCursor(wxCursor(wxNullCursor));
}

const int GLWidget::whichGLUTButton(const QMouseEvent* event) const
{
	int which = GLUT_NO_BUTTON;
	switch(event->button()) {
		case Qt::LeftButton:  
			which = GLUT_LEFT_BUTTON;
			break;
		case Qt::MidButton:  
			which = GLUT_MIDDLE_BUTTON;
			break;
		case Qt::RightButton:  
			which = GLUT_RIGHT_BUTTON;
			break;
		default:
			which = GLUT_NO_BUTTON;
	}
	return which;
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	int which = whichGLUTButton(event);
	m_mouseDown[event->button()-Qt::LeftButton] = false;  // the wxwidgets getbutton is one off from our left/mid/right array
	qcn_graphics::MouseButton(event->x(), event->y(), which, 0);
	//CMC SetCursor(wxNullCursor);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
	int which = whichGLUTButton(event);
    lastPos = event->pos();
	
// CMC	m_mouseDown[evt.GetButton()-1] = true;  // the wxwidgets getbutton is one off from our left/mid/right array
	
	switch(qcn_graphics::g_eView) {
		case VIEW_EARTH_DAY:
		case VIEW_EARTH_NIGHT:
// CMC			m_pframe->EarthRotate(false);
			break;
		case VIEW_PLOT_2D:
// CMC			m_pframe->ToggleStartStop(false);
			break;
		case VIEW_PLOT_3D:
		case VIEW_EARTH_COMBINED:
		case VIEW_CUBE:		
			break;
	}
	
	// CMC SetCursor(wxCursor(wxCURSOR_HAND));	
	
	qcn_graphics::MouseButton(event->x(), event->y(), which, 1);
	
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
   // int dx = event->x() - lastPos.x();
   // int dy = event->y() - lastPos.y();
   // lastPos = event->pos();
	qcn_graphics::MouseMove(event->x(), event->y(), 
		event->button() == Qt::LeftButton ? 1 : 0, event->button() == Qt::MidButton ? 1 : 0, event->button() == Qt::RightButton ? 1 : 0);
}

void GLWidget::animate()
{
    updateGL();
}
