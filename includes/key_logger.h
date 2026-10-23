#pragma once
#include <string>

// Independent of KeycastConfig.enabled / showMouse.
// Logs every key / mouse event to a file next to the executable.
void InitKeyLogger();
void ShutdownKeyLogger();
void LogKeyEvent(const std::wstring &text);
