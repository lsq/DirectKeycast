#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

void InitializeConfig();
bool LoadAppConfig();
bool SaveAppConfig();
void ApplyConfigToRuntime();
// Restores every setting to its default, then saves and applies it.
void ResetAppConfig();
// Shows/hides the key overlay, persisting the choice (tray menu, hotkey, settings).
void SetOverlayEnabled(bool enabled);

std::filesystem::path GetConfigDirectory();
std::filesystem::path GetConfigFilePath();
const std::vector<std::wstring> &GetSystemFontFamilies();
bool OpenConfigFile();
bool OpenConfigDirectory();
