#pragma once
#include "main.h"
#include <shellapi.h>

// System tray
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001

inline NOTIFYICONDATA nid = {};

HWND CreateTransparentWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void AddTrayIcon(HWND hwnd);
void RemoveTrayIcon();
void ShowTrayMenu(HWND hwnd);