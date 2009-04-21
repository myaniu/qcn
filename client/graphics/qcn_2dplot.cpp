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

static const float g_fScaleSig[6]  = { .3f, 1.0f, 2.5f, 5.0f, 10.0f, 20.0f }; // default scale for sig is 20
static const float g_fScaleAxes[6] = { .3f, 1.0f, 2.0f, 4.9f,  9.8f, 19.6f };

//static bool g_bAutoCenter = true; // automatically center the plots
static bool g_bAutoScale = true; // set to true when want each axes to scale around the last 100 data points (maybe need every second?)

static float g_fMaxAxesCurrent[4]  = { g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0] };  // save each scale level for autoscaling, so it's not jumping all around
static float g_fMinAxesCurrent[4]  = { -g_fScaleAxes[0], -g_fScaleAxes[0], -g_fScaleAxes[0], -g_fScaleAxes[0] };  // save each scale level for autoscaling, so it's not jumping all around

static const float fTransAlpha = 0.100f;
static const float fAxisLabel  = 1.061f;
static const float fVertLabel  = 0.988f;
static const float fYOffset    = 0.016f;
static const float fMSSLabel   = 0.050f; // m/s/s label
static const float fSigOffset[7] = { .562f, .588f, .616f, .643f, .670f, .697f, .719f };

static const float fAxesLabel[4] = { 0.124f, .284f, .444f, .584f };

static const float fBaseScale[4] = { 0.068f, .232f, .397f, .562f };
static const float fAxesOffset[7] = { .0f, .021f, .049f, .077f, .104f, .131f, .151f };

void draw_text_sensor_axis(int iAxis)
{
	char cbuf[10];
	if (g_fMaxAxesCurrent[iAxis] == SAC_NULL_FLOAT || g_fMinAxesCurrent[iAxis] == -1.0f * SAC_NULL_FLOAT) return;
	float fIncrement = (g_fMaxAxesCurrent[iAxis] - g_fMinAxesCurrent[iAxis]) / 6.0f;
	for (int i = 0; i <= 6; i++) {
		sprintf(cbuf, "%+6.3f", g_fMinAxesCurrent[iAxis] + (fIncrement * (float) i) );
	    txf_render_string(fTransAlpha, fVertLabel, fBaseScale[iAxis] + fAxesOffset[i], 0, MSG_SIZE_SMALL, g_bIsWhite ? black : grey_trans, TXF_COURIER_BOLD, cbuf);
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
    txf_render_string(.1f, .1f, fAxesLabel[0], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", g_fMin[1], g_fMax[1]);
    txf_render_string(.1f, .1f, fAxesLabel[1], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", g_fMin[2], g_fMax[2]);
    txf_render_string(.1f, .1f, fAxesLabel[2], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+6.3f %+6.3f", g_fMin[3], g_fMax[3]);
    txf_render_string(.1f, .1f, fAxesLabel[3], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
#endif
	*/

	// labels for each axis

	txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DS], 0, MSG_SIZE_NORMAL, red, TXF_HELVETICA, "Significance", 90.0f);
    txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DZ], 0, MSG_SIZE_NORMAL, blue, TXF_HELVETICA, "Z Axis", 90.0f);
    txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DY], 0, MSG_SIZE_NORMAL, orange, TXF_HELVETICA, "Y Axis", 90.0f);
    txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DX], 0, MSG_SIZE_NORMAL, green, TXF_HELVETICA, "X Axis", 90.0f);

	// labels for significance
	draw_text_sensor_axis(E_DS);

	// labels for Z axis
	draw_text_sensor_axis(E_DZ);

	// labels for Y axis
	draw_text_sensor_axis(E_DY);

	// labels for X axis  084
	draw_text_sensor_axis(E_DX);

	// units label (meters per second per second
    txf_render_string(fTransAlpha, fVertLabel, fMSSLabel, 0, MSG_SIZE_SMALL, g_bIsWhite ? black : grey_trans, TXF_COURIER_BOLD, " m/s/s");

    // time label
	switch(qcn_graphics::key_winsize) {
	case 0:
        txf_render_string(fTransAlpha, fVertLabel/2, 0.005f, 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 10 Second Window");
		break;
	case 1:
        txf_render_string(fTransAlpha, fVertLabel/2, 0.005f, 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 1 Minute Window");
		break;
	case 2:
        txf_render_string(fTransAlpha, fVertLabel/2, 0.005f, 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 10 Minute Window");
		break;
	case 3:
        txf_render_string(fTransAlpha, fVertLabel/2, 0.005f, 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC) - 1 Hour Window");
		break;
	default:
        txf_render_string(fTransAlpha, fVertLabel/2, 0.005f, 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, "Time (UTC)");
	}

    draw_text_sensor();

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
	static float fMeanLast = 0.0f, fStdDevLast, fVarianceLast;  // preserve state of last mean etc
	static float fMean = 0.0f, fStdDev = 0.0f, fVariance = 0.0f;

	float xmin = xax_qcnlive[0] - 0.1f;
	float xmax = xax_qcnlive[1] + 0.1f;
	float ymin = yax_qcnlive[E_DX] - 7.0f;
    float ymax = yax_qcnlive[4]; // + 15.0f;
    float x1, y1; // temp values to compare ranges for plotting
	//float fAvg;
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
			 glColor4fv(ee == E_DY ? orange : colorsPlot[ee]);  // set the color for data - CMC note the orange substitution for yellow on the Y
			 glLineWidth(2.0f);
			 glBegin(GL_LINE_STRIP);

			 // get the scale for each axis
			 			 
			 if (g_bAutoScale) { // compute fScale Factor from last 100 pts
			   //if (!(iFrameCounter % 20)) {
			     // if not enough points just use what we can
			     //if (sm->lOffset < PLOT_ARRAY_SIZE && sm->t0[MAXI-1] == 0.0f) {
		    	//	lStart = PLOT_ARRAY_SIZE-11;
				//	lEnd = PLOT_ARRAY_SIZE-1;
				 //}
	    		 //else  { 
		    		//lStart = PLOT_ARRAY_SIZE-101;
		    		lStart = 0;
					lEnd = PLOT_ARRAY_SIZE-1;
				 //}
				//iFrameCounter = 0;
					/* obsolete - g_fmin & fmax calculated in qcn_graphics namespace
				if (qcn_util::ComputeMeanStdDevVarianceKnuth((const float*) fdata, PLOT_ARRAY_SIZE, lStart, lEnd, 
				  &fMean, &fStdDev, &fVariance, &g_fMin[ee], &g_fMax[ee], true, true)) {
				    if (fMean != 0.0f && fabs((fMean - fMeanLast)/fMean) > 0.20f) { // mean's differ so reset
					    fMeanLast = fMean;
						fStdDevLast = fStdDev;
						fVarianceLast = fVariance;
					}
					else {
					    fMean = fMeanLast;
						fStdDev = fStdDevLast;
						fVariance = fVarianceLast;
					}
					// make the scale to fit 2 std dev above & below the mean, which is the center
					//g_fMaxAxesCurrent[ee] = fMean + (2.0f * fStdDev);  // save each scale level for autoscaling, so it's not jumping all around
					//g_fMinAxesCurrent[ee] = fMean - (2.0f * fStdDev);
					g_fMaxAxesCurrent[ee] = g_fMax[ee];  // save each scale level for autoscaling, so it's not jumping all around
					g_fMinAxesCurrent[ee] = g_fMin[ee];
					if (ee == E_DS) {
					    g_fMin[ee] = 0.0f;  // force min to always be 0 for significance
						g_fMinAxesCurrent[ee] = 0.0f;
					}
				  */
					g_fMaxAxesCurrent[ee] = qcn_graphics::g_fmax[ee] == SAC_NULL_FLOAT ? 1.0f : qcn_graphics::g_fmax[ee];  // save each scale level for autoscaling, so it's not jumping all around
					g_fMinAxesCurrent[ee] = qcn_graphics::g_fmin[ee] == SAC_NULL_FLOAT ? 0.0f : qcn_graphics::g_fmin[ee]; 
					if (ee == E_DS) {
						g_fMinAxesCurrent[ee] = 0.0f;
					}
				/*}
				else {
					g_fMaxAxesCurrent[ee] = ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] );
					g_fMinAxesCurrent[ee] = ( ee == E_DS ? 0.0f : -g_fScaleAxes[g_iScaleAxesOffset] );
				}*/
			  //}  // iFrameCounter
			 }
			 else {
					g_fMaxAxesCurrent[ee] = ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] );
					g_fMinAxesCurrent[ee] = ( ee == E_DS ? 0.0f : -g_fScaleAxes[g_iScaleAxesOffset] );
			 }

			 if ((g_fMaxAxesCurrent[ee] - g_fMinAxesCurrent[ee]) == 0.0f) {
                             g_fMaxAxesCurrent[ee] = 1.0f;
                             g_fMinAxesCurrent[ee] = 0.0f;  // avoid divide by zero
			 }
                         //fAvg = (g_fMax[ee] - g_fMin[ee]) / 2.0f;
			 for (int i=0; i<PLOT_ARRAY_SIZE; i++) {
				 x1 = xax_qcnlive[0] + (((float) i / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0]));
				 y1 = yax_qcnlive[ee] + (ee == E_DS ? 0.5f : 0.0f) + ( 15.0f * ( (fdata[i] - g_fMinAxesCurrent[ee]) / (g_fMaxAxesCurrent[ee] - g_fMinAxesCurrent[ee] ) )  );

				 if (fdata[i] != 0.0f && (y1 - yax_qcnlive[ee]) <= (ee==E_DS ? 16.0f : 15.4f) && (y1 - yax_qcnlive[ee]) >= -.2f) { // don't plot out of range
				     glVertex2f(x1, y1);
			     }
			     else { // close line segment
				 	 glEnd();
					 glBegin(GL_LINE_STRIP);  // start new line seg fromout of range data
				 }
			 }
	 	     glEnd();
		 }
	     //iFrameCounter++; // bump up the frame ctr
		 
         // x/y/z data points are +/- 19.6 m/s2 -- significance is 0-? make it 0-10		 		 

/*
		 float fmaxfactor;  // scale y -axes
		 if (fabs(l_fmax[ee]) > fabs(l_fmin[ee]))
		      fmaxfactor = fabs(l_fmax[ee]);
         else
		      fmaxfactor = fabs(l_fmin[ee]);
		 
		 if (fmaxfactor == 0) 
		     fmaxfactor = 1.0f;
		 else
		     fmaxfactor = MAX_PLOT_HEIGHT_QCNLIVE / fmaxfactor;
*/
		 // plot a "colored pointer" at the end for ease of seeing current value?
		if (fdata[PLOT_ARRAY_SIZE-1] != 0.0f)  {
			x1 = xax_qcnlive[0] + (xax_qcnlive[1]-xax_qcnlive[0]);
			y1 = yax_qcnlive[ee] + (ee == E_DS ? 0.5f : 0.0f) + ( 15.0f * ( (fdata[PLOT_ARRAY_SIZE-1] - g_fMinAxesCurrent[ee]) / (g_fMaxAxesCurrent[ee] - g_fMinAxesCurrent[ee] ) )  );

			if (fabs(y1 - yax_qcnlive[ee]) < (ee==E_DS ? 16.0f : 15.4f) && (y1 - yax_qcnlive[ee]) >= -.2f) { // don't plot out of range
				const float fRadius = 1.4f;
				float fAngle = PI/8.0f;
				glBegin(GL_TRIANGLE_FAN);
				glVertex2f(x1, y1);
				glVertex2f(x1 + (cos(fAngle) * fRadius), y1 + (sin(fAngle) * fRadius));
				glVertex2f(x1 + (cos(-fAngle) * fRadius), y1 + (sin(-fAngle) * fRadius));
				glEnd();
			}
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

