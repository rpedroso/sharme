#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

Display *disp;

void mouse_init(void)
{
    disp = XOpenDisplay(NULL);
}

// button= 1 left, 2 middle, 3 right
// ret = XTestFakeButtonEvent(dpy, button, is_press, CurrentTime);
void mouse_left_down(void)
{
    XTestFakeButtonEvent(disp, 1, 1, CurrentTime);
    XFlush(disp);
}

void mouse_left_up(void)
{
    XTestFakeButtonEvent(disp, 1, 0, CurrentTime);
    XFlush(disp);
}


void mouse_right_down(void)
{
    XTestFakeButtonEvent(disp, 3, 1, CurrentTime);
    XFlush(disp);
}

void mouse_right_up(void)
{
    XTestFakeButtonEvent(disp, 3, 0, CurrentTime);
    XFlush(disp);
}

void mouse_move(int x, int y)
{
  Window root = DefaultRootWindow(disp);
  XWarpPointer(disp, None, root, 0, 0, 0, 0, x, y);
  XFlush(disp);
}

void mouse_wheel(int flag)
{
    XTestFakeButtonEvent(disp, flag, 1, CurrentTime);
    XTestFakeButtonEvent(disp, flag, 0, CurrentTime);
    XFlush(disp);
}
