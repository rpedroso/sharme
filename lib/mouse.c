#include "mouse.h"

#ifdef __XLIB__
#include "mouse_x.c"
#elif __MSW__
#include "mouse_ms.c"
#endif

