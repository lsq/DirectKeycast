#include "window.h"
#include "d2d.h"

HWND CreateTransparentWindow(HINSTANCE hInstance)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DirectKeycastClass";

    if (!RegisterClass(&wc))
        return nullptr;

    HWND hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW, //
                               wc.lpszClassName,                                 //
                               L"DirectKeycastWindow",                           //
                               WS_POPUP,                                         //
                               100, 100, 800, 600,                               //
                               nullptr,                                          //
                               nullptr,                                          //
                               hInstance,                                        //
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

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}