#pragma once
#include <functional>
#include <windows.h>

enum DkMsimeuiHostFlags : DWORD
{
    DkMsimeuiHideOnClose = 1u,
    DkMsimeuiNoActivate = 2u,
    DkMsimeuiMinSize = 4u,
};

void AttachMsimeuiHost(HWND hwnd, DWORD flags, int minWidthDip = 0, int minHeightDip = 0);

// Caption drawn by the scene: the native caption strip becomes client area, and
// `hitTest` maps a client point (pixels) to HTCAPTION / HTMINBUTTON / HTMAXBUTTON /
// HTCLOSE, or HTCLIENT. Answering HTMAXBUTTON is what lets Windows 11 show the
// Snap Layouts flyout. `setButtonState` reports the hovered and pressed buttons
// (HTNOWHERE for none) so the scene can paint them. Call after AttachMsimeuiHost.
struct DkCustomCaption
{
    std::function<LRESULT(POINT clientPt)> hitTest;
    std::function<void(LRESULT hot, LRESULT pressed)> setButtonState;
};
void AttachCustomCaption(HWND hwnd, DkCustomCaption caption);
void ApplyImmersiveDarkMode(HWND hwnd, bool dark);
void ApplyCaptionColors(HWND hwnd, COLORREF caption, COLORREF text);
UINT MonitorDpiFromPoint(POINT pt);
int DipToPxForDpi(float dip, UINT dpi);
