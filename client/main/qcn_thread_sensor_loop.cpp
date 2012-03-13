#include "qcn_thread_sensor_util.h"

#ifndef QCN_USB

namespace qcn_thread_sensor_loop {
	
void TriggerDetectionAlgorithm()
{
 
      // CMC note -- in original onepoint.c code, sm->lOffset-1 was iM1 & sm->lOffset-iWindow was IB1, these were
      //     used to get the right "place" in case of the array being looped, but I don't see how this could
      //     happen because after the one minute mean/baseline above, we are past "iWindow" by this point
      //     as if sm->lOffset >= MAXI above will go to top of loop and reinitialize
      sm->xa[sm->lOffset]  = sm->xa[sm->lOffset-1] + ((sm->x0[sm->lOffset] - sm->x0[sm->lOffset-sm->iWindow]) / sm->iWindow);         //  AVERAGE X
      sm->ya[sm->lOffset]  = sm->ya[sm->lOffset-1] + ((sm->y0[sm->lOffset] - sm->y0[sm->lOffset-sm->iWindow]) / sm->iWindow);         //  AVERAGE Y
      sm->za[sm->lOffset]  = sm->za[sm->lOffset-1] + ((sm->z0[sm->lOffset] - sm->z0[sm->lOffset-sm->iWindow]) / sm->iWindow);         //  AVERAGE Z

// note -- the bottom three lines of the vari[i] expression are uncommented per JLF
//   (previously in the original onepoint.c code it ended at the first sm->iWindow)
// QCN_SQR(A) is a short-hand to square A (i.e. A^2 or A*A, not square root!)
      sm->vari[sm->lOffset] = sm->vari[sm->lOffset-1] + ( QCN_SQR(sm->x0[sm->lOffset] - sm->xa[sm->lOffset]   )
          + QCN_SQR(sm->y0[sm->lOffset]   - sm->ya[sm->lOffset]   )
          + QCN_SQR(sm->z0[sm->lOffset]   - sm->za[sm->lOffset]   ) ) / sm->iWindow
        - ( QCN_SQR(sm->x0[sm->lOffset-sm->iWindow] - sm->xa[sm->lOffset-sm->iWindow] )
        + QCN_SQR(sm->y0[sm->lOffset-sm->iWindow] - sm->ya[sm->lOffset-sm->iWindow] )
        + QCN_SQR(sm->z0[sm->lOffset-sm->iWindow] - sm->za[sm->lOffset-sm->iWindow] ) ) / sm->iWindow;

       sm->fmag[sm->lOffset]= sqrt(QCN_SQR(sm->x0[sm->lOffset]-sm->xa[sm->lOffset-1]) +
         QCN_SQR(sm->y0[sm->lOffset]-sm->ya[sm->lOffset-1])+QCN_SQR(sm->z0[sm->lOffset]-sm->za[sm->lOffset-1]));

// CMC added Jesse's mod to have a short term avg mag, and put this var in shared mem so it can be perturbed
    //  sm->fsig[sm->lOffset]= sm->fmag[sm->lOffset]/sqrt(sm->vari[sm->lOffset-1] + 1.0e-3f);  // .001 to prevent divide-by-zero but so we capture any fmag & vari

// jesse added a short term average magnitude rather than an instantaneous
//    magnitude.  eventually fShortTermAvgMag should be turned into a variable to
//    passed into here.  In theory the larger fShortTermAvgMag is, the harder it will
//    be to trigger with a delta function. fShortTermAvgMag should vary from 1 to ~5
//    (5 is looking at 10Hz variations rather than 50).
//      int fShortTermAvgMag = 3;

      sm->fsig[sm->lOffset]=sm->fmag[sm->lOffset] /
                            sqrt(sm->vari[sm->lOffset-1] + (g_fThreshold*0.10f));
// .001 to prevent divide-by-zero but so we capture any fmag & vari
      if (qcn_main::g_fPerturb[PERTURB_SHORT_TERM_AVG_MAG] > 0) {
         for (int i_off = 1; i_off < (int) qcn_main::g_fPerturb[PERTURB_SHORT_TERM_AVG_MAG]; i_off++) {
            sm->fsig[sm->lOffset]=sm->fsig[sm->lOffset] +
                             sm->fmag[sm->lOffset-i_off] /
                             sqrt(sm->vari[sm->lOffset-1] + 1.0e-3f);
// .001 to prevent divide-by-zero but so we capture any fmag & vari
         }
         sm->fsig[sm->lOffset]=sm->fsig[sm->lOffset] / (qcn_main::g_fPerturb[PERTURB_SHORT_TERM_AVG_MAG] + 1.0f);
// Normalize average magnitude over window
       }

	   sm->sgmx = sm->fsig[sm->lOffset];
	   sm->itl=0;

	   // CMC note: the threshold values are from above after sensor detection, the peturb values for sig cutoff are from the workunit generation
	   // also note the first clause prevents continual qcn app from doing a trigger, they are like demo mode with sac output every 10 minutes
	
	   // the threshold values are set in qcn_thread_sensor_util.cpp function SetSensorThresholdAndDiffFactor()
	
	   // if (!qcn_main::g_bContinual   // CMC 03/08/2010 -- continual now has regular triggers
	   if (
		 (sm->fsig[sm->lOffset] > qcn_main::g_fPerturb[PERTURB_SIG_CUTOFF])
		  && (sm->fmag[sm->lOffset] > g_fThreshold) ) {  // was 0.125,  >2 sigma (90% Conf)
				doTrigger(); // last trigger time will get set in the proc
		}
		
    // Find the largest significance in the time window
	if (sm->itl > sm->iWindow) {
        sm->sgmx = 0.;
        for (int j=sm->lOffset-(sm->iWindow+1); j<sm->lOffset; j++) {
          if (sm->fsig[j] > sm->sgmx) {          //  MAXIMUM SIGNIFICANCE
            sm->sgmx = sm->fsig[j];
            sm->itl = sm->lOffset-j;
          }
        }
        //fprintf(stdout, "Largest sig at lOffset=%ld iWindow=%d PT:%ld SGMX=%f \n", sm->lOffset, sm->iWindow, sm->itl, sm->sgmx);
        //fflush(stdout);
      }
		
      checkRecordState();   // test if we're recording in qcnlive
#ifdef _DEBUG
    DebugTime(4);
#endif

}
	
}  // namespace


#endif // ifndef QCN_USB
