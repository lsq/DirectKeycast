#include "d2d.h"
#include "globals.h"
#include "utils.h"
#include <d2d1.h>
#include <d2d1helper.h>
#include <iostream>
#include <vector>

HRESULT GetTextWidth(IDWriteFactory *pDWriteFactory, IDWriteTextFormat *pTextFormat, const std::wstring &text, float &textWidth);

//
// Dark mode colors, picked from Notion site with Dev Tools
// Text
// Default: #FFFFFFCF, (255, 255, 255, 0.81)
// Gray: #9B9B9B, (155, 155, 155)
// Brown: #BA856F, (186, 133, 111)
// Orange: #C77D48, (199, 125, 72)
// Yellow: #CA984D, (202, 152, 77)
// Green: #529E72, (82, 158, 114)
// Blue: #379AD3, (55, 154, 211)
// Purple: #9D68D3, (157, 104, 211)
// Pink: #D15796, (209, 87, 150)
// Red: #DF5452, (223, 84, 82)
//
// Background
// Default: #252525, (37, 37, 37)
// Gray: #2F2F2F, (47, 47, 47)
// Brown: #4A3228, (74, 50, 40)
// Orange: #5C3B23, (92, 59, 35)
// Yellow: #564328, (86, 67, 40)
// Green: #243D30, (36, 61, 48)
// Blue: #143A4E, (20, 58, 78)
// Purple: #3C2D49, (60, 45, 73)
// Pink: #4E2C3C, (78, 44, 60)
// Red: #522E2A, (82, 46, 42)
struct ColorFromNotion
{
    const D2D1_COLOR_F DarkTextDefault = D2D1::ColorF(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 0.81f);
    const D2D1_COLOR_F DarkTextGray = D2D1::ColorF(155 / 255.0f, 155 / 255.0f, 155 / 255.0f);
    const D2D1_COLOR_F DarkTextBrown = D2D1::ColorF(186 / 255.0f, 133 / 255.0f, 111 / 255.0f);
    const D2D1_COLOR_F DarkTextOrange = D2D1::ColorF(199 / 255.0f, 125 / 255.0f, 72 / 255.0f);
    const D2D1_COLOR_F DarkTextYellow = D2D1::ColorF(202 / 255.0f, 152 / 255.0f, 72 / 255.0f);
    const D2D1_COLOR_F DarkTextGreen = D2D1::ColorF(82 / 255.0f, 158 / 255.0f, 114 / 255.0f);
    const D2D1_COLOR_F DarkTextBlue = D2D1::ColorF(55 / 255.0f, 154 / 255.0f, 211 / 255.0f);
    const D2D1_COLOR_F DarkTextPurple = D2D1::ColorF(157 / 255.0f, 104 / 255.0f, 211 / 255.0f);
    const D2D1_COLOR_F DarkTextPink = D2D1::ColorF(209 / 255.0f, 87 / 255.0f, 150 / 255.0f);
    const D2D1_COLOR_F DarkTextRed = D2D1::ColorF(223 / 255.0f, 84 / 255.0f, 82 / 255.0f);

    const D2D1_COLOR_F DarkBgDefault = D2D1::ColorF(37 / 255.0f, 37 / 255.0f, 37 / 255.0f, 0.75f);
    const D2D1_COLOR_F DarkBgGray = D2D1::ColorF(47 / 255.0f, 47 / 255.0f, 47 / 255.0f);
    const D2D1_COLOR_F DarkBgBrown = D2D1::ColorF(74 / 255.0f, 50 / 255.0f, 40 / 255.0f);
    const D2D1_COLOR_F DarkBgOrange = D2D1::ColorF(92 / 255.0f, 59 / 255.0f, 35 / 255.0f);
    const D2D1_COLOR_F DarkBgYellow = D2D1::ColorF(86 / 255.0f, 67 / 255.0f, 40 / 255.0f);
    const D2D1_COLOR_F DarkBgGreen = D2D1::ColorF(36 / 255.0f, 61 / 255.0f, 48 / 255.0f);
    const D2D1_COLOR_F DarkBgBlue = D2D1::ColorF(20 / 255.0f, 58 / 255.0f, 78 / 255.0f);
    const D2D1_COLOR_F DarkBgPurple = D2D1::ColorF(60 / 255.0f, 45 / 255.0f, 73 / 255.0f);
    const D2D1_COLOR_F DarkBgPink = D2D1::ColorF(78 / 255.0f, 44 / 255.0f, 60 / 255.0f);
    const D2D1_COLOR_F DarkBgRed = D2D1::ColorF(82 / 255.0f, 46 / 255.0f, 42 / 255.0f);

    const D2D1_COLOR_F DarkOutlineDefault = D2D1::ColorF(86 / 255.0f, 86 / 255.0f, 86 / 255.0f, 0.9f);
} NotionColors;

std::vector<D2D1_COLOR_F> ColorsVec = {
    NotionColors.DarkTextRed,     //
    NotionColors.DarkTextGreen,   //
    NotionColors.DarkTextDefault, //
    NotionColors.DarkTextYellow,  //
    NotionColors.DarkTextBrown,   //
    NotionColors.DarkTextBlue,    //
    NotionColors.DarkTextOrange,  //
    NotionColors.DarkTextPurple,  //
    NotionColors.DarkTextGray,    //
    NotionColors.DarkTextPink,    //
};

bool InitD2DAndDWrite()
{

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    if (FAILED(hr))
        return false;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&pDWriteFactory));
    if (FAILED(hr))
        return false;

    hr = pDWriteFactory->CreateTextFormat( //
        ::fontName.c_str(),                //
        nullptr,                           //
        DWRITE_FONT_WEIGHT_NORMAL,         //
        DWRITE_FONT_STYLE_NORMAL,          //
        DWRITE_FONT_STRETCH_NORMAL,        //
        24.0f,                             //
        L"zh-cn",                          //
        &pTextFormat                       //
    );
    if (FAILED(hr))
        return false;

    return true;
}

bool InitD2DRenderTarget(HWND hwnd)
{
    if (!pD2DFactory)
        return false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    HRESULT hr = pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)),
                                                     D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top), D2D1_PRESENT_OPTIONS_IMMEDIATELY), &pRenderTarget);

    if (SUCCEEDED(hr))
    {
        hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
    }

    return SUCCEEDED(hr);
}

void OnPaint(HWND hwnd)
{

    float textWidth;
    GetTextWidth(pDWriteFactory, pTextFormat, ::KeyStringToCast, textWidth);
    int newWidth = textWidth + 70;

    RECT curRect;
    GetWindowRect(hwnd, &curRect);
    int currentWidth = curRect.right - curRect.left; // Current width
    int newLeft = curRect.right - newWidth;          // Make right end fixed

    // Move Content
    FLOAT scale = ::GetWindowScale();

    std::cout << curRect.top << std::endl;
    std::cout << 1980 - curRect.top << std::endl;
    int taskbarHeight = ::GetTaskbarHeight();
    int monitorHeight = ::GetPrimaryMonitorHeight();
    D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation((currentWidth - newWidth) / scale, ((monitorHeight - taskbarHeight) / 2.0 - 106) / scale);
    pRenderTarget->SetTransform(translation);

    if (!pRenderTarget)
        return;

    pRenderTarget->BeginDraw();

    pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));
    std::vector<std::wstring> words;
    if (::KeyStringToCast.empty())
    {
        goto Exit;
    }

    // Draw renderTarget background and outline
    D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
    std::cout << "rtSize.height: " << rtSize.height << std::endl;
    D2D1_RECT_F borderRect = D2D1::RectF(static_cast<FLOAT>(0 + 3),                                                    //
                                         static_cast<FLOAT>(0 + 3),                                                    //
                                         static_cast<FLOAT>(rtSize.width - 3 - (currentWidth - newWidth) / scale - 5), //
                                         static_cast<FLOAT>(rtSize.height) - ((monitorHeight - taskbarHeight) / 2.0 - 106) / scale - 3);
    D2D1_ROUNDED_RECT roundedBorderRect = D2D1::RoundedRect(borderRect, 12.0f, 12.0f);
    // Fill Round Rectangle
    pBrush->SetColor(::NotionColors.DarkBgDefault);
    pRenderTarget->FillRoundedRectangle(roundedBorderRect, pBrush);
    // Draw Outline
    pBrush->SetColor(::NotionColors.DarkTextBlue);
    pRenderTarget->DrawRoundedRectangle(roundedBorderRect, pBrush, 3.0f);

    // Draw Text
    words = splitString(::KeyStringToCast);
    FLOAT x = 22.0f;
    FLOAT y = 20.0f;
    FLOAT xPos = x;
    int i = 0;
    int colorCnt = ColorsVec.size();

    for (const auto &word : words)
    {
        pBrush->SetColor(ColorsVec[i % colorCnt]);
        i++;
        IDWriteTextLayout *pTextLayout = nullptr;
        pDWriteFactory->CreateTextLayout(word.c_str(), word.size(), pTextFormat, 2000, 1000, &pTextLayout);
        DWRITE_TEXT_METRICS textMetrics;
        pTextLayout->GetMetrics(&textMetrics);
        pRenderTarget->DrawTextLayout(D2D1::Point2F(xPos, y), pTextLayout, pBrush);
        xPos += textMetrics.width;
        pTextLayout->Release();
    }

Exit:
    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        pRenderTarget->Release();
        InitD2DRenderTarget(hwnd);
    }
}

HRESULT GetTextWidth(IDWriteFactory *pDWriteFactory, IDWriteTextFormat *pTextFormat, const std::wstring &text, float &textWidth)
{
    IDWriteTextLayout *pTextLayout = nullptr;
    HRESULT hr = pDWriteFactory->CreateTextLayout(text.c_str(), //
                                                  text.size(),  //
                                                  pTextFormat,  //
                                                  FLT_MAX,      //
                                                  FLT_MAX,      //
                                                  &pTextLayout);

    if (FAILED(hr))
    {
        return hr;
    }

    DWRITE_TEXT_METRICS textMetrics;
    pTextLayout->GetMetrics(&textMetrics);

    FLOAT scale = ::GetWindowScale();
    textWidth = textMetrics.width * scale;

    pTextLayout->Release();

    return S_OK;
}