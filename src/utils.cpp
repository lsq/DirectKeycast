#include "utils.h"
#include <winuser.h>

std::wstring string_to_wstring(const std::string &str)
{
    if (str.empty())
        return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    if (size_needed <= 0)
    {
        throw std::runtime_error("MultiByteToWideChar failed");
    }
    std::wstring wstr(size_needed, 0);
    int result = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    if (result <= 0)
    {
        throw std::runtime_error("MultiByteToWideChar failed");
    }
    return wstr;
}

std::string wstring_to_string(const std::wstring &wstr)
{
    if (wstr.empty())
        return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0)
    {
        throw std::runtime_error("WideCharToMultiByte failed");
    }
    std::string str(size_needed, 0);
    int result = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, nullptr, nullptr);
    if (result <= 0)
    {
        throw std::runtime_error("WideCharToMultiByte failed");
    }
    return str;
}

const std::unordered_map<int, std::string> &KeyCastMap()
{
    static const std::unordered_map<int, std::string> keymap = {
        {VK_ESCAPE, "󱊷"},       // 27
        {VK_F1, "󱊫"},           // 112
        {VK_F2, "󱊬"},           // 113
        {VK_F3, "󱊭"},           // 114
        {VK_F4, "󱊮"},           // 115
        {VK_F5, "󱊯"},           // 116
        {VK_F6, "󱊰"},           // 117
        {VK_F7, "󱊱"},           // 118
        {VK_F8, "󱊲"},           // 119
        {VK_F9, "󱊳"},           // 120
        {VK_F10, "󱊴"},          // 121
        {VK_F11, "󱊵"},          // 122
        {VK_F12, "󱊶"},          // 123
        {VK_OEM_3, ""},            // 192 (` ~)
        {0x31, "1"},               // 49 (1)
        {0x32, "2"},               // 50 (2)
        {0x33, "3"},               // 51 (3)
        {0x34, "4"},               // 52 (4)
        {0x35, "5"},               // 53 (5)
        {0x36, "6"},               // 54 (6)
        {0x37, "7"},               // 55 (7)
        {0x38, "8"},               // 56 (8)
        {0x39, "9"},               // 57 (9)
        {0x30, "0"},               // 48 (0)
        {VK_OEM_MINUS, "-"},       // 189 (- _)
        {VK_OEM_PLUS, "="},        // 187 (= +)
        {VK_BACK, "󰁮 "},        // 8 (Backspace)
        {VK_TAB, ""},           // 9
        {0x51, "q"},               // 81 (Q)
        {0x57, "w"},               // 87 (W)
        {0x45, "e"},               // 69 (E)
        {0x52, "r"},               // 82 (R)
        {0x54, "t"},               // 84 (T)
        {0x59, "y"},               // 89 (Y)
        {0x55, "u"},               // 85 (U)
        {0x49, "i"},               // 73 (I)
        {0x4F, "o"},               // 79 (O)
        {0x50, "p"},               // 80 (P)
        {VK_OEM_4, "["},           // 219 ([ {)
        {VK_OEM_6, "]"},           // 221 (] })
        {VK_OEM_5, "\\"},          // 220 (\ |)
        {VK_CAPITAL, "󰪛"},      // 20 (Caps Lock)
        {0x41, "a"},               // 65 (A)
        {0x53, "s"},               // 83 (S)
        {0x44, "d"},               // 68 (D)
        {0x46, "f"},               // 70 (F)
        {0x47, "g"},               // 71 (G)
        {0x48, "h"},               // 72 (H)
        {0x4A, "j"},               // 74 (J)
        {0x4B, "k"},               // 75 (K)
        {0x4C, "l"},               // 76 (L)
        {VK_OEM_1, ";"},           // 186 (; :)
        {VK_OEM_7, "'"},           // 222 (' ")
        {VK_RETURN, "󰌑"},       // 13 (Enter)
        {VK_LSHIFT, "󰘶(L)"},    // 160
        {0x5A, "z"},               // 90 (Z)
        {0x58, "x"},               // 88 (X)
        {0x43, "c"},               // 67 (C)
        {0x56, "v"},               // 86 (V)
        {0x42, "b"},               // 66 (B)
        {0x4E, "n"},               // 78 (N)
        {0x4D, "m"},               // 77 (M)
        {VK_OEM_COMMA, ","},       // 188 (, <)
        {VK_OEM_PERIOD, "."},      // 190 (. >)
        {VK_OEM_2, "/"},           // 191 (/ ?)
        {VK_RSHIFT, "󰘶(R)"},    // 161
        {VK_LCONTROL, "󰘴(L)"},  // 162
        {VK_LWIN, "(L)"},       // 91
        {VK_LMENU, "(L)"},      // 164 (Left Alt)
        {VK_SPACE, "󱁐"},        // 32
        {VK_RMENU, "(R)"},      // 165 (Right Alt)
        {VK_APPS, "󰀻"},         // 93 (Application Key)
        {VK_RCONTROL, "󰘴(R)"},  // 163
        {VK_SNAPSHOT, ""},      // 44 (Print Screen)
        {VK_SCROLL, ""},        // 145 (Scroll Lock)
        {VK_PAUSE, ""},         // 19 (Pause/Break)
        {VK_INSERT, ""},        // 45
        {VK_HOME, ""},          // 36
        {VK_PRIOR, ""},            // 33 (Page Up)
        {VK_DELETE, ""},           // 46
        {VK_END, ""},              // 35
        {VK_NEXT, ""},             // 34 (Page Down)
        {VK_UP, ""},               // 38
        {VK_LEFT, ""},             // 37
        {VK_DOWN, ""},             // 40
        {VK_RIGHT, ""},            // 39
        {VK_MEDIA_PREV_TRACK, ""}, // 177
        {VK_MEDIA_PLAY_PAUSE, ""}, // 179
        {VK_MEDIA_NEXT_TRACK, ""}, // 176
        {VK_NUMLOCK, ""},          // 144
    };
    return keymap;
}