#include "msimeui_host.h"
#include "theme.h"
#include <dwmapi.h>
#include <shellscalingapi.h>
#include <windowsx.h>

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
constexpr wchar_t kCaptionProp[] = L"DkMsimeuiCaption";

struct CaptionState
{
    DkCustomCaption caption;
    LRESULT hot = HTNOWHERE;
    LRESULT pressed = HTNOWHERE;
    bool tracking = false;
};

LRESULT CallOriginal(WNDPROC orig, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return orig ? CallWindowProcW(orig, hwnd, message, wParam, lParam) : DefWindowProcW(hwnd, message, wParam, lParam);
}

bool IsCaptionButton(WPARAM hit)
{
    return hit == HTMINBUTTON || hit == HTMAXBUTTON || hit == HTCLOSE;
}

// Height of the (invisible) resize border the removed top frame used to provide.
int TopFrameThickness(HWND hwnd)
{
    const UINT dpi = GetDpiForWindow(hwnd);
    return GetSystemMetricsForDpi(SM_CYFRAME, dpi) + GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
}

void SetCaptionButtonState(CaptionState &state, LRESULT hot, LRESULT pressed)
{
    if (state.hot == hot && state.pressed == pressed)
    {
        return;
    }
    state.hot = hot;
    state.pressed = pressed;
    if (state.caption.setButtonState)
    {
        state.caption.setButtonState(hot, pressed);
    }
}

// Returns true when the message is fully handled and `result` holds its result.
bool HandleCaptionMessage(HWND hwnd, CaptionState &state, WNDPROC orig, UINT message, WPARAM wParam, LPARAM lParam, LRESULT &result)
{
    switch (message)
    {
    case WM_NCCALCSIZE:
    {
        if (!wParam)
        {
            return false;
        }
        auto *params = reinterpret_cast<NCCALCSIZE_PARAMS *>(lParam);
        const LONG top = params->rgrc[0].top;
        result = CallOriginal(orig, hwnd, message, wParam, lParam);
        // Keep the side and bottom resize borders, hand the caption strip to the client.
        params->rgrc[0].top = top;
        if (IsZoomed(hwnd))
        {
            // A maximized window hangs its frame off the monitor edge.
            params->rgrc[0].top += TopFrameThickness(hwnd);
        }
        return true;
    }
    case WM_NCHITTEST:
    {
        result = CallOriginal(orig, hwnd, message, wParam, lParam);
        if (result != HTCLIENT)
        {
            return true;
        }
        POINT point = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        ScreenToClient(hwnd, &point);
        if (!IsZoomed(hwnd) && point.y < TopFrameThickness(hwnd))
        {
            result = HTTOP;
        }
        else if (state.caption.hitTest)
        {
            result = state.caption.hitTest(point);
        }
        return true;
    }
    case WM_NCMOUSEMOVE:
        if (!state.tracking)
        {
            TRACKMOUSEEVENT tracking = {sizeof(tracking), TME_LEAVE | TME_NONCLIENT, hwnd, 0};
            state.tracking = TrackMouseEvent(&tracking) != FALSE;
        }
        SetCaptionButtonState(state, IsCaptionButton(wParam) ? static_cast<LRESULT>(wParam) : HTNOWHERE, state.pressed == static_cast<LRESULT>(wParam) ? state.pressed : HTNOWHERE);
        return false;
    case WM_NCMOUSELEAVE:
        state.tracking = false;
        SetCaptionButtonState(state, HTNOWHERE, HTNOWHERE);
        return false;
    case WM_MOUSEMOVE:
        SetCaptionButtonState(state, HTNOWHERE, HTNOWHERE);
        return false;
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK:
        if (!IsCaptionButton(wParam))
        {
            return false;
        }
        // Swallowed so DefWindowProc does not paint its classic caption buttons.
        SetCaptionButtonState(state, static_cast<LRESULT>(wParam), static_cast<LRESULT>(wParam));
        result = 0;
        return true;
    case WM_NCLBUTTONUP:
    {
        if (!IsCaptionButton(wParam))
        {
            return false;
        }
        const bool clicked = state.pressed == static_cast<LRESULT>(wParam);
        SetCaptionButtonState(state, static_cast<LRESULT>(wParam), HTNOWHERE);
        if (clicked)
        {
            const WPARAM command = wParam == HTMINBUTTON ? SC_MINIMIZE : wParam == HTCLOSE ? SC_CLOSE : (IsZoomed(hwnd) ? SC_RESTORE : SC_MAXIMIZE);
            PostMessageW(hwnd, WM_SYSCOMMAND, command, 0);
        }
        result = 0;
        return true;
    }
    case WM_ACTIVATE:
        // The caption title dims while the window is inactive.
        InvalidateRect(hwnd, nullptr, FALSE);
        return false;
    default:
        return false;
    }
}

LRESULT CALLBACK HostWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto *orig = reinterpret_cast<WNDPROC>(GetPropW(hwnd, kOrigProcProp));
    const DWORD flags = static_cast<DWORD>(reinterpret_cast<ULONG_PTR>(GetPropW(hwnd, kFlagsProp)));

    if (auto *caption = reinterpret_cast<CaptionState *>(GetPropW(hwnd, kCaptionProp)))
    {
        LRESULT result = 0;
        if (HandleCaptionMessage(hwnd, *caption, orig, message, wParam, lParam, result))
        {
            return result;
        }
    }

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
        delete reinterpret_cast<CaptionState *>(RemovePropW(hwnd, kCaptionProp));
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

    return CallOriginal(orig, hwnd, message, wParam, lParam);
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

void AttachCustomCaption(HWND hwnd, DkCustomCaption caption)
{
    if (!hwnd || !GetPropW(hwnd, kOrigProcProp) || GetPropW(hwnd, kCaptionProp))
    {
        return;
    }
    auto *state = new CaptionState{std::move(caption)};
    SetPropW(hwnd, kCaptionProp, reinterpret_cast<HANDLE>(state));
    // Re-run WM_NCCALCSIZE so the caption strip turns into client area.
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
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
