#ifndef _QCN_2DPLOT_H_
#define _QCN_2DPLOT_H_

namespace qcn_2dplot {
	extern int g_TimerTick;
	extern bool g_bIsWhite;

	extern void draw_text();
	extern void draw_tick_marks();
	extern void draw_plot() ;
}  // end namespace

#endif // _QCN_2DPLOT_H_