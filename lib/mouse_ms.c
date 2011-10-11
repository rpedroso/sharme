#include <windows.h>

POINT p;

void mouse_init(void)
{
}

void mouse_left_down(void)
{
    GetCursorPos(&p);
    mouse_event(MOUSEEVENTF_LEFTDOWN, p.x, p.y, 0, 0);
}

void mouse_left_up(void)
{
    GetCursorPos(&p);
    mouse_event(MOUSEEVENTF_LEFTUP, p.x, p.y, 0, 0);
}


void mouse_right_down(void)
{
    GetCursorPos(&p);
    mouse_event(MOUSEEVENTF_RIGHTDOWN, p.x, p.y, 0, 0);
}

void mouse_right_up(void)
{
    GetCursorPos(&p);
    mouse_event(MOUSEEVENTF_RIGHTUP, p.x, p.y, 0, 0);
}

void mouse_move(int x, int y)
{
    SetCursorPos(x, y);
}
