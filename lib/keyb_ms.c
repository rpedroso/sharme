#include <windows.h>

void process_key(char action, int key)
{
            int key_flag = (action == 'K') ? KEYEVENTF_KEYUP: 0;

            int vkkey;
            if (key < 256)
            {
                vkkey = VkKeyScan(key);
            }
            else if (key > FL_F && key <= FL_F_Last)
            {
                key = key - FL_F;
                vkkey = VK_F1 + (key-1);
            }
            else if (key >= FL_KP && key <= FL_KP_Last)
            {
                key = key - FL_KP;
                vkkey = VK_NUMPAD0 + (key-48);
            }
            else if (key == FL_Shift_L)
            {
                vkkey = VK_LSHIFT;
            }
            else if (key == FL_Shift_R)
            {
                vkkey = VK_RSHIFT;
            }
            else if (key == FL_Caps_Lock)
            {
                vkkey = VK_CAPITAL;
            }
            else if (key == FL_Page_Up)
            {
                vkkey = VK_PRIOR;
            }
            else if (key == FL_Page_Down)
            {
                vkkey = VK_NEXT;
            }
            else if (key == FL_Home)
            {
                vkkey = VK_HOME;
            }
            else if (key == FL_End)
            {
                vkkey = VK_END;
            }
            else if (key == FL_Left)
            {
                vkkey = VK_LEFT;
            }
            else if (key == FL_Right)
            {
                vkkey = VK_RIGHT;
            }
            else if (key == FL_Up)
            {
                vkkey = VK_UP;
            }
            else if (key == FL_Down)
            {
                vkkey = VK_DOWN;
            }
            else if (key == FL_Insert)
            {
                vkkey = VK_INSERT;
            }
            else if (key == FL_Delete)
            {
                vkkey = VK_DELETE;
            }
            else if (key == FL_Num_Lock)
            {
                vkkey = VK_NUMLOCK;
            }
            else if (key == FL_Scroll_Lock)
            {
                vkkey = VK_SCROLL;
            }
            else if (key == FL_Control_L)
            {
                vkkey = VK_LCONTROL;
            }
            else if (key == FL_Control_R)
            {
                vkkey = VK_RCONTROL;
            }
            else if (key == FL_Alt_L)
            {
                vkkey = VK_MENU;
            }
            else if (key == 0xfe03)
            {
                vkkey = VK_RMENU;
            }
            else if (key == FL_Meta_L)
            {
                vkkey = VK_LWIN;
            }
            else if (key == FL_Meta_R)
            {
                vkkey = VK_RWIN;
            }
            else if (key == FL_Menu)
            {
                vkkey = VK_APPS;
            }
            else if (key == 0xfe53) // dead key ~
            {
                key = '~';
                vkkey = VkKeyScan(key);
            }
            else if (key == 0xfe52) // dead key ^
            {
                key = '^';
                vkkey = VkKeyScan(key);
            }
            else if (key == 0xfe51) // dead key ´
            {
                key = '´';
                vkkey = VkKeyScan(key);
            }
            else if (key == 0xfe50) // dead key `
            {
                key = '`';
                vkkey = VkKeyScan(key);
            }
            else
            {
                vkkey = key;
            }
            //int bShift = (key & 0x00100000);
            //int bCaps = (vkkey & 0x00100000);
            //int bCtrl = (key & 0x400);
            //int bAlt = (key & 0x800);
            //key = (vkkey & 0xFF);

            //pmesg(0, "%bbShift: %d\n", bShift);
            //pmesg(0, "%bbCaps: %d\n", bCaps);
            //pmesg(0, "%bbCtrl: %d\n", bCtrl);
            pmesg(9, (char*)"key: %d\n", key);

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
