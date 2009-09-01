#include "qcn_graphics.h"
#include "qcn_2dplot.h"

using namespace qcn_graphics;

namespace qcn_2dplot {

static int g_iTimerTick = 1; // seconds for each timer point

#ifdef QCNLIVE
	static bool g_bIsWhite = true;
#else
	static bool g_bIsWhite = false;
#endif

static const int g_iScaleSigMax  = 5;
static const int g_iScaleAxesMax = 5;

static int g_iScaleSigOffset  = 0;
static int g_iScaleAxesOffset = 0;

static const float g_fScaleSig[6]  = { 3.0f, 6.0f, 9.0f, 12.0f, 18.0f, 24.0f }; // default scale for sig is 20
static const float g_fScaleAxes[6] = { .12f, .36f, 1.8f, 3.0f,  9.8f, 19.6f };

//static bool g_bAutoCenter = true; // automatically center the plots
static bool g_bAutoScale = true; // set to true when want each axes to scale around the last 100 data points (maybe need every second?)

static float g_fMaxAxesCurrent[4]  = { g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0] };  // save each scale level for autoscaling, so it's not jumping all around
static float g_fMinAxesCurrent[4]  = { -g_fScaleAxes[0], -g_fScaleAxes[0], -g_fScaleAxes[0], -g_fScaleAxes[0] };  // save each scale level for autoscaling, so it's not jumping all around
	
static const float cfTransAlpha = 0.100f;
static const float cfAxisLabel  = 1.061f;
static const float cfVertLabel  = 0.988f;
static const float cfYOffset    = 0.016f;
static const float cfMSSLabel   = 0.050f; // m/s/s label
static const float cfSigOffset[7] = { .562f, .588f, .616f, .643f, .670f, .697f, .719f };

static const float cfAxesLabel[4] = { 0.124f, .284f, .444f, .584f };

static const float cfBaseScale[4] = { 0.068f, .232f, .397f, .562f };
static const float cfAxesOffset[7] = { .0f, .021f, .049f, .077f, .104f, .131f, .151f };
static const float cfLabelTime[2] = { cfVertLabel/2.0f - 0.1f, 0.00f};

void draw_text_sensor_axis(int iAxis)
{
	char cbuf[10];
	if (g_fMaxAxesCurrent[iAxis] == SAC_NULL_FLOAT || g_fMinAxesCurrent[iAxis] == -1.0f * SAC_NULL_FLOAT) return;
	float fIncrement = (g_fMaxAxesCurrent[iAxis] - g_fMinAxesCurrent[iAxis]) / 6.0f;
	for (int i = 0; i <= 6; i++) {
		sprintf(cbuf, "%+4.2f", g_fMinAxesCurrent[iAxis] + (fIncrement * (float) i) );
	    txf_render_string(cfTransAlpha, cfVertLabel, cfBaseScale[iAxis] + cfAxesOffset[i], 0, MSG_SIZE_SMALL, g_bIsWhite ? black : grey_trans, TXF_COURIER_BOLD, cbuf);
	}
}

void draw_text() 
{
   // draw text on top
   mode_unshaded();
   mode_ortho();

   // now draw time text at the bottom
   char strTime[16];
   //txf_render_string(.1, fWhere, Y_TRIGGER_LAST[0] - 3.0f, 0, 800, blue, TXF_HELVETICA, (char*) strTime);
    for (int i = 0; i < g_iTimeCtr; i++) {
       if (lTimeLast[i] > 0.0f) { // there's a marker to place here
	     float fWhere = (float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE;
		 // note the immediate if below - if timer ticks are far apart don't bother showing seconds
		 qcn_util::dtime_to_string(lTimeLast[i], (g_iTimerTick > 5 ? 'm' : 'h'), strTime);
		 txf_render_string(.1f, fWhere - (g_iTimerTick > 5 ? 0.038f : 0.042f), 0.030f, 0.0f, 
			 MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, (char*) strTime);
	   }
	}

	/*
#ifdef _DEBUG
	sprintf(strTime, "%+6.3f %+6.3f", g_fMin[0], g_fMax[0]);
    txf_render_string(.1f, .1f, cfAxesLabel[0], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", g_fMin[1], g_fMax[1]);
    txf_render_string(.1f, .1f, cfAxesLabel[1], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", g_fMin[2], g_fMax[2]);
    txf_render_string(.1f, .1f, cfAxesLabel[2], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", g_fMin[3], g_fMax[3]);
    txf_render_string(.1f, .1f, cfAxesLabel[3], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
#endif
	*/

	// labels for each axis

	txf_render_string(cfTransAlpha, cfAxisLabel, cfAxesLabel[E_DS], 0, MSG_SIZE_NORMAL, red, TXF_HELVETICA, "Significance", 90.0f);
    txf_render_string(cfTransAlpha, cfAxisLabel, cfAxesLabel[E_DZ], 0, MSG_SIZE_NORMAL, blue, TXF_HELVETICA, "Z Axis", 90.0f);
    txf_render_string(cfTransAlpha, cfAxisLabel, cfAxesLabel[E_DY], 0, MSG_SIZE_NORMAL, orange, TXF_HELVETICA, "Y Axis", 90.0f);
    txf_render_string(cfTransAlpha, cfAxisLabel, cfAxesLabel[E_DX], 0, MSG_SIZE_NORMAL, green, TXF_HELVETICA, "X Axis", 90.0f);

	// labels for significance
	draw_text_sensor_axis(E_DS);

	// labels for Z axis
	draw_text_sensor_axis(E_DZ);

	// labels for Y axis
	draw_text_sensor_axis(E_DY);

	// labels for X axis  084
	draw_text_sensor_axis(E_DX);

	// units label (meters per second per second
    txf_render_string(cfTransAlpha, cfVertLabel, cfMSSLabel, 0, MSG_SIZE_SMALL, g_bIsWhite ? black : grey_trans, TXF_COURIER_BOLD, " m/s/s");

    // time label

	switch(qcn_graphics::GetTimeWindowWidth()) {
	case 10:
        txf_render_string(cfTransAlpha, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 10 Second Window");
		break;
	case 60:
        txf_render_string(cfTransAlpha, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 1 Minute Window");
		break;
	case 600:
        txf_render_string(cfTransAlpha, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 10 Minute Window");
		break;
	case 3600:
        txf_render_string(cfTransAlpha, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 1 Hour Window");
		break;
	default:
        txf_render_string(cfTransAlpha, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC)");
	}

    draw_text_sensor();

	char bufout[64];
	sprintf(bufout, "%.2f  %.2f  %.2f  %.2f", g_fAvg[0], g_fAvg[1], g_fAvg[2], g_fAvg[3]);
	txf_render_string(cfTransAlpha, 0.04f, 0.1f, 0.0f, MSG_SIZE_SMALL, light_blue, TXF_HELVETICA, bufout);

	ortho_done();
}

void draw_tick_marks()
{  // draw vertical blue lines every 1/10/60/600 seconds depending on view size
	  // note the labels underneath are drawn in draw_text_plot_qcnlive
    // show the time markers, if any
    glPushMatrix();
    for (int i = 0; i < g_iTimeCtr; i++) {
       if (lTimeLast[i] > 0.0f) { // there's a marker to place here
	     float fWhere;
	     if (g_eView == VIEW_PLOT_2D) {
            fWhere = xax_qcnlive[0] + ( ((float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0]));
		 }
		 else  {
            fWhere = xax[0] + ( ((float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax[1]-xax[0]));
         }
         //fprintf(stdout, "%d  dTriggerLastTime=%f  lTriggerLastOffset=%ld  fWhere=%f\n",
         //    i, dTriggerLastTime[i], lTriggerLastOffset[i], fWhere);
         //fflush(stdout);
         glColor4fv((GLfloat*) g_bIsWhite ? light_blue : grey_trans);
         glLineWidth(1);
         //glLineStipple(4, 0xAAAA);
         //glEnable(GL_LINE_STIPPLE);
         glBegin(GL_LINE_STRIP);
         glVertex2f(fWhere, Y_TRIGGER_LAST[0]);
         glVertex2f(fWhere, Y_TRIGGER_LAST[1]);
         glEnd();
         //glDisable(GL_LINE_STIPPLE);
	   }
    }
    glPopMatrix();
}

bool CalcYPlot(const float& fVal, const float& fAvg, const int& ee, float&  myY)
{
    myY = yax_qcnlive[ee] + (ee == E_DS ? 0.5f : 0.0f) 
	     + ( 15.0f * ( (fVal - g_fMinAxesCurrent[ee])
                                   / (g_fMaxAxesCurrent[ee] - g_fMinAxesCurrent[ee] ) )  );

    //if (fdata[i] != 0.0f) { // suppress 0 values and check data ranges fit
    if ( fVal == SAC_NULL_FLOAT ) { // invalid, suppress
       myY = SAC_NULL_FLOAT;
       return false;
    }
    else if ( fVal > g_fMaxAxesCurrent[ee] ) { // max limit
       myY = yax_qcnlive[ee] + (ee==E_DS ? 15.5f : 15.0f);
    }
    else if ( fVal < g_fMinAxesCurrent[ee] ) { // min limit
       myY = yax_qcnlive[ee];
    }
    return true;
}

void draw_scrollbar()
{ // CMC HERE
   mode_unshaded();
   mode_ortho();

   glColor4fv(white);
     glLineWidth(15);
			 glBegin(GL_LINES);
				 glVertex2f(.7, cfLabelTime[1] + .05);
				 glVertex2f(1.0, cfLabelTime[1] + .05);
			 glEnd();

/* // gradient

glMatrixMode(GL_PROJECTION);
glLoadIdentity();

glMatrixMode(GL_MODELVIEW);
glLoadIdentity();

glBegin(GL_QUADS);
//red color
glColor3f(1.0,0.0,0.0);
glVertex2f(-1.0, 1.0);
glVertex2f(-1.0,-1.0);
//blue color
glColor3f(0.0,0.0,1.0);
glVertex2f(1.0,-1.0);
glVertex2f(1.0, 1.0);
glEnd();

*/

   ortho_done();
}

void draw_plot() 
{

/*
- boxes should be even, as well as plotting since all +/- 19.6 m/s2, sig 0 - 10
- bouncing ball at "tip" where drawn
- S/X/Y/Z on right side with vertical axis
- toggle background colors black/white?
*/

    float* fdata = NULL;

    // each plot section is 15 units high

	//static int iFrameCounter = 0;
	//static float fMeanLast = 0.0f, fStdDevLast, fVarianceLast;  // preserve state of last mean etc
	//static float fMean = 0.0f, fStdDev = 0.0f, fVariance = 0.0f;

    float xmin = xax_qcnlive[0] - 0.1f;
    float xmax = xax_qcnlive[1] + 0.1f;
    float ymin = yax_qcnlive[E_DX] - 7.0f;
    float ymax = yax_qcnlive[4]; // + 15.0f;
    float yPen[4] = { SAC_NULL_FLOAT, SAC_NULL_FLOAT, SAC_NULL_FLOAT, SAC_NULL_FLOAT }; // save "pen" position i.e. last point on plot

    float x1, y1; // temp values to compare ranges for plotting
    long lStart, lEnd;

    if (!sm) return; // not much point in continuing if shmem isn't setup!

    init_camera(viewpoint_distance[g_eView], 45.0f);
    init_lights();
    scale_screen(g_width, g_height);

    // should just be simple draw each graph in 2D using the info in dx/dy/dz/ds?
    
  //  glPushMatrix();
    mode_unshaded();

	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

    for (int ee = E_DX; ee <= E_DS; ee++)  {
	
         switch(ee) {
            case E_DX:  fdata = (float*) aryg[E_DX]; break;
            case E_DY:  fdata = (float*) aryg[E_DY]; break;
            case E_DZ:  fdata = (float*) aryg[E_DZ]; break;
            case E_DS:  fdata = (float*) aryg[E_DS]; break;
         }

         // first draw the axes
		 // draw 2 above & 2 below and one in the middle
		 const float yfactor = 2.50f, xfactor = 0.00f;
		 for (int j = -2; j <= 3; j++) {
			 /* make lines 1 wide
			 if (ee == E_DS) {
				 if (j == -2)
					 glLineWidth(3);
				 else
					 glLineWidth(1);
			 }
			 else {
				 if (j == 0)
					 glLineWidth(3);
				 else
					 glLineWidth(1);
			 }
			 */
			 glLineWidth(1);

			 glColor4fv(grey);
			 glBegin(GL_LINES);

			 if (ee == E_DS) {
				 glVertex2f(xax_qcnlive[0], yax_qcnlive[ee] + .5f + (yfactor * (float) (j+2)));
				 glVertex2f(xax_qcnlive[1] + xfactor, yax_qcnlive[ee] + .5f + (yfactor * (float) (j+2)));
			 }
			 else { 
				 if (j<3) { // only sig E_DS get's the j=3 line
					 glVertex2f(xax_qcnlive[0], yax_qcnlive[ee] + 7.5f + (yfactor * (float) j));
					 glVertex2f(xax_qcnlive[1] + xfactor, yax_qcnlive[ee] + 7.5f + (yfactor * (float) j));
				 }
			 }

			 glEnd();


			// need to have the "later" lines override the "earlier" lines (i.e. plot data replaces axes lines)
			//glBlendFunc (GL_DST_ALPHA, GL_SRC_ALPHA);
			 glColor4fv(ee == E_DY ? orange : colorsPlot[ee]);  // set the color for data - note the orange substitution for yellow on the Y
			 glLineWidth(2.0f);
			 glBegin(GL_LINE_STRIP);

			 // get the scale for each axis
			 			 
			 if (g_bAutoScale) { // compute fScale Factor from last 100 pts
				lStart = 0;
				lEnd = PLOT_ARRAY_SIZE-1;
				if (ee == E_DS) {
					g_fMaxAxesCurrent[ee] = (qcn_graphics::g_fmax[ee] == SAC_NULL_FLOAT ? 1.0f : qcn_graphics::g_fmax[ee]);  // save each scale level for autoscaling, so it's not jumping all around
					g_fMinAxesCurrent[ee] = 0.0f;
				}
				else {
					g_fMaxAxesCurrent[ee] = (qcn_graphics::g_fmax[ee] == SAC_NULL_FLOAT ? 1.0f : qcn_graphics::g_fmax[ee]);  // save each scale level for autoscaling, so it's not jumping all around
					g_fMinAxesCurrent[ee] = (qcn_graphics::g_fmin[ee] == -1.0f * SAC_NULL_FLOAT ? 0.0f : qcn_graphics::g_fmin[ee]);  // save each scale level for autoscaling, so it's not jumping all around
				}
			 }
			 else {
				g_fMaxAxesCurrent[ee] = ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] + g_fAvg[ee]);
				g_fMinAxesCurrent[ee] = ( ee == E_DS ? 0.0f : -g_fScaleAxes[g_iScaleAxesOffset] + g_fAvg[ee]);
			 }

			 if ((g_fMaxAxesCurrent[ee] - g_fMinAxesCurrent[ee]) == 0.0f) {
				g_fMaxAxesCurrent[ee] = 1.0f;
			    g_fMinAxesCurrent[ee] = 0.0f;  // avoid divide by zero
			 }
			 for (int i=0; i<PLOT_ARRAY_SIZE; i++) {
				 x1 = xax_qcnlive[0] + (((float) i / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0]));
				 if (CalcYPlot(fdata[i], (g_bAutoScale ? 0.0f : g_fAvg[ee]), ee, y1)) { // this gets complicated so call a function that I can reuse for drawing the "pen" below
					glVertex2f(x1, y1); // if this returns true then we have a valid point to draw
				}
			 }
			 yPen[ee] = y1; // this y1 will be the final plot position i.e. PLOT_ARRAY_SIZE-1 to be used below to draw the pen
	 	     glEnd();
		 }
	     //iFrameCounter++; // bump up the frame ctr
				 
         // x/y/z data points are +/- 19.6 m/s2 -- significance is 0-? make it 0-10		 		 

		const float fRadius = 1.4f;
		const float fAngle = PI/8.0f;
		// plot a "colored pointer" at the end for ease of seeing current value?
		if (yPen[ee] != SAC_NULL_FLOAT) {
			x1 = xax_qcnlive[0] + (xax_qcnlive[1]-xax_qcnlive[0]);
			glBegin(GL_TRIANGLE_FAN);
			   glVertex2f(x1, yPen[ee]);
			   glVertex2f(x1 + (cos(fAngle) * fRadius), yPen[ee] + (sin(fAngle) * fRadius));
			   glVertex2f(x1 + (cos(-fAngle) * fRadius), yPen[ee] + (sin(-fAngle) * fRadius));
			glEnd();
		} // colored pointer
	}

	// draw boxes around the plots

	 const float fExt = 7.05f;
	 const float fFudge = 0.02f;

	 glColor4fv((GLfloat*) g_bIsWhite ? black : grey_trans);
	 glLineWidth(2);

	 glBegin(GL_LINES);	   // really top line!
     glVertex2f(xmin, ymax+fFudge); 
     glVertex2f(xmax + fExt, ymax+fFudge);  
     glEnd();

	 glBegin(GL_LINES);	   // left side
     glVertex2f(xmin+fFudge, yax_qcnlive[E_DX]); 
     glVertex2f(xmin+fFudge, ymax+fFudge);  
     glEnd();

	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DS]);  // top line (ds)
     glVertex2f(xmax + fExt, yax_qcnlive[E_DS]);  
     glEnd();
	 		 
	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DZ]);  // z
     glVertex2f(xmax + fExt, yax_qcnlive[E_DZ]);  
     glEnd();

	 // right line
	 glBegin(GL_LINES);	 
     glVertex2f(xmax + fExt, yax_qcnlive[E_DX]);  // z
     glVertex2f(xmax + fExt, ymax+fFudge);  
     glEnd();
	 		 
	 // bottom section
	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DY]);  // y
     glVertex2f(xmax + fExt, yax_qcnlive[E_DY]); 
     glEnd();

	 glBegin(GL_LINES);	 
     glVertex2f(xmin, yax_qcnlive[E_DX]);  // x
     glVertex2f(xmax + fExt, yax_qcnlive[E_DX]); 
     glEnd();

     draw_tick_marks();

	 glColor4fv((GLfloat*) grey);
	 glRectf(xmin, yax_qcnlive[E_DX], xmax+fExt, ymin);  // bottom rectangle (timer ticks)

	 //right side rectangular region
	 glRectf(xmax, ymax, xmax+fExt, yax_qcnlive[E_DX]);
		 
//    glPopMatrix();    
		
    glFlush();
}

int GetTimerTick()
{
	return g_iTimerTick;
}

void SetTimerTick(const int iTT)
{
	g_iTimerTick = iTT;
}

bool IsWhite()
{
	return g_bIsWhite;
}

void SetWhite(const bool bValue)
{
	g_bIsWhite = bValue;
}

void TimeZoomOut()
{
}

void TimeZoomIn()
{
}

void SensorDataZoomAuto()
{
	if (!g_bAutoScale) g_bAutoScale = true;
}

void SensorDataZoomIn()
{
	if (g_bAutoScale) g_bAutoScale = false;
	if (g_iScaleSigOffset > 0) g_iScaleSigOffset--;
	if (g_iScaleAxesOffset > 0) g_iScaleAxesOffset--;
}

void SensorDataZoomOut()
{
	if (g_bAutoScale) g_bAutoScale = false;
	if (g_iScaleSigOffset < g_iScaleSigMax) g_iScaleSigOffset++;
	if (g_iScaleAxesOffset < g_iScaleAxesMax) g_iScaleAxesOffset++;
}

}  // end namespace qcn_2dplot

