#pragma once
#include <windows.h>

bool OpenSettingsWindow(bool about = false);
void CloseSettingsWindow();
bool IsSettingsWindowOpen();
void RefreshSettingsWindowTheme();
bool IsPointInSettingsWindow(POINT screenPt);
