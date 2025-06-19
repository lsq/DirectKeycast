#pragma once
#include "main.h"

bool InitD2DAndDWrite();
bool InitD2DRenderTarget(HWND hwnd);
bool InitGlobalConfigWithD2D();
void OnPaint(HWND hwnd);