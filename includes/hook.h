#pragma once
#include <windows.h>

void StartInputCapture(HWND hwnd);
void StopInputCapture();
void UpdateMouseRawInput();
void HandlePostedKeyEvent(WPARAM wParam, DWORD vkCode);
void HandleRawMouseInput(HRAWINPUT hRaw);
void PulseOverlay();
