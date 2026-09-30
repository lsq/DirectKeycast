#pragma once
#include <windows.h>

enum DkMsimeuiHostFlags : DWORD
{
    DkMsimeuiHideOnClose = 1u,
    DkMsimeuiNoActivate = 2u,
    DkMsimeuiMinSize = 4u,
};

void AttachMsimeuiHost(HWND hwnd, DWORD flags, int minWidthDip = 0, int minHeightDip = 0);
void ApplyImmersiveDarkMode(HWND hwnd, bool dark);
void ApplyCaptionColors(HWND hwnd, COLORREF caption, COLORREF text);
UINT MonitorDpiFromPoint(POINT pt);
int DipToPxForDpi(float dip, UINT dpi);
