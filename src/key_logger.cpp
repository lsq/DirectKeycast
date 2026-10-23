#include "key_logger.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <windows.h>

namespace
{
std::wofstream g_logFile;
std::mutex g_logMutex;

std::filesystem::path GetExeDirectory()
{
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

std::wstring Timestamp()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[32];
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return buf;
}
} // namespace

void InitKeyLogger()
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (g_logFile.is_open())
    {
        return;
    }
    const auto path = GetExeDirectory() / L"keycast_log.txt";
    // Keep the handle open for the whole process lifetime; append mode.
    g_logFile.open(path, std::ios::app);
    if (g_logFile.is_open())
    {
        g_logFile.imbue(std::locale(""));
    }
}

void ShutdownKeyLogger()
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (g_logFile.is_open())
    {
        g_logFile.flush();
        g_logFile.close();
    }
}

void LogKeyEvent(const std::wstring &text)
{
    if (text.empty())
    {
        return;
    }
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (!g_logFile.is_open())
    {
        return;
    }
    g_logFile << L"[" << Timestamp() << L"] " << text << L"\n";
    g_logFile.flush(); // small perf cost but guarantees no data loss on crash
}
