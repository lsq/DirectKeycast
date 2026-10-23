#include "hook.h"
#include "CTimer.h"
#include "d2d.h"
#include "globals.h"
#include "key_logger.h" // 新增头文件
#include "settings.h"
#include "tray_menu.h"
#include "utils.h"
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>
#include <winuser.h>

extern CTimer g_timerHide;

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
void appendCurrentKeyAndUpdateWnd(std::wstring curKeyStr);

namespace
{
constexpr int kFadeFallbackMs = 1500;

HWND g_uiHwnd = nullptr;
HANDLE g_hookThread = nullptr;
DWORD g_hookThreadId = 0;
HANDLE g_hookReady = nullptr;
HHOOK g_kbdHook = nullptr;
bool g_rawRegistered = false;

bool ShouldIgnoreMouseCast(POINT pt)
{
    return IsPointInTrayMenu(pt) || IsPointInSettingsWindow(pt);
}

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && g_uiHwnd)
    {
        const auto *s = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
        PostMessageW(g_uiHwnd, WM_APP_LLKEY, wParam, static_cast<LPARAM>(s->vkCode));
    }
    return CallNextHookEx(g_kbdHook, nCode, wParam, lParam);
}

DWORD WINAPI HookThreadProc(LPVOID)
{
    g_kbdHook = SetWindowsHookExW(WH_KEYBOARD_LL, KBDHook, GetModuleHandleW(nullptr), 0);
    if (g_hookReady)
    {
        SetEvent(g_hookReady);
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_kbdHook)
    {
        UnhookWindowsHookEx(g_kbdHook);
        g_kbdHook = nullptr;
    }
    return 0;
}

void CastMouseButton(const wchar_t *label, POINT pt)
{
    if (ShouldIgnoreMouseCast(pt))
    {
        return;
    }
    LogKeyEvent(label); // 独立于 showMouse 开关，始终记录
    if (!::KeycastConfig.showMouse)
    {
        return;
    }
    PulseOverlay();
    appendCurrentKeyAndUpdateWnd(label);
}
} // namespace

void PulseOverlay()
{
    if (!::KeycastConfig.enabled || !::D2DHwnd)
    {
        return;
    }
    if (!IsWindowVisible(::D2DHwnd))
    {
        ShowWindow(::D2DHwnd, SW_SHOWNOACTIVATE);
    }
    g_timerHide.Start(::KeycastConfig.fadeMs > 0 ? ::KeycastConfig.fadeMs : kFadeFallbackMs, false, true);
}

void UpdateMouseRawInput()
{
    if (!g_uiHwnd)
    {
        return;
    }
    const bool need = ::KeycastConfig.showMouse || IsTrayMenuVisible();
    if (need == g_rawRegistered)
    {
        return;
    }

    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    if (need)
    {
        rid.dwFlags = RIDEV_INPUTSINK;
        rid.hwndTarget = g_uiHwnd;
    }
    else
    {
        rid.dwFlags = RIDEV_REMOVE;
        rid.hwndTarget = nullptr;
    }
    if (RegisterRawInputDevices(&rid, 1, sizeof(rid)))
    {
        g_rawRegistered = need;
    }
}

void StartInputCapture(HWND hwnd)
{
    g_uiHwnd = hwnd;
    UpdateMouseRawInput();

    if (g_hookThread)
    {
        return;
    }
    g_hookReady = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_hookThread = CreateThread(nullptr, 0, HookThreadProc, nullptr, 0, &g_hookThreadId);
    if (g_hookThread && g_hookReady)
    {
        WaitForSingleObject(g_hookReady, 3000);
    }
}

void StopInputCapture()
{
    if (g_rawRegistered)
    {
        RAWINPUTDEVICE rid{};
        rid.usUsagePage = 0x01;
        rid.usUsage = 0x02;
        rid.dwFlags = RIDEV_REMOVE;
        rid.hwndTarget = nullptr;
        RegisterRawInputDevices(&rid, 1, sizeof(rid));
        g_rawRegistered = false;
    }

    if (g_hookThreadId)
    {
        PostThreadMessageW(g_hookThreadId, WM_QUIT, 0, 0);
    }
    if (g_hookThread)
    {
        WaitForSingleObject(g_hookThread, 3000);
        CloseHandle(g_hookThread);
        g_hookThread = nullptr;
        g_hookThreadId = 0;
    }
    if (g_hookReady)
    {
        CloseHandle(g_hookReady);
        g_hookReady = nullptr;
    }
    g_uiHwnd = nullptr;
}

void HandlePostedKeyEvent(WPARAM wParam, DWORD vkCode)
{
    KBDLLHOOKSTRUCT info{};
    info.vkCode = vkCode;
    switch (wParam)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        PulseOverlay();
        HandleKeyDown(&info);
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        HandleKeyUp(&info);
        break;
    default:
        break;
    }
}

void HandleRawMouseInput(HRAWINPUT hRaw)
{
    RAWINPUT raw{};
    UINT size = sizeof(raw);
    if (GetRawInputData(hRaw, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == static_cast<UINT>(-1))
    {
        return;
    }
    if (raw.header.dwType != RIM_TYPEMOUSE)
    {
        return;
    }

    const USHORT flags = raw.data.mouse.usButtonFlags;
    if (flags == 0)
    {
        return;
    }

    POINT pt{};
    GetCursorPos(&pt);

    if ((flags & (RI_MOUSE_LEFT_BUTTON_DOWN | RI_MOUSE_RIGHT_BUTTON_DOWN | RI_MOUSE_MIDDLE_BUTTON_DOWN)) && IsTrayMenuVisible())
    {
        const HWND hit = WindowFromPoint(pt);
        if (hit != ::TrayMenuHwnd && !IsPointInTrayMenu(pt))
        {
            PostMessageW(::D2DHwnd, WM_APP_HIDE_TRAY_MENU, 0, 0);
        }
    }

    if (flags & RI_MOUSE_LEFT_BUTTON_DOWN)
    {
        CastMouseButton(L"<LBtnDown>", pt);
    }
    if (flags & RI_MOUSE_LEFT_BUTTON_UP)
    {
        CastMouseButton(L"<LBtnUp>", pt);
    }
    if (flags & RI_MOUSE_RIGHT_BUTTON_DOWN)
    {
        CastMouseButton(L"<RBtnDown>", pt);
    }
    if (flags & RI_MOUSE_RIGHT_BUTTON_UP)
    {
        CastMouseButton(L"<RBtnUp>", pt);
    }
    if (flags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
    {
        CastMouseButton(L"<MBtnDown>", pt);
    }
    if (flags & RI_MOUSE_MIDDLE_BUTTON_UP)
    {
        CastMouseButton(L"<MBtnUp>", pt);
    }
    if (flags & RI_MOUSE_WHEEL)
    {
        const short delta = static_cast<short>(raw.data.mouse.usButtonData);
        CastMouseButton(delta > 0 ? L"<ScrollUp>" : L"<ScrollDown>", pt);
    }
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
        if (IsOnlyOneShiftPressed() && KeyCastMapOnShift().count(vkeyCode))
        {
            auto curKeyStr = KeyCastMapOnShift().at(vkeyCode);
            LogKeyEvent(curKeyStr); // 新增
            if (::KeyStringToCast.size() > static_cast<size_t>(::KeycastConfig.maxSize) || ::KeyStringToCast.size() + curKeyStr.size() > static_cast<size_t>(::KeycastConfig.maxSize))
                ::KeyStringToCast = L"";
            ::KeyStringToCast += curKeyStr;
            RequestOverlayRepaint();
            return;
        }
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
        LogKeyEvent(curResKeyStr); // 新增
        ::appendCurrentKeyAndUpdateWnd(curResKeyStr);
        return;
    }
    auto curKeyStr = KeyCastMap().at(vkeyCode);
    LogKeyEvent(curKeyStr); // 新增
    ::appendCurrentKeyAndUpdateWnd(curKeyStr);
}

void HandleModifierKeyDown(DWORD vkcode)
{
    if (::GetOneModifier(vkcode))
    {
        return;
    }
    SetOneModifier(vkcode);
    auto curKeyStr = KeyCastMap().at(vkcode);
    LogKeyEvent(curKeyStr); // 新增
    ::appendCurrentKeyAndUpdateWnd(curKeyStr);
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

void appendCurrentKeyAndUpdateWnd(std::wstring curKeyStr)
{
    if (!::KeycastConfig.enabled)
    {
        return;
    }
    static std::wstring lastKey = L"";
    static int counter = 1;
    std::wstring curStrToBeAppend;
    if (curKeyStr.empty())
    {
        return;
    }
    if (::KeyStringToCast.empty())
    {
        counter = 1;
        lastKey = L"";
    }
    if (curKeyStr == lastKey && lastKey[0] == L'<' && lastKey[lastKey.size() - 1] == L'>')
    {
        counter += 1;
        size_t pos = ::KeyStringToCast.find_last_of(L'<');
        if (pos != std::wstring::npos)
        {
            ::KeyStringToCast = ::KeyStringToCast.substr(0, pos);
            curStrToBeAppend = curKeyStr.substr(0, curKeyStr.size() - 1) + L"x" + std::to_wstring(counter) + L">";
        }
    }
    else
    {
        counter = 1;
        curStrToBeAppend = curKeyStr;
    }
    if (::KeyStringToCast.size() + curStrToBeAppend.size() > static_cast<size_t>(::KeycastConfig.maxSize))
    {
        ::KeyStringToCast = L"";
    }
    ::KeyStringToCast += curStrToBeAppend;
    RequestOverlayRepaint();
    lastKey = curKeyStr;
}
