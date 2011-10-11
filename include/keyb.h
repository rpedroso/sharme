#ifndef KEYB_H
#define KEYB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FL_Button       0xfee8  ///< A mouse button; use Fl_Button + n for mouse button n.
#define FL_BackSpace    0xff08  ///< The backspace key.
#define FL_Tab          0xff09  ///< The tab key.
#define FL_Enter        0xff0d  ///< The enter key. 
#define FL_Pause        0xff13  ///< The pause key.
#define FL_Scroll_Lock  0xff14  ///< The scroll lock key.
#define FL_Escape       0xff1b  ///< The escape key.
#define FL_Home         0xff50  ///< The home key.
#define FL_Left         0xff51  ///< The left arrow key.
#define FL_Up           0xff52  ///< The up arrow key.
#define FL_Right        0xff53  ///< The right arrow key.
#define FL_Down         0xff54  ///< The down arrow key.
#define FL_Page_Up      0xff55  ///< The page-up key.
#define FL_Page_Down    0xff56  ///< The page-down key.
#define FL_End          0xff57  ///< The end key.
#define FL_Print        0xff61  ///< The print (or print-screen) key.
#define FL_Insert       0xff63  ///< The insert key. 
#define FL_Menu         0xff67  ///< The menu key.
#define FL_Help         0xff68  ///< The 'help' key on Mac keyboards
#define FL_Num_Lock     0xff7f  ///< The num lock key.
#define FL_KP           0xff80  ///< One of the keypad numbers; use FL_KP + n for number n.
#define FL_KP_Enter     0xff8d  ///< The enter key on the keypad, same as Fl_KP+'\\r'.
#define FL_KP_Last      0xffbd  ///< The last keypad key; use to range-check keypad.
#define FL_F            0xffbd  ///< One of the function keys; use FL_F + n for function key n.
#define FL_F_Last       0xffe0  ///< The last function key; use to range-check function keys.
#define FL_Shift_L      0xffe1  ///< The lefthand shift key.
#define FL_Shift_R      0xffe2  ///< The righthand shift key.
#define FL_Control_L    0xffe3  ///< The lefthand control key.
#define FL_Control_R    0xffe4  ///< The righthand control key.
#define FL_Caps_Lock    0xffe5  ///< The caps lock key.
#define FL_Meta_L       0xffe7  ///< The left meta/Windows key.
#define FL_Meta_R       0xffe8  ///< The right meta/Windows key.
#define FL_Alt_L        0xffe9  ///< The left alt key.
#define FL_Alt_R        0xffea  ///< The right alt key. 
#define FL_Delete       0xffff  ///< The delete key.

void process_key(char action, int key);
void cleanup_keys(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

