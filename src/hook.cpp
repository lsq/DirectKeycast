#include "hook.h"
#include "globals.h"
#include "utils.h"

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
        auto curKeyStr = KeyCastMap().at(s->vkCode);
        if (KeyStringToCast.size() > KeycastConfig.maxSize | KeyStringToCast.size() + curKeyStr.size() > KeycastConfig.maxSize)
            KeyStringToCast = L"";
        KeyStringToCast += curKeyStr;
        InvalidateRect(D2DHwnd, nullptr, FALSE);
        break;
    }
    //
    // Keys like Alt would not be captured by WM_KEYDOWN, so we also need WM_SYSKEYDOWN
    //
    case WM_SYSKEYDOWN: {
        auto curKeyStr = KeyCastMap().at(s->vkCode);
        if (KeyStringToCast.size() > KeycastConfig.maxSize | KeyStringToCast.size() + curKeyStr.size() > KeycastConfig.maxSize)
            KeyStringToCast = L"";
        KeyStringToCast += curKeyStr;
        InvalidateRect(D2DHwnd, nullptr, FALSE);
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