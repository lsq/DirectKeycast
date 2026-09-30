#pragma once
#include <d2d1.h>
#include <string>

// Palette mirrors the MetasequoiaIME settings page (ime-settings/src/styles/variables.css)
// so both settings windows look like they belong to the same family.
struct ThemeColors
{
    D2D1_COLOR_F chromeBg{};
    D2D1_COLOR_F sidebarBg{};
    D2D1_COLOR_F sidebarHover{};
    D2D1_COLOR_F sidebarActive{};
    D2D1_COLOR_F sidebarItem{};
    D2D1_COLOR_F sidebarIcon{};
    D2D1_COLOR_F text{};
    D2D1_COLOR_F textSecondary{};
    D2D1_COLOR_F textMuted{};
    D2D1_COLOR_F border{};
    D2D1_COLOR_F borderStrong{};
    D2D1_COLOR_F divider{};
    D2D1_COLOR_F cardBg{};
    D2D1_COLOR_F surfaceRaised{};
    D2D1_COLOR_F surfaceSubtle{};
    D2D1_COLOR_F hover{};
    D2D1_COLOR_F active{};
    D2D1_COLOR_F accent{};
    D2D1_COLOR_F accentSoft{};
    D2D1_COLOR_F accentSoftBorder{};
    D2D1_COLOR_F toggleOn{};
    D2D1_COLOR_F toggleOff{};
    D2D1_COLOR_F knob{};
    D2D1_COLOR_F dropdownBg{};
    D2D1_COLOR_F dropdownMenuBg{};
    D2D1_COLOR_F dropdownMenuBorder{};
    D2D1_COLOR_F dropdownItemHover{};
    D2D1_COLOR_F buttonSecondaryBg{};
    D2D1_COLOR_F buttonSecondaryHover{};
    D2D1_COLOR_F buttonSecondaryText{};
    D2D1_COLOR_F kbdBg{};
    D2D1_COLOR_F kbdBorder{};
    D2D1_COLOR_F kbdBorderBottom{};
    D2D1_COLOR_F kbdText{};
    D2D1_COLOR_F scrollbarThumb{};
    D2D1_COLOR_F scrollbarThumbHover{};
    D2D1_COLOR_F titlebarBtnHover{};
    D2D1_COLOR_F closeHover{};
    D2D1_COLOR_F overlayBg{};
    D2D1_COLOR_F overlayBorder{};
    D2D1_COLOR_F overlayText{};
    D2D1_COLOR_F menuBg{};
    D2D1_COLOR_F menuBorder{};
    D2D1_COLOR_F menuHover{};
    D2D1_COLOR_F menuText{};
};

bool IsSystemLightTheme();
bool IsLightThemeResolved();
const ThemeColors &GetTheme();
void RefreshTheme();
D2D1_COLOR_F ColorFromHex(unsigned int rgb, float alpha = 1.0f);
COLORREF ToColorRef(const D2D1_COLOR_F &color);

// "Noto Sans SC" when installed (same as the IME settings page), otherwise "Microsoft YaHei UI".
const std::wstring &GetUiFontFamily();
