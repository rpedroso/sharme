#include "screenshot.h"

#ifdef __XLIB__
#include "screenshot_x.c"
#elif __MSW__
#include "screenshot_ms.c"
#endif

