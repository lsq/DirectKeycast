#include "window.h"
#include "d2d.h"
#include "globals.h"
#include <debugapi.h>

HWND CreateTransparentWindow(HINSTANCE hInstance)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DirectKeycastClass";

    if (!RegisterClass(&wc))
        return nullptr;

    // Get Main Mointor Handle
    HMONITOR hMonitor = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
    // Monitor Infor(exclude taskbar area)
    MONITORINFO monitorInfo = {0};
    monitorInfo.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(hMonitor, &monitorInfo);
    int workAreaWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
    int workAreaHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

    int windowWidth = workAreaWidth / 2;
    int windowHeight = workAreaHeight / 2;
    int windowX = monitorInfo.rcWork.right - windowWidth;
    int windowY = monitorInfo.rcWork.bottom - windowHeight;

    ::WindowArea = {windowX, windowY, monitorInfo.rcWork.right, monitorInfo.rcWork.bottom};

    UINT exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE;
    HWND hwnd = CreateWindowEx( //
        exStyle,                //
        wc.lpszClassName,       //
        L"DirectKeycastWindow", //
        WS_POPUP,               //
        windowX,                //
        windowY,                //
        windowWidth,            //
        windowHeight,           //
        nullptr,                //
        nullptr,                //
        hInstance,              //
        nullptr);

    if (hwnd)
    {
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

        MARGINS mar = {-1};
        DwmExtendFrameIntoClientArea(hwnd, &mar);
    }

    return hwnd;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        InitD2DRenderTarget(hwnd);
        InitGlobalConfigWithD2D();
        return 0;

    case WM_PAINT:
    case WM_DISPLAYCHANGE:
        OnPaint(hwnd);
        ValidateRect(hwnd, nullptr);
        return 0;
    case WM_MOUSEMOVE: {
        float x = (float)LOWORD(lParam);
        float y = (float)HIWORD(lParam);

        RECT rc;
        GetClientRect(hwnd, &rc);
        if (x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom)
        {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_TRAYICON: {
        if (lParam == WM_RBUTTONUP)
        {
            ShowTrayMenu(hwnd);
        }
        break;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_TRAY_EXIT)
        {
            PostQuitMessage(0);
        }
        break;
    }

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

// System tray
void AddTrayIcon(HWND hwnd)
{
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION); // Icon
    wcscpy_s(nid.szTip, L"DirectKeycast");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void RemoveTrayIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void ShowTrayMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT, L"exit");

    // Pop up the menu at the current mouse position
    SetForegroundWindow(hwnd);
    // TODO: Pop up position need to consider different monitor resolution
    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x + 10, pt.y - 25, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}