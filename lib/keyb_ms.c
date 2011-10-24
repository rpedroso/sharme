#include <windows.h>

/* from Fl_get_key_win32.cxx */
static const struct {unsigned short vk, fltk;} vktab[] = {
  {VK_SPACE,    ' '},
  {'1',         '!'},
  {0xde,        '\"'},
  {'3',         '#'},
  {'4',         '$'},
  {'5',         '%'},
  {'7',         '&'},
  {0xde,        '\''},
  {'9',         '('},
  {'0',         ')'},
  {'8',         '*'},
  {0xbb,        '+'},
  {0xbc,        ','},
  {0xbd,        '-'},
  {0xbe,        '.'},
  {0xbf,        '/'},
  {0xba,        ':'},
  {0xba,        ';'},
  {0xbc,        '<'},
  {0xbb,        '='},
  {0xbe,        '>'},
  {0xbf,        '?'},
  {'2',         '@'},
  {0xdb,        '['},
  {0xdc,        '\\'},
  {0xdd,        ']'},
  {'6',         '^'},
  {0xbd,        '_'},
  {0xc0,        '`'},
  {0xdb,        '{'},
  {0xdc,        '|'},
  {0xdd,        '}'},
  {0xc0,        '~'},
  {VK_BACK,     FL_BackSpace},
  {VK_TAB,      FL_Tab},
  {VK_CLEAR,    0xff0b/*XK_Clear*/},
  {VK_RETURN,   FL_Enter},
  {VK_PAUSE,    FL_Pause},
  {VK_SCROLL,   FL_Scroll_Lock},
  {VK_ESCAPE,   FL_Escape},
  {VK_HOME,     FL_Home},
  {VK_LEFT,     FL_Left},
  {VK_UP,       FL_Up},
  {VK_RIGHT,    FL_Right},
  {VK_DOWN,     FL_Down},
  {VK_PRIOR,    FL_Page_Up},
  {VK_NEXT,     FL_Page_Down},
  {VK_END,      FL_End},
  {VK_SNAPSHOT, FL_Print},
  {VK_INSERT,   FL_Insert},
  {VK_APPS,     FL_Menu},
  {VK_NUMLOCK,  FL_Num_Lock},
//{VK_???,      FL_KP_Enter},
  {VK_MULTIPLY, FL_KP+'*'},
  {VK_ADD,      FL_KP+'+'},
  {VK_SUBTRACT, FL_KP+'-'},
  {VK_DECIMAL,  FL_KP+'.'},
  {VK_DIVIDE,   FL_KP+'/'},
  {VK_LSHIFT,   FL_Shift_L},
  {VK_RSHIFT,   FL_Shift_R},
  {VK_LCONTROL, FL_Control_L},
  {VK_RCONTROL, FL_Control_R},
  {VK_CAPITAL,  FL_Caps_Lock},
  {VK_LWIN,     FL_Meta_L},
  {VK_RWIN,     FL_Meta_R},
  {VK_LMENU,    FL_Alt_L},
  {VK_RMENU,    FL_Alt_R},
  {VK_DELETE,   FL_Delete}
};

static int fltk2ms(int fltk) {
  if (fltk >= '0' && fltk <= '9') return fltk;
  if (fltk >= 'A' && fltk <= 'Z') return fltk;
  if (fltk >= 'a' && fltk <= 'z') return fltk-('a'-'A');
  if (fltk > FL_F && fltk <= FL_F+16) return fltk-(FL_F-(VK_F1-1));
  if (fltk >= FL_KP+'0' && fltk<=FL_KP+'9') return fltk-(FL_KP+'0'-VK_NUMPAD0);
  int a = 0;
  int b = sizeof(vktab)/sizeof(*vktab);
  while (a < b) {
    int c = (a+b)/2;
    if (vktab[c].fltk == fltk) return vktab[c].vk;
    if (vktab[c].fltk < fltk) a = c+1; else b = c;
  }
  return 0;
}
/* end: from Fl_get_key_win32.cxx */

void process_key(char action, int key)
{
    int key_flag = (action == 'K') ? KEYEVENTF_KEYUP: 0;
    
    int vkkey;

    //pmesg(9, (char*)"key: %d\n", key);
    vkkey = fltk2ms(key);
    keybd_event((char)vkkey, (char)key, key_flag, 0);
}

void cleanup_keys(void)
{
    int key;
    int vkkey;
    int key_flag = KEYEVENTF_KEYUP;

    key = FL_Alt_L;
    vkkey = VK_MENU;
    keybd_event((char)vkkey, (char)key, key_flag, 0);

    key = 0xfe03;
    vkkey = VK_RMENU;
    keybd_event((char)vkkey, (char)key, key_flag, 0);
}
