#include "settings.h"
#include "config.h"
#include "globals.h"
#include "msimeui_host.h"
#include "theme.h"
#include "tray_menu.h"
#include "utils.h"
#include "window.h"

#include "msimeui/Controls.h"
#include "msimeui/Layout.h"
#include "msimeui/Scene.h"
#include "msimeui/Theme.h"
#include "msimeui/Window.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace
{
constexpr int kWindowWidthDip = 960;
constexpr int kWindowHeightDip = 640;
constexpr int kMinWidthDip = 760;
constexpr int kMinHeightDip = 520;

enum class Page
{
    Appearance = 0,
    Display = 1,
    About = 2
};

std::unique_ptr<msimeui::Window> g_window;
std::shared_ptr<msimeui::Container> g_content;
std::shared_ptr<msimeui::ListView> g_nav;
Page g_page = Page::Appearance;
bool g_suspendApply = false;

msimeui::Brush CardBrush()
{
    const ThemeColors &theme = GetTheme();
    msimeui::Brush brush;
    brush.fill = theme.cardBg;
    brush.stroke = theme.border;
    brush.strokeWidth = 1.0f;
    brush.radiusX = 12.0f;
    brush.radiusY = 12.0f;
    return brush;
}

std::shared_ptr<msimeui::TextBlock> MakeText(const std::wstring &text, float size, bool bold = false, bool secondary = false)
{
    const ThemeColors &theme = GetTheme();
    return std::make_shared<msimeui::TextBlock>(text, size, secondary ? theme.textSecondary : theme.text, bold);
}

std::shared_ptr<msimeui::Visual> MakeComboRow(const std::wstring &label, const std::shared_ptr<msimeui::ComboBox> &combo)
{
    auto row = std::make_shared<msimeui::HorizontalStackPanel>(12.0f);
    row->SetHeight(44.0f);
    auto text = MakeText(label, 15.0f, true);
    text->SetVerticalAlignment(msimeui::VerticalAlignment::Center);
    combo->SetWidth(220.0f);
    combo->SetHeight(36.0f);
    combo->SetHorizontalAlignment(msimeui::HorizontalAlignment::Trailing);
    combo->SetVerticalAlignment(msimeui::VerticalAlignment::Center);
    row->AddChild(text);
    row->AddChild(combo);
    return row;
}

void CommitConfig()
{
    if (g_suspendApply)
    {
        return;
    }
    SaveAppConfig();
    ApplyConfigToRuntime();
}

std::shared_ptr<msimeui::ComboBox> MakeCombo(const std::vector<std::wstring> &items, size_t selected,
                                             const std::function<void(const std::wstring &)> &onPick)
{
    auto combo = std::make_shared<msimeui::ComboBox>(36.0f);
    for (const auto &item : items)
    {
        combo->AddItem(item);
    }
    if (selected < items.size())
    {
        combo->SetSelectedIndex(selected);
    }
    combo->SetOnSelectionChanged([onPick](size_t, const std::wstring &value) {
        if (g_suspendApply)
        {
            return;
        }
        onPick(value);
        CommitConfig();
    });
    return combo;
}

size_t IndexOf(const std::vector<std::wstring> &items, const std::wstring &value)
{
    const auto it = std::find(items.begin(), items.end(), value);
    if (it == items.end())
    {
        return 0;
    }
    return static_cast<size_t>(it - items.begin());
}

std::shared_ptr<msimeui::Visual> BuildAppearancePage()
{
    auto page = std::make_shared<msimeui::StackPanel>(16.0f);
    page->SetPadding({24.0f, 16.0f, 24.0f, 24.0f});
    page->AddChild(MakeText(L"外观", 22.0f, true));

    auto themeCard = std::make_shared<msimeui::Card>(CardBrush(), 16.0f);
    auto themeStack = std::make_shared<msimeui::StackPanel>(8.0f);
    const std::vector<std::wstring> themeLabels = {L"深色", L"浅色", L"跟随系统"};
    const std::vector<std::wstring> themeValues = {L"dark", L"light", L"system"};
    size_t themeIndex = IndexOf(themeValues, ::KeycastConfig.themeMode);
    auto themeCombo = MakeCombo(themeLabels, themeIndex, [themeValues](const std::wstring &label) {
        const size_t index = IndexOf({L"深色", L"浅色", L"跟随系统"}, label);
        if (index < themeValues.size())
        {
            ::KeycastConfig.themeMode = themeValues[index];
        }
    });

    const auto &fonts = GetSystemFontFamilies();
    std::vector<std::wstring> fontItems = fonts;
    if (fontItems.empty())
    {
        fontItems.push_back(::KeycastConfig.font);
    }
    size_t fontIndex = IndexOf(fontItems, ::KeycastConfig.font);
    auto fontCombo = MakeCombo(fontItems, fontIndex, [](const std::wstring &font) {
        ::KeycastConfig.font = font;
        if (IsFontExists(font))
        {
            ::fontName = font;
        }
    });

    std::vector<std::wstring> sizeItems;
    for (int size = 12; size <= 32; ++size)
    {
        sizeItems.push_back(std::to_wstring(size));
    }
    const std::wstring sizeValue = std::to_wstring(static_cast<int>(::KeycastConfig.fontSize + 0.5f));
    auto sizeCombo = MakeCombo(sizeItems, IndexOf(sizeItems, sizeValue), [](const std::wstring &value) {
        ::KeycastConfig.fontSize = static_cast<float>(_wtoi(value.c_str()));
    });

    themeStack->AddChild(MakeComboRow(L"主题模式", themeCombo));
    themeStack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    themeStack->AddChild(MakeComboRow(L"浮层字体", fontCombo));
    themeStack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    themeStack->AddChild(MakeComboRow(L"字体大小", sizeCombo));
    themeCard->AddChild(themeStack);
    page->AddChild(themeCard);

    auto lookCard = std::make_shared<msimeui::Card>(CardBrush(), 16.0f);
    auto lookStack = std::make_shared<msimeui::StackPanel>(8.0f);
    auto rainbow = std::make_shared<msimeui::CheckBox>(L"彩虹文字", ::KeycastConfig.rainbowText);
    rainbow->SetOnChanged([](bool checked) {
        ::KeycastConfig.rainbowText = checked;
        CommitConfig();
    });

    const std::vector<std::wstring> opacityLabels = {L"50%", L"65%", L"75%", L"90%", L"100%"};
    const std::vector<float> opacityValues = {0.50f, 0.65f, 0.75f, 0.90f, 1.00f};
    size_t opacityIndex = 2;
    for (size_t i = 0; i < opacityValues.size(); ++i)
    {
        if (std::abs(opacityValues[i] - ::KeycastConfig.bgOpacity) < 0.02f)
        {
            opacityIndex = i;
            break;
        }
    }
    auto opacityCombo = MakeCombo(opacityLabels, opacityIndex, [opacityValues](const std::wstring &label) {
        const size_t index = IndexOf({L"50%", L"65%", L"75%", L"90%", L"100%"}, label);
        if (index < opacityValues.size())
        {
            ::KeycastConfig.bgOpacity = opacityValues[index];
        }
    });

    const std::vector<std::wstring> radiusItems = {L"8", L"12", L"16"};
    auto radiusCombo = MakeCombo(radiusItems, IndexOf(radiusItems, std::to_wstring(static_cast<int>(::KeycastConfig.cornerRadius + 0.5f))),
                                 [](const std::wstring &value) { ::KeycastConfig.cornerRadius = static_cast<float>(_wtoi(value.c_str())); });

    const std::vector<std::wstring> borderItems = {L"0", L"1", L"2", L"3"};
    auto borderCombo = MakeCombo(borderItems, IndexOf(borderItems, std::to_wstring(static_cast<int>(::KeycastConfig.borderThickness + 0.5f))),
                                 [](const std::wstring &value) { ::KeycastConfig.borderThickness = static_cast<float>(_wtoi(value.c_str())); });

    lookStack->AddChild(rainbow);
    lookStack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    lookStack->AddChild(MakeComboRow(L"背景不透明度", opacityCombo));
    lookStack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    lookStack->AddChild(MakeComboRow(L"圆角半径", radiusCombo));
    lookStack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    lookStack->AddChild(MakeComboRow(L"描边宽度", borderCombo));
    lookCard->AddChild(lookStack);
    page->AddChild(lookCard);
    return page;
}

std::shared_ptr<msimeui::Visual> BuildDisplayPage()
{
    auto page = std::make_shared<msimeui::StackPanel>(16.0f);
    page->SetPadding({24.0f, 16.0f, 24.0f, 24.0f});
    page->AddChild(MakeText(L"显示", 22.0f, true));

    auto card = std::make_shared<msimeui::Card>(CardBrush(), 16.0f);
    auto stack = std::make_shared<msimeui::StackPanel>(8.0f);

    auto enabled = std::make_shared<msimeui::CheckBox>(L"显示按键浮层", ::KeycastConfig.enabled);
    enabled->SetOnChanged([](bool checked) {
        ::KeycastConfig.enabled = checked;
        CommitConfig();
    });
    auto mouse = std::make_shared<msimeui::CheckBox>(L"显示鼠标按键", ::KeycastConfig.showMouse);
    mouse->SetOnChanged([](bool checked) {
        ::KeycastConfig.showMouse = checked;
        CommitConfig();
    });

    const std::vector<std::wstring> fadeLabels = {L"0.5 秒", L"1 秒", L"1.5 秒", L"2 秒", L"3 秒"};
    const std::vector<int> fadeValues = {500, 1000, 1500, 2000, 3000};
    size_t fadeIndex = 2;
    for (size_t i = 0; i < fadeValues.size(); ++i)
    {
        if (fadeValues[i] == ::KeycastConfig.fadeMs)
        {
            fadeIndex = i;
            break;
        }
    }
    auto fadeCombo = MakeCombo(fadeLabels, fadeIndex, [fadeValues](const std::wstring &label) {
        const size_t index = IndexOf({L"0.5 秒", L"1 秒", L"1.5 秒", L"2 秒", L"3 秒"}, label);
        if (index < fadeValues.size())
        {
            ::KeycastConfig.fadeMs = fadeValues[index];
        }
    });

    const std::vector<std::wstring> positionLabels = {L"右下", L"左下", L"右上", L"左上"};
    const std::vector<std::wstring> positionValues = {L"bottom-right", L"bottom-left", L"top-right", L"top-left"};
    auto positionCombo = MakeCombo(positionLabels, IndexOf(positionValues, ::KeycastConfig.position),
                                   [positionValues](const std::wstring &label) {
                                       const size_t index = IndexOf({L"右下", L"左下", L"右上", L"左上"}, label);
                                       if (index < positionValues.size())
                                       {
                                           ::KeycastConfig.position = positionValues[index];
                                       }
                                   });

    stack->AddChild(enabled);
    stack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    stack->AddChild(mouse);
    stack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    stack->AddChild(MakeComboRow(L"自动隐藏", fadeCombo));
    stack->AddChild(std::make_shared<msimeui::Separator>(1.0f));
    stack->AddChild(MakeComboRow(L"浮层位置", positionCombo));
    card->AddChild(stack);
    page->AddChild(card);
    return page;
}

std::shared_ptr<msimeui::Visual> BuildAboutPage()
{
    auto page = std::make_shared<msimeui::StackPanel>(16.0f);
    page->SetPadding({24.0f, 16.0f, 24.0f, 24.0f});
    page->AddChild(MakeText(L"关于", 22.0f, true));

    auto intro = std::make_shared<msimeui::Card>(CardBrush(), 18.0f);
    auto introStack = std::make_shared<msimeui::StackPanel>(6.0f);
    auto kicker = std::make_shared<msimeui::TextBlock>(L"DIRECTKEYCAST", 12.0f, GetTheme().accent, true);
    introStack->AddChild(kicker);
    introStack->AddChild(MakeText(L"DirectKeycast", 20.0f, true));
    introStack->AddChild(MakeText(L"轻量按键浮层。设置与托盘菜单使用 msimeui 控件树渲染。", 13.0f, false, true));
    intro->AddChild(introStack);
    page->AddChild(intro);

    auto links = std::make_shared<msimeui::Card>(CardBrush(), 12.0f);
    auto linkStack = std::make_shared<msimeui::StackPanel>(8.0f);
    auto openFile = std::make_shared<msimeui::Button>(L"打开配置文件", 40.0f);
    openFile->SetOnClick([]() { OpenConfigFile(); });
    auto openDir = std::make_shared<msimeui::Button>(L"打开配置目录", 40.0f);
    openDir->SetOnClick([]() { OpenConfigDirectory(); });
    linkStack->AddChild(openFile);
    linkStack->AddChild(openDir);
    links->AddChild(linkStack);
    page->AddChild(links);

    auto hint = std::make_shared<msimeui::Card>(CardBrush(), 16.0f);
    auto hintStack = std::make_shared<msimeui::StackPanel>(6.0f);
    hintStack->AddChild(MakeText(L"退出快捷键：Ctrl + Alt + Shift + F12", 13.0f, false, true));
    hintStack->AddChild(MakeText(L"配置文件：%LOCALAPPDATA%\\DirectKeycast\\config.toml", 13.0f, false, true));
    hint->AddChild(hintStack);
    page->AddChild(hint);
    return page;
}

std::shared_ptr<msimeui::Visual> BuildPage(Page page)
{
    switch (page)
    {
    case Page::Display:
        return BuildDisplayPage();
    case Page::About:
        return BuildAboutPage();
    case Page::Appearance:
    default:
        return BuildAppearancePage();
    }
}

void ShowPage(Page page)
{
    g_page = page;
    if (g_content)
    {
        g_content->SetChild(BuildPage(page));
    }
    if (g_window)
    {
        g_window->Relayout();
        g_window->Invalidate();
    }
}

void BuildSettingsScene()
{
    if (!g_window)
    {
        return;
    }

    g_suspendApply = true;
    const ThemeColors &theme = GetTheme();
    auto root = std::make_shared<msimeui::Grid>();
    root->AddColumn({msimeui::GridUnitType::Pixel, 208.0f});
    root->AddColumn({msimeui::GridUnitType::Star, 1.0f});
    root->AddRow({msimeui::GridUnitType::Star, 1.0f});

    msimeui::Brush sidebarBrush;
    sidebarBrush.fill = theme.sidebarBg;
    sidebarBrush.stroke = theme.border;
    sidebarBrush.strokeWidth = 0.0f;
    auto sidebar = std::make_shared<msimeui::Border>(sidebarBrush);
    sidebar->SetPadding({16.0f, 20.0f, 16.0f, 16.0f});
    auto sideStack = std::make_shared<msimeui::StackPanel>(12.0f);
    sideStack->AddChild(MakeText(L"DirectKeycast", 20.0f, true));
    g_nav = std::make_shared<msimeui::ListView>(40.0f);
    g_nav->AddItem({L"外观", L"", L""});
    g_nav->AddItem({L"显示", L"", L""});
    g_nav->AddItem({L"关于", L"", L""});
    g_nav->SetOnSelectionChanged([](size_t index) { ShowPage(static_cast<Page>(index)); });
    g_nav->SetSelectedIndex(static_cast<size_t>(g_page));
    sideStack->AddChild(g_nav);
    sidebar->SetChild(sideStack);

    g_content = std::make_shared<msimeui::Container>();
    auto scroll = std::make_shared<msimeui::ScrollViewer>(g_content);
    g_content->SetChild(BuildPage(g_page));

    root->AddChild(sidebar, 0, 0);
    root->AddChild(scroll, 0, 1);

    auto scene = std::make_unique<msimeui::Scene>();
    scene->SetRoot(root);
    g_window->SetScene(std::move(scene));
    g_suspendApply = false;
}

bool EnsureSettingsWindow()
{
    if (g_window && g_window->GetHandle())
    {
        return true;
    }

    POINT origin{0, 0};
    const UINT dpi = MonitorDpiFromPoint(origin);
    g_window = std::make_unique<msimeui::Window>(L"DirectKeycastSettingsWindow", L"DirectKeycast 设置",
                                                 DipToPxForDpi(static_cast<float>(kWindowWidthDip), dpi),
                                                 DipToPxForDpi(static_cast<float>(kWindowHeightDip), dpi));
    g_window->SetWindowStyle(WS_OVERLAPPEDWINDOW, WS_EX_APPWINDOW);
    g_window->SetRoundedCorners(true);
    g_window->SetInitialPlacement(msimeui::WindowInitialPlacement::Center);
    if (!g_window->Create())
    {
        g_window.reset();
        return false;
    }

    HWND hwnd = g_window->GetHandle();
    AttachMsimeuiHost(hwnd, DkMsimeuiHideOnClose | DkMsimeuiMinSize, kMinWidthDip, kMinHeightDip);
    ApplyImmersiveDarkMode(hwnd, !IsLightThemeResolved());
    BuildSettingsScene();
    ::SettingsHwnd = hwnd;
    return true;
}
} // namespace

bool OpenSettingsWindow(bool about)
{
    HideTrayMenu();
    if (!EnsureSettingsWindow())
    {
        return false;
    }
    g_page = about ? Page::About : Page::Appearance;
    if (g_nav)
    {
        g_nav->SetSelectedIndex(static_cast<size_t>(g_page));
    }
    ShowPage(g_page);
    HWND hwnd = g_window->GetHandle();
    ShowWindow(hwnd, SW_SHOWNORMAL);
    SetForegroundWindow(hwnd);
    g_window->Invalidate();
    return true;
}

void CloseSettingsWindow()
{
    if (g_window && g_window->GetHandle())
    {
        DestroyWindow(g_window->GetHandle());
    }
    g_content.reset();
    g_nav.reset();
    g_window.reset();
    ::SettingsHwnd = nullptr;
}

bool IsSettingsWindowOpen()
{
    return g_window && g_window->GetHandle() && IsWindowVisible(g_window->GetHandle());
}

void RefreshSettingsWindowTheme()
{
    if (!g_window || !g_window->GetHandle())
    {
        return;
    }
    ApplyImmersiveDarkMode(g_window->GetHandle(), !IsLightThemeResolved());
    BuildSettingsScene();
    g_window->Invalidate();
}

bool IsPointInSettingsWindow(POINT screenPt)
{
    if (!g_window)
    {
        return false;
    }
    return IsPointInWindow(g_window->GetHandle(), screenPt);
}
