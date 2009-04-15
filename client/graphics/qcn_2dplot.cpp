#include "qcn_graphics.h"
#include "qcn_2dplot.h"

using namespace qcn_graphics;

namespace qcn_2dplot {

int g_TimerTick = 5; // seconds for each timer point

#ifdef QCNLIVE
	bool g_bIsWhite = true;
#else
	bool g_bIsWhite = false;
#endif

static const int g_iScaleSigMax  = 5;
static const int g_iScaleAxesMax = 5;

static int g_iScaleSigOffset  = 0;
static int g_iScaleAxesOffset = 0;

static const float g_fScaleSig[6]  = { .3f, 1.0f, 2.5f, 5.0f, 10.0f, 20.0f }; // default scale for sig is 20
static const float g_fScaleAxes[6] = { .3f, 1.0f, 2.0f, 4.9f,  9.8f, 19.6f };

static bool g_bAutoCenter = true; // automatically center the plots
static bool g_bAutoScale = true; // set to true when want each axes to scale around the last 100 data points (maybe need every second?)
static float g_fScaleAxesCurrent[4]  = { g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0], g_fScaleAxes[0] };  // save each scale level for autoscaling, so it's not jumping all around
static float g_fCenterAxesCurrent[4] = { 0.0f, 0.0f, 0.0f, 0.0f };  // save each scale level for autoscaling, so it's not jumping all around

static const float fTransAlpha = 0.100f;
static const float fAxisLabel  = 1.061f;
static const float fVertLabel  = 0.988f;
static const float fYOffset    = 0.016f;
static const float fMSSLabel   = 0.050f; // m/s/s label
static const float fSigOffset[7] = { .562f, .588f, .616f, .643f, .670f, .697f, .719f };

static const float fAxesLabel[4] = { 0.124f, .284f, .444f, .584f };

static const float fBaseScale[4] = { 0.068f, .232f, .397f, .562f };
static const float fAxesOffset[7] = { .0f, .021f, .049f, .077f, .104f, .131f, .151f };

static float fMin[4], fMax[4]; // max & min for each axis visible on the plot


void draw_text_sig_axis()
{
	char cbuf[10];
	for (int i = 0; i <= 6; i++) {
		sprintf(cbuf, "%5.2f", (float) i / 6.0f * g_fScaleAxesCurrent[E_DS]);
	    txf_render_string(fTransAlpha, fVertLabel, fSigOffset[i], 0, MSG_SIZE_SMALL, g_bIsWhite ? black : grey_trans, TXF_COURIER_BOLD, cbuf);
	}
}

void draw_text_sensor_axis(int iAxis)
{
	char cbuf[10];
	//for (int i = -3; i <= 3; i++) {
		//sprintf(cbuf, "%+5.2f", g_fCenterAxesCurrent[iAxis] + ((float) i / 3.0f * g_fScaleAxesCurrent[iAxis]));
	for (int i = 0; i <= 6; i++) {
		sprintf(cbuf, "%+5.2f", fMin[iAxis] + ((fMax[iAxis] - fMin[iAxis]) * (float) i) / 6.0f);
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
       if (lTimeLast[i] > 0) { // there's a marker to place here
	     float fWhere = (float) (lTimeLastOffset[i]) / (float) PLOT_ARRAY_SIZE;
		 qcn_util::dtime_to_string((const double) lTimeLast[i], 'h', strTime);
		 txf_render_string(.1f, fWhere - 0.042f, 0.030f, 0.0f, MSG_SIZE_SMALL, g_bIsWhite ? light_blue : grey_trans, TXF_HELVETICA, (char*) strTime);
	   }
	}

#ifdef _DEBUG
	sprintf(strTime, "%+5.2f %+5.2f", fMin[0], fMax[0]);
    txf_render_string(.1f, .1f, fAxesLabel[0], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+5.2f %+5.2f", fMin[1], fMax[1]);
    txf_render_string(.1f, .1f, fAxesLabel[1], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+5.2f %+5.2f", fMin[2], fMax[2]);
    txf_render_string(.1f, .1f, fAxesLabel[2], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
	sprintf(strTime, "%+5.2f %+5.2f", fMin[3], fMax[3]);
    txf_render_string(.1f, .1f, fAxesLabel[3], 0.0f, MSG_SIZE_SMALL, red, TXF_HELVETICA, (char*) strTime);
#endif

	// labels for each axis

	txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DS], 0, MSG_SIZE_NORMAL, red, TXF_HELVETICA, "Significance", 90.0f);
    txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DZ], 0, MSG_SIZE_NORMAL, blue, TXF_HELVETICA, "Z Axis", 90.0f);
    txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DY], 0, MSG_SIZE_NORMAL, orange, TXF_HELVETICA, "Y Axis", 90.0f);
    txf_render_string(fTransAlpha, fAxisLabel, fAxesLabel[E_DX], 0, MSG_SIZE_NORMAL, green, TXF_HELVETICA, "X Axis", 90.0f);

	// labels for significance
	//draw_text_sig_axis();
	draw_text_sensor_axis(E_DS);

	// labels for Z axis
	draw_text_sensor_axis(E_DZ);

	// labels for Y axis
	draw_text_sensor_axis(E_DY);

	// labels for X axis  084
	draw_text_sensor_axis(E_DX);

	// units label (meters per second per second
    txf_render_string(fTransAlpha, fVertLabel, fMSSLabel, 0, MSG_SIZE_SMALL, g_bIsWhite ? black : grey_trans, TXF_COURIER_BOLD, " m/s/s");

    draw_text_sensor();

	ortho_done();
}

void draw_tick_marks()
{  // draw vertical blue lines every 1/10/60/600 seconds depending on view size
	  // note the labels underneath are drawn in draw_text_plot_qcnlive
		 
    // show the time markers, if any
    glPushMatrix();
    for (int i = 0; i < g_iTimeCtr; i++) {
       if (lTimeLast[i] > 0) { // there's a marker to place here
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

	static int iFrameCounter = 0L;

	float xmin = xax_qcnlive[0] - 0.1f;
	float xmax = xax_qcnlive[1] + 0.1f;
	float ymin = yax_qcnlive[E_DX] - 7.0f;
    float ymax = yax_qcnlive[4]; // + 15.0f;
    float x1, y1; // temp values to compare ranges for plotting
	float fAvg;

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
			    float fMean, fStdDev, fVariance;
				if (qcn_util::ComputeMeanStdDevVarianceKnuth((const float*) fdata, PLOT_ARRAY_SIZE, PLOT_ARRAY_SIZE-100, PLOT_ARRAY_SIZE-1, 
				  &fMean, &fStdDev, &fVariance, &fMin[ee], &fMax[ee])) {
					// make the scale to fit 4 std dev above & below the mean, which is the center
					//g_fScaleAxesCurrent[ee] = fMean + (2.0f * fStdDev);  // save each scale level for autoscaling, so it's not jumping all around
					//g_fCenterAxesCurrent[ee] = fMean;
					if (ee == E_DS) fMin[ee] = 0.0f;  // force min to always be 0 for significance
				    g_fScaleAxesCurrent[ee] = fMax[ee];
					g_fCenterAxesCurrent[ee] = (fMax[ee] + fMin[ee]) / 2.0f;
				}
				else {
					g_fScaleAxesCurrent[ee] = ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] );
				    g_fCenterAxesCurrent[ee] = 0.0f;
				}
			 }
			 else {
				g_fScaleAxesCurrent[ee] = ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] );
			    g_fCenterAxesCurrent[ee] = 0.0f;
			 }

			 if (g_fScaleAxesCurrent[ee] == 0.0f) g_fScaleAxesCurrent[ee] = 1.0f; // avoid divide by zero
			 fAvg = (fMax[ee] - fMin[ee]) / 2.0f;
			 for (int i=0; i<PLOT_ARRAY_SIZE; i++) {
				 x1 = xax_qcnlive[0] + (((float) i / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0]));
				 //y1 = yax_qcnlive[ee] + ( ee == E_DS ? .5f : 7.5f) + ( (fdata[i] - g_fCenterAxesCurrent[ee] ) * 7.5f / g_fScaleAxesCurrent[ee] );
				 y1 = yax_qcnlive[ee] + ( ee == E_DS ? .5f : 0.0f ) + ( 15.0f * ((fdata[i] - fMin[ee]) / (fMax[ee] - fMin[ee])));

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
			//y1 = yax_qcnlive[ee] + ( ee == E_DS ? .5f : 7.5f) + ( (fdata[PLOT_ARRAY_SIZE-1] - g_fCenterAxesCurrent[ee] ) * 7.5f / g_fScaleAxesCurrent[ee] );
			y1 = yax_qcnlive[ee] + ( ee == E_DS ? .5f : 0.0f ) + ( 15.0f * ((fdata[PLOT_ARRAY_SIZE-1] - fMin[ee]) / (fMax[ee] - fMin[ee])));

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


}  // end namespace qcn_2dplot

