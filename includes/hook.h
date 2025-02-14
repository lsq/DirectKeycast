#pragma once
#include <windows.h>

LRESULT CALLBACK KBDHook(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MOUSEHook(int nCode, WPARAM wParam, LPARAM lParam);