#include "CTimer.h"
#include "config.h"
#include "d2d.h"
#include "globals.h"
#include "hook.h"
#include "window.h"
#include <winuser.h>

#define HOTKEY_ID 1

CTimer g_timerHide;
CTimer g_timerShow;

void OnHideWindow()
{
    ShowWindow(::D2DHwnd, SW_HIDE);
    ::KeyStringToCast = L"";
}

void OnShowWindow()
{
    ShowWindow(::D2DHwnd, SW_SHOW);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Register Ctrl + Alt + Shift + F12 hotkey
    UnregisterHotKey(nullptr, HOTKEY_ID);
    if (!RegisterHotKey(nullptr, HOTKEY_ID, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_F12))
    {
        MessageBox(nullptr, L"Cannot Register Hotkey！", L"Error", MB_ICONERROR);
        return 1;
    }

    //
    // Init config
    //
    ::InitializeConfig();

    HHOOK kbd = SetWindowsHookEx(WH_KEYBOARD_LL, &KBDHook, 0, 0);
    HHOOK mouse = SetWindowsHookEx(WH_MOUSE_LL, &MOUSEHook, 0, 0);

    if (!InitD2DAndDWrite())
        return -1;

    HWND hwnd = CreateTransparentWindow(hInstance);

    if (!hwnd)
        return -1;

    D2DHwnd = hwnd;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    AddTrayIcon(hwnd);

    g_timerHide.OnTimedEvent = OnHideWindow;
    g_timerShow.OnTimedEvent = OnShowWindow;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_HOTKEY && msg.wParam == HOTKEY_ID)
        {
            break; // Exit
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Release hotkey
    UnregisterHotKey(nullptr, HOTKEY_ID);

    if (pBrush)
        pBrush->Release();
    if (pRenderTarget)
        pRenderTarget->Release();
    if (pDWriteFactory)
        pDWriteFactory->Release();
    if (pTextFormat)
        pTextFormat->Release();
    if (pD2DFactory)
        pD2DFactory->Release();

    // Uninstall hook
    UnhookWindowsHookEx(mouse);
    UnhookWindowsHookEx(kbd);
    return (int)msg.wParam;
}
