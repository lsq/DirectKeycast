#pragma once
#include <windows.h>

bool CreateTrayMenuWindow(HINSTANCE hInstance);
void ShowTrayMenu(HWND owner);
void HideTrayMenu();
bool IsTrayMenuVisible();
void RefreshTrayMenuTheme();
void DestroyTrayMenuWindow();
bool IsPointInTrayMenu(POINT screenPt);
