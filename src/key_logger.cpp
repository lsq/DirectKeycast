// src/key_logger.cpp
#include "key_logger.h"
#include "config.h"  // for GetLogDirectory()
#include "globals.h" // for KeycastConfig.logToExeDir
#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <windows.h>

namespace
{
std::wofstream g_logFile;
std::mutex g_logMutex;
std::filesystem::path g_currentLogPath;
int g_pendingWrites = 0;
std::chrono::steady_clock::time_point g_lastFlush = std::chrono::steady_clock::now();

constexpr int kFlushEveryNWrites = 20;                   // flush after N buffered events
constexpr auto kFlushInterval = std::chrono::seconds(3); // or after this much time elapses

void FlushIfDue_Locked()
{
    const auto now = std::chrono::steady_clock::now();
    if (g_pendingWrites >= kFlushEveryNWrites || (now - g_lastFlush) >= kFlushInterval)
    {
        g_logFile.flush();
        g_pendingWrites = 0;
        g_lastFlush = now;
    }
}

std::wstring Timestamp()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[32];
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return buf;
}

void OpenLogFile_Locked()
{
    const auto path = GetLogFilePath(); // resolves exe-dir vs config-dir based on config
    if (g_logFile.is_open() && path == g_currentLogPath)
    {
        return; // already open at the right location
    }
    if (g_logFile.is_open())
    {
        g_logFile.flush();
        g_logFile.close();
    }
    g_currentLogPath = path;
    // Use a decently sized stream buffer so writes coalesce in memory before hitting disk.
    g_logFile.open(path, std::ios::app);
}
} // namespace

void InitKeyLogger()
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    OpenLogFile_Locked();
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

// Call this whenever the "log location" setting changes at runtime (from settings UI).
void ReopenKeyLoggerIfPathChanged()
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    OpenLogFile_Locked();
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
    ++g_pendingWrites;
    FlushIfDue_Locked();
}
