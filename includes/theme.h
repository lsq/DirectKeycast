#pragma once
#include <d2d1.h>
#include <string>

struct ThemeColors
{
    D2D1_COLOR_F chromeBg{};
    D2D1_COLOR_F sidebarBg{};
    D2D1_COLOR_F text{};
    D2D1_COLOR_F textSecondary{};
    D2D1_COLOR_F textMuted{};
    D2D1_COLOR_F border{};
    D2D1_COLOR_F cardBg{};
    D2D1_COLOR_F hover{};
    D2D1_COLOR_F active{};
    D2D1_COLOR_F accent{};
    D2D1_COLOR_F toggleOff{};
    D2D1_COLOR_F titlebarBtnHover{};
    D2D1_COLOR_F closeHover{};
    D2D1_COLOR_F overlayBg{};
    D2D1_COLOR_F overlayBorder{};
    D2D1_COLOR_F overlayText{};
    D2D1_COLOR_F menuBg{};
    D2D1_COLOR_F menuBorder{};
    D2D1_COLOR_F menuHover{};
    D2D1_COLOR_F menuText{};
    D2D1_COLOR_F knob{};
};

bool IsSystemLightTheme();
bool IsLightThemeResolved();
const ThemeColors &GetTheme();
void RefreshTheme();
D2D1_COLOR_F ColorFromHex(unsigned int rgb, float alpha = 1.0f);
