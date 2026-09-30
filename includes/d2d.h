#pragma once
#include "main.h"
#include <vector>

// Per-segment colors of the rainbow overlay text (shared with the settings preview).
std::vector<D2D1_COLOR_F> GetOverlayTextColors();

bool InitD2DAndDWrite();
bool InitD2DRenderTarget(HWND hwnd);
bool InitGlobalConfigWithD2D();
void OnPaint(HWND hwnd);
void RecreateOverlayTextFormat();
void InvalidateOverlayCache();
void ReleaseOverlayCache();
void ResizeOverlayRenderTarget(HWND hwnd);
void ReleaseOverlayD2D();
void RequestOverlayRepaint();
void GetOverlayContentDipSize(float &width, float &height);
