#include "hook.h"
#include <iostream>

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *s = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

    switch (wParam)
    {
    case WM_KEYDOWN: {
        char c = MapVirtualKey(s->vkCode, MAPVK_VK_TO_CHAR);
        std::cout << c << " ";
    }

    default:
        break;
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}