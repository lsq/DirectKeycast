#include "CTimer.h"
#include "config.h"
#include "d2d.h"
#include "globals.h"
#include "hook.h"
#include "settings.h"
#include "tray_menu.h"
#include "window.h"
#include "msimeui/Application.h"
#include <winuser.h>

#define HOTKEY_ID 1

CTimer g_timerHide;
CTimer g_timerShow;

void OnHideWindow()
{
    if (::D2DHwnd)
    {
        PostMessage(::D2DHwnd, WM_APP_HIDE_OVERLAY, 0, 0);
    }
}

void OnShowWindow()
{
    if (::D2DHwnd && ::KeycastConfig.enabled)
    {
        PostMessage(::D2DHwnd, WM_APP_SHOW_OVERLAY, 0, 0);
    }
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    const bool msimeuiReady = msimeui::Application::Initialize();
    if (!msimeuiReady)
    {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    }

    UnregisterHotKey(nullptr, HOTKEY_ID);
    if (!RegisterHotKey(nullptr, HOTKEY_ID, MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_F12))
    {
        MessageBox(nullptr, L"Cannot Register Hotkey！", L"Error", MB_ICONERROR);
        return 1;
    }

    ::InitializeConfig();

    if (!InitD2DAndDWrite())
    {
        return -1;
    }

    HWND hwnd = CreateTransparentWindow(hInstance);
    if (!hwnd)
    {
        return -1;
    }
    D2DHwnd = hwnd;

    CreateTrayMenuWindow(hInstance);
    ShowWindow(hwnd, SW_HIDE);
    AddTrayIcon(hwnd);
    StartInputCapture(hwnd);

    g_timerHide.OnTimedEvent = OnHideWindow;
    g_timerShow.OnTimedEvent = OnShowWindow;

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_HOTKEY && msg.wParam == HOTKEY_ID)
        {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterHotKey(nullptr, HOTKEY_ID);
    StopInputCapture();
    CloseSettingsWindow();
    DestroyTrayMenuWindow();
    RemoveTrayIcon();
    ReleaseOverlayD2D();
    if (msimeuiReady)
    {
        msimeui::Application::Shutdown();
    }
    return (int)msg.wParam;
}
