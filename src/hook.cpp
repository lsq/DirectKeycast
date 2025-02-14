#include "hook.h"
#include "globals.h"
#include <cctype>
#include <iostream>

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        goto KBDNext;
    }
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
        break;
    }
    case WM_KEYUP: {
        break;
    }
    case WM_SYSKEYUP: {
        break;
    }
    default:
        break;
    }
KBDNext:
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK MOUSEHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        goto MOUSENext;
    }
    switch (wParam)
    {
    case WM_LBUTTONDOWN: {
    }
    case WM_RBUTTONDOWN: {
    }
    case WM_LBUTTONUP: {
    }
    case WM_RBUTTONUP: {
    }
    }
MOUSENext:
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}