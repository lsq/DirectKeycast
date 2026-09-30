#include "d2d.h"
#include "globals.h"
#include "theme.h"
#include "utils.h"
#include "window.h"
#include <algorithm>
#include <d2d1.h>
#include <vector>

namespace
{
struct OverlayCache
{
    std::wstring text;
    std::vector<IDWriteTextLayout *> layouts;
    std::vector<float> widths;
    float width = 0.0f;
    float height = 0.0f;
    bool valid = false;

    void Clear()
    {
        for (auto *layout : layouts)
        {
            if (layout)
            {
                layout->Release();
            }
        }
        layouts.clear();
        widths.clear();
        text.clear();
        width = 0.0f;
        height = 0.0f;
        valid = false;
    }
};

OverlayCache g_cache;

std::pair<float, float> MeasureLayout(IDWriteTextLayout *layout)
{
    if (!layout)
    {
        return {0.0f, 0.0f};
    }
    DWRITE_TEXT_METRICS metrics{};
    layout->GetMetrics(&metrics);
    return {metrics.width, metrics.height};
}

void RebuildCache(const std::wstring &text)
{
    g_cache.Clear();
    g_cache.text = text;
    if (!pDWriteFactory || !pTextFormat || text.empty())
    {
        g_cache.valid = true;
        return;
    }

    const auto words = splitString(text);
    g_cache.layouts.reserve(words.size());
    g_cache.widths.reserve(words.size());
    float totalW = 0.0f;
    float totalH = 0.0f;
    for (const auto &word : words)
    {
        IDWriteTextLayout *layout = nullptr;
        HRESULT hr = pDWriteFactory->CreateTextLayout(word.c_str(), static_cast<UINT32>(word.size()), pTextFormat, FLT_MAX, FLT_MAX, &layout);
        if (FAILED(hr) || !layout)
        {
            continue;
        }
        const auto size = MeasureLayout(layout);
        g_cache.layouts.push_back(layout);
        g_cache.widths.push_back(size.first);
        totalW += size.first;
        totalH = (std::max)(totalH, size.second);
    }
    g_cache.width = totalW;
    g_cache.height = totalH;
    g_cache.valid = true;
}

void EnsureCache()
{
    if (!g_cache.valid || g_cache.text != ::KeyStringToCast)
    {
        RebuildCache(::KeyStringToCast);
    }
}
} // namespace

std::vector<D2D1_COLOR_F> GetOverlayTextColors()
{
    // clang-format off
    return {
        D2D1::ColorF(223 / 255.0f, 84 / 255.0f, 82 / 255.0f),
        D2D1::ColorF(82 / 255.0f, 158 / 255.0f, 114 / 255.0f),
        GetTheme().overlayText,
        D2D1::ColorF(202 / 255.0f, 152 / 255.0f, 72 / 255.0f),
        D2D1::ColorF(186 / 255.0f, 133 / 255.0f, 111 / 255.0f),
        D2D1::ColorF(55 / 255.0f, 154 / 255.0f, 211 / 255.0f),
        D2D1::ColorF(199 / 255.0f, 125 / 255.0f, 72 / 255.0f),
        D2D1::ColorF(157 / 255.0f, 104 / 255.0f, 211 / 255.0f),
        D2D1::ColorF(155 / 255.0f, 155 / 255.0f, 155 / 255.0f),
        D2D1::ColorF(209 / 255.0f, 87 / 255.0f, 150 / 255.0f),
    };
    // clang-format on
}

bool InitD2DAndDWrite()
{
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    if (FAILED(hr))
    {
        return false;
    }

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&pDWriteFactory));
    if (FAILED(hr))
    {
        return false;
    }

    RecreateOverlayTextFormat();
    return pTextFormat != nullptr;
}

void RecreateOverlayTextFormat()
{
    if (!pDWriteFactory)
    {
        return;
    }
    if (pTextFormat)
    {
        pTextFormat->Release();
        pTextFormat = nullptr;
    }
    pDWriteFactory->CreateTextFormat(::fontName.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, ::KeycastConfig.fontSize, L"zh-cn", &pTextFormat);
    InvalidateOverlayCache();
}

bool InitD2DRenderTarget(HWND hwnd)
{
    if (!pD2DFactory)
    {
        return false;
    }

    if (pBrush)
    {
        pBrush->Release();
        pBrush = nullptr;
    }
    if (pRenderTarget)
    {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    RECT rc{};
    GetClientRect(hwnd, &rc);
    const auto renderTargetProps = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
    const auto hwndRenderTargetProps = D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top), D2D1_PRESENT_OPTIONS_NONE);
    HRESULT hr = pD2DFactory->CreateHwndRenderTarget(renderTargetProps, hwndRenderTargetProps, &pRenderTarget);
    if (SUCCEEDED(hr))
    {
        hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
    }
    return SUCCEEDED(hr);
}

void ResizeOverlayRenderTarget(HWND hwnd)
{
    if (!pRenderTarget)
    {
        InitD2DRenderTarget(hwnd);
        return;
    }
    RECT rc{};
    GetClientRect(hwnd, &rc);
    pRenderTarget->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
}

void InvalidateOverlayCache()
{
    g_cache.Clear();
}

void ReleaseOverlayCache()
{
    g_cache.Clear();
}

void ReleaseOverlayD2D()
{
    ReleaseOverlayCache();
    if (pBrush)
    {
        pBrush->Release();
        pBrush = nullptr;
    }
    if (pRenderTarget)
    {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }
    if (pTextFormat)
    {
        pTextFormat->Release();
        pTextFormat = nullptr;
    }
    if (pDWriteFactory)
    {
        pDWriteFactory->Release();
        pDWriteFactory = nullptr;
    }
    if (pD2DFactory)
    {
        pD2DFactory->Release();
        pD2DFactory = nullptr;
    }
}

void OnPaint(HWND hwnd)
{
    if (!pRenderTarget || !pBrush)
    {
        return;
    }
    if (!IsWindowVisible(hwnd))
    {
        ValidateRect(hwnd, nullptr);
        return;
    }

    EnsureCache();
    const ThemeColors &theme = GetTheme();

    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));
    pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    if (!::KeyStringToCast.empty() && g_cache.valid && !g_cache.layouts.empty())
    {
        const float textMarginLeft = 8.0f;
        const float textMarginRight = 8.0f;
        const float textMarginTop = 8.0f;
        const float textMarginBottom = 8.0f;
        const float borderThickness = ::KeycastConfig.borderThickness;
        const float radius = ::KeycastConfig.cornerRadius;
        const float borderWidth = g_cache.width + textMarginLeft + textMarginRight;
        const float borderHeight = g_cache.height + textMarginTop + textMarginBottom;

        D2D1_ROUNDED_RECT rounded = D2D1::RoundedRect(D2D1::RectF(0.0f, 0.0f, borderWidth, borderHeight), radius, radius);
        pBrush->SetColor(theme.overlayBg);
        pRenderTarget->FillRoundedRectangle(rounded, pBrush);
        if (borderThickness > 0.1f)
        {
            const float inset = borderThickness * 0.5f;
            D2D1_ROUNDED_RECT outline = D2D1::RoundedRect(D2D1::RectF(inset, inset, borderWidth - inset, borderHeight - inset), radius, radius);
            pBrush->SetColor(theme.overlayBorder);
            pRenderTarget->DrawRoundedRectangle(outline, pBrush, borderThickness);
        }

        float curX = textMarginLeft;
        const float textY = textMarginTop;
        const auto colors = GetOverlayTextColors();
        const int colorCnt = static_cast<int>(colors.size());
        for (size_t i = 0; i < g_cache.layouts.size(); ++i)
        {
            if (::KeycastConfig.rainbowText)
            {
                pBrush->SetColor(colors[static_cast<int>(i) % colorCnt]);
            }
            else
            {
                pBrush->SetColor(theme.overlayText);
            }
            pRenderTarget->DrawTextLayout(D2D1::Point2F(curX, textY), g_cache.layouts[i], pBrush);
            curX += g_cache.widths[i];
        }
    }

    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        InvalidateOverlayCache();
        InitD2DRenderTarget(hwnd);
    }
}

void GetOverlayContentDipSize(float &width, float &height)
{
    EnsureCache();
    if (!g_cache.valid || g_cache.layouts.empty())
    {
        width = 32.0f;
        height = 32.0f;
        return;
    }
    width = g_cache.width + 20.0f;
    height = g_cache.height + 20.0f;
}

void RequestOverlayRepaint()
{
    if (!::D2DHwnd)
    {
        return;
    }
    UpdateOverlayWindowPlacement(::D2DHwnd);
    InvalidateRect(::D2DHwnd, nullptr, FALSE);
}

bool InitGlobalConfigWithD2D()
{
    if (!pDWriteFactory || !pTextFormat)
    {
        return false;
    }
    const float scale = GetWindowScale(::D2DHwnd ? ::D2DHwnd : nullptr);
    if (scale <= 0.0f)
    {
        return false;
    }

    MONITORINFO monitorInfo{sizeof(MONITORINFO)};
    GetMonitorInfo(MonitorFromWindow(::D2DHwnd ? ::D2DHwnd : GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY), &monitorInfo);
    const float maxPx = static_cast<float>(monitorInfo.rcWork.right - monitorInfo.rcWork.left) * 0.5f;

    IDWriteTextLayout *layout = nullptr;
    if (FAILED(pDWriteFactory->CreateTextLayout(L"M", 1, pTextFormat, FLT_MAX, FLT_MAX, &layout)) || !layout)
    {
        return false;
    }
    const float mWidth = MeasureLayout(layout).first * scale;
    layout->Release();
    if (mWidth <= 0.1f)
    {
        return false;
    }
    ::KeycastConfig.maxSize = (std::max)(8, static_cast<int>(maxPx / mWidth) - 3);
    return true;
}
