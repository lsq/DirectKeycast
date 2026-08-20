#pragma once
#include <d2d1.h>
#include <dwmapi.h>
#include <dwrite.h>
#include <string>
#include <windows.h>

inline ID2D1Factory *pD2DFactory = nullptr;
inline ID2D1HwndRenderTarget *pRenderTarget = nullptr;
inline ID2D1SolidColorBrush *pBrush = nullptr;
inline IDWriteFactory *pDWriteFactory = nullptr;
inline IDWriteTextFormat *pTextFormat = nullptr;

inline std::wstring fontName = L"";
inline std::wstring defaultFont = L"Cascadia Mono";
inline std::wstring backupFont = L"Microsoft YaHei";

inline std::wstring KeyStringToCast = L"";
inline HWND D2DHwnd = nullptr;
inline HWND TrayMenuHwnd = nullptr;
inline HWND SettingsHwnd = nullptr;

inline RECT WindowArea = {0, 0, 0, 0};

inline struct AppConfig
{
    int maxSize = 83;
    std::wstring themeMode = L"dark";
    std::wstring font = L"Cascadia Mono";
    std::wstring defaultFont = L"Microsoft YaHei";
    float fontSize = 24.0f;
    bool rainbowText = true;
    float bgOpacity = 0.75f;
    float cornerRadius = 12.0f;
    float borderThickness = 3.0f;
    bool enabled = true;
    bool showMouse = true;
    int fadeMs = 1500;
    std::wstring position = L"bottom-right";
} KeycastConfig;

inline constexpr UINT WM_APP_HIDE_OVERLAY = WM_APP + 10;
inline constexpr UINT WM_APP_SHOW_OVERLAY = WM_APP + 11;
inline constexpr UINT WM_APP_HIDE_TRAY_MENU = WM_APP + 12;
inline constexpr UINT WM_APP_OPEN_SETTINGS = WM_APP + 13;
inline constexpr UINT WM_APP_OPEN_ABOUT = WM_APP + 14;
inline constexpr UINT WM_APP_LLKEY = WM_APP + 15;
