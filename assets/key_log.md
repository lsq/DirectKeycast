[增加日志功能](https://deepwiki.com/search/_7e64f2b4-c755-4e9a-bad1-9856c814b35f?mode=deep)
# DeepWiki Q&A with Code Context for Repository: fanlusky/DirectKeycast
## Q1
在“显示按键开关”关闭后，屏幕不显示按键，增加功能，与开关无关，在程序同文件夹下保存每个按键的序列（包括鼠标点击）
### Answer
I should start by exploring the repo structure to get a sense of the project.

I'm now seeing how the config connects to the overlay: `AppConfig::enabled` toggles the "显示按键浮层" setting, which hides the `D2DHwnd` overlay through `SetOverlayEnabled`/`ApplyConfigToRuntime`, while `appendCurrentKeyAndUpdateWnd` in hook.cpp checks that same enabled flag before proceeding.

That means when the overlay is disabled, key events never get processed at all, and mouse button tracking depends separately on `showMouse`. I need to add a logging feature that works independently of the overlay toggle, recording every key and mouse click to a file next to the executable regardless of display state -- so I should check config.cpp and globals.h for the directory/path utilities and see whether a logging module already exists.

This feature doesn't currently exist in the codebase. Currently key/mouse logging is fully gated by the display toggle:

- `AppConfig::enabled` ("显示按键浮层") controls whether the overlay is shown, and is toggled via `SetOverlayEnabled` in `src/config.cpp:339-348`, which calls `ApplyConfigToRuntime` to hide `D2DHwnd` when disabled DirectKeycast:339-348 .
- Crucially, the keyboard handler `appendCurrentKeyAndUpdateWnd` in `src/hook.cpp` bails out immediately `if (!::KeycastConfig.enabled) return;`, meaning when overlay display is off, key events aren't even accumulated into `KeyStringToCast`, let alone recorded anywhere DirectKeycast:436-441 .
- Mouse clicks go through `CastMouseButton` in `src/hook.cpp:102-110`, which is gated by `::KeycastConfig.showMouse` (a separate toggle) and also calls `appendCurrentKeyAndUpdateWnd`, which has the same `enabled` early-return DirectKeycast:102-110 .
- There is no existing file-logging mechanism anywhere in the codebase — `src/config.cpp` only persists `config.toml` settings via `GetConfigDirectory`/`GetConfigFilePath`, which resolve to `%LocalAppData%\DirectKeycast`, not "the same folder as the program" DirectKeycast:276-297 .

To implement what you're asking for — recording every key/mouse sequence to a file in the executable's own directory, independent of the "显示按键浮层" (show overlay) toggle — the key changes needed are:
1. Add a new logging module that resolves the exe's own directory (via `GetModuleFileNameW`, not `GetConfigDirectory`), and appends key/mouse event strings to a log file there.
2. Decouple key/mouse event capture from `::KeycastConfig.enabled`: currently `HandleKeyDown`/`HandleModifierKeyDown`/`CastMouseButton` all funnel through `appendCurrentKeyAndUpdateWnd`, which returns early when `enabled` is false — the new logging call needs to happen regardless of that flag, likely by splitting "update overlay display string" from "log to file" into two separate code paths.


在仓库 fanlusky/DirectKeycast 中实现一个新的按键/鼠标事件日志功能，要求记录行为与“显示按键浮层”开关（`KeycastConfig.enabled`，位于 `includes/globals.h`）完全无关，即使该开关关闭、浮层不显示，也要持续记录按键与鼠标点击序列。

背景：
- 当前 `src/hook.cpp` 中的 `appendCurrentKeyAndUpdateWnd` 函数在 `::KeycastConfig.enabled` 为 false 时直接 return，导致按键事件完全不会被处理/记录（`src/hook.cpp` 约第 436-441 行）。
- 按键事件的捕获入口为 `HandleKeyDown`、`HandleModifierKeyDown`（在 `src/hook.cpp`），鼠标事件的捕获入口为 `HandleRawMouseInput` -> `CastMouseButton`（同文件），鼠标记录目前还受 `KeycastConfig.showMouse` 开关控制。
- 现有配置/文件路径逻辑在 `src/config.cpp` 的 `GetConfigDirectory()`/`GetConfigFilePath()` 中，使用的是 `%LocalAppData%\DirectKeycast`，而不是程序所在目录；需要新增一个单独的函数获取可执行文件自身所在目录（可用 `GetModuleFileNameW(nullptr, ...)` 实现），而不是复用 `GetConfigDirectory`。

需要实现的改动：
1. 新增一个日志模块（例如 `src/key_logger.cpp` / `includes/key_logger.h`），提供类似 `void LogKeyEvent(const std::wstring &text)` 的接口，内部：
   - 通过 `GetModuleFileNameW` 获取当前 exe 的完整路径，取其所在目录作为日志文件存放位置（即“程序同文件夹下”）。
   - 以追加模式打开一个日志文件（例如 `keycast_log.txt`，可按日期生成文件名，如 `keycast_log_YYYYMMDD.txt`，具体命名可自行决定但要清晰可维护），将事件文本（含时间戳）写入文件。
   - 注意线程安全：键盘钩子回调运行在独立的钩子线程（`g_hookThread`，见 `src/hook.cpp` 的 `HookThreadProc`），而鼠标事件通过 `WM_APP_LLKEY`/原始输入消息在 UI 线程处理，写文件操作需要加锁（例如用 `std::mutex`）以避免并发写入损坏文件。
   - 文件写入要考虑性能，避免每次按键都重新打开关闭文件造成明显卡顿；可以保持文件句柄打开，或使用缓冲写入。

2. 修改 `src/hook.cpp`：
   - 将“记录到日志文件”的逻辑与“更新浮层显示字符串 `KeyStringToCast`”的逻辑解耦。目前两者都在 `appendCurrentKeyAndUpdateWnd` 内，且被 `::KeycastConfig.enabled` 提前拦截。
   - 在 `HandleKeyDown`、`HandleModifierKeyDown`、`CastMouseButton`（或更底层的统一入口处）中，新增对日志模块的调用，该调用必须在 `::KeycastConfig.enabled` 判断之前执行（或完全绕开该判断），确保无论浮层是否显示都会被记录。
   - 鼠标点击记录目前依赖 `::KeycastConfig.showMouse`（见 `CastMouseButton`，`src/hook.cpp` 约102-110行）；根据需求“与开关无关”，需要让日志记录不受 `showMouse` 开关影响（除非用户确认希望鼠标记录仍遵循 `showMouse` 开关，需要在实现前与用户确认这一点，若无法确认则默认鼠标点击记录也不受任何开关限制，记录全部按键与鼠标点击）。
   - 记录的内容应包含每个按键的标识（可复用 `KeyCastMap()`、`KeyCastMapOnShift()` 中已有的按键文本映射，定义在 `src/utils.cpp`）以及修饰键组合信息（`HandleKeyDown` 中已有拼接修饰键字符串的逻辑，可以复用或提炼为公共函数），鼠标点击/滚轮事件同样复用 `CastMouseButton` 中已有的标签（如 `<LBtnDown>`、`<RBtnUp>`、`<ScrollUp>` 等）。
   - 建议记录格式为：时间戳 + 事件文本，一行一个事件，便于后续查看和分析。

3. 不要修改 `AppConfig::enabled` 字段本身的语义或现有浮层显示逻辑（`ApplyConfigToRuntime`、`SetOverlayEnabled`，位于 `src/config.cpp`），仅新增独立的日志路径，不影响原有“显示/隐藏浮层”功能。

4. 在 `src/main.cpp` 的 `wWinMain` 中，适当位置初始化日志模块（如打开日志文件/创建互斥锁），并在程序退出清理流程中关闭文件句柄，参考现有 `StartInputCapture`/`StopInputCapture` 的初始化与清理顺序。

5. 更新 `CMakeLists.txt`（如果新增了独立的 `.cpp` 源文件）以确保新文件被纳入编译。

6. 构建并验证：确认在设置界面中关闭“显示按键浮层”开关后，程序目录下仍能生成/追加包含按键与鼠标点击记录的日志文件；同时确认开关打开时浮层显示功能不受影响。
### Citations
**File:** src/config.cpp (L276-297)
```cpp
std::filesystem::path GetConfigDirectory()
{
    PWSTR appData = nullptr;
    std::filesystem::path dir;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appData)) && appData)
    {
        dir = std::filesystem::path(appData) / L"DirectKeycast";
        CoTaskMemFree(appData);
    }
    else
    {
        dir = std::filesystem::current_path() / L"config";
    }
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}

std::filesystem::path GetConfigFilePath()
{
    return GetConfigDirectory() / L"config.toml";
}
```
**File:** src/config.cpp (L339-348)
```cpp
void SetOverlayEnabled(bool enabled)
{
    if (::KeycastConfig.enabled == enabled)
    {
        return;
    }
    ::KeycastConfig.enabled = enabled;
    SaveAppConfig();
    ApplyConfigToRuntime();
}
```
**File:** src/hook.cpp (L102-110)
```cpp
void CastMouseButton(const wchar_t *label, POINT pt)
{
    if (!::KeycastConfig.showMouse || ShouldIgnoreMouseCast(pt))
    {
        return;
    }
    PulseOverlay();
    appendCurrentKeyAndUpdateWnd(label);
}
```
**File:** src/hook.cpp (L436-441)
```cpp
void appendCurrentKeyAndUpdateWnd(std::wstring curKeyStr)
{
    if (!::KeycastConfig.enabled)
    {
        return;
    }
```
## Q2
很好，请给出实现代码
### Answer
I'm realizing I'm in ask-only mode, so even though the user wants actual implementation code, I shouldn't write it out directly—I should provide a plan instead since I can't edit files myself.

Since full implementation requires file edits beyond ask-only capabilities, I'll provide a detailed plan with embedded code snippets for key_logger.h/cpp and hook.cpp modifications as guidance. First I need to review hook.cpp, globals.h, utils.cpp's KeyCastMap, config.cpp, main.cpp's capture functions, and CMakeLists.txt for context.

I should check the HandleKeyDown function, CMakeLists.txt, globals.h, main.cpp's StartInputCapture/StopInputCapture, and the utils files for KeyCastMap to get more context.

I have enough context now—since I'm in ask-only mode and can't make actual edits, I'll present the implementation directly in my response as code blocks, covering new files like key_logger.h/cpp plus the needed changes to hook.cpp, main.cpp, and CMakeLists.txt.

下面是具体实现代码建议（新增一个独立的按键/鼠标日志模块，写入 exe 同目录，不受“显示按键浮层”开关影响）。

## 1. 新增 `includes/key_logger.h`

```cpp
#pragma once
#include <string>

// Independent of KeycastConfig.enabled / showMouse.
// Logs every key / mouse event to a file next to the executable.
void InitKeyLogger();
void ShutdownKeyLogger();
void LogKeyEvent(const std::wstring &text);
```

## 2. 新增 `src/key_logger.cpp`

```cpp
#include "key_logger.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <ctime>

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
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
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
```

> 说明：`std::wofstream` 每次 flush 对高频按键有一定性能开销；如果后续发现有卡顿，可以改成定时 flush（例如每 N 次或每秒 flush 一次），而不是逐次 flush。

## 3. 修改 `src/hook.cpp`

在现有逻辑基础上，把"写日志"从 `appendCurrentKeyAndUpdateWnd`（它被 `KeycastConfig.enabled` 拦截）中解耦出来，新增一个不受开关影响的调用点：

```cpp
#include "key_logger.h"   // 新增头文件
```

在 `CastMouseButton` 中，把日志调用放在 `showMouse` 判断之前：

```cpp
void CastMouseButton(const wchar_t *label, POINT pt)
{
    if (ShouldIgnoreMouseCast(pt))
    {
        return;
    }
    LogKeyEvent(label); // 独立于 showMouse 开关，始终记录
    if (!::KeycastConfig.showMouse)
    {
        return;
    }
    PulseOverlay();
    appendCurrentKeyAndUpdateWnd(label);
}
```

在 `HandleKeyDown` 中，三个 return 分支（Shift 映射、组合键、普通键）分别在调用 `appendCurrentKeyAndUpdateWnd`/`RequestOverlayRepaint` 之前加上 `LogKeyEvent(...)`：

```cpp
if (IsOnlyOneShiftPressed() && KeyCastMapOnShift().count(vkeyCode))
{
    auto curKeyStr = KeyCastMapOnShift().at(vkeyCode);
    LogKeyEvent(curKeyStr); // 新增
    if (::KeyStringToCast.size() > static_cast<size_t>(::KeycastConfig.maxSize) || ...)
        ::KeyStringToCast = L"";
    ...
}
...
curResKeyStr = L"<" + curResKeyStr + L">";
LogKeyEvent(curResKeyStr); // 新增
::appendCurrentKeyAndUpdateWnd(curResKeyStr);
return;
...
auto curKeyStr = KeyCastMap().at(vkeyCode);
LogKeyEvent(curKeyStr); // 新增
::appendCurrentKeyAndUpdateWnd(curKeyStr);
```

`HandleModifierKeyDown` 同理：

```cpp
void HandleModifierKeyDown(DWORD vkcode)
{
    if (::GetOneModifier(vkcode))
    {
        return;
    }
    SetOneModifier(vkcode);
    auto curKeyStr = KeyCastMap().at(vkcode);
    LogKeyEvent(curKeyStr); // 新增
    ::appendCurrentKeyAndUpdateWnd(curKeyStr);
}
```

这样无论 `KeycastConfig.enabled` 是否为 `true`，日志都会记录；而浮层显示逻辑（`appendCurrentKeyAndUpdateWnd` 内部仍然检查 `enabled`）完全不受影响 DirectKeycast:436-441 。

## 4. 修改 `src/main.cpp`

```cpp
#include "key_logger.h"   // 新增头文件
...
    ::InitializeConfig();
    InitKeyLogger();       // 新增：在 InitializeConfig 之后、StartInputCapture 之前初始化

    if (!InitD2DAndDWrite())
    ...
    UnregisterHotkeys();
    StopInputCapture();
    ShutdownKeyLogger();   // 新增：进程退出前关闭文件句柄
    g_timerHide.Stop();
```

## 5. 修改 `CMakeLists.txt`

在 `SRC_FILES` 中加入新文件：

```cmake
    "./includes/key_logger.h"
    ...
    "./src/key_logger.cpp"
```

---

以上代码片段是给出的实现建议，尚未应用到仓库中。如果需要，我可以把这些改动整理成一个任务，交由后台 Devin agent 实际创建/修改文件并验证编译。


在仓库 fanlusky/DirectKeycast 中应用以下改动（具体代码已在对话中给出，可直接参考使用）：

1. 新增文件 `includes/key_logger.h` 和 `src/key_logger.cpp`：
   - 提供 `InitKeyLogger()`、`ShutdownKeyLogger()`、`LogKeyEvent(const std::wstring&)` 三个函数。
   - `InitKeyLogger` 通过 `GetModuleFileNameW(nullptr, ...)` 获取 exe 所在目录，在该目录下以追加模式打开 `keycast_log.txt`（`std::wofstream`），并保持文件句柄常驻直到 `ShutdownKeyLogger` 调用。
   - `LogKeyEvent` 使用 `std::mutex` 加锁，写入格式为 `[时间戳] 事件文本`，每行一条，调用后 flush（如后续发现性能问题可改为批量 flush）。
   - 时间戳通过 `GetLocalTime` 获取，精确到毫秒。

2. 修改 `src/hook.cpp`：
   - `#include "key_logger.h"`。
   - 在 `CastMouseButton`（约第102-110行）中，将 `LogKeyEvent(label)` 的调用移到 `showMouse` 判断**之前**，使鼠标事件记录完全独立于 `KeycastConfig.showMouse` 开关（仅保留 `ShouldIgnoreMouseCast` 过滤，用于排除点击在托盘菜单/设置窗口上的误触发）。
   - 在 `HandleKeyDown`（约第288-358行）的三个分支（Shift 映射键、组合键、普通按键）中，分别在调用 `appendCurrentKeyAndUpdateWnd` / 更新 `KeyStringToCast` 之前插入对应的 `LogKeyEvent(...)` 调用，确保无论 `KeycastConfig.enabled` 是否为真，都会记录。
   - 在 `HandleModifierKeyDown`（约第360-369行）中同样在 `appendCurrentKeyAndUpdateWnd` 调用前插入 `LogKeyEvent(curKeyStr)`。
   - 不要修改 `appendCurrentKeyAndUpdateWnd` 本身对 `KeycastConfig.enabled` 的提前 return 逻辑，该函数继续只负责浮层显示字符串的维护。

3. 修改 `src/main.cpp`：
   - `#include "key_logger.h"`。
   - 在 `wWinMain` 中 `::InitializeConfig();` 之后调用 `InitKeyLogger();`。
   - 在退出清理流程中，`StopInputCapture();` 之后调用 `ShutdownKeyLogger();`，确保文件被正确 flush/关闭。

4. 修改 `CMakeLists.txt` 的 `SRC_FILES` 列表，加入 `./includes/key_logger.h` 和 `./src/key_logger.cpp`。

5. 构建验证：
   - 确认工程能正常编译链接。
   - 手动测试：关闭设置中的"显示按键浮层"开关后，按键和鼠标点击仍然能在 exe 所在目录下的 `keycast_log.txt` 中被追加记录；开关打开时浮层显示功能和原有行为不受影响。
   - 检查高频连续按键/快速鼠标点击场景下程序无明显卡顿和文件写入异常（如有性能问题，调整为非逐次 flush 的缓冲写入策略）。
### Citations
**File:** src/hook.cpp (L436-441)
```cpp
void appendCurrentKeyAndUpdateWnd(std::wstring curKeyStr)
{
    if (!::KeycastConfig.enabled)
    {
        return;
    }
```
## Q3
1. 调整为非逐次 flush 的缓冲写入策略
2. 在配置文件加一个bool选项，true的时候日志文件存储在程序同目录，false存储在配置文件同目录
### Answer
The user wants two changes: buffered flushing instead of per-event, and a new config boolean for log location. I should check the AppConfig struct and the config.cpp/toml parsing logic to figure out the right way to add this option.

Let me check the AppConfig struct in globals.h to see how its fields are laid out.

I have enough context now to answer with updated code for both requirements, working in ask-only mode, so I'll provide code suggestions plus an implementation plan. I'm planning out the updated key_logger.cpp with buffered writing and config.cpp changes adding a new `log_use_exe_dir` boolean option and a GetLogDirectory function, including adding a `logToExeDir` field to AppConfig in globals.h.

I need the logger to pick the exe directory versus config directory based on this setting, use buffered writes instead of flushing every event (with periodic or count-based flushing plus flush-on-shutdown), and reopen the log file if the directory changes at runtime since this can be toggled from the settings window. Since I'm in ask-only mode, I'll present the code along with a final plan.

Here's the updated implementation addressing both points.

## 1. Buffered (non-per-flush) write strategy

Instead of flushing on every `LogKeyEvent` call, buffer writes and flush periodically (count-based + a background/idle flush), flushing immediately only on shutdown:

```cpp
// src/key_logger.cpp
#include "key_logger.h"
#include "config.h"      // for GetLogDirectory()
#include "globals.h"      // for KeycastConfig.logToExeDir
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <chrono>

namespace
{
std::wofstream g_logFile;
std::mutex g_logMutex;
std::filesystem::path g_currentLogPath;
int g_pendingWrites = 0;
std::chrono::steady_clock::time_point g_lastFlush = std::chrono::steady_clock::now();

constexpr int kFlushEveryNWrites = 20;                 // flush after N buffered events
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
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
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
```

Also flush periodically even when idle (so a log isn't stuck in the buffer for a long time after the last keystroke) by calling `ReopenKeyLoggerIfPathChanged`/a small flush tick from the existing `g_timerHide`-style timer in `main.cpp`, or simply rely on `ShutdownKeyLogger` at exit — up to you; the count/time-based flush above already bounds staleness to ~3s or 20 events, whichever comes first.

## 2. New config option: log location toggle

Add to `AppConfig` in `includes/globals.h`:

```cpp
bool logToExeDir = true; // true: exe 所在目录；false: 配置文件所在目录
```

In `src/config.cpp`, add a new `[logging]` section and a path resolver:

```cpp
// BuildConfigText()
out << "[logging]\n";
out << "# 日志文件位置：true=程序所在目录，false=配置文件所在目录\n";
out << "log_to_exe_dir = " << (c.logToExeDir ? "true" : "false") << "\n";

// ApplyParsed()
c.logToExeDir = ParseBool(GetValue(table, "logging", "log_to_exe_dir", "true"), true);
```

Add a path resolver (e.g. in `config.cpp`/`config.h`):

```cpp
std::filesystem::path GetExeDirectory()
{
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

std::filesystem::path GetLogFilePath()
{
    const auto dir = ::KeycastConfig.logToExeDir ? GetExeDirectory() : GetConfigDirectory();
    return dir / L"keycast_log.txt";
}
```

Then whenever this setting is toggled in the settings UI and saved (similar to `SetOverlayEnabled`), call `ReopenKeyLoggerIfPathChanged()` so the logger switches files without requiring a restart.


在仓库 fanlusky/DirectKeycast 中，基于此前已讨论的按键/鼠标事件文件日志功能（新增 `includes/key_logger.h`、`src/key_logger.cpp`），补充以下两点改动：

1. 缓冲写入（非逐次 flush）：
   - 修改 `src/key_logger.cpp` 中的 `LogKeyEvent`，不要每次写入后都 `flush()`。改为计数+时间双重触发：维护 `g_pendingWrites` 计数器和 `g_lastFlush` 时间戳，当达到例如 20 次写入或距上次 flush 超过 3 秒时才执行一次 `g_logFile.flush()`。
   - `ShutdownKeyLogger()` 中仍需在关闭文件前执行一次 `flush()`，保证程序退出前缓冲区内容落盘，避免丢失最后几条记录。
   - 可选：在现有的 `g_timerHide`（`src/main.cpp` 中维护的定时器）或类似的空闲定时机制上挂一个周期性的 flush 调用，确保长时间无操作时已有的缓冲数据也能及时落盘（例如每隔几秒检查一次是否有未 flush 的待写入内容）。

2. 新增配置项控制日志文件存放位置：
   - 在 `includes/globals.h` 的 `AppConfig` 结构体中新增字段 `bool logToExeDir = true;`，表示日志文件存放位置：`true` 表示与程序可执行文件同目录，`false` 表示与配置文件（`config.toml`，位于 `GetConfigDirectory()` 返回的目录，即 `%LocalAppData%\DirectKeycast`）同目录。
   - 在 `src/config.cpp` 中：
     - `BuildConfigText()` 增加一个新的 TOML 段 `[logging]`，写入 `log_to_exe_dir = true/false`，并附带中文注释说明其含义。
     - `ApplyParsed()` 增加对应的解析逻辑：`c.logToExeDir = ParseBool(GetValue(table, "logging", "log_to_exe_dir", "true"), true);`。
     - 新增函数 `std::filesystem::path GetExeDirectory()`（通过 `GetModuleFileNameW(nullptr, ...)` 获取可执行文件所在目录），以及 `std::filesystem::path GetLogFilePath()`，根据 `::KeycastConfig.logToExeDir` 的值在 `GetExeDirectory()` 或 `GetConfigDirectory()` 之间选择目录，拼接上日志文件名（如 `keycast_log.txt`）后返回。这两个函数需要在对应头文件（如 `config.h`）中声明，供 `key_logger.cpp` 调用。
   - 在 `src/key_logger.cpp` 中：
     - `InitKeyLogger()` 和内部的 `OpenLogFile_Locked()` 辅助函数使用 `GetLogFilePath()`（而不是硬编码 exe 目录）来决定日志文件的实际路径。
     - 新增导出函数 `void ReopenKeyLoggerIfPathChanged()`：当检测到当前已打开的日志文件路径与 `GetLogFilePath()` 的最新结果不同（即用户切换了日志位置设置）时，先 flush 并关闭旧文件句柄，再以追加模式打开新路径下的文件。该函数需要在 `includes/key_logger.h` 中声明。
   - 在设置窗口（`src/settings.cpp` 或相应 UI 处理文件，需先查找当前"显示按键浮层"等开关在设置界面中的实现方式，参考其调用 `SetOverlayEnabled` 的模式）中新增一个对应的 UI 控件（复选框/开关），用于让用户切换"日志存储位置"选项；该控件的事件处理函数应：
     a. 更新 `::KeycastConfig.logToExeDir`；
     b. 调用 `SaveAppConfig()` 持久化到 `config.toml`；
     c. 调用 `ReopenKeyLoggerIfPathChanged()`，使日志记录立即切换到新路径，无需重启程序。
   - 如果项目中已有类似 `SetOverlayEnabled` 的集中式 setter 函数模式，为保持一致性，可以新增一个 `SetLogToExeDir(bool value)` 函数（放在 `src/config.cpp`，仿照 `SetOverlayEnabled` 的写法），封装"更新值 + 保存 + 重新打开日志文件"这三步，供设置界面调用。

3. 构建与验证：
   - 确认新增的 `[logging]` 段能被正确读写，`config.toml` 文件内容符合预期格式。
   - 手动测试：将设置中的日志位置选项从"程序目录"切换为"配置文件目录"（或反之）后，确认后续按键/鼠标事件被写入到新选择的目录下的 `keycast_log.txt`，且旧文件不再被追加写入。
   - 测试缓冲写入逻辑：连续快速按键后立即检查文件内容是否因为缓冲而短暂落后，几秒后或程序正常退出后确认所有记录都已完整写入文件，没有数据丢失。
