#pragma once
#include "svg_icons.h"

#include "msimeui/Controls.h"
#include "msimeui/Layout.h"

#include <dwrite.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <wrl/client.h>

// Controls for the settings window. They mirror the look of the MetasequoiaIME
// settings page and read every color from GetTheme() at render time, so a theme
// switch only needs a repaint and the control tree never has to be rebuilt
// (msimeui::Window keeps raw pointers to hovered/focused visuals).
namespace dkui
{
enum class TextRole
{
    Primary,
    Secondary,
    Muted,
    Sidebar,
};

class Label : public msimeui::Visual
{
  public:
    Label(std::wstring text, float fontSize, DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL, TextRole role = TextRole::Primary);

    void SetText(std::wstring text);
    void SetWrap(bool wrap);
    void SetFontFamily(std::wstring family);

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;

  private:
    IDWriteTextLayout *EnsureLayout(float maxWidth);

    std::wstring text_;
    std::wstring family_;
    float fontSize_ = 14.0f;
    DWRITE_FONT_WEIGHT weight_ = DWRITE_FONT_WEIGHT_NORMAL;
    TextRole role_ = TextRole::Primary;
    bool wrap_ = false;
    float layoutWidth_ = -1.0f;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> layout_;
};

// Centers its child and caps the child's width, like `max-width` + `margin: auto`.
class CenteredColumn : public msimeui::Container
{
  public:
    CenteredColumn(std::shared_ptr<msimeui::Visual> child, float maxWidth);

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;

  private:
    float maxWidth_ = 900.0f;
};

// `.section`: rounded card with a hairline border.
class Section : public msimeui::Container
{
  public:
    explicit Section(std::shared_ptr<msimeui::Visual> child);

    void Render(msimeui::DeviceResources &deviceResources) override;
};

class Divider : public msimeui::Visual
{
  public:
    Divider();

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;
};

// `.section-header`: title (+ optional description) on the left, control on the right.
class SettingRow : public msimeui::Panel
{
  public:
    SettingRow(std::wstring title, std::wstring description, std::shared_ptr<msimeui::Visual> control);

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;

  private:
    std::shared_ptr<msimeui::Visual> left_;
    std::shared_ptr<msimeui::Visual> control_;
    msimeui::SizeF leftSize_{};
    msimeui::SizeF controlSize_{};
};

class ToggleSwitch : public msimeui::Visual
{
  public:
    using ChangeHandler = std::function<void(bool on)>;

    explicit ToggleSwitch(bool on);

    void SetOnChanged(ChangeHandler handler);
    void SetChecked(bool on);
    bool IsChecked() const;

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;
    bool IsFocusable() const override;
    bool OnMouseDown(const POINT &point, WPARAM keyState) override;
    bool OnMouseUp(const POINT &point, WPARAM keyState) override;
    bool OnKeyDown(WPARAM key, LPARAM lParam) override;

  private:
    void Toggle();

    bool on_ = false;
    bool pressed_ = false;
    ChangeHandler onChanged_;
};

class DropdownMenu;
class DropdownPopup;

class Dropdown : public msimeui::Visual
{
  public:
    using ChangeHandler = std::function<void(size_t index)>;

    Dropdown(std::vector<std::wstring> items, size_t selected);

    void SetOnChanged(ChangeHandler handler);
    void SetSelectedIndex(size_t index);
    size_t GetSelectedIndex() const;

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;
    void Attach(msimeui::Window *window) override;
    bool HitTest(const msimeui::PointF &point) const override;
    bool IsFocusable() const override;
    bool OnMouseDown(const POINT &point, WPARAM keyState) override;
    bool OnMouseUp(const POINT &point, WPARAM keyState) override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;
    bool OnKeyDown(WPARAM key, LPARAM lParam) override;
    bool KeepsPopupsOpenOnClick() const override;

  private:
    void Open();
    void Close();
    void Commit(size_t index);

    std::vector<std::wstring> items_;
    size_t selected_ = 0;
    float maxTextWidth_ = -1.0f;
    bool hovered_ = false;
    bool pressed_ = false;
    bool open_ = false;
    ChangeHandler onChanged_;
    std::shared_ptr<DropdownMenu> menu_;
    std::shared_ptr<DropdownPopup> popup_;
};

class ActionButton : public msimeui::Visual
{
  public:
    enum class Style
    {
        Secondary, // `.restart-server-button`
        Soft,      // accent-tinted, like the IME's "跟随主题"
    };
    using ClickHandler = std::function<void()>;

    ActionButton(std::wstring text, Style style = Style::Secondary);

    void SetOnClick(ClickHandler handler);

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;
    bool IsFocusable() const override;
    bool OnMouseDown(const POINT &point, WPARAM keyState) override;
    bool OnMouseUp(const POINT &point, WPARAM keyState) override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;

  private:
    std::wstring text_;
    Style style_ = Style::Secondary;
    bool hovered_ = false;
    bool pressed_ = false;
    ClickHandler onClick_;
};

// `.shortcut-keys`: a row of <kbd> caps joined by "+".
class KeyCaps : public msimeui::Visual
{
  public:
    explicit KeyCaps(std::vector<std::wstring> keys);

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;

  private:
    std::vector<std::wstring> keys_;
    std::vector<float> capWidths_;
    float plusWidth_ = 0.0f;
};

class NavItem : public msimeui::Visual
{
  public:
    using ClickHandler = std::function<void()>;

    NavItem(SvgIcon icon, std::wstring text);

    void SetSelected(bool selected);
    void SetOnClick(ClickHandler handler);

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;
    bool IsFocusable() const override;
    bool OnMouseDown(const POINT &point, WPARAM keyState) override;
    bool OnMouseUp(const POINT &point, WPARAM keyState) override;
    void OnMouseEnter() override;
    void OnMouseLeave() override;

  private:
    SvgIcon icon_;
    std::wstring text_;
    bool selected_ = false;
    bool hovered_ = false;
    bool pressed_ = false;
    ClickHandler onClick_;
};

// DirectKeycast mark: accent keycap with a "K" legend. Also used for the .ico.
class AppLogo : public msimeui::Visual
{
  public:
    explicit AppLogo(float size);

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;

  private:
    float size_ = 25.0f;
};

// Live preview of the key overlay using the current appearance settings.
class OverlayPreview : public msimeui::Visual
{
  public:
    OverlayPreview();

    msimeui::SizeF Measure(const msimeui::SizeF &availableSize) override;
    void Arrange(const msimeui::RectF &finalRect) override;
    void Render(msimeui::DeviceResources &deviceResources) override;

  private:
    void EnsureLayouts();

    std::wstring cachedFont_;
    float cachedSize_ = 0.0f;
    std::vector<Microsoft::WRL::ComPtr<IDWriteTextLayout>> layouts_;
    std::vector<float> widths_;
    float textWidth_ = 0.0f;
    float textHeight_ = 0.0f;
};
} // namespace dkui
