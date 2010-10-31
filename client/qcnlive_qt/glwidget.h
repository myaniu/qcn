#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "qcnqt.h"

#include <QGLWidget>
#include <QTimer>

class MyFrame;
class MyGLTimer;

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    int xRotation() const { return xRot; }
    int yRotation() const { return yRot; }
    int zRotation() const { return zRot; }

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
	
	MyFrame* m_pframe;  // reference to parent frame
	MyGLTimer* m_ptimer;
	bool m_mouseDown[3];  // bools for mouse down -- in order of left/middle/right
	
	// methods
	int getWidth();
	int getHeight();
	
	// events
	/*
	void render(wxPaintEvent& evt);
	void resized(wxSizeEvent& evt);
	
	void OnMouseMove(wxMouseEvent& evt);
	void OnMouseDoubleClick(wxMouseEvent& evt);
	void OnMouseDown(wxMouseEvent& evt);
	void OnMouseRelease(wxMouseEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt);
	 */

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void advanceGears();

private:
    GLuint makeGear(const GLfloat *reflectance, GLdouble innerRadius,
                    GLdouble outerRadius, GLdouble thickness,
                    GLdouble toothSize, GLint toothCount);
    void drawGear(GLuint gear, GLdouble dx, GLdouble dy, GLdouble dz,
                  GLdouble angle);
    void normalizeAngle(int *angle);

    GLuint gear1;
    GLuint gear2;
    GLuint gear3;
    int xRot;
    int yRot;
    int zRot;
    int gear1Rot;
    
    QPoint lastPos;
};

/*
class MyGLTimer : public QTimer
{
public:
	MyGLTimer(GLWidget* pglw);
	void Notify();
private:
	GLWidget* m_pGLW;
};
*/

#endif
