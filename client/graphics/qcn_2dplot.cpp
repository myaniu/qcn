#include "qcn_graphics.h"
#include "qcn_2dplot.h"

using namespace qcn_graphics;

namespace qcn_2dplot {

static short g_iShowSig = 0;  // toggles between 0 (don't show sig plot) and 1 (show sig plot) - used to select index of the 2d array of graph axis placement

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
static const float g_fScaleAxes[6] = { .12f, .36f, 1.8f, 3.0f,  9.8f, 19.6f  };

static bool g_bAutoScale = true; // set to true when want each axes to scale around the last 100 data points (maybe need every second?)

static float g_fMaxAxesCurrent[4]  = { g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0] };  // save each scale level for autoscaling, so it's not jumping all around
static float g_fMinAxesCurrent[4]  = { -g_fScaleAxes[0], -g_fScaleAxes[0], -g_fScaleAxes[0], -g_fScaleAxes[0] };  // save each scale level for autoscaling, so it's not jumping all around
	
static const float cfTransAlpha = 1.000f;
static const float cfVertLabel  = 0.988f;
static const float cfMSSLabel   = 0.051f; // m/s/s label
static const float cfLabelTime[2] = { cfVertLabel/2.0f - 0.1f, 0.005f};

// 2d arrays for the points for drawing - 2d because first element is without the sig plot, 2nd element (array) is with the sig plot

// the Y/height in 2d view to draw the axis label (i.e. X-Axis, Y-Axis etc)
static const float cfAxesLabelX       = 1.051f;
static const float cfAxesLabelY[2][4] = { { 0.140f, .360f, .590f, .790f }, { 0.124f, .284f, .444f, .584f } };

// the base height for each E_DS/X/Y/Z level i.e. for text labels
static const float cfBaseScale[2][4] = { { .072f, .291f, .511f, .793f }, { 0.071f, .235f, .400f, .565f } };

// the offsets to draw each horizontal axis text value within an E_DX/Y/Z/S region
static const float cfAxesOffsetY[2][7] = { { .0f, .028f, .065f, .103f, .139f, .175f, .201f }, { .0f, .021f, .049f, .077f, .104f, .131f, .151f } };

void ShowSigPlot(bool bShow)
{
	g_iShowSig = bShow ? 1 : 0;
}
	
bool IsSigPlot()
{
	return (bool) (g_iShowSig == 1);
}
	
void draw_text_sensor_axis(int iAxis)
{
	char cbuf[10];
	if (g_fMaxAxesCurrent[iAxis] == SAC_NULL_FLOAT || g_fMinAxesCurrent[iAxis] == -1.0f * SAC_NULL_FLOAT) return;
	float fIncrement = (g_fMaxAxesCurrent[iAxis] - g_fMinAxesCurrent[iAxis]) / 6.0f;
	float fFudge = 0.0; //(iAxis == E_DS) ? .005 : 0.00; // bump up sig axis
	for (int i = 0; i <= 6; i++) {
		sprintf(cbuf, "%+5.3f", g_fMinAxesCurrent[iAxis] + (fIncrement * (float) i) );
	    TTFont::ttf_render_string(cfTransAlpha, cfVertLabel, cfBaseScale[g_iShowSig][iAxis] + cfAxesOffsetY[g_iShowSig][i] + fFudge, 0, MSG_SIZE_TINY, g_bIsWhite ? black : grey_trans, TTF_MONOSPACE, cbuf);
	}
}

bool canDrawTick(const float& fWhere, const bool bText)
{
	return (bool)(fWhere > (bText ? 0.02 : xax_2d[0] + 2.));
}
	
void draw_tick_marks()
{  // draw vertical blue lines every 1/10/60/600 seconds depending on view size
		// note the labels underneath are drawn in draw_text_plot
		// show the time markers, if any
		glPushMatrix();
		for (int i = 0; i < g_iTimeCtr; i++) {
			if (lTimeLast[i] > 0.0f) { // there's a marker to place here
				float fWhere;
				if (g_eView == VIEW_PLOT_2D) {
					fWhere = xax_2d[0] + ( ((float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax_2d[1]-xax_2d[0]));
				}
				else  {
					fWhere = xax[0] + ( ((float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE) * (xax[1]-xax[0]));
				}
				if (!canDrawTick(fWhere, false)) continue; // off the left edge
				
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
	
void draw_text() 
{
   // draw text on top
   mode_unshaded();
	qcn_graphics::mode_ortho_qcn();

#ifdef QCNLIVE  // check for "make quake" feature
	draw_makequake_message(); // draw any msg if we're doing the quake stuff
#endif
	
	
   // now draw time text at the bottom
   char strTime[16];
   //TTFont::ttf_render_string(qcn_graphics::cfTextAlpha, fWhere, Y_TRIGGER_LAST[0] - 3.0f, 0, 800, blue, TTF_ARIAL, (const char*) strTime);
    for (int i = 0; i < g_iTimeCtr; i++) {
       if (lTimeLast[i] > 0.0f) { // there's a marker to place here
	     float fWhere = (float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE;
		if (!canDrawTick(fWhere, true)) continue; // off the left edge

		 // note the immediate if below - if timer ticks are far apart don't bother showing seconds
		 qcn_util::dtime_to_string(lTimeLast[i], (g_iTimerTick > 5 ? 'm' : 'h'), strTime);
		 TTFont::ttf_render_string(qcn_graphics::cfTextAlpha, fWhere - (g_iTimerTick > 5 ? 0.038f : 0.042f), 0.036f, 0.0f, 
			 MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TTF_ARIAL, (const char*) strTime);
	   }
	}

/*
#ifdef _DEBUG
	sprintf(strTime, "sampsize=%ld", sm->lSampleSize);
    TTFont::ttf_render_string(qcn_graphics::cfTextAlpha, .3f, cfAxesLabelY[g_iShowSig][0], 0.0f, MSG_SIZE_SMALL, red, TTF_ARIAL, (const char*) strTime);
#endif
*/	

/*
 #ifdef _DEBUG_QCNLIVE
	sprintf(strTime, "%+6.3f %+6.3f", qcn_graphics::g_fmin[0], qcn_graphics::g_fmax[0]);
	TTFont::ttf_render_string(qcn_graphics::cfTextAlpha, .1f, cfAxesLabelY[g_iShowSig][0], 0.0f, MSG_SIZE_SMALL, red, TTF_ARIAL, (const char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", qcn_graphics::g_fmin[1], qcn_graphics::g_fmax[1]);
    TTFont::ttf_render_string(qcn_graphics::cfTextAlpha, .1f, cfAxesLabelY[g_iShowSig][1], 0.0f, MSG_SIZE_SMALL, red, TTF_ARIAL, (const char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", qcn_graphics::g_fmin[2], qcn_graphics::g_fmax[2]);
    TTFont::ttf_render_string(qcn_graphics::cfTextAlpha, .1f, cfAxesLabelY[g_iShowSig][2], 0.0f, MSG_SIZE_SMALL, red, TTF_ARIAL, (const char*) strTime);
	if (g_iShowSig) {
	  sprintf(strTime, "%+6.3f %+6.3f", qcn_graphics::g_fmin[3], qcn_graphics::g_fmax[3]);
      TTFont::ttf_render_string(qcn_graphics::cfTextAlpha, .1f, cfAxesLabelY[g_iShowSig][3], 0.0f, MSG_SIZE_SMALL, red, TTF_ARIAL, (const char*) strTime);
	}
#endif
 */

	// labels for each axis

	if (g_iShowSig) {
		TTFont::ttf_render_string(cfTransAlpha, cfAxesLabelX, cfAxesLabelY[g_iShowSig][E_DS], 0, MSG_SIZE_NORMAL, red, TTF_ARIAL, "Significance", 90.0f);
	}
	TTFont::ttf_render_string(cfTransAlpha, cfAxesLabelX, cfAxesLabelY[g_iShowSig][E_DZ], 0, MSG_SIZE_NORMAL, blue, TTF_ARIAL, "Z Axis", 90.0f);
    TTFont::ttf_render_string(cfTransAlpha, cfAxesLabelX, cfAxesLabelY[g_iShowSig][E_DY], 0, MSG_SIZE_NORMAL, orange, TTF_ARIAL, "Y Axis", 90.0f);
    TTFont::ttf_render_string(cfTransAlpha, cfAxesLabelX, cfAxesLabelY[g_iShowSig][E_DX], 0, MSG_SIZE_NORMAL, green, TTF_ARIAL, "X Axis", 90.0f);

	// labels for significance
	if (g_iShowSig) {
		draw_text_sensor_axis(E_DS);
	}

	// labels for Z axis
	draw_text_sensor_axis(E_DZ);

	// labels for Y axis
	draw_text_sensor_axis(E_DY);

	// labels for X axis  084
	draw_text_sensor_axis(E_DX);

	// units label (meters per second per second
    TTFont::ttf_render_string(cfTransAlpha, cfVertLabel, cfMSSLabel, 0, MSG_SIZE_SMALL, g_bIsWhite ? black : grey_trans, TTF_MONOSPACE, "m/s/s");

    // time label

	switch(qcn_graphics::GetTimeWindowWidth()) {
	case 10:
			TTFont::ttf_render_string(qcn_graphics::g_alphaText, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TTF_ARIAL, "Time (UTC) - 10 Second Window");
		break;
	case 60:
        TTFont::ttf_render_string(qcn_graphics::g_alphaText, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TTF_ARIAL, "Time (UTC) - 1 Minute Window");
		break;
	case 600:
        TTFont::ttf_render_string(qcn_graphics::g_alphaText, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TTF_ARIAL, "Time (UTC) - 10 Minute Window");
		break;
	case 3600:
        TTFont::ttf_render_string(qcn_graphics::g_alphaText, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TTF_ARIAL, "Time (UTC) - 1 Hour Window");
		break;
	default:
        TTFont::ttf_render_string(qcn_graphics::g_alphaText, cfLabelTime[0], cfLabelTime[1], 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TTF_ARIAL, "Time (UTC)");
	}

    draw_text_sensor();

	/*
	char strdiff[32];
	sprintf(strdiff, "%f", fDiff2D);
	TTFont::ttf_render_string(qcn_graphics::g_alphaText, .2, .2, 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TTF_ARIAL, strdiff);
	*/

/*
	char bufout[64];
	sprintf(bufout, "%.2f  %.2f  %.2f  %.2f", g_fAvg[0], g_fAvg[1], g_fAvg[2], g_fAvg[3]);
	TTFont::ttf_render_string(qcn_graphics::g_alphaText, 0.04f, 0.1f, 0.0f, MSG_SIZE_SMALL, light_blue, TTF_ARIAL, bufout);
*/
	
	ortho_done();
}

void draw_plot_boxes(const float& xmin, const float& xmax, const float& ymin, const float& ymax, const float& fExt, const float& fFudge)
{
		// draw boxes around the plots
		
		glColor4fv((GLfloat*) g_bIsWhite ? black : grey_trans);
		glLineWidth(2);
		
		glBegin(GL_LINES);	   // really top line!
		glVertex2f(xmin, ymax+fFudge); 
		glVertex2f(xmax + fExt, ymax+fFudge);  
		glEnd();
		
		glBegin(GL_LINES);	   // left side
		glVertex2f(xmin+fFudge, yax_2d[g_iShowSig][E_DX]); 
		glVertex2f(xmin+fFudge, ymax+fFudge);  
		glEnd();

		if (g_iShowSig) {
		  glBegin(GL_LINES);	 
		  glVertex2f(xmin, yax_2d[g_iShowSig][E_DS]);  // top line (ds)
		  glVertex2f(xmax + fExt, yax_2d[g_iShowSig][E_DS]);  
		  glEnd();
		}
		
		glBegin(GL_LINES);	 
		glVertex2f(xmin, yax_2d[g_iShowSig][E_DZ]);  // z
		glVertex2f(xmax + fExt, yax_2d[g_iShowSig][E_DZ]);  
		glEnd();
		
		// right line
		glBegin(GL_LINES);	 
		glVertex2f(xmax + fExt, yax_2d[g_iShowSig][E_DX]);  // z
		glVertex2f(xmax + fExt, ymax+fFudge);  
		glEnd();
		
		// bottom section
		glBegin(GL_LINES);	 
		glVertex2f(xmin, yax_2d[g_iShowSig][E_DY]);  // y
		glVertex2f(xmax + fExt, yax_2d[g_iShowSig][E_DY]); 
		glEnd();
		
		glBegin(GL_LINES);	 
		glVertex2f(xmin, yax_2d[g_iShowSig][E_DX]);  // x
		glVertex2f(xmax + fExt, yax_2d[g_iShowSig][E_DX]); 
		glEnd();
}
	

bool CalcYPlot(const float& fVal, const float& fAvg, const int& ee, float&  myY)
{
	//const float fHeight = (g_iShowSig ? 15.0f : 20.0f) + (ee == E_DS ? 0.5f : 0.0f);   // height changes based on how many values plotting, if if showing sig pad .5
	const float fHeight = (g_iShowSig ? 15.0f : 20.0f);   // height changes based on how many values plotting, if if showing sig pad .5
	
    myY = yax_2d[g_iShowSig][ee]
	     + ( fHeight * ( (fVal - g_fMinAxesCurrent[ee])
                                   / (g_fMaxAxesCurrent[ee] - g_fMinAxesCurrent[ee] ) )  )
	     // + (ee == E_DS ? 0.5f : 0.0f) 
	   ;

    //if (fdata[i] != 0.0f) { // suppress 0 values and check data ranges fit
    if ( fVal == SAC_NULL_FLOAT || (fVal == 0. && ee == E_DS)) { // invalid, suppress
       myY = SAC_NULL_FLOAT;
       return false;
    }
    else if ( fVal > g_fMaxAxesCurrent[ee] ) { // max limit
       myY = yax_2d[g_iShowSig][ee] + fHeight;
    }
    else if ( fVal < g_fMinAxesCurrent[ee] ) { // min limit
       myY = yax_2d[g_iShowSig][ee];
    }
    return true;
}

void draw_plot() 
{
    float* fdata = NULL;

    // each plot section is 15 units high if g_iSigShow = 1 (x/y/z/s needs to fit in 60 units), or 20 high if not showing siz (x/y/z only)

	//static int iFrameCounter = 0;
	//static float fMeanLast = 0.0f, fStdDevLast, fVarianceLast;  // preserve state of last mean etc
	//static float fMean = 0.0f, fStdDev = 0.0f, fVariance = 0.0f;

    float xmin = xax_2d[0] - 0.1f;
    float xmax = xax_2d[1] + 0.1f;
    float ymin = yax_2d[g_iShowSig][E_DX] - 7.0f;
    float ymax = yax_2d[g_iShowSig][4]; // + 15.0f;
    float yPen[4] = { SAC_NULL_FLOAT, SAC_NULL_FLOAT, SAC_NULL_FLOAT, SAC_NULL_FLOAT }; // save "pen" position i.e. last point on plot

    float x1, y1; // temp values to compare ranges for plotting
    long lStart, lEnd;

    if (!sm) return; // not much point in continuing if shmem isn't setup!

    init_camera(viewpoint_distance[g_eView], 45.0f);
    init_lights();

    // should just be simple draw each graph in 2D using the info in dx/dy/dz/ds?
    
  //  glPushMatrix();
    mode_unshaded();

	glEnable (GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	
	qcn_graphics::scale_screen_qcn(g_width, g_height);  // boinc api/gutil function to get good aspect ratio
	const int iMaxArray = g_iShowSig ? E_DS : E_DZ;  // don't bother plotting E_DS if they aren't showing sig

    for (int ee = E_DX; ee <= iMaxArray; ee++)  {
         switch(ee) {
            case E_DX:  fdata = (float*) aryg[E_DX]; break;
            case E_DY:  fdata = (float*) aryg[E_DY]; break;
            case E_DZ:  fdata = (float*) aryg[E_DZ]; break;
            case E_DS:  fdata = (float*) aryg[E_DS]; break;
         }

         // first draw the axes
		 // draw 2 above & 2 below and one in the middle
		const float yfactor = g_iShowSig ? 2.50f : 3.333f, xfactor = 0.00f, yfudge = g_iShowSig ? 7.5f : 10.0f;  // note that depending on how many regions x/y/z/s showing, the fudge factor changes!
		 for (int j = -2; j <= 3; j++) {
//#ifndef _DEBUG  // to suppress lines
			 glLineWidth(1);

			 glColor4fv(grey);
			 glBegin(GL_LINES);

/*
			 if (ee == E_DS) {
				 glVertex2f(xax_2d[0], yax_2d[g_iShowSig][ee] + .5f + (yfactor * (float) (j+2)));
				 glVertex2f(xax_2d[1] + xfactor, yax_2d[g_iShowSig][ee] + .5f + (yfactor * (float) (j+2)));
			 }
			 else { 
*/
				 if (j<3) { // only sig E_DS get's the j=3 line
					 glVertex2f(xax_2d[0], yax_2d[g_iShowSig][ee] + yfudge + (yfactor * (float) j));
					 glVertex2f(xax_2d[1] + xfactor, yax_2d[g_iShowSig][ee] + yfudge + (yfactor * (float) j));
				 }
//			 }

			 glEnd();
//#endif

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
					//g_fMaxAxesCurrent[ee] = ((qcn_graphics::g_fmax[ee] == SAC_NULL_FLOAT || abs(qcn_graphics::g_fmax[ee] - qcn_graphics::g_fmin[ee]) < 1.e-8f) ? 1.f : qcn_graphics::g_fmax[ee]);  // save each scale level for autoscaling, so it's not jumping all around
					//g_fMinAxesCurrent[ee] = ((qcn_graphics::g_fmin[ee] == -1.0f * SAC_NULL_FLOAT || abs(qcn_graphics::g_fmax[ee] - qcn_graphics::g_fmin[ee]) < 1.e-8f) ? 0.0f : qcn_graphics::g_fmin[ee]);  // save each scale level for autoscaling, so it's not jumping all around
					g_fMaxAxesCurrent[ee] = ((qcn_graphics::g_fmax[ee] == SAC_NULL_FLOAT) ? 1.0f : qcn_graphics::g_fmax[ee]);  // save each scale level for autoscaling, so it's not jumping all around
					g_fMinAxesCurrent[ee] = ((qcn_graphics::g_fmin[ee] == -1.0f * SAC_NULL_FLOAT) ? 0.0f : qcn_graphics::g_fmin[ee]);  // save each scale level for autoscaling, so it's not jumping all around
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
				 x1 = xax_2d[0] + (((float) i / (float) PLOT_ARRAY_SIZE) * (xax_2d[1]-xax_2d[0]));
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
			x1 = xax_2d[0] + (xax_2d[1]-xax_2d[0]);
			glBegin(GL_TRIANGLE_FAN);
			   glVertex2f(x1, yPen[ee]);
			   glVertex2f(x1 + (cos(fAngle) * fRadius), yPen[ee] + (sin(fAngle) * fRadius));
			   glVertex2f(x1 + (cos(-fAngle) * fRadius), yPen[ee] + (sin(-fAngle) * fRadius));
			glEnd();
		} // colored pointer
	}

//#ifndef _DEBUG  // suppress line/box drawing
	const float fExt = 7.05f;
	const float fFudge = 0.00f;
	draw_plot_boxes(xmin, xmax, ymin, ymax, fExt, fFudge);
	draw_tick_marks();
	glColor4fv((GLfloat*) grey);
	glRectf(xmin, yax_2d[g_iShowSig][E_DX], xmax+fExt, ymin);  // bottom rectangle (timer ticks)
	
	//right side rectangular region
	glRectf(xmax, ymax, xmax+fExt, yax_2d[g_iShowSig][E_DX]);
//#endif
	
		 
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

#ifdef QCNLIVE

// just an overlay for the countdown etc
void draw_makequake_message()
{
	static int iSize[2] = {0,0};
	int iCheck[2] = {0,0};
	if (!qcn_graphics::g_MakeQuake.bActive) return;

	int iLen = strlen(g_MakeQuake.strName);
	char strApos[3];
	char strMsg[_MAX_PATH];
	memset(strApos, 0x00, sizeof(char) * 3);
	memset(strMsg, 0x00, sizeof(char) * _MAX_PATH);

	// get the right suffix i.e. apostrophe if name ends in s else 's
	if (iLen > 0 && g_MakeQuake.strName[iLen-1] == 's') strcpy(strApos, "'");
	else strcpy(strApos, "'s");
				
	sprintf(strMsg, "%s%s Earthquake", g_MakeQuake.strName, strApos);

	TTFont::ttf_render_string(1.0, -0.022, 0.10, 0, MSG_SIZE_BIG, black, TTF_ARIAL, strMsg, 90);

	// draw countdown msg
	if (qcn_graphics::g_MakeQuake.iCountdown > 0) {
		if (iCheck[0] != qcn_graphics::g_MakeQuake.iCountdown) {
			iSize[0] = 0;
			iCheck[0] = qcn_graphics::g_MakeQuake.iCountdown;
		}
		TTFont::ttf_render_string(1.0, 0.0, 0.67, 0, MSG_SIZE_BIG / 4, red, TTF_ARIAL, "Countdown!");
		sprintf(strApos, "%d", iCheck[0]);
		if ( iSize[0] > 29 ) iSize[0] = 29;
		TTFont::ttf_render_string(1.0, 0.3, 0.10, 0, (float) MSG_SIZE_BIG / (30 - iSize[0]++), red, TTF_ARIAL, strApos);
		return; 
	}
	
	// draw monitoring msg
	if (qcn_graphics::g_MakeQuake.iTime > 0) {
		if (iCheck[1] != qcn_graphics::g_MakeQuake.iTime) {
			iSize[1] = 0;
			iCheck[1] = qcn_graphics::g_MakeQuake.iTime;
		}
		sprintf(strMsg, "Monitoring.......%ds", iCheck[1]);
		TTFont::ttf_render_string(1.0, 0.0, 0.67, 0, MSG_SIZE_BIG / 2, red, TTF_ARIAL, strMsg);
		return; 
	}
	
		
}


#endif  // QCNLive makequake stuff


}  // end namespace qcn_2dplot
