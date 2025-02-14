#include "config.h"
#include "d2d.h"
#include "globals.h"
#include "hook.h"
#include "window.h"
#include <consoleapi2.h>

#ifdef FANY_DEBUG
#include "InitConsole.h"
#include <iostream>
#endif

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

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
    InitializeConfig();

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

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

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
