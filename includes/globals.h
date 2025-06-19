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

inline RECT WindowArea = {0, 0, 0, 0};

inline struct AppConfig
{
    int maxSize;
} KeycastConfig;