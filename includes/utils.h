#pragma once
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <windows.h>

std::wstring string_to_wstring(const std::string &str);
std::string wstring_to_string(const std::wstring &wstr);

const std::unordered_map<int, std::string> &KeyCastMap();