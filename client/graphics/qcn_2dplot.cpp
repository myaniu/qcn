#include "qcn_graphics.h"
#include "qcn_2dplot.h"

using namespace qcn_graphics;

namespace qcn_2dplot {

void draw_text_plot_qcnlive() 
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
		 txf_render_string(.1, fWhere - 0.042f, 0.030f, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? light_blue : grey_trans, TXF_HELVETICA, (char*) strTime);
	   }
	}

	// labels for each axis
	const float fAxisLabel = 1.061f;
	const float fVertLabel = 0.988f;
	const float fYOffset = 0.016f;

    txf_render_string(.1, fAxisLabel, 0.60f - fYOffset, 0, MSG_SIZE_NORMAL, red, TXF_HELVETICA, "Significance", 90.0f);
    txf_render_string(.1, fAxisLabel, 0.46f - fYOffset, 0, MSG_SIZE_NORMAL, blue, TXF_HELVETICA, "Z Axis", 90.0f);
    txf_render_string(.1, fAxisLabel, 0.30f - fYOffset, 0, MSG_SIZE_NORMAL, orange, TXF_HELVETICA, "Y Axis", 90.0f);
    txf_render_string(.1, fAxisLabel, 0.14f - fYOffset, 0, MSG_SIZE_NORMAL, green, TXF_HELVETICA, "X Axis", 90.0f);

	// labels for significance
    txf_render_string(.1, fVertLabel, 0.578f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " 0.00", 0.0f);
    txf_render_string(.1, fVertLabel, 0.604f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " 3.33", 0.0f);
    txf_render_string(.1, fVertLabel, 0.632f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " 6.67", 0.0f);
    txf_render_string(.1, fVertLabel, 0.659f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "10.00", 0.0f);
    txf_render_string(.1, fVertLabel, 0.686f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "13.33", 0.0f);
    txf_render_string(.1, fVertLabel, 0.713f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "16.67", 0.0f);
    txf_render_string(.1, fVertLabel, 0.735f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "20.00", 0.0f);

	// labels for Z axis
    txf_render_string(.1, fVertLabel, 0.413f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-19.62", 0.0f);

	// labels for Y axis
    txf_render_string(.1, fVertLabel, 0.248f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-19.62", 0.0f);

	// labels for X axis
    txf_render_string(.1, fVertLabel, 0.084f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-19.62", 0.0f);
    txf_render_string(.1, fVertLabel, 0.105f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "-13.08", 0.0f);
    txf_render_string(.1, fVertLabel, 0.133f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " -6.54", 0.0f);
    txf_render_string(.1, fVertLabel, 0.161f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "  0.00", 0.0f);
    txf_render_string(.1, fVertLabel, 0.188f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " +6.54", 0.0f);
    txf_render_string(.1, fVertLabel, 0.215f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "+13.08", 0.0f);
    txf_render_string(.1, fVertLabel, 0.235f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, "+19.62", 0.0f);

	// units label (meters per second per second
    txf_render_string(.1, fVertLabel, 0.066f - fYOffset, 0, MSG_SIZE_SMALL, g_b2DPlotWhite ? black : grey_trans, TXF_COURIER_BOLD, " m/s/s", 0.0f);

    draw_text_sensor();

	ortho_done();
}

void draw_tick_marks_qcnlive()
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
         glColor4fv((GLfloat*) g_b2DPlotWhite ? light_blue : grey_trans);
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

void draw_plots_2d_qcnlive() 
{

/*
- boxes should be even, as well as plotting since all +/- 19.6 m/s2, sig 0 - 10
- bouncing ball at "tip" where drawn
- S/X/Y/Z on right side with vertical axis
- toggle background colors black/white?
*/

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
	glLineWidth(2);

    float* fdata = NULL;

    // each plot section is 15 units high

	float xmin = xax_qcnlive[0] - 0.1f;
	float xmax = xax_qcnlive[1] + 0.1f;
	float ymin = yax_qcnlive[E_DX] - 7.0f;
    float ymax = yax_qcnlive[4]; // + 15.0f;

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
		 
         glLineWidth(1);
		 glColor4fv(ee == E_DY ? orange : colorsPlot[ee]);  // set the color for data - CMC note the orange substitution for yellow on the Y
		 glLineWidth(2.0f);
         glBegin(GL_LINE_STRIP);

         for (int i=0; i<PLOT_ARRAY_SIZE; i++) {
          if (fdata[i] != 0.0f)  {
			if (ee == E_DS && fdata[i] > 20.0f) fdata[i] = 20.0f; // clip significance at 20 for display
            glVertex2f(
              xax_qcnlive[0] + (((float) i / (float) PLOT_ARRAY_SIZE) * (xax_qcnlive[1]-xax_qcnlive[0])), 
              yax_qcnlive[ee] + ( ee == E_DS ? .5f : 7.5f) + ( fdata[i] * ( 7.5f / ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] ) ))
            );
			
		   }
         }
         glEnd();

		 // plot a "colored pointer" at the end for ease of seeing current value?
		if (fdata[PLOT_ARRAY_SIZE-1] != 0.0f)  {
			float x1 = xax_qcnlive[0] + (xax_qcnlive[1]-xax_qcnlive[0]);
			float y1 = yax_qcnlive[ee] + ( ee == E_DS ? .5f : 7.5f) + ( fdata[PLOT_ARRAY_SIZE-1] * ( 7.5f / ( ee == E_DS ? g_fScaleSig[g_iScaleSigOffset] : g_fScaleAxes[g_iScaleAxesOffset] ) ));
			const float fRadius = 1.4f;
			float fAngle = PI/8.0f;

			glBegin(GL_TRIANGLE_FAN);
			glVertex2f(x1, y1);
			glVertex2f(x1 + (cos(fAngle) * fRadius), y1 + (sin(fAngle) * fRadius));
			glVertex2f(x1 + (cos(-fAngle) * fRadius), y1 + (sin(-fAngle) * fRadius));
			glEnd();
		} // colored pointer
	}


	
	// draw boxes around the plots

	 const float fExt = 7.05f;
	 const float fFudge = 0.02f;

	 glColor4fv((GLfloat*) g_b2DPlotWhite ? black : grey_trans);
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

 	draw_tick_marks_qcnlive();

	 glColor4fv((GLfloat*) grey);
	 glRectf(xmin, yax_qcnlive[E_DX], xmax+fExt, ymin);  // bottom rectangle (timer ticks)

	 //right side rectangular region
	 glRectf(xmax, ymax, xmax+fExt, yax_qcnlive[E_DX]);
		 
//    glPopMatrix();    
		
    glFlush();
}


}  // end namespace qcn_2dplot

