#include "tray_menu.h"
#include "config.h"
#include "globals.h"
#include "hook.h"
#include "msimeui_host.h"
#include "settings.h"
#include "theme.h"
#include "window.h"

#include "msimeui/Controls.h"
#include "msimeui/DeviceResources.h"
#include "msimeui/Layout.h"
#include "msimeui/Scene.h"
#include "msimeui/Theme.h"
#include "msimeui/Window.h"

#include <cmath>
#include <functional>
#include <memory>
#include <shellapi.h>
#include <string>
#include <wrl/client.h>

namespace
{
// Inset 6px on all sides; item radius stays concentric with the 8px chrome.
constexpr float kFontSizeDip = 14.0f;
constexpr float kBorderStrokeDip = 1.0f;
constexpr float kMenuPadDip = 6.0f;
constexpr float kItemGapDip = 2.0f;
constexpr float kMenuRadiusDip = 8.0f;
constexpr float kItemRadiusDip = kMenuRadiusDip - kMenuPadDip;
constexpr float kItemPadDip = 6.0f;
constexpr float kIconSizeDip = 18.0f;
constexpr float kIconGapDip = 8.0f;
constexpr float kToggleWidthDip = 32.0f;
constexpr float kToggleHeightDip = 16.0f;
constexpr float kToggleKnobDip = 14.0f;
constexpr float kItemHeightDip = kItemPadDip * 2.0f + kIconSizeDip;
constexpr float kMenuWidthDip = 216.0f;
constexpr int kItemCount = 4;

enum class MenuIcon
{
    Keyboard,
    Gear,
    Info,
    Exit
};

struct MenuPalette
{
    D2D1_COLOR_F background{};
    D2D1_COLOR_F border{};
    D2D1_COLOR_F hover{};
    D2D1_COLOR_F text{};
    D2D1_COLOR_F textHover{};
    D2D1_COLOR_F toggleOff{};
    D2D1_COLOR_F toggleOn{};
    D2D1_COLOR_F knob{};
};

D2D1_COLOR_F Rgb(int r, int g, int b, float a = 1.0f)
{
    return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, a);
}

MenuPalette MakePalette()
{
    if (IsLightThemeResolved())
    {
        return {Rgb(255, 255, 255), Rgb(0, 0, 0, 0.10f), Rgb(240, 240, 240), Rgb(26, 26, 26), Rgb(26, 26, 26),
                Rgb(200, 200, 200), Rgb(142, 140, 216), Rgb(255, 255, 255)};
    }
    return {Rgb(43, 43, 43), Rgb(58, 58, 58), Rgb(59, 59, 59), Rgb(224, 224, 224), Rgb(255, 255, 255),
            Rgb(85, 85, 85), Rgb(142, 140, 216), Rgb(255, 255, 255)};
}

Microsoft::WRL::ComPtr<ID2D1StrokeStyle> MakeRoundStroke(ID2D1RenderTarget *target)
{
    Microsoft::WRL::ComPtr<ID2D1StrokeStyle> style;
    ID2D1Factory *factory = nullptr;
    target->GetFactory(&factory);
    if (!factory)
    {
        return style;
    }
    const D2D1_STROKE_STYLE_PROPERTIES props = {D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND, D2D1_CAP_STYLE_ROUND,
                                                D2D1_LINE_JOIN_ROUND, 10.0f, D2D1_DASH_STYLE_SOLID, 0.0f};
    factory->CreateStrokeStyle(props, nullptr, 0, style.GetAddressOf());
    factory->Release();
    return style;
}

void DrawIconKeyboard(ID2D1RenderTarget *target, ID2D1SolidColorBrush *brush, ID2D1StrokeStyle *stroke, float l, float t,
                      float s)
{
    const float sw = (std::max)(1.2f, s * 1.8f / 24.0f);
    D2D1_ROUNDED_RECT body =
        D2D1::RoundedRect(D2D1::RectF(l + s * 0.06f, t + s * 0.22f, l + s * 0.94f, t + s * 0.78f), s * 0.14f, s * 0.14f);
    target->DrawRoundedRectangle(body, brush, sw, stroke);
    const float key = s * 0.10f;
    const float y = t + s * 0.40f;
    for (int i = 0; i < 4; ++i)
    {
        const float x = l + s * 0.20f + i * s * 0.16f;
        target->FillRectangle(D2D1::RectF(x, y, x + key, y + key), brush);
    }
    target->FillRectangle(D2D1::RectF(l + s * 0.28f, t + s * 0.58f, l + s * 0.72f, t + s * 0.68f), brush);
}

void DrawIconGear(ID2D1RenderTarget *target, ID2D1SolidColorBrush *brush, ID2D1StrokeStyle *stroke, float l, float t,
                  float s)
{
    const float sw = (std::max)(1.2f, s * 1.5f / 24.0f);
    const float cx = l + s * 0.5f;
    const float cy = t + s * 0.5f;
    target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), s * 0.22f, s * 0.22f), brush, sw, stroke);
    target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), s * 0.10f, s * 0.10f), brush, sw, stroke);
    for (int i = 0; i < 8; ++i)
    {
        const float angle = i * 3.14159265f / 4.0f;
        const float c = std::cos(angle);
        const float sn = std::sin(angle);
        target->DrawLine(D2D1::Point2F(cx + c * s * 0.30f, cy + sn * s * 0.30f),
                         D2D1::Point2F(cx + c * s * 0.42f, cy + sn * s * 0.42f), brush, sw, stroke);
    }
}

void DrawIconInfo(ID2D1RenderTarget *target, ID2D1SolidColorBrush *brush, ID2D1StrokeStyle *stroke, float l, float t,
                  float s)
{
    const float sw = (std::max)(1.2f, s * 1.5f / 12.0f * 0.55f);
    const float cx = l + s * 0.5f;
    target->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, t + s * 0.5f), s * 0.38f, s * 0.38f), brush, sw, stroke);
    target->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, t + s * 0.32f), s * 0.045f, s * 0.045f), brush);
    target->DrawLine(D2D1::Point2F(cx, t + s * 0.44f), D2D1::Point2F(cx, t + s * 0.70f), brush, sw, stroke);
}

void DrawIconExit(ID2D1RenderTarget *target, ID2D1SolidColorBrush *brush, ID2D1StrokeStyle *stroke, float l, float t,
                  float s)
{
    const float sw = (std::max)(1.2f, s * 1.5f / 18.0f);
    D2D1_ROUNDED_RECT door =
        D2D1::RoundedRect(D2D1::RectF(l + s * 0.16f, t + s * 0.18f, l + s * 0.52f, t + s * 0.82f), s * 0.08f, s * 0.08f);
    target->DrawRoundedRectangle(door, brush, sw, stroke);
    const float cy = t + s * 0.5f;
    target->DrawLine(D2D1::Point2F(l + s * 0.40f, cy), D2D1::Point2F(l + s * 0.84f, cy), brush, sw, stroke);
    target->DrawLine(D2D1::Point2F(l + s * 0.68f, t + s * 0.34f), D2D1::Point2F(l + s * 0.84f, cy), brush, sw, stroke);
    target->DrawLine(D2D1::Point2F(l + s * 0.68f, t + s * 0.66f), D2D1::Point2F(l + s * 0.84f, cy), brush, sw, stroke);
}

void DrawMenuIcon(ID2D1RenderTarget *target, ID2D1SolidColorBrush *brush, ID2D1StrokeStyle *stroke, MenuIcon icon,
                  float l, float t, float s)
{
    switch (icon)
    {
    case MenuIcon::Keyboard:
        DrawIconKeyboard(target, brush, stroke, l, t, s);
        break;
    case MenuIcon::Gear:
        DrawIconGear(target, brush, stroke, l, t, s);
        break;
    case MenuIcon::Info:
        DrawIconInfo(target, brush, stroke, l, t, s);
        break;
    case MenuIcon::Exit:
        DrawIconExit(target, brush, stroke, l, t, s);
        break;
    }
}

void DrawToggle(msimeui::DeviceResources &deviceResources, const MenuPalette &palette, float x, float y, bool on)
{
    ID2D1HwndRenderTarget *target = deviceResources.GetRenderTarget();
    ID2D1SolidColorBrush *trackBrush = deviceResources.GetSolidColorBrush(on ? palette.toggleOn : palette.toggleOff);
    ID2D1SolidColorBrush *knobBrush = deviceResources.GetSolidColorBrush(palette.knob);
    if (!target || !trackBrush || !knobBrush)
    {
        return;
    }
    const D2D1_ROUNDED_RECT track =
        D2D1::RoundedRect(D2D1::RectF(x, y, x + kToggleWidthDip, y + kToggleHeightDip), 8.0f, 8.0f);
    target->FillRoundedRectangle(track, trackBrush);
    const float knobX = x + 1.0f + (on ? 16.0f : 0.0f);
    const float knobY = y + 1.0f;
    target->FillEllipse(D2D1::Ellipse(D2D1::Point2F(knobX + kToggleKnobDip * 0.5f, knobY + kToggleKnobDip * 0.5f),
                                      kToggleKnobDip * 0.5f, kToggleKnobDip * 0.5f),
                        knobBrush);
}

class TrayMenuItem : public msimeui::Visual
{
  public:
    using ClickHandler = std::function<void()>;

    TrayMenuItem(std::wstring text, MenuIcon icon, bool toggle)
        : text_(std::move(text)), icon_(icon), toggle_(toggle)
    {
    }

    void SetOnClick(ClickHandler handler)
    {
        onClick_ = std::move(handler);
    }

    void SetChecked(bool checked)
    {
        if (checked_ == checked)
        {
            return;
        }
        checked_ = checked;
        InvalidateVisual();
    }

    bool IsChecked() const
    {
        return checked_;
    }

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override
    {
        return {availableSize.width, kItemHeightDip};
    }

    void Arrange(const msimeui::RectF &finalRect) override
    {
        bounds_ = finalRect;
    }

    void Render(msimeui::DeviceResources &deviceResources) override
    {
        ID2D1HwndRenderTarget *target = deviceResources.GetRenderTarget();
        if (!target)
        {
            return;
        }
        target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

        const MenuPalette palette = MakePalette();
        if (hovered_ || pressed_)
        {
            if (ID2D1SolidColorBrush *hoverBrush = deviceResources.GetSolidColorBrush(palette.hover))
            {
                const D2D1_ROUNDED_RECT hover = D2D1::RoundedRect(
                    D2D1::RectF(bounds_.x, bounds_.y, bounds_.x + bounds_.width, bounds_.y + bounds_.height),
                    kItemRadiusDip, kItemRadiusDip);
                target->FillRoundedRectangle(hover, hoverBrush);
            }
        }

        const D2D1_COLOR_F textColor = (hovered_ || pressed_) ? palette.textHover : palette.text;
        ID2D1SolidColorBrush *textBrush = deviceResources.GetSolidColorBrush(textColor);
        if (!textBrush)
        {
            return;
        }

        const auto stroke = MakeRoundStroke(target);
        const float iconY = bounds_.y + (bounds_.height - kIconSizeDip) * 0.5f;
        const float iconX = bounds_.x + kItemPadDip;
        DrawMenuIcon(target, textBrush, stroke.Get(), icon_, iconX, iconY, kIconSizeDip);

        const msimeui::Theme &ui = msimeui::ThemeManager::GetCurrent();
        IDWriteTextFormat *format = deviceResources.GetTextFormat(
            ui.uiFontFamily.empty() ? L"Segoe UI" : ui.uiFontFamily, kFontSizeDip, DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_CENTER, DWRITE_WORD_WRAPPING_NO_WRAP);
        if (format)
        {
            const float textX = iconX + kIconSizeDip + kIconGapDip;
            const float rightPad = toggle_ ? (kItemPadDip + kToggleWidthDip + 8.0f) : kItemPadDip;
            const D2D1_RECT_F textRc =
                D2D1::RectF(textX, bounds_.y, bounds_.x + bounds_.width - rightPad, bounds_.y + bounds_.height);
            target->DrawTextW(text_.c_str(), static_cast<UINT32>(text_.size()), format, textRc, textBrush,
                              D2D1_DRAW_TEXT_OPTIONS_CLIP);
        }

        if (toggle_)
        {
            const float toggleX = bounds_.x + bounds_.width - kItemPadDip - kToggleWidthDip;
            const float toggleY = bounds_.y + (bounds_.height - kToggleHeightDip) * 0.5f;
            DrawToggle(deviceResources, palette, toggleX, toggleY, checked_);
        }
    }

    void OnMouseEnter() override
    {
        hovered_ = true;
        InvalidateVisual();
    }

    void OnMouseLeave() override
    {
        hovered_ = false;
        pressed_ = false;
        InvalidateVisual();
    }

    bool OnMouseDown(const POINT &point, WPARAM) override
    {
        if (!window_)
        {
            return false;
        }
        pressed_ = HitTest(window_->ClientPixelsToDips(point));
        InvalidateVisual();
        return pressed_;
    }

    bool OnMouseUp(const POINT &point, WPARAM) override
    {
        if (!window_ || !pressed_)
        {
            return false;
        }
        const bool shouldClick = HitTest(window_->ClientPixelsToDips(point));
        pressed_ = false;
        InvalidateVisual();
        if (shouldClick)
        {
            if (toggle_)
            {
                SetChecked(!checked_);
            }
            if (onClick_)
            {
                onClick_();
            }
        }
        return true;
    }

    HCURSOR GetCursor() const override
    {
        return LoadCursor(nullptr, IDC_ARROW);
    }

  private:
    std::wstring text_;
    MenuIcon icon_ = MenuIcon::Keyboard;
    bool toggle_ = false;
    bool checked_ = false;
    bool hovered_ = false;
    bool pressed_ = false;
    ClickHandler onClick_;
};

// Rounded menu background. Colors are read at render time so a theme switch only
// needs a repaint; replacing the scene would leave msimeui::Window with dangling
// hovered/captured visual pointers.
class MenuChrome : public msimeui::Container
{
  public:
    explicit MenuChrome(std::shared_ptr<msimeui::Visual> child) : msimeui::Container(std::move(child))
    {
    }

    void Render(msimeui::DeviceResources &deviceResources) override
    {
        ID2D1HwndRenderTarget *target = deviceResources.GetRenderTarget();
        const MenuPalette palette = MakePalette();
        ID2D1SolidColorBrush *fill = deviceResources.GetSolidColorBrush(palette.background);
        ID2D1SolidColorBrush *stroke = deviceResources.GetSolidColorBrush(palette.border);
        if (target && fill && stroke)
        {
            const float inset = kBorderStrokeDip * 0.5f;
            const D2D1_ROUNDED_RECT rect =
                D2D1::RoundedRect(D2D1::RectF(bounds_.x + inset, bounds_.y + inset, bounds_.x + bounds_.width - inset,
                                              bounds_.y + bounds_.height - inset),
                                  kMenuRadiusDip, kMenuRadiusDip);
            target->FillRoundedRectangle(rect, fill);
            target->DrawRoundedRectangle(rect, stroke, kBorderStrokeDip);
        }
        msimeui::Container::Render(deviceResources);
    }
};

std::unique_ptr<msimeui::Window> g_window;
std::shared_ptr<TrayMenuItem> g_enabledItem;
bool g_visible = false;

int MenuWidthPx(UINT dpi)
{
    return DipToPxForDpi(kMenuWidthDip, dpi);
}

int MenuHeightPx(UINT dpi)
{
    const float height = kBorderStrokeDip * 2.0f + kMenuPadDip * 2.0f + kItemHeightDip * static_cast<float>(kItemCount) +
                         kItemGapDip * static_cast<float>(kItemCount - 1);
    return DipToPxForDpi(height, dpi);
}

void BuildMenuScene()
{
    if (!g_window)
    {
        return;
    }

    auto stack = std::make_shared<msimeui::StackPanel>(kItemGapDip);

    g_enabledItem = std::make_shared<TrayMenuItem>(L"显示按键", MenuIcon::Keyboard, true);
    g_enabledItem->SetChecked(::KeycastConfig.enabled);
    g_enabledItem->SetOnClick([]() {
        if (g_enabledItem)
        {
            SetOverlayEnabled(g_enabledItem->IsChecked());
        }
    });

    auto settingsItem = std::make_shared<TrayMenuItem>(L"设置", MenuIcon::Gear, false);
    settingsItem->SetOnClick([]() {
        HideTrayMenu();
        OpenSettingsWindow(false);
    });

    auto aboutItem = std::make_shared<TrayMenuItem>(L"关于", MenuIcon::Info, false);
    aboutItem->SetOnClick([]() {
        HideTrayMenu();
        OpenSettingsWindow(true);
    });

    auto exitItem = std::make_shared<TrayMenuItem>(L"退出", MenuIcon::Exit, false);
    exitItem->SetOnClick([]() {
        HideTrayMenu();
        PostQuitMessage(0);
    });

    stack->AddChild(g_enabledItem);
    stack->AddChild(settingsItem);
    stack->AddChild(aboutItem);
    stack->AddChild(exitItem);

    auto root = std::make_shared<MenuChrome>(stack);
    root->SetPadding(kMenuPadDip);
    root->SetHorizontalAlignment(msimeui::HorizontalAlignment::Stretch);
    root->SetVerticalAlignment(msimeui::VerticalAlignment::Stretch);

    auto scene = std::make_unique<msimeui::Scene>();
    scene->SetRoot(root);
    g_window->SetScene(std::move(scene));
}

void PositionMenu(HWND owner)
{
    if (!g_window || !g_window->GetHandle())
    {
        return;
    }

    RECT iconRc{};
    NOTIFYICONIDENTIFIER id{};
    id.cbSize = sizeof(id);
    id.hWnd = owner;
    id.uID = 1;
    const bool haveIcon = SUCCEEDED(Shell_NotifyIconGetRect(&id, &iconRc));
    POINT cursor{};
    GetCursorPos(&cursor);

    const POINT anchor = haveIcon ? POINT{(iconRc.left + iconRc.right) / 2, iconRc.top} : cursor;
    const UINT dpi = MonitorDpiFromPoint(anchor);
    const int width = MenuWidthPx(dpi);
    const int height = MenuHeightPx(dpi);
    int x = cursor.x - width / 2;
    int y = cursor.y - height - DipToPxForDpi(8.0f, dpi);
    if (haveIcon)
    {
        x = iconRc.left + (iconRc.right - iconRc.left - width) / 2;
        y = iconRc.top - height - DipToPxForDpi(6.0f, dpi);
    }

    HMONITOR monitor = MonitorFromPoint(anchor, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfo(monitor, &mi);
    if (x + width > mi.rcWork.right)
    {
        x = mi.rcWork.right - width - 4;
    }
    if (x < mi.rcWork.left)
    {
        x = mi.rcWork.left + 4;
    }
    if (y < mi.rcWork.top)
    {
        y = (haveIcon ? iconRc.bottom : cursor.y) + 4;
    }
    if (y + height > mi.rcWork.bottom)
    {
        y = mi.rcWork.bottom - height - 4;
    }

    SetWindowPos(g_window->GetHandle(), HWND_TOPMOST, x, y, width, height, SWP_NOACTIVATE);
}
} // namespace

bool CreateTrayMenuWindow(HINSTANCE)
{
    if (g_window && g_window->GetHandle())
    {
        return true;
    }

    POINT origin{0, 0};
    const UINT dpi = MonitorDpiFromPoint(origin);
    g_window = std::make_unique<msimeui::Window>(L"DirectKeycastTrayMenu", L"DirectKeycastMenu", MenuWidthPx(dpi),
                                                 MenuHeightPx(dpi));
    g_window->SetWindowStyle(WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE);
    g_window->SetRoundedCorners(true);
    if (!g_window->Create())
    {
        g_window.reset();
        return false;
    }

    HWND hwnd = g_window->GetHandle();
    AttachMsimeuiHost(hwnd, DkMsimeuiHideOnClose | DkMsimeuiNoActivate);
    ApplyImmersiveDarkMode(hwnd, !IsLightThemeResolved());
    BuildMenuScene();
    ::TrayMenuHwnd = hwnd;
    return true;
}

void ShowTrayMenu(HWND owner)
{
    if (!CreateTrayMenuWindow(GetModuleHandleW(nullptr)))
    {
        return;
    }
    if (g_enabledItem)
    {
        g_enabledItem->SetChecked(::KeycastConfig.enabled);
    }
    PositionMenu(owner);
    g_visible = true;
    ShowWindow(g_window->GetHandle(), SW_SHOWNOACTIVATE);
    g_window->Invalidate();
    UpdateWindow(g_window->GetHandle());
    UpdateMouseRawInput();
}

void HideTrayMenu()
{
    if (g_window && g_window->GetHandle() && g_visible)
    {
        ShowWindow(g_window->GetHandle(), SW_HIDE);
    }
    g_visible = false;
    UpdateMouseRawInput();
}

bool IsTrayMenuVisible()
{
    return g_visible;
}

void RefreshTrayMenuTheme()
{
    if (!g_window || !g_window->GetHandle())
    {
        return;
    }
    ApplyImmersiveDarkMode(g_window->GetHandle(), !IsLightThemeResolved());
    if (g_enabledItem)
    {
        g_enabledItem->SetChecked(::KeycastConfig.enabled);
    }
    g_window->Invalidate();
}

void DestroyTrayMenuWindow()
{
    HideTrayMenu();
    if (g_window && g_window->GetHandle())
    {
        DestroyWindow(g_window->GetHandle());
    }
    g_enabledItem.reset();
    g_window.reset();
    ::TrayMenuHwnd = nullptr;
}

bool IsPointInTrayMenu(POINT screenPt)
{
    if (!g_visible || !g_window)
    {
        return false;
    }
    return IsPointInWindow(g_window->GetHandle(), screenPt);
}
