#include "hook.h"
#include "globals.h"
#include "utils.h"
#include <numeric>

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
UINT ModifierKeyState = 0;

void HandleKeyDown(const KBDLLHOOKSTRUCT *s);
void HandleModifierKeyDown(DWORD vkcode);
void HandleKeyUp(const KBDLLHOOKSTRUCT *s);
void HandleModifierKeyUp(DWORD vkcode);

void SetOneModifier(DWORD vkCode);
UINT GetOneModifier(DWORD vkCode);
void ClearOneModifier(DWORD vkCode);
void ClearAllModifiers();

bool IsOnlyOneShiftPressed();

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
    case WM_KEYUP:
    case WM_SYSKEYUP: {
        HandleKeyUp(s);
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
    DWORD vkeyCode = s->vkCode;
    if (!::KeyCastMap().count(vkeyCode))
    {
        return;
    }
    if (VkCodeMap.count(vkeyCode))
    {
        HandleModifierKeyDown(vkeyCode);
        return;
    }
    if (ModifierKeyState > 0)
    {
        // Only Shift Key is pressed, and current key is alpha chars or numbers
        if (IsOnlyOneShiftPressed() && KeyCastMapOnShift().count(vkeyCode))
        {
            auto curKeyStr = KeyCastMapOnShift().at(vkeyCode);
            if (::KeyStringToCast.size() > ::KeycastConfig.maxSize | ::KeyStringToCast.size() + curKeyStr.size() > ::KeycastConfig.maxSize)
                ::KeyStringToCast = L"";
            ::KeyStringToCast += curKeyStr;
            InvalidateRect(::D2DHwnd, nullptr, FALSE);
            return;
        }
        // Combination Key
        std::vector<std::wstring> modifierList;
        if ((1 << LCtrl) & ::ModifierKeyState)
        {
            modifierList.push_back(L"LCtrl");
        }
        if ((1 << RCtrl) & ::ModifierKeyState)
        {
            modifierList.push_back(L"RCtrl");
        }
        if ((1 << LShift) & ::ModifierKeyState)
        {
            modifierList.push_back(L"LShift");
        }
        if ((1 << RShift) & ::ModifierKeyState)
        {
            modifierList.push_back(L"RShift");
        }
        if ((1 << LAlt) & ::ModifierKeyState)
        {
            modifierList.push_back(L"LAlt");
        }
        if ((1 << RAlt) & ::ModifierKeyState)
        {
            modifierList.push_back(L"RAlt");
        }
        if ((1 << LWindows) & ::ModifierKeyState)
        {
            modifierList.push_back(L"LWin");
        }
        if ((1 << RWindows) & ::ModifierKeyState)
        {
            modifierList.push_back(L"RWin");
        }
        auto curKeyStr = KeyCastMap().at(vkeyCode);
        if (curKeyStr[0] == L'<' && curKeyStr[curKeyStr.size() - 1] == L'>')
        {
            curKeyStr = curKeyStr.substr(1, curKeyStr.size() - 2);
        }
        modifierList.push_back(curKeyStr);
        std::wstring delimiter = L"-";
        std::wstring curResKeyStr = std::accumulate(modifierList.begin(), modifierList.end(), std::wstring(), [&](const std::wstring &a, const std::wstring &b) { return a.empty() ? b : a + delimiter + b; });
        curResKeyStr = L"<" + curResKeyStr + L">";
        if (::KeyStringToCast.size() > ::KeycastConfig.maxSize | ::KeyStringToCast.size() + curResKeyStr.size() > ::KeycastConfig.maxSize)
            ::KeyStringToCast = L"";
        ::KeyStringToCast += curResKeyStr;
        InvalidateRect(::D2DHwnd, nullptr, FALSE);
        return;
    }
    auto curKeyStr = KeyCastMap().at(vkeyCode);
    if (::KeyStringToCast.size() > ::KeycastConfig.maxSize | ::KeyStringToCast.size() + curKeyStr.size() > ::KeycastConfig.maxSize)
        ::KeyStringToCast = L"";
    ::KeyStringToCast += curKeyStr;
    InvalidateRect(::D2DHwnd, nullptr, FALSE);
}

void HandleModifierKeyDown(DWORD vkcode)
{
    if (::GetOneModifier(vkcode))
    {
        return;
    }
    SetOneModifier(vkcode);
    auto curKeyStr = KeyCastMap().at(vkcode);
    if (::KeyStringToCast.size() > ::KeycastConfig.maxSize | ::KeyStringToCast.size() + curKeyStr.size() > ::KeycastConfig.maxSize)
        ::KeyStringToCast = L"";
    ::KeyStringToCast += curKeyStr;
    InvalidateRect(::D2DHwnd, nullptr, FALSE);
}

void HandleKeyUp(const KBDLLHOOKSTRUCT *s)
{
    DWORD vkeyCode = s->vkCode;
    if (VkCodeMap.count(vkeyCode))
    {
        HandleModifierKeyUp(vkeyCode);
        return;
    }
}

void HandleModifierKeyUp(DWORD vkcode)
{
    if (!::GetOneModifier(vkcode))
    {
        return;
    }
    ClearOneModifier(vkcode);
}

void SetOneModifier(DWORD vkCode)
{
    ModifierKey curKey = VkCodeMap.at(vkCode);
    UINT mask = 1 << curKey;
    ::ModifierKeyState |= mask;
}

UINT GetOneModifier(DWORD vkCode)
{
    ModifierKey curKey = VkCodeMap.at(vkCode);
    return (::ModifierKeyState >> curKey) & 1;
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

// We do not consider two shift keys are pressed at the same time
bool IsOnlyOneShiftPressed()
{
    UINT state = 0;
    ModifierKey lShift = VkCodeMap.at(VK_LSHIFT);
    UINT mask = 1 << lShift;
    state |= mask;
    if (state == ::ModifierKeyState)
    {
        return true;
    }
    state = 0;
    ModifierKey rShift = VkCodeMap.at(VK_RSHIFT);
    mask = 1 << rShift;
    state |= mask;
    if (state == ::ModifierKeyState)
    {
        return true;
    }
    return false;
}