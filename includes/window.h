#pragma once
#include "main.h"

HWND CreateTransparentWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);