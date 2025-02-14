#include "hook.h"
#include "globals.h"
#include <cctype>
#include <iostream>

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    KBDLLHOOKSTRUCT *s = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

    switch (wParam)
    {
    case WM_KEYDOWN: {
        char c = MapVirtualKey(s->vkCode, MAPVK_VK_TO_CHAR);
        c = std::tolower(c);
        if (std::isalpha(c))
        {
            InvalidateRect(D2DHwnd, nullptr, FALSE);
            if (KeyStringToCast.size() > KeycastConfig.maxSize)
                KeyStringToCast = L"";
            KeyStringToCast += c;
            // KeyStringToCast += L" ";
            std::cout << c << "";
        }
        else
        {
            InvalidateRect(D2DHwnd, nullptr, FALSE);
            if (KeyStringToCast.size() > KeycastConfig.maxSize)
                KeyStringToCast = L"";
            if (!KeyStringToCast.empty() && std::isalpha(KeyStringToCast[KeyStringToCast.size() - 1]))
                KeyStringToCast += L"";
            KeyStringToCast += L"󰁮 ";
        }
        break;
    }

    //
    // Keys like Alt would not be captured by WM_KEYDOWN, so we also need WM_SYSKEYDOWN
    //
    case WM_SYSKEYDOWN: {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
        std::cout << "virtual key code: " << p->vkCode;
        break;
    }

    default:
        break;
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}