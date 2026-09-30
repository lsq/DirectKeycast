#include "msimeui_host.h"
#include "theme.h"
#include <dwmapi.h>
#include <shellscalingapi.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif

#pragma comment(lib, "shcore.lib")

namespace
{
constexpr wchar_t kOrigProcProp[] = L"DkMsimeuiOrigProc";
constexpr wchar_t kFlagsProp[] = L"DkMsimeuiFlags";
constexpr wchar_t kMinWidthProp[] = L"DkMsimeuiMinWidth";
constexpr wchar_t kMinHeightProp[] = L"DkMsimeuiMinHeight";

LRESULT CALLBACK HostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto *orig = reinterpret_cast<WNDPROC>(GetPropW(hwnd, kOrigProcProp));
    const DWORD flags = static_cast<DWORD>(reinterpret_cast<ULONG_PTR>(GetPropW(hwnd, kFlagsProp)));

    switch (message)
    {
    case WM_CLOSE:
        if (flags & DkMsimeuiHideOnClose)
        {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        break;
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        RemovePropW(hwnd, kOrigProcProp);
        RemovePropW(hwnd, kFlagsProp);
        RemovePropW(hwnd, kMinWidthProp);
        RemovePropW(hwnd, kMinHeightProp);
        return 0;
    case WM_MOUSEACTIVATE:
        if (flags & DkMsimeuiNoActivate)
        {
            return MA_NOACTIVATE;
        }
        break;
    case WM_GETMINMAXINFO:
        if (flags & DkMsimeuiMinSize)
        {
            auto *info = reinterpret_cast<MINMAXINFO *>(lParam);
            const UINT dpi = GetDpiForWindow(hwnd);
            const int minWidth = static_cast<int>(reinterpret_cast<LONG_PTR>(GetPropW(hwnd, kMinWidthProp)));
            const int minHeight = static_cast<int>(reinterpret_cast<LONG_PTR>(GetPropW(hwnd, kMinHeightProp)));
            info->ptMinTrackSize.x = DipToPxForDpi(static_cast<float>(minWidth), dpi);
            info->ptMinTrackSize.y = DipToPxForDpi(static_cast<float>(minHeight), dpi);
            return 0;
        }
        break;
    default:
        break;
    }

    if (orig)
    {
        return CallWindowProcW(orig, hwnd, message, wParam, lParam);
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}
} // namespace

void AttachMsimeuiHost(HWND hwnd, DWORD flags, int minWidthDip, int minHeightDip)
{
    if (!hwnd || GetPropW(hwnd, kOrigProcProp))
    {
        return;
    }

    auto *orig = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HostWndProc)));
    SetPropW(hwnd, kOrigProcProp, reinterpret_cast<HANDLE>(orig));
    SetPropW(hwnd, kFlagsProp, reinterpret_cast<HANDLE>(static_cast<ULONG_PTR>(flags)));
    SetPropW(hwnd, kMinWidthProp, reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(minWidthDip)));
    SetPropW(hwnd, kMinHeightProp, reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(minHeightDip)));
}

void ApplyImmersiveDarkMode(HWND hwnd, bool dark)
{
    if (!hwnd)
    {
        return;
    }
    BOOL value = dark ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

void ApplyCaptionColors(HWND hwnd, COLORREF caption, COLORREF text)
{
    if (!hwnd)
    {
        return;
    }
    // Windows 11 only; older builds ignore the attributes and keep the default caption.
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &caption, sizeof(caption));
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text, sizeof(text));
}

UINT MonitorDpiFromPoint(POINT pt)
{
    UINT dpiX = 96;
    UINT dpiY = 96;
    HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (FAILED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)) || dpiX == 0)
    {
        return 96;
    }
    return dpiX;
}

int DipToPxForDpi(float dip, UINT dpi)
{
    if (dpi == 0)
    {
        dpi = 96;
    }
    return static_cast<int>(dip * static_cast<float>(dpi) / 96.0f + 0.5f);
}
