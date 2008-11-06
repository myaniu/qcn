#include "main.h"
#include "qcn_graphics.h"
#include "qcn_thread.h"
#include "qcn_thread_graphics.h"

#ifdef _WIN32  // Win fn ptr
UINT WINAPI QCNThreadGraphics(LPVOID /* lpParameter */)
#else  // Mac & Linux thread function ptr
void* QCNThreadGraphics(void*)
#endif
{ 
  char* strArgs[] = { "qcngraphics" };
  qcn_graphics::graphics_main(1, strArgs);

  #ifdef _WIN32
		return 0;
  #else
		return (void*) 0; //bRet ? 0 : 1;
  #endif
} 
 
