#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <windows.h>

void InitializeConfig();
bool LoadAppConfig();
bool SaveAppConfig();
void ApplyConfigToRuntime();

std::filesystem::path GetConfigDirectory();
std::filesystem::path GetConfigFilePath();
const std::vector<std::wstring> &GetSystemFontFamilies();
bool OpenConfigFile();
bool OpenConfigDirectory();
