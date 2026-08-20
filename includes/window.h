#pragma once
#include "main.h"
#include <shellapi.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SETTINGS 1002
#define ID_TRAY_ABOUT 1003
#define ID_TRAY_TOGGLE 1004

inline NOTIFYICONDATA nid = {};

HWND CreateTransparentWindow(HINSTANCE hInstance);
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon();
void UpdateOverlayWindowPlacement(HWND hwnd);
bool IsPointInWindow(HWND hwnd, POINT screenPt);
