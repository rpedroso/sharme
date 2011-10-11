//#include <stdio.h>
#include <X11/Xlib.h>
//#include <X11/Intrinsic.h>
#include <X11/extensions/XTest.h>
//#include <unistd.h>

extern Display *disp;

static void send_key (Display *disp, int key_up_flag, KeySym keysym, KeySym modsym)
{
  KeyCode keycode = 0, modcode = 0;

  keycode = XKeysymToKeycode (disp, keysym);
  if (keycode == 0) return;

  //XTestGrabControl (disp, True);

  /* Generate modkey press */
  if (modsym != 0)
  {
    modcode = XKeysymToKeycode(disp, modsym);
    XTestFakeKeyEvent (disp, modcode, True, 0);
  }

  /* Generate regular key press and release */
  XTestFakeKeyEvent (disp, keycode, key_up_flag, 0);
  //XTestFakeKeyEvent (disp, keycode, False, 0);

  /* Generate modkey release */
  if (modsym != 0)
    XTestFakeKeyEvent (disp, modcode, False, 0);

  XSync (disp, False);
  //XTestGrabControl (disp, False);
}

void process_key(char action, int key)
{
    //printf("%c: %d\n", action, key);
    int key_flag = (action == 'K') ? False: True;
    send_key(disp, key_flag, key, 0); 
}

void cleanup_keys(void)
{
}

