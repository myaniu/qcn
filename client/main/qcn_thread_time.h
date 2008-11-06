#ifndef _QCN_THREAD_TIME_H_
#define _QCN_THREAD_TIME_H_

#include "main.h"

// forward declarations
#ifdef _WIN32  // Win fn ptr
     // UINT WINAPI QCNThreadTime(LPVOID /* lpParameter */)
     DWORD WINAPI QCNThreadTime(LPVOID);
#else  // Mac & Linux thread function ptr
     void* QCNThreadTime(void*);
#endif

#endif

