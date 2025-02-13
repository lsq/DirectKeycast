#include "d2d.h"
#include "globals.h"

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

    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 0.5f));

    pRenderTarget->FillRectangle(D2D1::RectF(50, 50, 400, 300), pBrush);

    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red, 1.0f));

    pRenderTarget->FillRectangle(D2D1::RectF(200, 200, 500, 400), pBrush);

    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Green, 0.3f));

    pRenderTarget->FillRectangle(D2D1::RectF(300, 100, 600, 250), pBrush);

    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 1.0f));   //
    pRenderTarget->DrawTextW(L"Transparent Window and Direct2D", //
                             31,                                 //
                             pTextFormat,                        //
                             D2D1::RectF(5, 5, 600, 200),        //
                             pBrush);

    HRESULT hr = pRenderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET)
    {
        pRenderTarget->Release();
        InitD2DRenderTarget(hwnd);
    }
}