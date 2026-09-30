#pragma once
#include <windows.h>

bool OpenSettingsWindow(bool about = false);
void CloseSettingsWindow();
bool IsSettingsWindowOpen();
// Re-applies theme colors and pushes current config values into the controls.
void RefreshSettingsWindow();
bool IsPointInSettingsWindow(POINT screenPt);
