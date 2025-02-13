#include "hook.h"
#include "globals.h"
#include <cctype>
#include <iostream>
#include <minwindef.h>
#include <winuser.h>

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *s = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

    switch (wParam)
    {
    case WM_KEYDOWN: {
        char c = MapVirtualKey(s->vkCode, MAPVK_VK_TO_CHAR);
        if (std::isalpha(c))
        {
            InvalidateRect(D2DHwnd, nullptr, FALSE);
            if (KeyStringToCast.size() > 30)
                KeyStringToCast = L"";
            KeyStringToCast += c;
            std::cout << c << "";
        }
    }

    default:
        break;
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}