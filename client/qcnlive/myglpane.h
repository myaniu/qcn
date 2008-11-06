#ifndef _MYGLPANE_H_
#define _MYGLPANE_H_

#include "qcnwx.h"

class MyFrame;
class MyGLTimer;

class MyGLPane : public wxGLCanvas
{

public:
        MyGLPane(MyFrame* parent, int* aiAttrib);
        ~MyGLPane();

        MyFrame* m_pframe;  // reference to parent frame
        MyGLTimer* m_ptimer;
        bool m_mouseDown[3];  // bools for mouse down -- in order of left/middle/right

        // methods
        int getWidth();
        int getHeight();

        // events
        void render(wxPaintEvent& evt);
        void resized(wxSizeEvent& evt);

        void OnMouseMove(wxMouseEvent& evt);
        void OnMouseDown(wxMouseEvent& evt);
        void OnMouseRelease(wxMouseEvent& evt);
        void OnEraseBackground(wxEraseEvent& evt);

        //void OnIdle(wxIdleEvent &evt); 

        //void mouseWheelMoved(wxMouseEvent& evt);
        //void rightClick(wxMouseEvent& evt);
        //void mouseLeftWindow(wxMouseEvent& evt);
        //void keyPressed(wxKeyEvent& evt);
        //void keyReleased(wxKeyEvent& evt);

        DECLARE_EVENT_TABLE()
};

class MyGLTimer : public wxTimer
{
   public:
      MyGLTimer(MyGLPane* pglp);
          void Notify();
   private:
          MyGLPane* m_pGLPane;
};

#endif // _MYGLPANE_H_
