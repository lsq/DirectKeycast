#include "utils.h"

std::wstring string_to_wstring(const std::string &str)
{
    if (str.empty())
        return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (size_needed <= 0)
    {
        throw std::runtime_error("MultiByteToWideChar failed");
    }
    std::wstring wstr(size_needed, 0);
    int result = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    if (result <= 0)
    {
        throw std::runtime_error("MultiByteToWideChar failed");
    }
    return wstr;
}

std::string wstring_to_string(const std::wstring &wstr)
{
    if (wstr.empty())
        return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0)
    {
        throw std::runtime_error("WideCharToMultiByte failed");
    }
    std::string str(size_needed, 0);
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    if (result <= 0)
    {
        throw std::runtime_error("WideCharToMultiByte failed");
    }
    return str;
}