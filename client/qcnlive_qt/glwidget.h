#ifndef _GLWIDGET_H
#define _GLWIDGET_H

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

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
	const int whichGLUTButton(const QMouseEvent* event) const;
	
public slots:
    void setTimePosition(const double& dValue);
	
signals:
	void TimePositionChanged(const double& dValue);
	
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void animate();

private:
	int m_width;
	int m_height;
    QPoint m_lastPos;

public:
	MyFrame* m_pframe;  // reference to parent frame
	//bool m_mouseDown[3];  // bools for mouse down -- in order of left/middle/right
	QTimer* m_timer;  // animation timer
	
};

#endif
