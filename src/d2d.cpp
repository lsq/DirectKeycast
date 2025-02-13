#include "d2d.h"
#include "globals.h"
#include <minwindef.h>

bool InitD2DAndDWrite()
{

    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    if (FAILED(hr))
        return false;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown **>(&pDWriteFactory));
    if (FAILED(hr))
        return false;

    hr = pDWriteFactory->CreateTextFormat(L"Microsoft YaHei UI",      //
                                          nullptr,                    //
                                          DWRITE_FONT_WEIGHT_NORMAL,  //
                                          DWRITE_FONT_STYLE_NORMAL,   //
                                          DWRITE_FONT_STRETCH_NORMAL, //
                                          24.0f,                      //
                                          L"zh-cn",                   //
                                          &pTextFormat);
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
    if (!pRenderTarget)
        return;
    pRenderTarget->BeginDraw();

    pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));
    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));
    pRenderTarget->DrawTextW(KeyStringToCast.c_str(),       //
                             KeyStringToCast.size(),        //
                             pTextFormat,                   //
                             D2D1::RectF(25, 20, 600, 200), //
                             pBrush);

    // Draw renderTarget outline
    D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
    D2D1_RECT_F borderRect = D2D1::RectF(static_cast<FLOAT>(0),            //
                                         static_cast<FLOAT>(0),            //
                                         static_cast<FLOAT>(rtSize.width), //
                                         static_cast<FLOAT>(rtSize.height));
    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Blue, 1.0f));
    pRenderTarget->DrawRectangle(&borderRect, pBrush, 3.0f);

    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        pRenderTarget->Release();
        InitD2DRenderTarget(hwnd);
    }
}