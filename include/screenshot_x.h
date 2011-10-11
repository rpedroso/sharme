#ifndef __SCREENSHOT_X_H__
#define __SCREENSHOT_X_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct _rect {
   int x;
   int y;
   int w;
   int h;
} RECT;

struct _Context {
  Display *disp;

  Screen *screen;
  int screen_num;

  Visual *visual;

  Window root;

  unsigned long white, black;

  int depth;
  int bpp;
  int endianness;

  int width, height;
  int widthmm, heightmm;

  /* these are the output masks 
   * for buffers from ximagesrc
   * and are in big endian */
  unsigned int r_mask_output, g_mask_output, b_mask_output;

  //GValue *par;                  /* calculated pixel aspect ratio */

  //gboolean use_xshm;

  //GstCaps *caps;
};

typedef struct _Context Context;

//RECT screenshot_get_monitor_rect (unsigned int index);
Context* display_open(const char *display_name);
//XImage* screenshot(Context *xcontext, int startx, int starty, int width, int height);
screenshot_t* screenshot_new();

#endif
