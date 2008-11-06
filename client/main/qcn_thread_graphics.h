#ifndef _QCN_THREAD_GRAPHICS_H_
#define _QCN_THREAD_GRAPHICS_H_

// functions to be used for the main thread (from QCNLIVE)
#ifdef _WIN32  // Win fn ptr
  extern UINT WINAPI QCNThreadGraphics(LPVOID /* lpParameter */);
#else  // Mac & Linux thread function ptr
  extern void* QCNThreadGraphics(void*);
#endif

#endif // _QCN_THREAD_MAIN_H

