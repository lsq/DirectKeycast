#include "theme.h"
#include "globals.h"
#include "msimeui/Theme.h"

namespace
{
ThemeColors g_theme{};
bool g_light = false;

D2D1_COLOR_F Rgba(int r, int g, int b, float a = 1.0f)
{
    return D2D1::ColorF(r / 255.0f, g / 255.0f, b / 255.0f, a);
}

void BuildDark(ThemeColors &t)
{
    t.chromeBg = Rgba(32, 32, 32);
    t.sidebarBg = Rgba(25, 25, 25);
    t.text = Rgba(243, 243, 243);
    t.textSecondary = Rgba(200, 200, 200);
    t.textMuted = Rgba(160, 160, 160);
    t.border = Rgba(255, 255, 255, 0.08f);
    t.cardBg = Rgba(38, 38, 38, 0.92f);
    t.hover = Rgba(255, 255, 255, 0.08f);
    t.active = Rgba(255, 255, 255, 0.06f);
    t.accent = Rgba(142, 140, 216);
    t.toggleOff = Rgba(85, 85, 85);
    t.titlebarBtnHover = Rgba(255, 255, 255, 0.06f);
    t.closeHover = Rgba(196, 43, 28);
    t.overlayBg = Rgba(37, 37, 37, ::KeycastConfig.bgOpacity);
    t.overlayBorder = Rgba(55, 154, 211, 0.9f);
    t.overlayText = Rgba(255, 255, 255, 0.81f);
    t.menuBg = Rgba(43, 43, 43);
    t.menuBorder = Rgba(58, 58, 58);
    t.menuHover = Rgba(59, 59, 59);
    t.menuText = Rgba(224, 224, 224);
    t.knob = Rgba(255, 255, 255);
}

void BuildLight(ThemeColors &t)
{
    t.chromeBg = Rgba(243, 243, 243);
    t.sidebarBg = Rgba(250, 250, 250);
    t.text = Rgba(36, 36, 36);
    t.textSecondary = Rgba(66, 66, 66);
    t.textMuted = Rgba(97, 97, 97);
    t.border = Rgba(0, 0, 0, 0.08f);
    t.cardBg = Rgba(255, 255, 255);
    t.hover = Rgba(0, 0, 0, 0.04f);
    t.active = Rgba(0, 0, 0, 0.06f);
    t.accent = Rgba(111, 107, 199);
    t.toggleOff = Rgba(200, 200, 200);
    t.titlebarBtnHover = Rgba(0, 0, 0, 0.05f);
    t.closeHover = Rgba(196, 43, 28);
    t.overlayBg = Rgba(243, 243, 243, ::KeycastConfig.bgOpacity);
    t.overlayBorder = Rgba(0, 0, 0, 0.18f);
    t.overlayText = Rgba(26, 26, 26);
    t.menuBg = Rgba(255, 255, 255);
    t.menuBorder = Rgba(0, 0, 0, 0.10f);
    t.menuHover = Rgba(240, 240, 240);
    t.menuText = Rgba(26, 26, 26);
    t.knob = Rgba(255, 255, 255);
}
} // namespace

D2D1_COLOR_F ColorFromHex(unsigned int rgb, float alpha)
{
    const float r = ((rgb >> 16) & 0xFF) / 255.0f;
    const float g = ((rgb >> 8) & 0xFF) / 255.0f;
    const float b = (rgb & 0xFF) / 255.0f;
    return D2D1::ColorF(r, g, b, alpha);
}

bool IsSystemLightTheme()
{
    HKEY hKey = nullptr;
    DWORD value = 0;
    DWORD size = sizeof(value);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        return false;
    }
    const LSTATUS st = RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &size);
    RegCloseKey(hKey);
    return st == ERROR_SUCCESS && value != 0;
}

bool IsLightThemeResolved()
{
    return g_light;
}

const ThemeColors &GetTheme()
{
    return g_theme;
}

void RefreshTheme()
{
    const std::wstring &mode = ::KeycastConfig.themeMode;
    if (mode == L"light")
    {
        g_light = true;
    }
    else if (mode == L"system")
    {
        g_light = IsSystemLightTheme();
    }
    else
    {
        g_light = false;
    }

    if (g_light)
    {
        BuildLight(g_theme);
    }
    else
    {
        BuildDark(g_theme);
    }

    msimeui::Theme ui;
    ui.uiFontFamily = L"Microsoft YaHei UI";
    ui.textInputFontFamily = L"Microsoft YaHei UI";
    ui.windowBackground = g_theme.chromeBg;
    ui.surface = g_theme.cardBg;
    ui.surfaceMuted = g_theme.sidebarBg;
    ui.border = g_theme.border;
    ui.borderStrong = g_theme.menuBorder;
    ui.primary = g_theme.accent;
    ui.primaryPressed = D2D1::ColorF(g_theme.accent.r * 0.85f, g_theme.accent.g * 0.85f, g_theme.accent.b * 0.85f, g_theme.accent.a);
    ui.primarySoft = g_theme.active;
    ui.primarySoftPressed = g_theme.hover;
    ui.primaryFocus = g_theme.accent;
    ui.primaryFocusStrong = g_theme.accent;
    ui.track = g_theme.toggleOff;
    ui.thumb = g_theme.textMuted;
    ui.thumbActive = g_theme.accent;
    ui.textPrimary = g_theme.text;
    ui.textSecondary = g_theme.textSecondary;
    ui.textInverse = g_theme.knob;
    msimeui::ThemeManager::SetCurrent(std::move(ui));
}
