#include "hook.h"
#include "globals.h"
#include "utils.h"
#include <winuser.h>

//
// We need 8 bits to store our modifier keys state
// Left Shift, Right Shift
// Left Ctrl, Right Ctrl
// Left Alt, Right Alt
// Left Windows, Right Windows
//
enum ModifierKey
{
    LShift,
    RShift,
    LCtrl,
    RCtrl,
    LAlt,
    RAlt,
    LWindows,
    RWindows
};
std::unordered_map<DWORD, ModifierKey> VkCodeMap = {
    {VK_LSHIFT, LShift},  //
    {VK_RSHIFT, RShift},  //
    {VK_LCONTROL, LCtrl}, //
    {VK_RCONTROL, RCtrl}, //
    {VK_LMENU, LAlt},     //
    {VK_RMENU, RAlt},     //
    {VK_LWIN, LWindows},  //
    {VK_RWIN, RWindows},  //
};
// std::unordered_map<ModifierKey, UINT> ModifierKeyMask = {
//     { LShift,  },  //
//     { LShift, },  //
// };
UINT ModifierKeyState = 0;

void HandleKeyDown(const KBDLLHOOKSTRUCT *s);

void SetOneModifier(DWORD vkCode);
void GetOneModifier(DWORD vkCode);
void ClearOneModifier(DWORD vkCode);
void ClearAllModifiers();

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        goto KBDNext;
    }
    KBDLLHOOKSTRUCT *s = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    switch (wParam)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        HandleKeyDown(s);
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

void HandleKeyDown(const KBDLLHOOKSTRUCT *s)
{
    auto curKeyStr = KeyCastMap().at(s->vkCode);
    if (::KeyStringToCast.size() > ::KeycastConfig.maxSize | ::KeyStringToCast.size() + curKeyStr.size() > ::KeycastConfig.maxSize)
        ::KeyStringToCast = L"";
    ::KeyStringToCast += curKeyStr;
    InvalidateRect(::D2DHwnd, nullptr, FALSE);
}

void ClearOneModifier(DWORD vkCode)
{
    ModifierKey curKey = VkCodeMap.at(vkCode);
    UINT mask = ~(1 << curKey);
    ::ModifierKeyState &= mask;
}

void ClearAllModifiers()
{
    ::ModifierKeyState &= 0;
}