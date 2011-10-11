#include "keyb.h"

#ifdef __XLIB__
#include "keyb_x.c"
#elif __MSW__
#include "keyb_ms.c"
#else
#error "platform not supported"
#endif

