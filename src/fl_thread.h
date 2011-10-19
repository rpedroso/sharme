#ifndef FL_THREAD_H
#define FL_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MSWINDOWS
#include <windows.h>
#include <process.h>

typedef unsigned long Fl_Thread;
static int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p) {
  return t = (Fl_Thread)_beginthread((void( __cdecl * )( void * ))f, 0, p);
}

#else

#include <unistd.h>
#include <pthread.h>

typedef pthread_t Fl_Thread;
static int fl_create_thread(Fl_Thread& t, void *(*f) (void *), void* p) {
  return pthread_create((pthread_t*)&t, 0, f, p);
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

