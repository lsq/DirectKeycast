#include "CTimer.h"
#include "config.h"
#include "d2d.h"
#include "globals.h"
#include "hook.h"
#include "window.h"
#include <consoleapi2.h>
#include <winuser.h>

#ifdef FANY_DEBUG
#include "InitConsole.h"
#include <iostream>
#endif

#define HOTKEY_ID 1
#ifdef __MINGW64__
/* function call for direct to dll */

/* select for highest DPI */
typedef BOOL (WINAPI *PGNSI)(DPI_AWARENESS_CONTEXT); 
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

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
#ifdef __MINGW64__
    /* turn off scaling. The following call gets around the lack of a
       declaration in mingw */
    PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
                                   "SetProcessDpiAwarenessContext");
    if (NULL != pGNSI) {

       BOOL r = pGNSI(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
       // if (!r) winerr();
       if (!r) return -1;

    }
#else
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

    // Register Ctrl + Alt + F12 hotkey
    if (!RegisterHotKey(nullptr, HOTKEY_ID, MOD_CONTROL | MOD_ALT, VK_F12))
    {
        MessageBox(nullptr, L"Cannot Register Hotkey！", L"Error", MB_ICONERROR);
        return 1;
    }

#ifdef FANY_DEBUG
    if (AllocConsole())
    {
        FILE *pCout;
        freopen_s(&pCout, "CONOUT$", "w", stdout);
        SetConsoleTitleW(L"Fany Debug");
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
    }
    outbuf ob;
    std::streambuf *sb = std::cout.rdbuf(&ob);
    printf("Debug console started successfully.\n");
    std::cout.rdbuf(sb);
#endif

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
