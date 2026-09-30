#pragma once
#include <d2d1.h>

// Icons share their path data with the MetasequoiaIME settings sidebar so the
// two settings windows use the same glyphs.
enum class SvgIcon
{
    Appearance,
    Keyboard,
    Shortcut,
    About,
    Arrow,
};

// Fills `icon` (authored in a square viewBox) into `rect` with `brush`.
void DrawSvgIcon(ID2D1RenderTarget *target, ID2D1Brush *brush, SvgIcon icon, const D2D1_RECT_F &rect);
